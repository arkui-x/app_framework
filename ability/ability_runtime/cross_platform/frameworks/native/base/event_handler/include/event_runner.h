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

#ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_RUNNER_H
#define BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_RUNNER_H

#include <atomic>

#include "dumper.h"
#include "event_queue.h"
#include "logger.h"

namespace OHOS {
namespace AppExecFwk {
class EventInnerRunner;

enum class ThreadMode: uint32_t {
    NEW_THREAD = 0,    // for new thread mode, event handler create thread
    FFRT,           // for new thread mode, use ffrt
};

class EventRunner final {
public:
    EventRunner() = delete;
    ~EventRunner();

    using CallbackTime = std::function<void(int64_t)>;
    static CallbackTime distributeCallback_;

    /**
     * Create new 'EventRunner'.
     *
     * @param inNewThread True if create new thread to start the 'EventRunner' automatically.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static std::shared_ptr<EventRunner> Create(bool inNewThread = true);

    /**
     * Create new 'EventRunner'.
     *
     * @param inNewThread True if create new thread to start the 'EventRunner' automatically.
     * @param threadMode thread mode, use ffrt or new thread, for inNewThread = true.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static std::shared_ptr<EventRunner> Create(bool inNewThread, ThreadMode threadMode)
    {
        (void) threadMode;
        return Create(inNewThread);
    }

    /**
     * Create new 'EventRunner' and start to run in a new thread.
     *
     * @param threadName Thread name of the new created thread.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static std::shared_ptr<EventRunner> Create(const std::string& threadName);

    /**
     * Create new 'EventRunner' and start to run in a new thread.
     *
     * @param threadName Thread name of the new created thread.
     * @param threadMode thread mode, use ffrt or new thread.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static std::shared_ptr<EventRunner> Create(const std::string &threadName, ThreadMode threadMode)
    {
        (void) threadMode;
        return Create(threadName);
    }

    /**
     * Create new 'EventRunner' and start to run in a new thread.
     * Eliminate ambiguity, while calling like 'EventRunner::Create("threadName")'.
     *
     * @param threadName Thread name of the new created thread.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static inline std::shared_ptr<EventRunner> Create(const char* threadName)
    {
        return Create((threadName != nullptr) ? std::string(threadName) : std::string());
    }

    /**
     * Create new 'EventRunner' and start to run in a new thread.
     * Eliminate ambiguity, while calling like 'EventRunner::Create("threadName")'.
     *
     * @param threadName Thread name of the new created thread.
     * @param threadMode thread mode, use ffrt or new thread.
     * @return Returns shared pointer of the new 'EventRunner'.
     */
    static inline std::shared_ptr<EventRunner> Create(const char *threadName, ThreadMode threadMode)
    {
        (void) threadMode;
        return Create((threadName != nullptr) ? std::string(threadName) : std::string());
    }

    /**
     * Get event runner on current thread.
     *
     * @return Returns shared pointer of the current 'EventRunner'.
     */
    static std::shared_ptr<EventRunner> Current();

    /**
     * Start to run the 'EventRunner'. Only used for the 'EventRunner' which is not running in new thread.
     * Only running on single thread.
     *
     * @return Returns 'ERR_OK' on success.
     */
    ErrCode Run();

    /**
     * Stop to run the 'EventRunner'. Only used for the 'EventRunner' which is not running in new thread.
     * It is a good practice to call {@link #Stop} on the same thread that called {@link #Run}.
     *
     * @return Returns 'ERR_OK' on success.
     */
    ErrCode Stop();

    /**
     * Get thread name
     *
     * @return Returns thread name.
     */
    std::string GetRunnerThreadName() const;

    /**
     * Get event queue from event runner.
     * This method only called by 'EventHandler'.
     *
     * @return Returns event queue.
     */
    inline const std::shared_ptr<EventQueue>& GetEventQueue() const
    {
        return queue_;
    }

    /**
     * Obtain the event queue of the EventRunner associated with the current thread.
     *
     * @return Return current event queue.
     */
    static std::shared_ptr<EventQueue> GetCurrentEventQueue();

    /**
     * Print out the internal information about an object in the specified format,
     * helping you diagnose internal errors of the object.
     *
     * @param dumpr The Dumper object you have implemented to process the output internal information.
     */
    void Dump(Dumper& dumper);

    /**
     * Print out the internal information about an object in the specified format,
     * helping you diagnose internal errors of the object.
     *
     * @param runnerInfo runner Info.
     */
    void DumpRunnerInfo(std::string& runnerInfo);

    /**
     * Set the Logger object for logging messages that are processed by this event runner.
     *
     * @param logger The Logger object you have implemented for logging messages.
     */
    void SetLogger(const std::shared_ptr<Logger>& logger);

    /**
     * Obtain the ID of the worker thread associated with this EventRunner.
     *
     * @return thread id.
     */
    uint64_t GetThreadId();

    /**
     * Check whether the current thread is the worker thread of this EventRunner.
     *
     * @return Returns true if the current thread is the worker thread of this EventRunner; returns false otherwise.
     */
    bool IsCurrentRunnerThread();

    /**
     * Set the distribution standard expiration time.
     *
     * @param deliveryTimeout the distribution standard expiration time.
     */
    void SetDeliveryTimeout(int64_t deliveryTimeout)
    {
        deliveryTimeout_ = deliveryTimeout;
    }

    /**
     * Get the distribution standard expiration time.
     *
     * @return the distribution standard expiration time.
     */
    int64_t GetDeliveryTimeout() const
    {
        return deliveryTimeout_;
    }

    /**
     * Set the execution standard timeout period.
     *
     * @param distributeTimeout the distribution standard expiration time.
     */
    void SetDistributeTimeout(int64_t distributeTimeout)
    {
        distributeTimeout_ = distributeTimeout;
    }

    /**
     * Get the execution standard timeout period.
     *
     * @return the distribution standard expiration time.
     */
    int64_t GetDistributeTimeout() const
    {
        return distributeTimeout_;
    }

    /**
     * Set the main thread timeout period.
     *
     * @param distributeTimeout the distribution standard expiration time.
     */
    void SetTimeout(int64_t distributeTimeout)
    {
        timeout_ = distributeTimeout;
    }

    /**
     * Set distribute time out callback.
     *
     * @param callback Distribute Time out callback.
     */
    void SetTimeoutCallback(CallbackTime callback)
    {
        distributeCallback_ = callback;
    }

    /**
     * Get the execution standard timeout period.
     *
     * @return the distribution standard expiration time.
     */
    int64_t GetTimeout() const
    {
        return timeout_;
    }

    /**
     * Check if the current application is the main thread.
     */
    static bool IsAppMainThread();

    /**
     * Obtains the EventRunner for the main thread of the application.
     *
     * @return Returns the EventRunner for the main thread of the application.
     */
    static std::shared_ptr<EventRunner> GetMainEventRunner();

private:
    explicit EventRunner(bool deposit);

    friend class EventHandler;

    /**
     * Check whether this event runner is running.
     *
     * @return if this event runner is running return true otherwise return false
     */
    inline bool IsRunning() const
    {
        // If this runner is deposited, it it always running
        return (deposit_) || (running_.load());
    }

    int64_t deliveryTimeout_ = 0;
    int64_t distributeTimeout_ = 0;
    int64_t timeout_ = 0;
    bool deposit_ { true };
    std::atomic<bool> running_ { false };
    std::shared_ptr<EventQueue> queue_;
    std::shared_ptr<EventInnerRunner> innerRunner_;
    static std::shared_ptr<EventRunner> mainRunner_;
};
} // namespace AppExecFwk
namespace EventHandling = AppExecFwk;
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_RUNNER_H
