/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "platform_event_runner.h"

#include <dlfcn.h>
#include <functional>
#include <memory>
#include <sys/timerfd.h>
#include <unistd.h>
#include <unordered_set>

#include "event_handler.h"
#include "event_queue.h"
#include "event_runner.h"
#include "hilog.h"
#include "io_waiter.h"

#ifdef _cplusplus
extern "C" {
#endif

struct ALooper;
using ALooper = struct ALooper;

/**Result from android looper ALooper pollOnce() and ALooper pollAll().*/
enum {
    ANDROID_LOOPER_POLL_WAKE = -1,
    ANDROID_LOOPER_POLL_CALLBACK = -2,
    ANDROID_LOOPER_POLL_TIMEOUT = -3,
    ANDROID_LOOPER_POLL_ERROR = -4,
};

/** Flags for android looper file descriptor events that a looper can monitor. */
enum {
    ANDROID_LOOPER_EVENT_INPUT = 1 << 0,
    ANDROID_LOOPER_EVENT_OUTPUT = 1 << 1,
    ANDROID_LOOPER_EVENT_ERROR = 1 << 2,
    ANDROID_LOOPER_EVENT_HANGUP = 1 << 3,
    ANDROID_LOOPER_EVENT_INVALID = 1 << 4,
};

using ALooperCallbackFunc = int (*)(int, int, void*);
using ALooperForThreadFun = ALooper* (*)();
using ALooperAcquireFun = void (*)(ALooper*);
using ALooperReleaseFun = void (*)(ALooper*);
using ALooperPollAllFun = int (*)(int, int*, int*, void**);
using ALooperWakeFun = void (*)(ALooper*);
using ALooperAddFdFun = int (*)(ALooper*, int, int, int, ALooperCallbackFunc, void*);
using ALooperRemoveFdFun = int (*)(ALooper*, int);

#ifdef _cplusplus
};
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {
using AndroidLooper = std::unique_ptr<ALooper, ALooperReleaseFun>;
using LibHandle = void*;

// android lib info we used
const std::string LOOPER_FOR_THREAD = "ALooper_forThread";
const std::string LOOPER_ACQUIRE = "ALooper_acquire";
const std::string LOOPER_RELEASE = "ALooper_release";
const std::string LOOPER_POLLALL = "ALooper_pollAll";
const std::string LOOPER_WAKE = "ALooper_wake";
const std::string LOOPER_ADD_FD = "ALooper_addFd";
const std::string LOOPER_REMOVE_FD = "ALooper_removeFd";

class ALooperFun {
public:
    ALooperFun();
    ~ALooperFun();

    static ALooperFun& GetInstance()
    {
        static ALooperFun instance;
        return instance;
    }

    void InitAndroidLib()
    {
        HILOG_INFO("InitAndroidLib");
        std::lock_guard<std::mutex> lock(libLock_);
        if (isLibInitialed_) {
            HILOG_INFO("isLibInitialed_ is true");
            return;
        }

        auto handle = dlopen("libandroid.so", RTLD_NOW);
        if (handle == nullptr) {
            HILOG_ERROR("handle is nullptr");
            isLibInitialed_ = true;
            return;
        }

        ALooperForThreadFun forThreadFun = (ALooperForThreadFun)dlsym(handle, LOOPER_FOR_THREAD.c_str());
        ALooperAcquireFun acquireFun = (ALooperAcquireFun)dlsym(handle, LOOPER_ACQUIRE.c_str());
        ALooperReleaseFun releaseFun = (ALooperReleaseFun)dlsym(handle, LOOPER_RELEASE.c_str());
        ALooperPollAllFun pollAllFun = (ALooperPollAllFun)dlsym(handle, LOOPER_POLLALL.c_str());
        ALooperWakeFun wakeFun = (ALooperWakeFun)dlsym(handle, LOOPER_WAKE.c_str());
        ALooperAddFdFun addFdFun = (ALooperAddFdFun)dlsym(handle, LOOPER_ADD_FD.c_str());
        ALooperRemoveFdFun removeFdFun = (ALooperRemoveFdFun)dlsym(handle, LOOPER_REMOVE_FD.c_str());

        if (forThreadFun && acquireFun && releaseFun && pollAllFun && wakeFun && addFdFun && removeFdFun) {
            HILOG_ERROR("fun bind success.");
            funInfo_.aLooperForThreadFun = forThreadFun;
            funInfo_.aLooperAcquireFun = acquireFun;
            funInfo_.aLooperReleaseFun = releaseFun;
            funInfo_.aLooperPollAllFun = pollAllFun;
            funInfo_.aLooperWakeFun = wakeFun;
            funInfo_.aLooperAddFdFun = addFdFun;
            funInfo_.aLooperRemoveFdFun = removeFdFun;
            funInfo_.handle = handle;
        } else {
            HILOG_ERROR("fun bind failed.");
            dlclose(handle);
            handle = nullptr;
        }

        isLibInitialed_ = true;
        return;
    }

    AndroidLooper CurrentAndroidLooper()
    {
        InitAndroidLib();
        if (funInfo_.handle == nullptr) {
            return AndroidLooper(nullptr, nullptr);
        }

        auto looper = funInfo_.aLooperForThreadFun();
        if (looper == nullptr) {
            return AndroidLooper(nullptr, nullptr);
        }

        funInfo_.aLooperAcquireFun(looper);
        return AndroidLooper(looper, funInfo_.aLooperReleaseFun);
    }

    bool CheckCurrent()
    {
        InitAndroidLib();
        if (funInfo_.aLooperForThreadFun != nullptr) {
            return funInfo_.aLooperForThreadFun() != nullptr;
        }
        return false;
    }

    bool IsEqualCurrentLooper(const AndroidLooper& looper)
    {
        auto fun = funInfo_.aLooperForThreadFun;
        if ((!looper) || (fun == nullptr)) {
            return false;
        }
        return looper.get() == fun();
    }

    int PollAll(int timeoutMillis)
    {
        auto fun = funInfo_.aLooperPollAllFun;
        if (fun == nullptr) {
            return ANDROID_LOOPER_POLL_ERROR;
        }
        return fun(timeoutMillis, nullptr, nullptr, nullptr);
    }

    void Wake(const AndroidLooper& looper)
    {
        auto fun = funInfo_.aLooperWakeFun;
        if ((!looper) || (fun == nullptr)) {
            return;
        }
        fun(looper.get());
    }

    int AddFd(const AndroidLooper& looper, int fd, int events, ALooperCallbackFunc callback, uintptr_t data)
    {
        auto fun = funInfo_.aLooperAddFdFun;
        if ((!looper) || (fun == nullptr)) {
            return -1;
        }
        return fun(looper.get(), fd, ANDROID_LOOPER_POLL_CALLBACK, events, callback, reinterpret_cast<void*>(data));
    }

    int RemoveFd(const AndroidLooper& looper, int fd)
    {
        auto fun = funInfo_.aLooperRemoveFdFun;
        if ((!looper) || (fun == nullptr)) {
            return -1;
        }
        return fun(looper.get(), fd);
    }

private:
    struct ALooperFunInfo {
        LibHandle handle = nullptr;
        ALooperForThreadFun aLooperForThreadFun = nullptr;
        ALooperAcquireFun aLooperAcquireFun = nullptr;
        ALooperReleaseFun aLooperReleaseFun = nullptr;
        ALooperPollAllFun aLooperPollAllFun = nullptr;
        ALooperWakeFun aLooperWakeFun = nullptr;
        ALooperAddFdFun aLooperAddFdFun = nullptr;
        ALooperRemoveFdFun aLooperRemoveFdFun = nullptr;
    };

    void UnInitAndroidLib()
    {
        std::lock_guard<std::mutex> lock(libLock_);
        if (!isLibInitialed_) {
            return;
        }
        auto handle = funInfo_.handle;
        if (handle != nullptr) {
            funInfo_.handle = nullptr;
            dlclose(handle);
        }
    }

    ALooperFunInfo funInfo_;
    std::mutex libLock_;
    bool isLibInitialed_ { false };
};

ALooperFun::ALooperFun() {}

ALooperFun::~ALooperFun()
{
    UnInitAndroidLib();
}
} // unnamed namespace
class PlatformIoWaiter final : public IoWaiter {
public:
    using TimerCallback = std::function<void()>;
    PlatformIoWaiter() : looper_(ALooperFun::GetInstance().CurrentAndroidLooper()) {}
    ~PlatformIoWaiter() final
    {
        if (!looper_) {
            return;
        }

        StopTimer();

        for (int32_t fileDescriptor : fileDescriptors_) {
            ALooperFun::GetInstance().RemoveFd(looper_, fileDescriptor);
        }
    }

    bool StartTimer(const TimerCallback& callback)
    {
        if (!callback) {
            HILOG_ERROR("StartTimer: Invalid callback");
            return false;
        }
        if (timerFd_ < 0) {
            int32_t fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
            if (fd < 0) {
                HILOG_ERROR("StartTimer Failed to create timer");
                return false;
            }

            if (!AddFileDescriptor(fd, FILE_DESCRIPTOR_INPUT_EVENT)) {
                HILOG_ERROR("StartTimer Failed to listen timer");
                close(fd);
                return false;
            }
            timerFd_ = fd;
        }

        timerCallback_ = callback;
        return true;
    }

    void StopTimer()
    {
        if (timerFd_ >= 0) {
            RemoveFileDescriptor(timerFd_);
            close(timerFd_);
            timerFd_ = -1;
        }
        timerCallback_ = nullptr;
    }

    void SetTimer(int64_t nanoseconds)
    {
        if (timerFd_ < 0) {
            HILOG_ERROR("SetTimer. Timer is not start yet");
            return;
        }

        int32_t flags = 0;
        struct itimerspec value = {
            .it_interval = { .tv_sec = 0, .tv_nsec = 0 },
            .it_value = { .tv_sec = 0, .tv_nsec = 0 },
        };
        if (nanoseconds > 0) {
            value.it_value = {
                .tv_sec = nanoseconds / NANOSECONDS_PER_ONE_SECOND,
                .tv_nsec = nanoseconds % NANOSECONDS_PER_ONE_SECOND,
            };
        } else if (nanoseconds == 0) {
            if (clock_gettime(CLOCK_MONOTONIC, &value.it_value) < 0) {
                HILOG_ERROR("SetTimer Failed to get current time");
                return;
            }
            flags = TFD_TIMER_ABSTIME;
        }
        if (timerfd_settime(timerFd_, flags, &value, nullptr) < 0) {
            HILOG_ERROR("SetTimer Failed to set timer");
            return;
        }
    }

    bool WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds) final
    {
        if ((!looper_) || (!ALooperFun::GetInstance().IsEqualCurrentLooper(looper_))) {
            HILOG_ERROR("WaitFor: No android looper or looper mismatch with current thread");
            return false;
        }
        ++waitingCount_;
        lock.unlock();

        // Wait for timed out, wake up or some file descriptors to be triggered.
        int32_t result = ALooperFun::GetInstance().PollAll(NanosecondsToTimeout(nanoseconds));

        lock.lock();
        --waitingCount_;
        return result != ANDROID_LOOPER_POLL_ERROR;
    }
    void NotifyOne() final
    {
        NotifyAll();
    }

    void NotifyAll() final
    {
        if (!looper_) {
            HILOG_ERROR("NotifyAll: No android looper");
            return;
        }

        if (waitingCount_ > 0) {
            // Wake up while android looper is waiting
            ALooperFun::GetInstance().Wake(looper_);
        } else {
            // Trigger timer at once, if android looper is not blocked by us.
            SetTimer(0);
        }
    }

    bool SupportListeningFileDescriptor() const final
    {
        return true;
    }

    bool AddFileDescriptor(int32_t fileDescriptor, uint32_t events) final
    {
        if (!looper_) {
            HILOG_ERROR("AddFileDescriptor: No android looper");
            return false;
        }
        // Transform file descriptor listener events into android looper events.
        uint32_t aLooperEvents = 0;
        if ((events & FILE_DESCRIPTOR_INPUT_EVENT) != 0) {
            aLooperEvents |= ANDROID_LOOPER_EVENT_INPUT;
        }
        if ((events & FILE_DESCRIPTOR_OUTPUT_EVENT) != 0) {
            aLooperEvents |= ANDROID_LOOPER_EVENT_OUTPUT;
        }

        if (ALooperFun::GetInstance().AddFd(looper_, fileDescriptor, static_cast<int32_t>(aLooperEvents),
                OnFileDescriptorEvent, reinterpret_cast<uintptr_t>(this)) < 0) {
            HILOG_ERROR("Failed to add file descriptor to android looper");
            return false;
        }

        fileDescriptors_.emplace(fileDescriptor);
        return true;
    }

    void RemoveFileDescriptor(int32_t fileDescriptor) final
    {
        if (!looper_) {
            HILOG_ERROR("RemoveFileDescriptor- No android looper");
            return;
        }
        if (ALooperFun::GetInstance().RemoveFd(looper_, fileDescriptor) < 0) {
            HILOG_ERROR("Failed to remove file descriptor to android looper");
            return;
        }
        fileDescriptors_.erase(fileDescriptor);
    }

    void SetFileDescriptorEventCallback(const FileDescriptorEventCallback& callback) final
    {
        callback_ = callback;
    }

private:
    static int OnFileDescriptorEvent(int32_t fileDescriptor, int32_t events, void* userData)
    {
        auto me = reinterpret_cast<PlatformIoWaiter*>(userData);
        if (me != nullptr) {
            me->OnFileDescriptorEvent(fileDescriptor, static_cast<uint32_t>(events));
            // Continue listening.
            return 1;
        }

        HILOG_ERROR("OnFileDescriptorEvent: MUST NOT RUN HERE !!!");
        return 0;
    }

    void ResetTimerFileDescriptor() const
    {
        uint64_t value = 0;
        ssize_t retVal = read(timerFd_, &value, sizeof(value));
        if (retVal < 0) {
            HILOG_ERROR("ResetTimerFileDescriptor. Failed to read data from timer fd");
        }
    }

    void OnFileDescriptorEvent(int32_t fileDescriptor, uint32_t aLooperEvents)
    {
        if (fileDescriptor == timerFd_) {
            ResetTimerFileDescriptor();
            if (timerCallback_) {
                timerCallback_();
            }
            return;
        }

        // Transform epoll events into file descriptor listener events
        uint32_t events = 0;
        if ((aLooperEvents & ANDROID_LOOPER_EVENT_INPUT) != 0) {
            events |= FILE_DESCRIPTOR_INPUT_EVENT;
        }
        if ((aLooperEvents & ANDROID_LOOPER_EVENT_OUTPUT) != 0) {
            events |= FILE_DESCRIPTOR_OUTPUT_EVENT;
        }
        if ((aLooperEvents & ANDROID_LOOPER_EVENT_HANGUP) != 0) {
            events |= FILE_DESCRIPTOR_SHUTDOWN_EVENT;
        }
        if ((aLooperEvents & ANDROID_LOOPER_EVENT_ERROR) != 0) {
            events |= FILE_DESCRIPTOR_EXCEPTION_EVENT;
        }
        if ((aLooperEvents & ANDROID_LOOPER_EVENT_INVALID) != 0) {
            HILOG_ERROR("File descriptor MUST NOT be closed before removed");
        }
        if (callback_) {
            callback_(fileDescriptor, events);
        }
    }

    AndroidLooper looper_ { nullptr, nullptr };
    FileDescriptorEventCallback callback_;
    TimerCallback timerCallback_;
    uint32_t waitingCount_ { 0 };
    std::unordered_set<int32_t> fileDescriptors_;
    int32_t timerFd_ { -1 };
};

bool PlatformEventRunner::CheckCurrent()
{
    return ALooperFun::GetInstance().CheckCurrent();
}

PlatformEventRunner::PlatformEventRunner(const std::shared_ptr<EventRunner>& runner)
    : EventInnerRunner(runner), ioWaiter_(std::make_shared<PlatformIoWaiter>())
{
    queue_ = std::make_shared<EventQueue>(ioWaiter_);

    // Set current event runner
    currentEventRunner = owner_;
}

PlatformEventRunner::~PlatformEventRunner()
{
    ioWaiter_->StopTimer();
}

void PlatformEventRunner::Run()
{
    InnerEvent::TimePoint wakeUpTime = InnerEvent::TimePoint::max();
    for (auto event = queue_->GetExpiredEvent(wakeUpTime); event; event = queue_->GetExpiredEvent(wakeUpTime)) {
        std::shared_ptr<EventHandler> handler = event->GetOwner();
        // Make sure owner of the event exists.
        if (handler) {
            handler->DistributeEvent(event);
        }
    }
    ioWaiter_->SetTimer(TimePointToTimeOut(wakeUpTime));
}

void PlatformEventRunner::Stop() {}

bool PlatformEventRunner::Init()
{
    threadId_ = std::this_thread::get_id();
    return ioWaiter_->StartTimer(std::bind(&PlatformEventRunner::Run, this));
}
} // namespace AppExecFwk
} // namespace OHOS