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

#include "event_queue.h"

#include <algorithm>

#include "epoll_io_waiter.h"
#include "event_handler.h"
#include "event_handler_utils.h"
#include "hilog.h"
#include "none_io_waiter.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
// Help to insert events into the event queue sorted by handle time.
inline void InsertEventsLocked(std::list<InnerEvent::Pointer>& events, InnerEvent::Pointer& event)
{
    auto f = [](const InnerEvent::Pointer& first, const InnerEvent::Pointer& second) {
        if (!first || !second) {
            return false;
        }
        return first->GetHandleTime() < second->GetHandleTime();
    };
    auto it = std::upper_bound(events.begin(), events.end(), event, f);
    events.insert(it, std::move(event));
}

// Help to remove file descriptor listeners.
template<typename T>
void RemoveFileDescriptorListenerLocked(std::map<int32_t, std::shared_ptr<FileDescriptorListener>>& listeners,
    const std::shared_ptr<IoWaiter>& ioWaiter, const T& filter)
{
    if (!ioWaiter) {
        return;
    }
    for (auto it = listeners.begin(); it != listeners.end();) {
        if (filter(it->second)) {
            ioWaiter->RemoveFileDescriptor(it->first);
            it = listeners.erase(it);
        } else {
            ++it;
        }
    }
}

// Help to check whether there is a valid event in list and update wake up time.
inline bool CheckEventInListLocked(const std::list<InnerEvent::Pointer>& events, const InnerEvent::TimePoint& now,
    InnerEvent::TimePoint& nextWakeUpTime)
{
    if (!events.empty()) {
        const auto& handleTime = events.front()->GetHandleTime();
        if (handleTime < nextWakeUpTime) {
            nextWakeUpTime = handleTime;
            return handleTime <= now;
        }
    }

    return false;
}

inline InnerEvent::Pointer PopFrontEventFromListLocked(std::list<InnerEvent::Pointer>& events)
{
    InnerEvent::Pointer event = std::move(events.front());
    events.pop_front();
    return event;
}
} // unnamed namespace

EventQueue::EventQueue() : ioWaiter_(std::make_shared<NoneIoWaiter>()) {}

EventQueue::EventQueue(const std::shared_ptr<IoWaiter>& ioWaiter)
    : ioWaiter_(ioWaiter ? ioWaiter : std::make_shared<NoneIoWaiter>())
{
    if (ioWaiter_->SupportListeningFileDescriptor()) {
        // Set callback to handle events from file descriptors.
        ioWaiter_->SetFileDescriptorEventCallback(
            std::bind(&EventQueue::HandleFileDescriptorEvent, this, std::placeholders::_1, std::placeholders::_2));
    }
}

EventQueue::~EventQueue()
{
    std::lock_guard<std::mutex> lock(queueLock_);
    usable_.store(false);
}

void EventQueue::Insert(InnerEvent::Pointer& event, Priority priority)
{
    if (!event) {
        HILOG_ERROR("Insert: Could not insert an invalid event");
        return;
    }

    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    bool needNotify = false;
    switch (priority) {
        case Priority::VIP:
        case Priority::IMMEDIATE:
        case Priority::HIGH:
        case Priority::LOW: {
            needNotify = (event->GetHandleTime() < wakeUpTime_);
            InsertEventsLocked(subEventQueues_[static_cast<uint32_t>(priority)].queue, event);
            break;
        }
        case Priority::IDLE: {
            // Never wake up thread if insert an idle event.
            InsertEventsLocked(idleEvents_, event);
            break;
        }
        default:
            break;
    }

    if (needNotify) {
        ioWaiter_->NotifyOne();
    }
}

void EventQueue::RemoveOrphan()
{
    // Remove all events which lost its owner.
    auto filter = [](const InnerEvent::Pointer& p) { return !p->GetOwner(); };

    Remove(filter);

    // Remove all listeners which lost its owner.
    auto listenerFilter = [](const std::shared_ptr<FileDescriptorListener>& listener) {
        if (!listener) {
            return true;
        }
        return !listener->GetOwner();
    };

    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    RemoveFileDescriptorListenerLocked(listeners_, ioWaiter_, listenerFilter);
}

void EventQueue::Remove(const std::shared_ptr<EventHandler>& owner)
{
    if (!owner) {
        HILOG_ERROR("Remove: Invalid owner");
        return;
    }

    auto filter = [&owner](const InnerEvent::Pointer& p) { return (p->GetOwner() == owner); };

    Remove(filter);
}

void EventQueue::Remove(const std::shared_ptr<EventHandler>& owner, uint32_t innerEventId)
{
    if (!owner) {
        HILOG_ERROR("Remove: Invalid owner");
        return;
    }

    auto filter = [&owner, innerEventId](const InnerEvent::Pointer& p) {
        return (!p->HasTask()) && (p->GetOwner() == owner) && (p->GetInnerEventId() == innerEventId);
    };

    Remove(filter);
}

void EventQueue::Remove(const std::shared_ptr<EventHandler>& owner, uint32_t innerEventId, int64_t param)
{
    if (!owner) {
        HILOG_ERROR("Remove: Invalid owner");
        return;
    }

    auto filter = [&owner, innerEventId, param](const InnerEvent::Pointer& p) {
        return (!p->HasTask()) && (p->GetOwner() == owner) && (p->GetInnerEventId() == innerEventId) &&
               (p->GetParam() == param);
    };

    Remove(filter);
}

void EventQueue::Remove(const std::shared_ptr<EventHandler>& owner, const std::string& name)
{
    if ((!owner) || (name.empty())) {
        HILOG_ERROR("Remove: Invalid owner or task name");
        return;
    }

    auto filter = [&owner, &name](const InnerEvent::Pointer& p) {
        return (p->HasTask()) && (p->GetOwner() == owner) && (p->GetTaskName() == name);
    };

    Remove(filter);
}

void EventQueue::Remove(const RemoveFilter& filter)
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        subEventQueues_[i].queue.remove_if(filter);
    }
    idleEvents_.remove_if(filter);
}

bool EventQueue::HasInnerEvent(const std::shared_ptr<EventHandler>& owner, uint32_t innerEventId)
{
    if (!owner) {
        HILOG_ERROR("HasInnerEvent: Invalid owner");
        return false;
    }
    auto filter = [&owner, innerEventId](const InnerEvent::Pointer& p) {
        return (!p->HasTask()) && (p->GetOwner() == owner) && (p->GetInnerEventId() == innerEventId);
    };
    return HasInnerEvent(filter);
}

bool EventQueue::HasInnerEvent(const std::shared_ptr<EventHandler>& owner, int64_t param)
{
    if (!owner) {
        HILOG_ERROR("HasInnerEvent: Invalid owner");
        return false;
    }
    auto filter = [&owner, param](const InnerEvent::Pointer& p) {
        return (!p->HasTask()) && (p->GetOwner() == owner) && (p->GetParam() == param);
    };
    return HasInnerEvent(filter);
}

bool EventQueue::HasInnerEvent(const HasFilter& filter)
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return false;
    }
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        std::list<InnerEvent::Pointer>::iterator iter =
            std::find_if(subEventQueues_[i].queue.begin(), subEventQueues_[i].queue.end(), filter);
        if (iter != subEventQueues_[i].queue.end()) {
            return true;
        }
    }
    std::list<InnerEvent::Pointer>::iterator iter = std::find_if(idleEvents_.begin(), idleEvents_.end(), filter);
    return iter != idleEvents_.end();
}

InnerEvent::Pointer EventQueue::PickEventLocked(const InnerEvent::TimePoint& now, InnerEvent::TimePoint& nextWakeUpTime)
{
    uint32_t priorityIndex = SUB_EVENT_QUEUE_NUM;
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        // Check whether any event need to be distributed.
        if (!CheckEventInListLocked(subEventQueues_[i].queue, now, nextWakeUpTime)) {
            continue;
        }

        // Check whether any event in higher priority need to be distributed.
        if (priorityIndex < SUB_EVENT_QUEUE_NUM) {
            SubEventQueue& subQueue = subEventQueues_[priorityIndex];
            // Check whether enough events in higher priority queue are handled continuously.
            if (subQueue.handledEventsCount < subQueue.maxHandledEventsCount) {
                subQueue.handledEventsCount++;
                break;
            }
        }

        // Try to pick event from this queue.
        priorityIndex = i;
    }

    if (priorityIndex >= SUB_EVENT_QUEUE_NUM) {
        // If not found any event to distribute, return nullptr.
        return InnerEvent::Pointer(nullptr, nullptr);
    }

    // Reset handled event count for sub event queues in higher priority.
    for (uint32_t i = 0; i < priorityIndex; ++i) {
        subEventQueues_[i].handledEventsCount = 0;
    }

    return PopFrontEventFromListLocked(subEventQueues_[priorityIndex].queue);
}

InnerEvent::Pointer EventQueue::GetExpiredEventLocked(InnerEvent::TimePoint& nextExpiredTime)
{
    auto now = InnerEvent::Clock::now();
    wakeUpTime_ = InnerEvent::TimePoint::max();
    // Find an event which could be distributed right now.
    InnerEvent::Pointer event = PickEventLocked(now, wakeUpTime_);
    if (event) {
        // Exit idle mode, if found an event to distribute.
        isIdle_ = false;
        return event;
    }

    // If found nothing, enter idle mode and make a time stamp.
    if (!isIdle_) {
        idleTimeStamp_ = now;
        isIdle_ = true;
    }

    if (!idleEvents_.empty()) {
        const auto& idleEvent = idleEvents_.front();

        // Return the idle event that has been sent before time stamp and reaches its handle time.
        if ((idleEvent->GetSendTime() <= idleTimeStamp_) && (idleEvent->GetHandleTime() <= now)) {
            return PopFrontEventFromListLocked(idleEvents_);
        }
    }

    // Update wake up time.
    nextExpiredTime = wakeUpTime_;
    return InnerEvent::Pointer(nullptr, nullptr);
}

InnerEvent::Pointer EventQueue::GetEvent()
{
    std::unique_lock<std::mutex> lock(queueLock_);
    while (!finished_) {
        InnerEvent::TimePoint nextWakeUpTime = InnerEvent::TimePoint::max();
        InnerEvent::Pointer event = GetExpiredEventLocked(nextWakeUpTime);
        if (event) {
            return event;
        }
        WaitUntilLocked(nextWakeUpTime, lock);
    }

    HILOG_INFO("GetEvent: Break out");
    return InnerEvent::Pointer(nullptr, nullptr);
}

InnerEvent::Pointer EventQueue::GetExpiredEvent(InnerEvent::TimePoint& nextExpiredTime)
{
    std::unique_lock<std::mutex> lock(queueLock_);
    return GetExpiredEventLocked(nextExpiredTime);
}

ErrCode EventQueue::AddFileDescriptorListener(
    int32_t fileDescriptor, uint32_t events, const std::shared_ptr<FileDescriptorListener>& listener)
{
    if ((fileDescriptor < 0) || ((events & FILE_DESCRIPTOR_EVENTS_MASK) == 0) || (!listener)) {
        HILOG_ERROR("AddFileDescriptorListener(%{public}d, %{public}u, %{public}s): Invalid parameter", fileDescriptor,
            events, listener ? "valid" : "null");
        return EVENT_HANDLER_ERR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return EVENT_HANDLER_ERR_NO_EVENT_RUNNER;
    }
    auto it = listeners_.find(fileDescriptor);
    if (it != listeners_.end()) {
        HILOG_ERROR("AddFileDescriptorListener: File descriptor %{public}d is already in listening", fileDescriptor);
        return EVENT_HANDLER_ERR_FD_ALREADY;
    }

    if (!EnsureIoWaiterSupportListerningFileDescriptorLocked()) {
        return EVENT_HANDLER_ERR_FD_NOT_SUPPORT;
    }

    if (!ioWaiter_->AddFileDescriptor(fileDescriptor, events)) {
        HILOG_ERROR("AddFileDescriptorListener: Failed to add file descriptor into IO waiter");
        return EVENT_HANDLER_ERR_FD_FAILED;
    }

    listeners_.emplace(fileDescriptor, listener);
    return ERR_OK;
}

void EventQueue::RemoveFileDescriptorListener(const std::shared_ptr<EventHandler>& owner)
{
    if (!owner) {
        HILOG_ERROR("RemoveFileDescriptorListener: Invalid owner");
        return;
    }

    auto listenerFilter = [&owner](const std::shared_ptr<FileDescriptorListener>& listener) {
        if (!listener) {
            return false;
        }
        return listener->GetOwner() == owner;
    };

    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    RemoveFileDescriptorListenerLocked(listeners_, ioWaiter_, listenerFilter);
}

void EventQueue::RemoveFileDescriptorListener(int32_t fileDescriptor)
{
    if (fileDescriptor < 0) {
        HILOG_ERROR("RemoveFileDescriptorListener(%{public}d): Invalid file descriptor", fileDescriptor);
        return;
    }

    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    if (listeners_.erase(fileDescriptor) > 0) {
        ioWaiter_->RemoveFileDescriptor(fileDescriptor);
    }
}

void EventQueue::Prepare()
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    finished_ = false;
}

void EventQueue::Finish()
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    finished_ = true;
    ioWaiter_->NotifyAll();
}

void EventQueue::WaitUntilLocked(const InnerEvent::TimePoint& when, std::unique_lock<std::mutex>& lock)
{
    // Get a temp reference of IO waiter, otherwise it maybe released while waiting.
    auto ioWaiterHolder = ioWaiter_;
    if (!ioWaiterHolder->WaitFor(lock, TimePointToTimeOut(when))) {
        HILOG_ERROR("WaitUntilLocked: Failed to call wait, reset IO waiter");
        ioWaiter_ = std::make_shared<NoneIoWaiter>();
        listeners_.clear();
    }
}

void EventQueue::HandleFileDescriptorEvent(int32_t fileDescriptor, uint32_t events)
{
    std::shared_ptr<FileDescriptorListener> listener;

    {
        std::lock_guard<std::mutex> lock(queueLock_);
        if (!usable_.load()) {
            HILOG_ERROR("!usable_.load");
            return;
        }
        auto it = listeners_.find(fileDescriptor);
        if (it == listeners_.end()) {
            HILOG_ERROR("HandleFileDescriptorEvent: Can not found listener, maybe it is removed");
            return;
        }

        // Hold instance of listener.
        listener = it->second;
        if (!listener) {
            HILOG_ERROR("listener is nullptr");
            return;
        }
    }

    auto handler = listener->GetOwner();
    if (!handler) {
        HILOG_ERROR("HandleFileDescriptorEvent: Owner of listener is released");
        return;
    }

    std::weak_ptr<FileDescriptorListener> wp = listener;
    auto f = [fileDescriptor, events, wp]() {
        auto listener = wp.lock();
        if (!listener) {
            HILOG_ERROR("HandleFileDescriptorEvent-Lambda: Listener is released");
            return;
        }

        if ((events & FILE_DESCRIPTOR_INPUT_EVENT) != 0) {
            listener->OnReadable(fileDescriptor);
        }

        if ((events & FILE_DESCRIPTOR_OUTPUT_EVENT) != 0) {
            listener->OnWritable(fileDescriptor);
        }

        if ((events & FILE_DESCRIPTOR_SHUTDOWN_EVENT) != 0) {
            listener->OnShutdown(fileDescriptor);
        }

        if ((events & FILE_DESCRIPTOR_EXCEPTION_EVENT) != 0) {
            listener->OnException(fileDescriptor);
        }
    };

    // Post a high priority task to handle file descriptor events.
    handler->PostHighPriorityTask(f);
}

bool EventQueue::EnsureIoWaiterSupportListerningFileDescriptorLocked()
{
#ifndef IOS_PLATFORM
    if (ioWaiter_->SupportListeningFileDescriptor()) {
        return true;
    }

    auto newIoWaiter = std::make_shared<EpollIoWaiter>();
    if (!newIoWaiter->Init()) {
        HILOG_ERROR("EnsureIoWaiterSupportListerningFileDescriptorLocked: Failed to initialize epoll");
        return false;
    }

    // Set callback to handle events from file descriptors.
    newIoWaiter->SetFileDescriptorEventCallback(
        std::bind(&EventQueue::HandleFileDescriptorEvent, this, std::placeholders::_1, std::placeholders::_2));

    ioWaiter_->NotifyAll();
    ioWaiter_ = newIoWaiter;
#endif
    return true;
}

void EventQueue::Dump(Dumper& dumper)
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    std::string priority[] = { "Immediate", "High", "Low" };
    uint32_t total = 0;
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        uint32_t n = 0;
        dumper.Dump(dumper.GetTag() + " " + priority[i] + " priority event queue information:" + LINE_SEPARATOR);
        for (auto it = subEventQueues_[i].queue.begin(); it != subEventQueues_[i].queue.end(); ++it) {
            ++n;
            dumper.Dump(dumper.GetTag() + " No." + std::to_string(n) + " : " + (*it)->Dump());
            ++total;
        }
        dumper.Dump(
            dumper.GetTag() + " Total size of " + priority[i] + " events : " + std::to_string(n) + LINE_SEPARATOR);
    }

    dumper.Dump(dumper.GetTag() + " Idle priority event queue information:" + LINE_SEPARATOR);
    int n = 0;
    for (auto it = idleEvents_.begin(); it != idleEvents_.end(); ++it) {
        ++n;
        dumper.Dump(dumper.GetTag() + " No." + std::to_string(n) + " : " + (*it)->Dump());
        ++total;
    }
    dumper.Dump(dumper.GetTag() + " Total size of Idle events : " + std::to_string(n) + LINE_SEPARATOR);

    dumper.Dump(dumper.GetTag() + " Total event size : " + std::to_string(total) + LINE_SEPARATOR);
}

void EventQueue::DumpQueueInfo(std::string& queueInfo)
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return;
    }
    std::string priority[] = { "Immediate", "High", "Low" };
    uint32_t total = 0;
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        uint32_t n = 0;
        queueInfo += "            " + priority[i] + " priority event queue:" + LINE_SEPARATOR;
        for (auto it = subEventQueues_[i].queue.begin(); it != subEventQueues_[i].queue.end(); ++it) {
            ++n;
            queueInfo += "            No." + std::to_string(n) + " : " + (*it)->Dump();
            ++total;
        }
        queueInfo += "              Total size of " + priority[i] + " events : " + std::to_string(n) + LINE_SEPARATOR;
    }

    queueInfo += "            Idle priority event queue:" + LINE_SEPARATOR;

    int n = 0;
    for (auto it = idleEvents_.begin(); it != idleEvents_.end(); ++it) {
        ++n;
        queueInfo += "            No." + std::to_string(n) + " : " + (*it)->Dump();
        ++total;
    }
    queueInfo += "              Total size of Idle events : " + std::to_string(n) + LINE_SEPARATOR;

    queueInfo += "            Total event size : " + std::to_string(total);
}

bool EventQueue::IsIdle()
{
    return isIdle_;
}

bool EventQueue::IsQueueEmpty()
{
    std::lock_guard<std::mutex> lock(queueLock_);
    if (!usable_.load()) {
        return false;
    }
    for (uint32_t i = 0; i < SUB_EVENT_QUEUE_NUM; ++i) {
        uint32_t queueSize = subEventQueues_[i].queue.size();
        if (queueSize != 0) {
            return false;
        }
    }

    return idleEvents_.size() == 0;
}
} // namespace AppExecFwk
} // namespace OHOS
