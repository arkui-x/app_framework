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

#include "none_io_waiter.h"

#include <chrono>

#include "event_handler_utils.h"
#include "hilog.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t HOURS_PER_DAY = 24;
const int32_t DAYS_PER_YEAR = 365;
const int32_t HOURS_PER_YEAR = HOURS_PER_DAY * DAYS_PER_YEAR;
} // unnamed namespace

// Nothing to do, but used to fix a codex warning.
NoneIoWaiter::~NoneIoWaiter() {}

bool NoneIoWaiter::WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds)
{
    ++waitingCount_;
    if (nanoseconds < 0) {
        condition_.wait(lock, [this] { return this->pred_; });
    } else {
        /*
         * Fix a problem in some versions of STL.
         * Parameter 'nanoseconds' is too large to cause overflow by adding 'now'.
         * So limit it to no more than one year.
         */
        static const auto oneYear = std::chrono::hours(HOURS_PER_YEAR);
        auto duration = std::chrono::nanoseconds(nanoseconds);
        (void)condition_.wait_for(lock, (duration > oneYear) ? oneYear : duration, [this] { return this->pred_; });
    }
    --waitingCount_;
    pred_ = false;
    return true;
}

void NoneIoWaiter::NotifyOne()
{
    if (waitingCount_ > 0) {
        pred_ = true;
        condition_.notify_one();
    }
}

void NoneIoWaiter::NotifyAll()
{
    if (waitingCount_ > 0) {
        pred_ = true;
        condition_.notify_all();
    }
}

bool NoneIoWaiter::SupportListeningFileDescriptor() const
{
    return false;
}

bool NoneIoWaiter::AddFileDescriptor(int32_t, uint32_t)
{
    HILOG_WARN("AddFileDescriptor: Function is not supported !!!");
    return false;
}

void NoneIoWaiter::RemoveFileDescriptor(int32_t)
{
    HILOG_WARN("RemoveFileDescriptor: Function is not supported !!!");
}

void NoneIoWaiter::SetFileDescriptorEventCallback(const IoWaiter::FileDescriptorEventCallback&)
{
    HILOG_WARN("SetFileDescriptorEventCallback: Function is not supported !!!");
}
} // namespace AppExecFwk
} // namespace OHOS
