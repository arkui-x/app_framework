/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_INNER_EVENT_H
#define BASE_EVENTHANDLER_INTERFACES_INNER_API_INNER_EVENT_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <typeinfo>
#include <variant>

namespace OHOS {
namespace HiviewDFX {
class HiTraceId;
}

namespace AppExecFwk {
constexpr static uint32_t TYPE_U32_INDEX = 0u;
using HiTraceId = OHOS::HiviewDFX::HiTraceId;

class EventHandler;

const std::string LINE_SEPARATOR = "\n";

class InnerEvent final {
public:
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
    using Callback = std::function<void()>;
    using Pointer = std::unique_ptr<InnerEvent, void (*)(InnerEvent*)>;
    using EventId = std::variant<uint32_t, std::string>;

    class Waiter {
    public:
        Waiter() = default;
        virtual ~Waiter() = default;

        virtual void Wait() = 0;
        virtual void Notify() = 0;
    };

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    static Pointer Get(uint32_t innerEventId, int64_t param = 0);

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    static Pointer Get(const EventId& innerEventId, int64_t param = 0);

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Shared pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T>
    static inline Pointer Get(uint32_t innerEventId, const std::shared_ptr<T>& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveSharedPtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Weak pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T>
    static inline Pointer Get(uint32_t innerEventId, const std::weak_ptr<T>& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveSharedPtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(uint32_t innerEventId, std::unique_ptr<T, D>&& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(const EventId& innerEventId, std::unique_ptr<T, D>&& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(uint32_t innerEventId, std::unique_ptr<T, D>& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param object Unique pointer of the object.
     * @param param Basic parameter of the event, default is 0.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(const EventId& innerEventId, std::unique_ptr<T, D>& object, int64_t param = 0)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event.
     * @param object Shared pointer of the object.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T>
    static inline Pointer Get(uint32_t innerEventId, int64_t param, const std::shared_ptr<T>& object)
    {
        auto event = Get(innerEventId, param);
        event->SaveSharedPtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event.
     * @param object Weak pointer of the object.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T>
    static inline Pointer Get(uint32_t innerEventId, int64_t param, const std::weak_ptr<T>& object)
    {
        auto event = Get(innerEventId, param);
        event->SaveSharedPtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event.
     * @param object Unique pointer of the object.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(uint32_t innerEventId, int64_t param, std::unique_ptr<T, D>&& object)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param innerEventId The id of the event.
     * @param param Basic parameter of the event.
     * @param object Unique pointer of the object.
     * @return Returns the pointer of InnerEvent instance.
     */
    template<typename T, typename D>
    static inline Pointer Get(uint32_t innerEventId, int64_t param, std::unique_ptr<T, D>& object)
    {
        auto event = Get(innerEventId, param);
        event->SaveUniquePtr(object);
        return event;
    }

    /**
     * Get InnerEvent instance from pool.
     *
     * @param callback Callback for task.
     * @param name Name of task.
     * @return Returns the pointer of InnerEvent instance, if callback is invalid, returns nullptr object.
     */
    static Pointer Get(const Callback& callback, const std::string& name = std::string());

    /**
     * Get InnerEvent instance from pool.
     *
     * @return Returns the pointer of InnerEvent instance
     */
    static Pointer Get();

    /**
     * Get owner of the event.
     *
     * @return Returns owner of the event after it has been sent.
     */
    inline std::shared_ptr<EventHandler> GetOwner() const
    {
        return owner_.lock();
    }

    /**
     * Set owner of the event.
     *
     * @param owner Owner for the event.
     */
    inline void SetOwner(const std::shared_ptr<EventHandler>& owner)
    {
        owner_ = owner;
    }

    /**
     * Get handle time of the event.
     *
     * @return Returns handle time of the event after it has been sent.
     */
    inline const TimePoint& GetHandleTime() const
    {
        return handleTime_;
    }

    /**
     * Set handle time of the event.
     *
     * @param handleTime Handle time of the event.
     */
    inline void SetHandleTime(const TimePoint& handleTime)
    {
        handleTime_ = handleTime;
    }

    /**
     * Get send time of the event.
     *
     * @return Returns send time of the event after it has been sent.
     */
    inline const TimePoint& GetSendTime() const
    {
        return sendTime_;
    }

    /**
     * Set send time of the event.
     *
     * @param sendTime Send time of the event.
     */
    inline void SetSendTime(const TimePoint& sendTime)
    {
        sendTime_ = sendTime;
    }

    /**
     * Get id of the event.
     * Make sure {@link #hasTask} returns false.
     *
     * @return Returns id of the event after it has been sent.
     */
    uint32_t GetInnerEventId() const;

    /**
     * Get id of the event.
     * Make sure {@link #hasTask} returns false.
     *
     * @return Returns id of the event after it has been sent.
     */
    inline EventId GetInnerEventIdEx() const
    {
        return innerEventId_;
    }

    /**
     * Get basic param of the event.
     * Make sure {@link #hasTask} returns false.
     *
     * @return Returns basic param of the event after it has been sent.
     */
    inline int64_t GetParam() const
    {
        return param_;
    }

    /**
     * Get saved object.
     *
     * @return Returns shared pointer of saved object.
     */
    template<typename T>
    std::shared_ptr<T> GetSharedObject() const
    {
        const std::shared_ptr<T>& sharedObject = *reinterpret_cast<const std::shared_ptr<T>*>(smartPtr_);
        if (CalculateSmartPtrTypeId(sharedObject) == smartPtrTypeId_) {
            return sharedObject;
        }

        const std::weak_ptr<T>& weakObject = *reinterpret_cast<const std::weak_ptr<T>*>(smartPtr_);
        if (CalculateSmartPtrTypeId(weakObject) == smartPtrTypeId_) {
            return weakObject.lock();
        }

        WarnSmartPtrCastMismatch();
        return nullptr;
    }

    /**
     * Get saved object.
     *
     * @return Returns unique pointer of saved object.
     */
    template<typename T>
    std::unique_ptr<T> GetUniqueObject() const
    {
        std::unique_ptr<T>& object = *reinterpret_cast<std::unique_ptr<T>*>(smartPtr_);
        if (CalculateSmartPtrTypeId(object) == smartPtrTypeId_) {
            return std::move(object);
        }

        WarnSmartPtrCastMismatch();
        return nullptr;
    }

    /**
     * Get saved object.
     *
     * @return Returns unique pointer of saved object.
     */
    template<typename T, typename D>
    std::unique_ptr<T, D> GetUniqueObject() const
    {
        std::unique_ptr<T, D>& object = *reinterpret_cast<std::unique_ptr<T, D>*>(smartPtr_);
        if (CalculateSmartPtrTypeId(object) == smartPtrTypeId_) {
            return std::move(object);
        }

        WarnSmartPtrCastMismatch();
        return std::unique_ptr<T, D>(nullptr, nullptr);
    }

    /**
     * Get task name.
     * Make sure {@link #hasTask} returns true.
     *
     * @return Returns the task name.
     */
    inline const std::string& GetTaskName() const
    {
        return taskName_;
    }

    /**
     * Get task callback.
     * Make sure {@link #hasTask} returns true.
     *
     * @return Returns the callback of the task.
     */
    inline const Callback& GetTaskCallback() const
    {
        return taskCallback_;
    }

    /**
     * Obtains the Runnable task that will be executed when this InnerEvent is handled.
     *
     * @return Returns the callback of the task.
     */
    inline const Callback& GetTask() const
    {
        return GetTaskCallback();
    }

    /**
     * Check whether it takes a task callback in event.
     *
     * @return Returns true if it takes a task callback.
     */
    inline bool HasTask() const
    {
        return static_cast<bool>(taskCallback_);
    }

    /**
     * Prints out the internal information about an object in the specified format,
     * helping you diagnose internal errors of the object.
     *
     * @param return The content of the event.
     */
    std::string Dump();

    /**
     * Check the event whether sent by enhanced api.
     */
    inline bool IsEnhanced()
    {
        return isEnhanced_;
    }

private:
    using SmartPtrDestructor = void (*)(void*);

    InnerEvent() = default;
    ~InnerEvent() = default;

    void ClearEvent();

    static void WarnSmartPtrCastMismatch();

    template<typename T>
    static void ReleaseSmartPtr(void* smartPtr)
    {
        if (smartPtr != nullptr) {
            delete reinterpret_cast<T*>(smartPtr);
        }
    }

    template<typename T>
    inline void SaveSharedPtr(const T& object)
    {
        smartPtr_ = new T(object);
        smartPtrDtor_ = ReleaseSmartPtr<T>;
        smartPtrTypeId_ = CalculateSmartPtrTypeId(object);
    }

    template<typename T>
    inline void SaveUniquePtr(T& object)
    {
        smartPtr_ = new T(std::move(object));
        smartPtrDtor_ = ReleaseSmartPtr<T>;
        smartPtrTypeId_ = CalculateSmartPtrTypeId(object);
    }

    /**
     * return trace id.
     *
     * @return return hiTrace Id.
     */
    const std::shared_ptr<HiTraceId> GetTraceId();

    /*
     * Calculate the type id for different smart pointers.
     */
#ifdef __GXX_RTTI
    // If RTTI(Run-Time Type Info) is enabled, use hash code of type info.
    template<typename T>
    static inline size_t CalculateSmartPtrTypeId(const T&)
    {
        return typeid(T).hash_code();
    }
#else  // #ifdef __GXX_RTTI
    // Otherwise, generate type id using smart pointer type and the size of the elements they contain.
    static const size_t SHARED_PTR_TYPE = 0x10000000;
    static const size_t WEAK_PTR_TYPE = 0x20000000;
    static const size_t UNIQUE_PTR_TYPE = 0x30000000;
    static const size_t UNIQUE_PTR_ARRAY_TYPE = 0x40000000;

    template<typename T>
    static inline size_t CalculateSmartPtrTypeId(const std::shared_ptr<T>&)
    {
        return (sizeof(T) | SHARED_PTR_TYPE);
    }

    template<typename T>
    static inline size_t CalculateSmartPtrTypeId(const std::weak_ptr<T>&)
    {
        return (sizeof(T) | WEAK_PTR_TYPE);
    }

    template<typename T, typename D>
    static inline size_t CalculateSmartPtrTypeId(const std::unique_ptr<T, D>&)
    {
        return (sizeof(T) | (sizeof(D) - 1) | UNIQUE_PTR_TYPE);
    }

    template<typename T, typename D>
    static inline size_t CalculateSmartPtrTypeId(const std::unique_ptr<T[], D>&)
    {
        return (sizeof(T) | (sizeof(D) - 1) | UNIQUE_PTR_ARRAY_TYPE);
    }
#endif // #ifdef __GXX_RTTI

    // Used by event handler to create waiter.
    const std::shared_ptr<Waiter>& CreateWaiter();

    // Used by event handler to tell whether event has waiter.
    bool HasWaiter() const;

    // Let event pool to create instance of events.
    friend class InnerEventPool;
    // Let event handler to access private interface.
    friend class EventHandler;

    std::weak_ptr<EventHandler> owner_;
    TimePoint handleTime_;
    TimePoint sendTime_;

    // Event id of the event, if it is not a task object
    EventId innerEventId_ = { 0u };

    // Simple parameter for the event.
    int64_t param_ { 0 };

    // Using to save smart pointer
    size_t smartPtrTypeId_ { 0 };
    void* smartPtr_ { nullptr };
    SmartPtrDestructor smartPtrDtor_ { nullptr };

    // Task callback and its name.
    Callback taskCallback_;
    std::string taskName_;

    // Used for synchronized event.
    std::shared_ptr<Waiter> waiter_;

    // use to store hitrace Id
    std::shared_ptr<HiTraceId> hiTraceId_;

    bool isEnhanced_ = false;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_INNER_EVENT_H
