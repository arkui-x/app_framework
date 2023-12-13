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

#ifndef BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_ERRORS_H
#define BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_ERRORS_H

#include "errors.h"

namespace OHOS {
namespace AppExecFwk {
// Module id of event handler is assigned in "appexecfwk_errors.h"
const uint32_t APPEXECFWK_MODULE_EVENT_HANDLER = 0x10;
constexpr ErrCode EVENT_HANDLER_ERR_OFFSET = ErrCodeOffset(SUBSYS_APPEXECFWK, APPEXECFWK_MODULE_EVENT_HANDLER);

enum {
    // Invalid parameters.
    EVENT_HANDLER_ERR_INVALID_PARAM = EVENT_HANDLER_ERR_OFFSET,
    // Have not set event runner yet.
    EVENT_HANDLER_ERR_NO_EVENT_RUNNER,
    // Not support to listen file descriptors.
    EVENT_HANDLER_ERR_FD_NOT_SUPPORT,
    // File descriptor is already in listening.
    EVENT_HANDLER_ERR_FD_ALREADY,
    // Failed to listen file descriptor.
    EVENT_HANDLER_ERR_FD_FAILED,
    // No permit to start or stop deposited event runner.
    EVENT_HANDLER_ERR_RUNNER_NO_PERMIT,
    // Event runner is already running.
    EVENT_HANDLER_ERR_RUNNER_ALREADY,
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // BASE_EVENTHANDLER_INTERFACES_INNER_API_EVENT_HANDLER_ERRORS_H
