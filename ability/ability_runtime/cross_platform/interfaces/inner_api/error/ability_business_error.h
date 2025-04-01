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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_BUSINESS_ERROR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_BUSINESS_ERROR_H

#include <string>

namespace OHOS {
namespace AbilityRuntime {
enum class AbilityErrorCode {
    // success
    ERROR_OK = 0,

    // no such permission.
    ERROR_CODE_PERMISSION_DENIED = 201,

    // non-system-app use system-api.
    ERROR_CODE_NOT_SYSTEM_APP = 202,

    // invalid param.
    ERROR_CODE_INVALID_PARAM = 401,

    // no such systemcap.
    ERROR_CODE_SYSTEMCAP = 801,

    // common inner error.
    ERROR_CODE_INNER = 16000050,

    // can not find target ability.
    ERROR_CODE_RESOLVE_ABILITY = 16000001,

    // ability type is wrong.
    ERROR_CODE_INVALID_ABILITY_TYPE = 16000002,

    // id does not exist.
    ERROR_CODE_INVALID_ID = 16000003,

    // no start invisible ability permission.
    ERROR_CODE_NO_INVISIBLE_PERMISSION = 16000004,

    // check static permission failed.
    ERROR_CODE_STATIC_CFG_PERMISSION = 16000005,

    // no permission to cross user.
    ERROR_CODE_CROSS_USER = 16000006,

    // Service busy.Try again later.
    ERROR_CODE_SERVICE_BUSY = 16000007,

    // crowdtest app expiration.
    ERROR_CODE_CROWDTEST_EXPIRED = 16000008,

    // wukong mode.
    ERROR_CODE_WUKONG_MODE = 16000009,

    // not allowed for continuation flag.
    ERROR_CODE_CONTINUATION_FLAG = 16000010,

    // context is invalid.
    ERROR_CODE_INVALID_CONTEXT = 16000011,

    // free install network abnormal.
    ERROR_CODE_NETWORK_ABNORMAL = 16000051,

    // not support free install.
    ERROR_CODE_NOT_SUPPORT_FREE_INSTALL = 16000052,

    // not top ability, not enable to free install.
    ERROR_CODE_NOT_TOP_ABILITY = 16000053,

    // too busy for free install.
    ERROR_CODE_FREE_INSTALL_TOO_BUSY = 16000054,

    // free install timeout.
    ERROR_CODE_FREE_INSTALL_TIMEOUT = 16000055,

    // free install other ability.
    ERROR_CODE_FREE_INSTALL_OTHERS = 16000056,

    // Cross-device installation-free is not supported.
    ERROR_CODE_FREE_INSTALL_CROSS_DEVICE = 16000057,

    // invalid caller.
    ERROR_CODE_INVALID_CALLER = 16200001,

    // no such mission id.
    ERROR_CODE_NO_MISSION_ID = 16300001,

    // no such mission listener.
    ERROR_CODE_NO_MISSION_LISTENER = 16300002,

    // observer not found.
    ERROR_CODE_OBSERVER_NOT_FOUND = 16300004,
};

std::string GetErrorMsg(const AbilityErrorCode& errCode);
std::string GetNoPermissionErrorMsg(const std::string& permission);
AbilityErrorCode GetJsErrorCodeByNativeError(int32_t errCode);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif