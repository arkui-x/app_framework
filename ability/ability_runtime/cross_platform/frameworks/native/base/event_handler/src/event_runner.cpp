/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "event_runner.h"

#include <condition_variable>
#include <mutex>
#include <sstream>
#ifndef IOS_PLATFORM
#include <sys/prctl.h>
#endif
#include <thread>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "event_handler.h"
#include "event_handler_utils.h"
#include "event_inner_runner.h"
#include "hilog.h"
#include "platform_event_runner.h"
#include "singleton.h"
#include "thread_local_data.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
// Invoke system call to set name of current thread.
inline void SystemCallSetThreadName(const std::string& name)
{
#ifndef IOS_PLATFORM
    if (prctl(PR_SET_NAME, name.c_str()) < 0) {
        char errmsg[MAX_ERRORMSG_LEN] = { 0 };
        GetLastErr(errmsg, MAX_ERRORMSG_LEN);
        HILOG_ERROR("SystemCallSetThreadName: Failed to set thread name, %{public}s", errmsg);
    }
#endif
}

// Help to calculate hash code of object.
template<typename T>
inline size_t CalculateHashCode(const T& obj)
{
    std::hash<T> calculateHashCode;
    return calculateHashCode(obj);
}

// Thread collector is used to reclaim thread that needs to finish running.
class ThreadCollector : public DelayedRefSingleton<ThreadCollector> {
    DECLARE_DELAYED_REF_SINGLETON(ThreadCollector);

public:
    using ExitFunction = std::function<void()>;

    void ReclaimCurrentThread()
    {
        // Get id of current thread.
        auto threadId = std::this_thread::get_id();
        HILOG_INFO("Reclaim: Thread id: %{public}zu", CalculateHashCode(threadId));

        {
            // Add thread id to list and notify to reclaim.
            std::lock_guard<std::mutex> lock(collectorLock_);
            if (destroying_) {
                HILOG_INFO("Reclaim: Thread collector is destroying");
                return;
            }

            reclaims_.emplace_back(threadId);
            if (isWaiting_) {
                condition_.notify_one();
            }
        }

        if (threadLock_.try_lock()) {
            if ((!thread_) && (needCreateThread_)) {
                // Start daemon thread to collect finished threads, if not exist.
                thread_ = std::make_unique<std::thread>(&ThreadCollector::Run, this);
            }
            threadLock_.unlock();
        }
    }

    bool Deposit(std::unique_ptr<std::thread>& thread, const ExitFunction& threadExit)
    {
        if ((!thread) || (!thread->joinable()) || (!threadExit)) {
            auto threadState = thread ? (thread->joinable() ? "active" : "finished") : "null";
            HILOG_ERROR(
                "Deposit(%{public}s, %{public}s): Invalid parameter", threadState, threadExit ? "valid" : "null");
            return false;
        }

        auto threadId = thread->get_id();
        HILOG_INFO("Deposit: New thread id: %{public}zu", CalculateHashCode(threadId));
        // Save these information into map.
        std::lock_guard<std::mutex> lock(collectorLock_);
        if (destroying_) {
            HILOG_WARN("Deposit: Collector thread is destroying");
            return false;
        }
        // Save thread object and its exit callback.
        depositMap_.emplace(threadId, ThreadExitInfo {
                                          .thread = std::move(thread),
                                          .threadExit = threadExit,
                                      });
        return true;
    }

private:
    struct ThreadExitInfo {
        std::unique_ptr<std::thread> thread;
        ExitFunction threadExit;
    };

    inline void ReclaimAll()
    {
        std::unique_lock<std::mutex> lock(collectorLock_);
        // All thread deposited need to stop one by one.
        while (!depositMap_.empty()) {
            DoReclaimLocked(lock, depositMap_.begin());
        }
    }

    void Stop()
    {
        {
            // Stop the collect thread, while destruction of collector.
            std::lock_guard<std::mutex> lock(collectorLock_);
            destroying_ = true;
            if (isWaiting_) {
                condition_.notify_all();
            }
        }

        {
            std::lock_guard<std::mutex> lock(threadLock_);
            if ((thread_) && (thread_->joinable())) {
                // Wait utils collect thread finished, if exists.
                thread_->join();
            }
            needCreateThread_ = false;
        }

        ReclaimAll();
    }

    void DoReclaimLocked(std::unique_lock<std::mutex>& lock,
        std::unordered_map<std::thread::id, ThreadExitInfo>::iterator it, bool needCallExit = true)
    {
        if (it == depositMap_.end()) {
            return;
        }

        // Remove thread information from map.
        auto threadId = it->first;
        auto exitInfo = std::move(it->second);
        (void)depositMap_.erase(it);

        // Unlock, because stopping thread maybe spend lot of time, it should be out of the lock.
        lock.unlock();

        size_t hashThreadId = CalculateHashCode(threadId);
        HILOG_INFO("DoReclaim: Thread id: %{public}zu", hashThreadId);
        if (needCallExit) {
            // Call exit callback to stop loop in thread.
            exitInfo.threadExit();
        }
        // Wait until thread finished.
        exitInfo.thread->join();
        HILOG_INFO("DoReclaim: Done, thread id: %{public}zu", hashThreadId);

        // Lock again.
        lock.lock();
    }

    void Run()
    {
        HILOG_INFO("Run: Collector thread is started");

        std::unique_lock<std::mutex> lock(collectorLock_);
        while (!destroying_) {
            // Reclaim threads in list one by one.
            while (!reclaims_.empty()) {
                auto threadId = reclaims_.back();
                reclaims_.pop_back();
                DoReclaimLocked(lock, depositMap_.find(threadId), false);
            }

            // Maybe stop running while doing reclaim, so check before waiting.
            if (destroying_) {
                break;
            }

            isWaiting_ = true;
            condition_.wait(lock);
            isWaiting_ = false;
        }

        HILOG_INFO("Run: Collector thread is stopped");
    }

    std::mutex collectorLock_;
    std::condition_variable condition_;
    bool isWaiting_ { false };
    bool destroying_ { false };
    std::vector<std::thread::id> reclaims_;
    std::unordered_map<std::thread::id, ThreadExitInfo> depositMap_;

    std::mutex threadLock_;
    // Thread for collector
    std::unique_ptr<std::thread> thread_;
    bool needCreateThread_ { true };

    // Avatar of thread collector, used to stop collector at the specified opportunity.
    class Avatar {
    public:
        Avatar() = default;
        ~Avatar()
        {
            if (avatarEnabled_) {
                GetInstance().avatarDestructed_ = true;
                GetInstance().Stop();
            }
        }

        static inline void Disable()
        {
            avatarEnabled_ = false;
        }
    };

    // Mark whether avatar is destructed.
    volatile bool avatarDestructed_ { false };
    // Mark whether avatar is enabled.
    static volatile bool avatarEnabled_;
    static Avatar avatar_;
};

ThreadCollector::ThreadCollector()
    : collectorLock_(), condition_(), reclaims_(), depositMap_(), threadLock_(), thread_(nullptr)
{
    // Thread collector is created, so enable avatar.
    avatarEnabled_ = true;
}

ThreadCollector::~ThreadCollector()
{
    // If avatar is not destructed, stop collector by itself.
    if (!avatarDestructed_) {
        avatar_.Disable();
        Stop();
    }
}

class EventRunnerImpl final : public EventInnerRunner {
public:
    explicit EventRunnerImpl(const std::shared_ptr<EventRunner>& runner) : EventInnerRunner(runner)
    {
        queue_ = std::make_shared<EventQueue>();
    }

    ~EventRunnerImpl() final = default;

    static void ThreadMain(const std::weak_ptr<EventRunnerImpl>& wp)
    {
        std::shared_ptr<EventRunnerImpl> inner = wp.lock();
        if (inner) {
            HILOG_INFO("ThreadMain: Start running for thread '%{public}s'", inner->threadName_.c_str());

            // Call system call to modify thread name.
            SystemCallSetThreadName(inner->threadName_);

            // Enter event loop.
            inner->Run();

            HILOG_INFO("ThreadMain: Stopped running for thread '%{public}s'", inner->threadName_.c_str());
        } else {
            HILOG_WARN("ThreadMain: EventRunner has been released just after its creation");
        }

        // Reclaim current thread.
        ThreadCollector::GetInstance().ReclaimCurrentThread();
    }

    void Run() final
    {
        // Prepare to start event loop.
        queue_->Prepare();

        // Make sure instance of 'EventRunner' exists.
        if (owner_.expired()) {
            return;
        }

        threadId_ = std::this_thread::get_id();

        // Save old event runner.
        std::weak_ptr<EventRunner> oldRunner = currentEventRunner;
        // Set current event runner into thread local data.
        currentEventRunner = owner_;

        // Start event looper.
        for (auto event = queue_->GetEvent(); event; event = queue_->GetEvent()) {
            std::shared_ptr<EventHandler> handler = event->GetOwner();
            // Make sure owner of the event exists.
            if (handler) {
                std::shared_ptr<Logger> logging = logger_;
                if (logging != nullptr) {
                    if (!event->HasTask()) {
                        InnerEvent::EventId eventId = event->GetInnerEventIdEx();
                        if (eventId.index() == TYPE_U32_INDEX) {
                            logging->Log(
                                "Dispatching to handler event id = " + std::to_string(std::get<uint32_t>(eventId)));
                        } else {
                            logging->Log("Dispatching to handler event id = " + std::get<std::string>(eventId));
                        }
                    } else {
                        logging->Log("Dispatching to handler event task name = " + event->GetTaskName());
                    }
                }
                handler->DistributeEvent(event);
            }
            // Release event manually, otherwise event will be released until next event coming.
            event.reset();
        }

        // Restore current event runner.
        currentEventRunner = oldRunner;
    }

    void Stop() final
    {
        queue_->Finish();
    }

    inline bool Attach(std::unique_ptr<std::thread>& thread)
    {
        auto exitThread = [queue = queue_]() { queue->Finish(); };

        return ThreadCollector::GetInstance().Deposit(thread, exitThread);
    }

    inline void SetThreadName(const std::string& threadName)
    {
        static std::atomic<uint32_t> idGenerator(1);

        if (threadName.empty()) {
            // Generate a default name
            threadName_ = "EventRunner#";
            threadName_ += std::to_string(idGenerator++);
        } else {
            threadName_ = threadName;
        }
    }
};
} // unnamed namespace

EventInnerRunner::EventInnerRunner(const std::shared_ptr<EventRunner>& runner)
    : queue_(nullptr), owner_(runner), logger_(nullptr), threadName_(""), threadId_()
{}

std::shared_ptr<EventRunner> EventInnerRunner::GetCurrentEventRunner()
{
    const std::weak_ptr<EventRunner>& wp = currentEventRunner;
    return wp.lock();
}

ThreadLocalData<std::weak_ptr<EventRunner>> EventInnerRunner::currentEventRunner;

namespace {
volatile bool ThreadCollector::avatarEnabled_ = false;

/*
 * All event runners are relied on 'currentEventRunner', so make sure destruction of 'currentEventRunner'
 * should after all event runners finished. All event runners finished in destruction of 'ThreadCollector::Avatar',
 * so instance of 'ThreadCollector::Avatar' MUST defined after 'currentEventRunner'.
 */
ThreadCollector::Avatar ThreadCollector::avatar_;
} // unnamed namespace

std::shared_ptr<EventRunner> EventRunner::mainRunner_;
EventRunner::CallbackTime EventRunner::distributeCallback_ = nullptr;

std::shared_ptr<EventRunner> EventRunner::Create(bool inNewThread)
{
    if (inNewThread) {
        return Create(std::string());
    }

    // Constructor of 'EventRunner' is private, could not use 'std::make_shared' to construct it.
    std::shared_ptr<EventRunner> sp(new EventRunner(false));
    auto innerRunner = std::make_shared<EventRunnerImpl>(sp);
    sp->innerRunner_ = innerRunner;
    sp->queue_ = innerRunner->GetEventQueue();

    return sp;
}

std::shared_ptr<EventRunner> EventRunner::Create(const std::string& threadName)
{
    // Constructor of 'EventRunner' is private, could not use 'std::make_shared' to construct it.
    std::shared_ptr<EventRunner> sp(new EventRunner(true));
    auto innerRunner = std::make_shared<EventRunnerImpl>(sp);
    sp->innerRunner_ = innerRunner;
    sp->queue_ = innerRunner->GetEventQueue();

    // Start new thread
    innerRunner->SetThreadName(threadName);
    auto thread =
        std::make_unique<std::thread>(EventRunnerImpl::ThreadMain, std::weak_ptr<EventRunnerImpl>(innerRunner));
    if (!innerRunner->Attach(thread)) {
        HILOG_WARN("Create: Failed to attach thread, maybe process is exiting");
        innerRunner->Stop();
        thread->join();
    }

    return sp;
}

std::shared_ptr<EventRunner> EventRunner::Current()
{
    HILOG_INFO("Current.");
    auto runner = EventInnerRunner::GetCurrentEventRunner();
    if (runner) {
        HILOG_INFO("EventInnerRunner::GetCurrentEventRunner()");
        return runner;
    }

    if (PlatformEventRunner::CheckCurrent()) {
        HILOG_INFO("Current: Create platform event runner on this thread.");

        std::shared_ptr<EventRunner> sp(new EventRunner(true));
        auto innerRunner = std::make_shared<PlatformEventRunner>(sp);
        sp->innerRunner_ = innerRunner;
        sp->queue_ = innerRunner->GetEventQueue();
        if (innerRunner->Init()) {
            return sp;
        }

        HILOG_ERROR("Current: Failed to start platform event runner.");
    }

    HILOG_ERROR("return nullptr");
    return nullptr;
}

EventRunner::EventRunner(bool deposit) : deposit_(deposit) {}

std::string EventRunner::GetRunnerThreadName() const
{
    return innerRunner_->GetThreadName();
}

EventRunner::~EventRunner()
{
    if (deposit_) {
        innerRunner_->Stop();
    }
}

ErrCode EventRunner::Run()
{
    if (deposit_) {
        HILOG_ERROR("Run: Do not call, if event runner is deposited");
        return EVENT_HANDLER_ERR_RUNNER_NO_PERMIT;
    }

    // Avoid more than one thread to start this runner.
    if (running_.exchange(true)) {
        HILOG_WARN("Run: Already running");
        return EVENT_HANDLER_ERR_RUNNER_ALREADY;
    }

    // Entry event loop.
    innerRunner_->Run();

    running_.store(false);
    return ERR_OK;
}

ErrCode EventRunner::Stop()
{
    if (deposit_) {
        HILOG_ERROR("Stop: Do not call, if event runner is deposited");
        return EVENT_HANDLER_ERR_RUNNER_NO_PERMIT;
    }

    if (running_.load()) {
        innerRunner_->Stop();
    } else {
        HILOG_WARN("Stop: Already stopped");
    }

    return ERR_OK;
}

void EventRunner::Dump(Dumper& dumper)
{
    if (!IsRunning()) {
        dumper.Dump(dumper.GetTag() + " Event runner is not running" + LINE_SEPARATOR);
        return;
    }

    if (queue_ == nullptr) {
        dumper.Dump(dumper.GetTag() + " Queue is nullLINE_SEPARATOR" + LINE_SEPARATOR);
        return;
    }

    dumper.Dump(dumper.GetTag() + " Event runner (" + "Thread name = " + innerRunner_->GetThreadName() +
                ", Thread ID = " + std::to_string(GetThreadId()) + ") is running" + LINE_SEPARATOR);

    queue_->Dump(dumper);
}

void EventRunner::DumpRunnerInfo(std::string& runnerInfo)
{
    if (!IsRunning()) {
        runnerInfo = "        Event runner is not running" + LINE_SEPARATOR;
    }

    if (queue_ == nullptr) {
        runnerInfo = "        Queue is null" + LINE_SEPARATOR;
        return;
    }

    std::string queueInfo;
    queue_->DumpQueueInfo(queueInfo);
    runnerInfo += queueInfo;
}

void EventRunner::SetLogger(const std::shared_ptr<Logger>& logger)
{
    innerRunner_->SetLogger(logger);
}

std::shared_ptr<EventQueue> EventRunner::GetCurrentEventQueue()
{
    auto runner = EventRunner::Current();
    if (!runner) {
        return nullptr;
    }
    return runner->queue_;
}

uint64_t EventRunner::GetThreadId()
{
    std::thread::id tid = innerRunner_->GetThreadId();
    std::stringstream buf;
    buf << tid;
    std::string stid = buf.str();
    return std::stoull(stid);
}

bool EventRunner::IsCurrentRunnerThread()
{
    return std::this_thread::get_id() == innerRunner_->GetThreadId();
}

std::shared_ptr<EventRunner> EventRunner::GetMainEventRunner()
{
    if (!mainRunner_) {
        mainRunner_ = Create(false);
        if (!mainRunner_) {
            HILOG_ERROR("mainRunner_ create fail");
        }
    }

    return mainRunner_;
}

bool EventRunner::IsAppMainThread()
{
#ifdef IOS_PLATFORM
    return pthread_main_np();
#else
    static int pid = -1;

    if (pid == -1) {
        pid = getpid();
    }
    if (pid == gettid()) {
        return true;
    }
    return false;
#endif
}
} // namespace AppExecFwk
} // namespace OHOS
