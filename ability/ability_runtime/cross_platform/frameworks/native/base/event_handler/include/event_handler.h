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

#ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_H
#define BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_H

#include "dumper.h"
#include "event_runner.h"

#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

namespace OHOS {
namespace AppExecFwk {
enum class EventType {
    SYNC_EVENT = 0,
    DELAY_EVENT = 1,
    TIMING_EVENT = 2,
};

struct Caller {
    std::string file_ { "" };
    int line_ { 0 };
    std::string func_ { "" };
#if __has_builtin(__builtin_FILE)
    Caller(std::string file = __builtin_FILE(), int line = __builtin_LINE(), std::string func = __builtin_FUNCTION())
        : file_(file), line_(line), func_(func)
    {}
#else
    Caller() {}
#endif
    std::string ToString()
    {
        if (file_.empty()) {
            return "[ ]";
        }
        size_t split = file_.find_last_of("/\\");
        if (split == std::string::npos) {
            split = 0;
        }
        std::string caller("[" + file_.substr(split + 1) + "(" + func_ + ":" + std::to_string(line_) + ")]");
        return caller;
    }
};

template<typename T>
class ThreadLocalData;

class EventHandler : public std::enable_shared_from_this<EventHandler> {
public:
    using CallbackTimeout = std::function<void()>;
    using Callback = InnerEvent::Callback;
    using Priority = EventQueue::Priority;

    /**
     * Constructor, set 'EventRunner' automatically.
     *
     * @param runner The 'EventRunner'.
     */
    explicit EventHandler(const std::shared_ptr<EventRunner>& runner = nullptr);
    virtual ~EventHandler();

    /**
     * Get event handler that running on current thread.
     *
     * @return Returns shared pointer of the current 'EventHandler'.
     */
    static std::shared_ptr<EventHandler> Current();

    /**
     * Send an event.
     *
     * @param event Event which should be handled.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully. If returns false, event should be released manually.
     */
    bool SendEvent(InnerEvent::Pointer& event, int64_t delayTime = 0, Priority priority = Priority::LOW);

    /**
     * Send an event.
     *
     * @param event Event which should be handled.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully. If returns false, event should be released manually.
     */
    bool SendTimingEvent(InnerEvent::Pointer& event, int64_t taskTime, Priority priority = Priority::LOW);

    /**
     * Send an event.
     *
     * @param event Event which should be handled.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully. If returns false, event should be released manually.
     */
    inline bool SendEvent(InnerEvent::Pointer& event, Priority priority)
    {
        return SendEvent(event, 0, priority);
    }

    /**
     * Send an event.
     *
     * @param event Event which should be handled.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendEvent(InnerEvent::Pointer&& event, int64_t delayTime = 0, Priority priority = Priority::LOW)
    {
        return SendEvent(event, delayTime, priority);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendEvent(uint32_t innerEventId, int64_t param, int64_t delayTime)
    {
        return SendEvent(InnerEvent::Get(innerEventId, param), delayTime);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendEvent(uint32_t innerEventId, int64_t delayTime = 0, Priority priority = Priority::LOW)
    {
        return SendEvent(InnerEvent::Get(innerEventId, 0), delayTime, priority);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendEvent(uint32_t innerEventId, Priority priority)
    {
        return SendEvent(InnerEvent::Get(innerEventId, 0), 0, priority);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendEvent(uint32_t innerEventId, const std::shared_ptr<T>& object, int64_t delayTime = 0)
    {
        return SendEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendEvent(uint32_t innerEventId, const std::weak_ptr<T>& object, int64_t delayTime = 0)
    {
        return SendEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendEvent(uint32_t innerEventId, std::unique_ptr<T, D>& object, int64_t delayTime = 0)
    {
        return SendEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendEvent(uint32_t innerEventId, std::unique_ptr<T, D>&& object, int64_t delayTime = 0)
    {
        return SendEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an immediate event.
     *
     * @param event Event which should be handled.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendImmediateEvent(InnerEvent::Pointer& event)
    {
        return SendEvent(event, 0, Priority::IMMEDIATE);
    }

    /**
     * Send an immediate event.
     *
     * @param event Event which should be handled.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendImmediateEvent(InnerEvent::Pointer&& event)
    {
        return SendImmediateEvent(event);
    }

    /**
     * Send an immediate event.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendImmediateEvent(uint32_t innerEventId, int64_t param = 0)
    {
        return SendImmediateEvent(InnerEvent::Get(innerEventId, param));
    }

    /**
     * Send an immediate event.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of object.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendImmediateEvent(uint32_t innerEventId, const std::shared_ptr<T>& object)
    {
        return SendImmediateEvent(InnerEvent::Get(innerEventId, object));
    }

    /**
     * Send an immediate event.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of object.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendImmediateEvent(uint32_t innerEventId, const std::weak_ptr<T>& object)
    {
        return SendImmediateEvent(InnerEvent::Get(innerEventId, object));
    }

    /**
     * Send an immediate event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendImmediateEvent(uint32_t innerEventId, std::unique_ptr<T, D>& object)
    {
        return SendImmediateEvent(InnerEvent::Get(innerEventId, object));
    }

    /**
     * Send an immediate event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendImmediateEvent(uint32_t innerEventId, std::unique_ptr<T, D>&& object)
    {
        return SendImmediateEvent(InnerEvent::Get(innerEventId, object));
    }

    /**
     * Send an high priority event.
     *
     * @param event Event which should be handled.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendHighPriorityEvent(InnerEvent::Pointer& event, int64_t delayTime = 0)
    {
        return SendEvent(event, delayTime, Priority::HIGH);
    }

    /**
     * Send an high priority event.
     *
     * @param event Event which should be handled.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendHighPriorityEvent(InnerEvent::Pointer&& event, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(event, delayTime);
    }

    /**
     * Send an high priority event.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendHighPriorityEvent(uint32_t innerEventId, int64_t param = 0, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(InnerEvent::Get(innerEventId, param), delayTime);
    }

    /**
     * Send an high priority event.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendHighPriorityEvent(uint32_t innerEventId, const std::shared_ptr<T>& object, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an high priority event.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendHighPriorityEvent(uint32_t innerEventId, const std::weak_ptr<T>& object, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an high priority event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendHighPriorityEvent(uint32_t innerEventId, std::unique_ptr<T, D>& object, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Send an high priority event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendHighPriorityEvent(uint32_t innerEventId, std::unique_ptr<T, D>&& object, int64_t delayTime = 0)
    {
        return SendHighPriorityEvent(InnerEvent::Get(innerEventId, object), delayTime);
    }

    /**
     * Post a task.
     *
     * @param callback Task callback.
     * @param name Name of the task.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostTask(const Callback& callback, const std::string& name = std::string(), int64_t delayTime = 0,
        Priority priority = Priority::LOW, Caller caller = {})
    {
        return SendEvent(InnerEvent::Get(callback, name.empty() ? caller.ToString() : name), delayTime, priority);
    }

    /**
     * Set delivery time out callback.
     *
     * @param callback Delivery Time out callback.
     */
    void SetDeliveryTimeoutCallback(const Callback& callback)
    {
        deliveryTimeoutCallback_ = callback;
    }

    /**
     * Set distribute time out callback.
     *
     * @param callback Distribute Time out callback.
     */
    void SetDistributeTimeoutCallback(const Callback& callback)
    {
        distributeTimeoutCallback_ = callback;
    }

    /**
     * Post a task.
     *
     * @param callback Task callback.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostTask(const Callback& callback, Priority priority, Caller caller = {})
    {
        return PostTask(callback, caller.ToString(), 0, priority);
    }

    /**
     * Post a task.
     *
     * @param callback Task callback.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostTask(
        const Callback& callback, int64_t delayTime, Priority priority = Priority::LOW, Caller caller = {})
    {
        return PostTask(callback, caller.ToString(), delayTime, priority);
    }

    /**
     * Post an immediate task.
     *
     * @param callback Task callback.
     * @param name Remove events by name of the task.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostImmediateTask(const Callback& callback, const std::string& name = std::string(), Caller caller = {})
    {
        return SendEvent(InnerEvent::Get(callback, name.empty() ? caller.ToString() : name), 0, Priority::IMMEDIATE);
    }

    /**
     * Post a high priority task.
     *
     * @param callback Task callback.
     * @param name Name of the task.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostHighPriorityTask(
        const Callback& callback, const std::string& name = std::string(), int64_t delayTime = 0, Caller caller = {})
    {
        return PostTask(callback, name.empty() ? caller.ToString() : name, delayTime, Priority::HIGH);
    }

    /**
     * Post a high priority task.
     *
     * @param callback Task callback.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostHighPriorityTask(const Callback& callback, int64_t delayTime, Caller caller = {})
    {
        return PostHighPriorityTask(callback, caller.ToString(), delayTime);
    }

    /**
     * Post a idle task.
     *
     * @param callback task callback.
     * @param name Name of the task.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostIdleTask(
        const Callback& callback, const std::string& name = std::string(), int64_t delayTime = 0, Caller caller = {})
    {
        return PostTask(callback, name.empty() ? caller.ToString() : name, delayTime, Priority::IDLE);
    }

    /**
     * Post a idle task.
     *
     * @param callback Task callback.
     * @param delayTime Process the event after 'delayTime' milliseconds.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostIdleTask(const Callback& callback, int64_t delayTime, Caller caller = {})
    {
        return PostIdleTask(callback, caller.ToString(), delayTime);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param event Event which should be handled.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully. If returns false, event should be released manually.
     */
    bool SendSyncEvent(InnerEvent::Pointer& event, Priority priority = Priority::LOW);

    /**
     * Send an event.
     *
     * @param event Event which should be handled.
     * @param priority Priority of the event queue for this event.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendSyncEvent(InnerEvent::Pointer&& event, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(event, priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendSyncEvent(uint32_t innerEventId, int64_t param = 0, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, param), priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendSyncEvent(uint32_t innerEventId, Priority priority)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, 0), priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of object.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendSyncEvent(
        uint32_t innerEventId, const std::shared_ptr<T>& object, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, object), priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of object.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendSyncEvent(uint32_t innerEventId, const std::weak_ptr<T>& object, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, object), priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendSyncEvent(uint32_t innerEventId, std::unique_ptr<T, D>& object, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, object), priority);
    }

    /**
     * Send an event, and wait until this event has been handled.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendSyncEvent(uint32_t innerEventId, std::unique_ptr<T, D>&& object, Priority priority = Priority::LOW)
    {
        return SendSyncEvent(InnerEvent::Get(innerEventId, object), priority);
    }

    /**
     * Post a task, and wait until this task has been handled.
     *
     * @param callback Task callback.
     * @param name Name of the task.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostSyncTask(
        const Callback& callback, const std::string& name, Priority priority = Priority::LOW, Caller caller = {})
    {
        return SendSyncEvent(InnerEvent::Get(callback, name.empty() ? caller.ToString() : name), priority);
    }

    /**
     * Post a task, and wait until this task has been handled.
     *
     * @param callback Task callback.
     * @param priority Priority of the event queue for this event, IDLE is not permitted for sync event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostSyncTask(const Callback& callback, Priority priority = Priority::LOW, Caller caller = {})
    {
        return PostSyncTask(callback, caller.ToString(), priority);
    }

    /**
     * Send a timing event.
     *
     * @param event Event which should be handled.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendTimingEvent(InnerEvent::Pointer&& event, int64_t taskTime, Priority priority)
    {
        return SendTimingEvent(event, taskTime, priority);
    }

    /**
     * Send a timing event.
     *
     * @param event Event which should be handled.
     * @param taskTime Process the event at taskTime.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendTimingEvent(InnerEvent::Pointer&& event, int64_t taskTime)
    {
        return SendTimingEvent(event, taskTime, Priority::LOW);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param taskTime Process the event at taskTime.
     * @param param Basic parameter of the event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendTimingEvent(uint32_t innerEventId, int64_t taskTime, int64_t param)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, param), taskTime);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendTimingEvent(uint32_t innerEventId, int64_t taskTime, Priority priority)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, 0), taskTime, priority);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if event has been sent successfully.
     */
    inline bool SendTimingEvent(uint32_t innerEventId, int64_t taskTime)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, 0), taskTime, Priority::LOW);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of object.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendTimingEvent(
        uint32_t innerEventId, const std::shared_ptr<T>& object, int64_t taskTime, Priority priority = Priority::LOW)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, object), taskTime, priority);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of object.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event
     * @return Returns true if event has been sent successfully.
     */
    template<typename T>
    inline bool SendTimingEvent(
        uint32_t innerEventId, const std::weak_ptr<T>& object, int64_t taskTime, Priority priority = Priority::LOW)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, object), taskTime, priority);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendTimingEvent(
        uint32_t innerEventId, std::unique_ptr<T, D>& object, int64_t taskTime, Priority priority = Priority::LOW)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, object), taskTime, priority);
    }

    /**
     * Send a timing event.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of object.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event
     * @return Returns true if event has been sent successfully.
     */
    template<typename T, typename D>
    inline bool SendTimingEvent(
        uint32_t innerEventId, std::unique_ptr<T, D>&& object, int64_t taskTime, Priority priority = Priority::LOW)
    {
        return SendTimingEvent(InnerEvent::Get(innerEventId, object), taskTime, priority);
    }

    /**
     * Post a timing task.
     *
     * @param callback Task callback.
     * @param taskTime Process the event at taskTime.
     * @param name Name of the task.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostTimingTask(const Callback& callback, int64_t taskTime, const std::string& name = std::string(),
        Priority priority = Priority::LOW, Caller caller = {})
    {
        return SendTimingEvent(InnerEvent::Get(callback, name.empty() ? caller.ToString() : name), taskTime, priority);
    }

    /**
     * Post a timing task.
     *
     * @param callback Task callback.
     * @param taskTime Process the event at taskTime.
     * @param priority Priority of the event queue for this event.
     * @return Returns true if task has been sent successfully.
     */
    inline bool PostTimingTask(
        const Callback& callback, int64_t taskTime, Priority priority = Priority::LOW, Caller caller = {})
    {
        return PostTimingTask(callback, taskTime, caller.ToString(), priority);
    }

    /**
     * Remove all sent events.
     */
    void RemoveAllEvents();

    /**
     * Remove sent events.
     *
     * @param innerEventId The id of the event.
     */
    void RemoveEvent(uint32_t innerEventId);

    /**
     * Remove sent events.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event.
     */
    void RemoveEvent(uint32_t innerEventId, int64_t param);

    /**
     * Remove a task.
     *
     * @param name Name of the task.
     */
    void RemoveTask(const std::string& name);

    /**
     * Add file descriptor listener for a file descriptor.
     *
     * @param fileDescriptor File descriptor.
     * @param events Events from file descriptor, such as input, output, error
     * @param listener Listener callback.
     * @return Return 'ERR_OK' on success.
     */
    ErrCode AddFileDescriptorListener(
        int32_t fileDescriptor, uint32_t events, const std::shared_ptr<FileDescriptorListener>& listener);

    /**
     * Remove all file descriptor listeners.
     */
    void RemoveAllFileDescriptorListeners();

    /**
     * Remove file descriptor listener for a file descriptor.
     *
     * @param fileDescriptor File descriptor.
     */
    void RemoveFileDescriptorListener(int32_t fileDescriptor);

    /**
     * Set the 'EventRunner' to the 'EventHandler'.
     *
     * @param runner The 'EventRunner'.
     */
    void SetEventRunner(const std::shared_ptr<EventRunner>& runner);

    /**
     * Get the 'EventRunner' of the 'EventHandler'.
     *
     * @return Return the 'EventRunner'.
     */
    inline const std::shared_ptr<EventRunner>& GetEventRunner() const
    {
        return eventRunner_;
    }

    /**
     * Distribute time out handler.
     *
     * @param beginTime Dotting before distribution.
     */
    void DistributeTimeoutHandler(const InnerEvent::TimePoint& beginTime);

    /**
     * Distribute the event.
     *
     * @param event The event should be distributed.
     */
    void DistributeEvent(const InnerEvent::Pointer& event);

    /**
     * Distribute time out action.
     *
     * @param event The event should be distribute.
     * @param nowStart Dotting before distribution.
     */
    void DistributeTimeAction(const InnerEvent::Pointer& event, InnerEvent::TimePoint nowStart);

    /**
     * Delivery time out action.
     *
     * @param event The event should be distribute.
     * @param nowStart Dotting before distribution.
     */
    void DeliveryTimeAction(const InnerEvent::Pointer& event, InnerEvent::TimePoint nowStart);

    /**
     * Print out the internal information about an object in the specified format,
     * helping you diagnose internal errors of the object.
     *
     * @param dumpr The Dumper object you have implemented to process the output internal information.
     */
    void Dump(Dumper& dumper);

    /**
     * Check whether an event with the given ID can be found among the events that have been sent but not processed.
     *
     * @param innerEventId The id of the event.
     */
    bool HasInnerEvent(uint32_t innerEventId);

    /**
     * Check whether an event carrying the given param can be found among the events that have been sent but not
     * processed.
     *
     * @param param Basic parameter of the event.
     */
    bool HasInnerEvent(int64_t param);

    /**
     * Check whether an event carrying the given param can be found among the events that have been sent but not
     * processed.
     *
     * @param event InnerEvent whose name is to be obtained.
     * @return Returns the task name if the given event contains a specific task; returns the event ID otherwise.
     */
    std::string GetEventName(const InnerEvent::Pointer& event);

    /**
     * Checks whether the current event handler is idle
     * @return Returns true if current event handler is idle otherwise return false.
     */
    bool IsIdle();

protected:
    /**
     * Process the event. Developers should override this method.
     *
     * @param event The event should be processed.
     */
    virtual void ProcessEvent(const InnerEvent::Pointer& event);

private:
    std::shared_ptr<EventRunner> eventRunner_;
    CallbackTimeout deliveryTimeoutCallback_;
    CallbackTimeout distributeTimeoutCallback_;

    static thread_local std::shared_ptr<EventHandler> currentEventHandler;
};
} // namespace AppExecFwk
namespace EventHandling = AppExecFwk;
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_H
