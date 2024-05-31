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

#ifndef AAFWK_APPEXECFWK_NATIVE_IMPLEMENT_EVENTHANDLER_H
#define AAFWK_APPEXECFWK_NATIVE_IMPLEMENT_EVENTHANDLER_H

#include "event_runner.h"

using OHOS::ErrCode;
using OHOS::AppExecFwk::EventHandler;
using OHOS::AppExecFwk::EventRunner;

typedef void (*FileFDCallback)(int32_t filedescriptor);

struct FileDescriptorCallbacks;
struct EventRunnerNativeImplement {
public:
    explicit EventRunnerNativeImplement(bool current);
    ~EventRunnerNativeImplement();

    /**
     * Get current thread 'EventRunnerNativeImplement'.
     *
     * @return Returns pointer of the new 'EventRunnerNativeImplement'.
     */
    static const EventRunnerNativeImplement* GetEventRunnerNativeObj();

    /**
     * Create new 'EventRunnerNativeImplement'.
     *
     * @return Returns pointer of the new 'EventRunnerNativeImplement'.
     */
    static const EventRunnerNativeImplement* CreateEventRunnerNativeObj();

    /**
     * Start to run the 'EventRunnerNativeImplement'. Only running on single thread.
     *
     * @return Returns 'ERR_OK' on success.
     */
    ErrCode RunEventRunnerNativeObj() const;

    /**
     * Stop to run the 'EventRunnerNativeImplement'.
     *
     * @return Returns 'ERR_OK' on success.
     */
    ErrCode StopEventRunnerNativeObj() const;

    /**
     * Add file descriptor listener for a file descriptor.
     *
     * @param fileDescriptor File descriptor.
     * @param events Events from file descriptor, such as input, output, error
     * @param onReadableCallback Called while file descriptor is readable.
     * @param onWritableCallback Called while file descriptor is writable.
     * @param onShutdownCallback Called while shutting down this file descriptor.
     * @param onExceptionCallback Called while error happened on this file descriptor.
     * @return Return 'ERR_OK' on success.
     */
    ErrCode AddFileDescriptorListener(
        int32_t fileDescriptor, uint32_t events, const FileDescriptorCallbacks* fdCallbacks) const;

    /**
     * Remove file descriptor listener for a file descriptor.
     *
     * @param fileDescriptor File descriptor.
     */
    void RemoveFileDescriptorListener(int32_t fileDescriptor) const;

private:
    std::shared_ptr<EventRunner> eventRunner_ = nullptr;
};

#endif // AAFWK_APPEXECFWK_NATIVE_IMPLEMENT_EVENTHANDLER_H
