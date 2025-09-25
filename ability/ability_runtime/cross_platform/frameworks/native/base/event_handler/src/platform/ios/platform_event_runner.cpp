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
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unistd.h>
#include <unordered_set>

#include "event_handler.h"
#include "event_queue.h"
#include "event_runner.h"
#include "hilog.h"
#include "io_waiter.h"
#include "ios_message_loop.h"

namespace OHOS {
namespace AppExecFwk {
class PlatformIoWaiter final : public IoWaiter,
                               public IEventObserver,
                               public std::enable_shared_from_this<PlatformIoWaiter> {
public:
    using TimerCallback = std::function<void()>;
    PlatformIoWaiter() : messageLoop_(std::make_shared<IOSMessageLoop>()) {}

    virtual ~PlatformIoWaiter() final
    {
        StopTimer();

        for (int32_t fileDescriptor : fileDescriptors_) {
            if (messageLoop_) {
                messageLoop_->Invalidate(fileDescriptor);
            }
        }
    }

    void Attach()
    {
        if (messageLoop_) {
            messageLoop_->AttachEventObserver(shared_from_this());
        }
    }

    bool StartTimer(const TimerCallback& callback)
    {
        HILOG_INFO("StartTimer");
        if (!callback) {
            HILOG_ERROR("callback is nullptr");
            return false;
        }

        if (messageLoop_ == nullptr) {
            HILOG_ERROR("messageLoop_ is nullptr");
            return false;
        }

        if (!messageLoop_->AddTimer()) {
            HILOG_ERROR("Add timer failed");
            return false;
        }

        timerCallback_ = callback;
        return true;
    }

    void StopTimer()
    {
        if (messageLoop_) {
            messageLoop_->Terminate();
        }
        timerCallback_ = nullptr;
    }

    void SetTimer(int64_t nanoseconds)
    {
        if (messageLoop_ == nullptr) {
            HILOG_ERROR("messageLoop_ is nullptr");
            return;
        }

        messageLoop_->WakeUp(nanoseconds);
    }

    bool WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds) final
    {
        HILOG_INFO("WaitFor");
        if (!IOSMessageLoop::IsMainThread()) {
            HILOG_ERROR("WaitFor: No ios looper or looper mismatch with current thread");
            return false;
        }

        ++waitingCount_;
        lock.unlock();

        // Wait for timed out, wake up or some file descriptors to be triggered.
        messageLoop_->WakeUp(nanoseconds);

        lock.lock();
        --waitingCount_;
        return true;
    }

    void NotifyOne() final
    {
        NotifyAll();
    }

    void NotifyAll() final
    {
        if (waitingCount_ > 0) {
            messageLoop_->WakeUp(0);
        } else {
            SetTimer(0);
        }
    }

    bool SupportListeningFileDescriptor() const final
    {
        return true;
    }

    bool AddFileDescriptor(int32_t fileDescriptor, uint32_t events) final
    {
        if (messageLoop_ == nullptr) {
            HILOG_ERROR("messageLoop_ is nullptr");
            return false;
        }

        if (!messageLoop_->AddFileDescriptor(fileDescriptor, events)) {
            HILOG_ERROR("Failed to add file descriptor to ios looper");
            return false;
        }

        fileDescriptors_.emplace(fileDescriptor);
        return true;
    }

    void RemoveFileDescriptor(int32_t fileDescriptor) final
    {
        if (messageLoop_ == nullptr) {
            HILOG_ERROR("messageLoop_ is nullptr");
            return;
        }

        messageLoop_->Invalidate(fileDescriptor);
        fileDescriptors_.erase(fileDescriptor);
    }

    void SetFileDescriptorEventCallback(const FileDescriptorEventCallback& callback) final
    {
        callback_ = callback;
    }

private:
    void OnFileDescriptorEvent(int32_t fd, int32_t event) override
    {
        HILOG_DEBUG("PlatformIoWaiter::OnFileDescriptorEvent fd %{public}d ", fd);
        if (callback_) {
            callback_(fd, event);
        }
    }

    void OnTimerEvent() override
    {
        if (timerCallback_) {
            timerCallback_();
        }
    }

    FileDescriptorEventCallback callback_;
    TimerCallback timerCallback_;
    uint32_t waitingCount_ { 0 };
    std::unordered_set<int32_t> fileDescriptors_;
    std::shared_ptr<IOSMessageLoop> messageLoop_ = nullptr;
};

bool PlatformEventRunner::CheckCurrent()
{
    return IOSMessageLoop::IsMainThread();
}

PlatformEventRunner::PlatformEventRunner(const std::shared_ptr<EventRunner>& runner)
    : EventInnerRunner(runner), ioWaiter_(std::make_shared<PlatformIoWaiter>())
{
    ioWaiter_->Attach();
    queue_ = std::make_shared<EventQueue>(ioWaiter_);

    currentEventRunner = owner_;
}

PlatformEventRunner::~PlatformEventRunner()
{
    if (ioWaiter_) {
        ioWaiter_->StopTimer();
    }
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