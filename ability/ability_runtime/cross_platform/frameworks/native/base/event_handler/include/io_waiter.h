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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IO_WAITER_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IO_WAITER_H

#include <cerrno>
#include <cstdint>
#include <functional>
#include <mutex>

namespace OHOS {
namespace AppExecFwk {
// Interface of IO waiter
class IoWaiter {
public:
    using FileDescriptorEventCallback = std::function<void(int32_t, uint32_t)>;

    IoWaiter() = default;
    virtual ~IoWaiter() = default;

    /**
     * Wait until IO event coming or timed out.
     *
     * @param lock An unique lock which must be locked by the current thread.
     * @param nanoseconds Nanoseconds for time out, negative value indicate waiting forever.
     * @return True if succeeded.
     */
    virtual bool WaitFor(std::unique_lock<std::mutex>& lock, int64_t nanoseconds) = 0;

    /**
     * Unblocks one of the waiting threads.
     */
    virtual void NotifyOne() = 0;

    /**
     * Unblocks all of the waiting threads.
     */
    virtual void NotifyAll() = 0;

    /**
     * Check whether this waiter support listening file descriptor.
     *
     * @return True if supported.
     */
    virtual bool SupportListeningFileDescriptor() const = 0;

    /**
     * Add file descriptor.
     *
     * @param fileDescriptor File descriptor which need to listen.
     * @param events Events from file descriptor, such as input, output.
     * @return True if succeeded.
     */
    virtual bool AddFileDescriptor(int32_t fileDescriptor, uint32_t events) = 0;

    /**
     * Remove file descriptor.
     *
     * @param fileDescriptor File descriptor which need to remove.
     */
    virtual void RemoveFileDescriptor(int32_t fileDescriptor) = 0;

    /**
     * Set callback to handle events from file descriptors, such as readable, writable and so on.
     *
     * @param callback Callback function to handle events from file descriptors.
     */
    virtual void SetFileDescriptorEventCallback(const FileDescriptorEventCallback& callback) = 0;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_IO_WAITER_H
