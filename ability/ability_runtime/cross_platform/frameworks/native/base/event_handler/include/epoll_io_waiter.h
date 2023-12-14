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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EPOLL_IO_WAITER_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EPOLL_IO_WAITER_H

#include <atomic>
#include <mutex>

#include "io_waiter.h"

namespace OHOS {
namespace AppExecFwk {
// Use epoll to listen file descriptor.
class EpollIoWaiter final : public IoWaiter {
public:
    EpollIoWaiter() = default;
    ~EpollIoWaiter() final;

    /**
     * Initialize epoll.
     *
     * @return True if succeeded.
     */
    bool Init();

    bool WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds) final;

    void NotifyOne() final;
    void NotifyAll() final;

    bool SupportListeningFileDescriptor() const final;

    bool AddFileDescriptor(int32_t fileDescriptor, uint32_t events) final;
    void RemoveFileDescriptor(int32_t fileDescriptor) final;

    void SetFileDescriptorEventCallback(const FileDescriptorEventCallback& callback) final;

private:
    void DrainAwakenPipe() const;

    // File descriptor for epoll.
    int32_t epollFd_ { -1 };
    // File descriptor used to wake up epoll.
    int32_t awakenFd_ { -1 };

    FileDescriptorEventCallback callback_;
    std::atomic<uint32_t> waitingCount_ { 0 };
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EPOLL_IO_WAITER_H
