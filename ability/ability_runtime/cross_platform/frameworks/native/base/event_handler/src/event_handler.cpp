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

#include "event_handler.h"

#include <unistd.h>

#include "event_handler_utils.h"
#ifdef HAS_HICHECKER_NATIVE_PART
#include "hichecker.h"
#endif // HAS_HICHECKER_NATIVE_PART
#include "hilog.h"
#include "thread_local_data.h"

using namespace OHOS::HiviewDFX;
namespace OHOS {
namespace AppExecFwk {
static constexpr int DATETIME_STRING_LENGTH = 80;

thread_local std::shared_ptr<EventHandler> EventHandler::currentEventHandler = nullptr;

std::shared_ptr<EventHandler> EventHandler::Current()
{
    return currentEventHandler;
}

EventHandler::EventHandler(const std::shared_ptr<EventRunner>& runner) : eventRunner_(runner) {}

EventHandler::~EventHandler()
{
    if (eventRunner_) {
        /*
         * This handler is finishing, need to remove all events belong to it.
         * But events only have weak pointer of this handler,
         * now weak pointer is invalid, so these events become orphans.
         */
        eventRunner_->GetEventQueue()->RemoveOrphan();
    }
}

bool EventHandler::SendEvent(InnerEvent::Pointer& event, int64_t delayTime, Priority priority)
{
    if (!event) {
        HILOG_ERROR("SendEvent: Could not send an invalid event");
        return false;
    }

    if (eventRunner_ == nullptr) {
        HILOG_ERROR("SendEvent: MUST Set event runner before sending events");
        return false;
    }

    InnerEvent::TimePoint now = InnerEvent::Clock::now();
    event->SetSendTime(now);

    if (delayTime > 0) {
        event->SetHandleTime(now + std::chrono::milliseconds(delayTime));
    } else {
        event->SetHandleTime(now);
    }

    event->SetOwner(shared_from_this());

    eventRunner_->GetEventQueue()->Insert(event, priority);
    return true;
}

bool EventHandler::SendTimingEvent(InnerEvent::Pointer& event, int64_t taskTime, Priority priority)
{
    InnerEvent::TimePoint nowSys = InnerEvent::Clock::now();
    auto epoch = nowSys.time_since_epoch();
    long nowSysTime = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
    int64_t delayTime = taskTime - nowSysTime;
    if (delayTime < 0) {
        HILOG_ERROR("SendTimingEvent: SendTime is before now systime, change to 0 delaytime Event");
        return SendEvent(event, 0, priority);
    }

    return SendEvent(event, delayTime, priority);
}

bool EventHandler::SendSyncEvent(InnerEvent::Pointer& event, Priority priority)
{
    if ((!event) || (priority == Priority::IDLE)) {
        HILOG_ERROR("SendSyncEvent: Could not send an invalid event or idle event");
        return false;
    }

    if ((!eventRunner_) || (!eventRunner_->IsRunning())) {
        HILOG_ERROR("SendSyncEvent: MUST Set a running event runner before sending sync events");
        return false;
    }

    // If send a sync event in same event runner, distribute here.
    if (eventRunner_ == EventRunner::Current()) {
        DistributeEvent(event);
        return true;
    }

    // Create waiter, used to block.
    auto waiter = event->CreateWaiter();
    // Send this event as normal one.
    if (!SendEvent(event, 0, priority)) {
        return false;
    }
    // Wait until event is processed(recycled).
    waiter->Wait();

    return true;
}

void EventHandler::RemoveAllEvents()
{
    if (!eventRunner_) {
        HILOG_ERROR("RemoveAllEvents: MUST Set event runner before removing all events");
        return;
    }

    eventRunner_->GetEventQueue()->Remove(shared_from_this());
}

void EventHandler::RemoveEvent(uint32_t innerEventId)
{
    if (!eventRunner_) {
        HILOG_ERROR("RemoveEvent: MUST Set event runner before removing events by id");
        return;
    }

    eventRunner_->GetEventQueue()->Remove(shared_from_this(), innerEventId);
}

void EventHandler::RemoveEvent(uint32_t innerEventId, int64_t param)
{
    if (!eventRunner_) {
        HILOG_ERROR("RemoveEvent: MUST Set event runner before removing events by id and param");
        return;
    }

    eventRunner_->GetEventQueue()->Remove(shared_from_this(), innerEventId, param);
}

void EventHandler::RemoveTask(const std::string& name)
{
    if (!eventRunner_) {
        HILOG_ERROR("RemoveTask: MUST Set event runner before removing events by task name");
        return;
    }

    eventRunner_->GetEventQueue()->Remove(shared_from_this(), name);
}

ErrCode EventHandler::AddFileDescriptorListener(
    int32_t fileDescriptor, uint32_t events, const std::shared_ptr<FileDescriptorListener>& listener)
{
    if ((fileDescriptor < 0) || ((events & FILE_DESCRIPTOR_EVENTS_MASK) == 0) || (!listener)) {
        HILOG_ERROR("AddFileDescriptorListener(%{public}d, %{public}u, %{public}s): Invalid parameter", fileDescriptor,
            events, listener ? "valid" : "null");
        return EVENT_HANDLER_ERR_INVALID_PARAM;
    }

    if (!eventRunner_) {
        HILOG_ERROR("AddFileDescriptorListener: MUST Set event runner before adding fd listener");
        return EVENT_HANDLER_ERR_NO_EVENT_RUNNER;
    }

    listener->SetOwner(shared_from_this());
    return eventRunner_->GetEventQueue()->AddFileDescriptorListener(fileDescriptor, events, listener);
}

void EventHandler::RemoveAllFileDescriptorListeners()
{
    if (!eventRunner_) {
        HILOG_ERROR("RemoveAllFileDescriptorListeners: MUST Set event runner before removing all fd listener");
        return;
    }

    eventRunner_->GetEventQueue()->RemoveFileDescriptorListener(shared_from_this());
}

void EventHandler::RemoveFileDescriptorListener(int32_t fileDescriptor)
{
    if (fileDescriptor < 0) {
        HILOG_ERROR("RemoveFileDescriptorListener(%{public}d): Invalid parameter", fileDescriptor);
        return;
    }

    if (!eventRunner_) {
        HILOG_ERROR("RemoveFileDescriptorListener: MUST Set event runner before removing fd listener by fd");
        return;
    }

    eventRunner_->GetEventQueue()->RemoveFileDescriptorListener(fileDescriptor);
}

void EventHandler::SetEventRunner(const std::shared_ptr<EventRunner>& runner)
{
    if (eventRunner_ == runner) {
        return;
    }

    if (eventRunner_) {
        HILOG_WARN("SetEventRunner: It is not recommended to change the event runner dynamically");

        // Remove all events and listeners from old event runner.
        RemoveAllEvents();
        RemoveAllFileDescriptorListeners();
    }

    // Switch event runner.
    eventRunner_ = runner;
    return;
}

void EventHandler::DeliveryTimeAction(const InnerEvent::Pointer& event, InnerEvent::TimePoint nowStart)
{
#ifdef HAS_HICHECKER_NATIVE_PART
    if (!HiChecker::NeedCheckSlowEvent()) {
        return;
    }
    int64_t deliveryTimeout = eventRunner_->GetDeliveryTimeout();
    if (deliveryTimeout > 0) {
        std::string threadName = eventRunner_->GetRunnerThreadName();
        std::string eventName = GetEventName(event);
        int64_t threadId = gettid();
        std::string threadIdCharacter = std::to_string(threadId);
        std::chrono::duration<double> deliveryTime = nowStart - event->GetSendTime();
        std::string deliveryTimeCharacter = std::to_string((deliveryTime).count());
        std::string deliveryTimeoutCharacter = std::to_string(deliveryTimeout);
        std::string handOutTag = "threadId: " + threadIdCharacter + "," + "threadName: " + threadName + "," +
                                 "eventName: " + eventName + "," + "deliveryTime: " + deliveryTimeCharacter + "," +
                                 "deliveryTimeout: " + deliveryTimeoutCharacter;
        if ((nowStart - std::chrono::milliseconds(deliveryTimeout)) > event->GetHandleTime()) {
            HiChecker::NotifySlowEvent(handOutTag);
            if (deliveryTimeoutCallback_) {
                deliveryTimeoutCallback_();
            }
        }
    }
#endif // HAS_HICHECKER_NATIVE_PART
}

void EventHandler::DistributeTimeAction(const InnerEvent::Pointer& event, InnerEvent::TimePoint nowStart)
{
#ifdef HAS_HICHECKER_NATIVE_PART
    if (!HiChecker::NeedCheckSlowEvent()) {
        return;
    }
    int64_t distributeTimeout = eventRunner_->GetDistributeTimeout();
    if (distributeTimeout > 0) {
        std::string threadName = eventRunner_->GetRunnerThreadName();
        std::string eventName = GetEventName(event);
        int64_t threadId = gettid();
        std::string threadIdCharacter = std::to_string(threadId);
        InnerEvent::TimePoint nowEnd = InnerEvent::Clock::now();
        std::chrono::duration<double> distributeTime = nowEnd - nowStart;
        std::string distributeTimeCharacter = std::to_string((distributeTime).count());
        std::string distributeTimeoutCharacter = std::to_string(distributeTimeout);
        std::string executeTag = "threadId: " + threadIdCharacter + "," + "threadName: " + threadName + "," +
                                 "eventName: " + eventName + "," + "distributeTime: " + distributeTimeCharacter + "," +
                                 "distributeTimeout: " + distributeTimeoutCharacter;
        if ((nowEnd - std::chrono::milliseconds(distributeTimeout)) > nowStart) {
            HiChecker::NotifySlowEvent(executeTag);
            if (distributeTimeoutCallback_) {
                distributeTimeoutCallback_();
            }
        }
    }
#endif // HAS_HICHECKER_NATIVE_PART
}

void EventHandler::DistributeTimeoutHandler(const InnerEvent::TimePoint& beginTime)
{
    int64_t distributeTimeout = EventRunner::GetMainEventRunner()->GetTimeout();
    if (distributeTimeout > 0) {
        InnerEvent::TimePoint endTime = InnerEvent::Clock::now();
        if ((endTime - std::chrono::milliseconds(distributeTimeout)) > beginTime &&
            EventRunner::distributeCallback_) {
            auto diff = endTime - beginTime;
            int64_t durationTime = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count();
            EventRunner::distributeCallback_(durationTime);
        }
    }
}

void EventHandler::DistributeEvent(const InnerEvent::Pointer& event)
{
    if (!event) {
        HILOG_ERROR("DistributeEvent: Could not distribute an invalid event");
        return;
    }

    currentEventHandler = shared_from_this();

    InnerEvent::TimePoint nowStart = InnerEvent::Clock::now();
    DeliveryTimeAction(event, nowStart);
    InnerEvent::TimePoint beginTime;
    bool isAppMainThread = EventRunner::IsAppMainThread();
    if (isAppMainThread) {
        beginTime = InnerEvent::Clock::now();
    }

    if (event->HasTask()) {
        // Call task callback directly if contains a task.
        (event->GetTaskCallback())();
    } else {
        // Otherwise let developers to handle it.
        ProcessEvent(event);
    }

    if (isAppMainThread) {
        DistributeTimeoutHandler(beginTime);
    }
}

void EventHandler::Dump(Dumper& dumper)
{
    auto now = std::chrono::system_clock::now();
    auto tp = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto us = tp.time_since_epoch().count() % 1000;
    struct tm curTime = { 0 };
    localtime_r(&tt, &curTime);
    char sysTime[DATETIME_STRING_LENGTH];
    std::strftime(sysTime, sizeof(char) * DATETIME_STRING_LENGTH, "%Y%m%d %I:%M:%S.", &curTime);
    auto message = dumper.GetTag() + " EventHandler dump begain curTime:" + std::string(sysTime) + std::to_string(us) +
                   LINE_SEPARATOR;
    dumper.Dump(message);

    if (eventRunner_ == nullptr) {
        dumper.Dump(dumper.GetTag() + " event runner uninitialized!" + LINE_SEPARATOR);
    } else {
        eventRunner_->Dump(dumper);
    }
}

bool EventHandler::HasInnerEvent(uint32_t innerEventId)
{
    if (!eventRunner_) {
        HILOG_ERROR("event runner uninitialized!");
        return false;
    }
    return eventRunner_->GetEventQueue()->HasInnerEvent(shared_from_this(), innerEventId);
}

bool EventHandler::HasInnerEvent(int64_t param)
{
    if (!eventRunner_) {
        HILOG_ERROR("event runner uninitialized!");
        return false;
    }
    return eventRunner_->GetEventQueue()->HasInnerEvent(shared_from_this(), param);
}

std::string EventHandler::GetEventName(const InnerEvent::Pointer& event)
{
    std::string eventName;
    if (!event) {
        return eventName;
    }

    if (event->HasTask()) {
        eventName = event->GetTaskName();
    } else {
        InnerEvent::EventId eventId = event->GetInnerEventIdEx();
        if (eventId.index() == TYPE_U32_INDEX) {
            eventName = std::to_string(std::get<uint32_t>(eventId));
        } else {
            eventName = std::get<std::string>(eventId);
        }

    }
    return eventName;
}

bool EventHandler::IsIdle()
{
    return eventRunner_->GetEventQueue()->IsIdle();
}

void EventHandler::ProcessEvent(const InnerEvent::Pointer&) {}
} // namespace AppExecFwk
} // namespace OHOS
