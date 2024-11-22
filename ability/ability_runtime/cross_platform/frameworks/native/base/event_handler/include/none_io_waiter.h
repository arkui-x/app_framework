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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_NONE_IO_WAITER_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_NONE_IO_WAITER_H

#include <condition_variable>
#include <mutex>

#include "io_waiter.h"

namespace OHOS {
namespace AppExecFwk {
// Io waiter which does not support listening file descriptor.
class NoneIoWaiter final : public IoWaiter {
public:
    NoneIoWaiter() = default;
    ~NoneIoWaiter() final;

    bool WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds) final;

    void NotifyOne() final;
    void NotifyAll() final;

    bool SupportListeningFileDescriptor() const final;

    bool AddFileDescriptor(int32_t fileDescriptor, uint32_t events) final;
    void RemoveFileDescriptor(int32_t fileDescriptor) final;

    void SetFileDescriptorEventCallback(const FileDescriptorEventCallback& callback) final;

private:
    std::condition_variable condition_;
    uint32_t waitingCount_ { 0 };
    bool pred_ { false };
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_NONE_IO_WAITER_H
