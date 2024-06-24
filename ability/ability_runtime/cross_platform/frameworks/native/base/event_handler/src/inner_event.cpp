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

#include "inner_event.h"

#include <condition_variable>
#include <mutex>
#include <vector>

#include "event_handler_utils.h"
#include "hilog.h"
#include "singleton.h"

namespace OHOS {
namespace AppExecFwk {
constexpr static uint32_t DEFAULT_INDEX = 0u;
namespace {
class WaiterImp final : public InnerEvent::Waiter {
public:
    WaiterImp() {};
    ~WaiterImp() override {};

    void Wait() final
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (!finished_) {
            ++waitingCount_;
            condition_.wait(lock);
            --waitingCount_;
        }
    }

    void Notify() final
    {
        std::lock_guard<std::mutex> lock(mutex_);
        finished_ = true;
        if (waitingCount_ > 0) {
            condition_.notify_all();
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable condition_;
    uint32_t waitingCount_ { 0 };
    bool finished_ { false };
};
} // unnamed namespace

// Implementation for event pool.
class InnerEventPool : public DelayedRefSingleton<InnerEventPool> {
    DECLARE_DELAYED_REF_SINGLETON(InnerEventPool);

public:
    InnerEvent::Pointer Get()
    {
        size_t newPeakUsingCount = 0;

        {
            // Check whether pool is empty.
            std::lock_guard<std::mutex> lock(poolLock_);
            ++usingCount_;
            if (!events_.empty()) {
                auto event = std::move(events_.back());
                events_.pop_back();
                return InnerEvent::Pointer(event.release(), Drop);
            }

            // Update peak using events count.
            if (usingCount_ >= nextPeakUsingCount_) {
                if (UINT32_MAX - nextPeakUsingCount_ > MAX_BUFFER_POOL_SIZE) {
                    nextPeakUsingCount_ += MAX_BUFFER_POOL_SIZE;
                } else {
                    nextPeakUsingCount_ = UINT32_MAX;
                }

                newPeakUsingCount = usingCount_;
            }
        }

        // Print the new peak using count of inner events
        if (newPeakUsingCount > 0) {
            HILOG_INFO("Peak using count of inner events is up to %{public}zu", newPeakUsingCount);
        }

        // Allocate new memory, while pool is empty.
        return InnerEvent::Pointer(new InnerEvent, Drop);
    }

private:
    static void Drop(InnerEvent* event)
    {
        if (event == nullptr) {
            return;
        }

        auto destructor = [](InnerEvent* event) {
            if (event != nullptr) {
                delete event;
            }
        };

        // Clear content of the event
        event->ClearEvent();
        // Put event into event buffer pool
        GetInstance().Put(InnerEvent::Pointer(event, destructor));
    }

    void Put(InnerEvent::Pointer&& event)
    {
        // Check whether pool is full.
        std::lock_guard<std::mutex> lock(poolLock_);
        --usingCount_;
        if (events_.size() < MAX_BUFFER_POOL_SIZE) {
            events_.push_back(std::move(event));
        }
    }

    static const size_t MAX_BUFFER_POOL_SIZE = 64;

    std::mutex poolLock_;
    std::vector<InnerEvent::Pointer> events_;

    // Used to statistical peak value of count of using inner events.
    size_t usingCount_ { 0 };
    size_t nextPeakUsingCount_ { MAX_BUFFER_POOL_SIZE };
};

InnerEventPool::InnerEventPool() : poolLock_(), events_()
{
    // Reserve enough memory
    std::lock_guard<std::mutex> lock(poolLock_);
    events_.reserve(MAX_BUFFER_POOL_SIZE);
}

InnerEventPool::~InnerEventPool()
{
    // Release all memory in the poll
    std::lock_guard<std::mutex> lock(poolLock_);
    events_.clear();
}

InnerEvent::Pointer InnerEvent::Get()
{
    auto event = InnerEventPool::GetInstance().Get();
    return event;
}

InnerEvent::Pointer InnerEvent::Get(uint32_t innerEventId, int64_t param)
{
    auto event = InnerEventPool::GetInstance().Get();
    if (event != nullptr) {
        event->innerEventId_ = innerEventId;
        event->param_ = param;
    }
    return event;
}

InnerEvent::Pointer InnerEvent::Get(const EventId& innerEventId, int64_t param)
{
    auto event = InnerEventPool::GetInstance().Get();
    if (event != nullptr) {
        event->innerEventId_ = innerEventId;
        event->param_ = param;
        if (innerEventId.index() == TYPE_U32_INDEX) {
            HILOG_INFO("InnerEventId is %{public}u",
                std::get<uint32_t>(innerEventId));
        } else {
            HILOG_INFO("InnerEventId is %{public}s",
                std::get<std::string>(innerEventId).c_str());
        }
    }
    return event;
}

InnerEvent::Pointer InnerEvent::Get(const Callback& callback, const std::string& name)
{
    // Returns nullptr while callback is invalid.
    if (!callback) {
        HILOG_WARN("Failed to create inner event with an invalid callback");
        return InnerEvent::Pointer(nullptr, nullptr);
    }

    auto event = InnerEventPool::GetInstance().Get();
    if (event != nullptr) {
        event->taskCallback_ = callback;
        event->taskName_ = name;
    }
    return event;
}

void InnerEvent::ClearEvent()
{
    // Wake up all waiting threads.
    if (waiter_) {
        waiter_->Notify();
        waiter_.reset();
    }

    if (HasTask()) {
        // Clear members for task
        taskCallback_ = nullptr;
        taskName_.clear();
    } else {
        // Clear members for event
        if (smartPtrDtor_) {
            smartPtrDtor_(smartPtr_);
            smartPtrDtor_ = nullptr;
            smartPtr_ = nullptr;
            smartPtrTypeId_ = 0;
        }
    }

    if (hiTraceId_) {
        hiTraceId_.reset();
    }

    // Clear owner
    owner_.reset();
}

void InnerEvent::WarnSmartPtrCastMismatch()
{
    HILOG_ERROR("Type of the shared_ptr, weak_ptr or unique_ptr mismatched");
}

const std::shared_ptr<InnerEvent::Waiter>& InnerEvent::CreateWaiter()
{
    waiter_ = std::make_shared<WaiterImp>();
    return waiter_;
}

bool InnerEvent::HasWaiter() const
{
    return (waiter_ != nullptr);
}

const std::shared_ptr<HiTraceId> InnerEvent::GetTraceId()
{
    return hiTraceId_;
}

uint32_t InnerEvent::GetInnerEventId() const
{
    if (innerEventId_.index() != TYPE_U32_INDEX) {
        return DEFAULT_INDEX;
    }
    return std::get<uint32_t>(innerEventId_);
}

std::string InnerEvent::Dump()
{
    std::string content;

    content.append("Event { ");
    if (!owner_.expired()) {
        if (HasTask()) {
            content.append("task name = " + taskName_);
        } else {
            if (innerEventId_.index() == TYPE_U32_INDEX) {
                content.append(", id = " + std::to_string(std::get<uint32_t>(innerEventId_)));
            } else {
                content.append(", id = " + std::get<std::string>(innerEventId_));
            }
        }
        if (param_ != 0) {
            content.append(", param = " + std::to_string(param_));
        }
    } else {
        content = "No handler";
    }
    content.append(" }" + LINE_SEPARATOR);

    return content;
}
} // namespace AppExecFwk
} // namespace OHOS
