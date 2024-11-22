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
#include "js_error_utils.h"

#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ERR_MSG_TOO_FEW_PARAM = "Parameter error. Too few parameters.";
} // namespace

void ThrowError(napi_env env, int32_t errCode, const std::string& errorMsg)
{
    napi_throw(env, CreateJsError(env, errCode, errorMsg));
}

void ThrowTooFewParametersError(napi_env env)
{
    napi_throw(env, CreateJsError(env,
        static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM),
        ERR_MSG_TOO_FEW_PARAM));
}


void ThrowNoPermissionError(napi_env env, const std::string& permission)
{
}

void ThrowErrorByNativeErr(napi_env env, int32_t err)
{
    napi_throw(env, CreateJsErrorByNativeErr(env, err));
}

napi_value CreateJsError(napi_env env, const AbilityErrorCode& err)
{
    return CreateJsError(env, static_cast<int32_t>(err), GetErrorMsg(err));
}

napi_value CreateNoPermissionError(napi_env env, const std::string& permission)
{
    return nullptr;
}

napi_value CreateJsErrorByNativeErr(napi_env env, int32_t err, const std::string& permission)
{
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty()) ?
        GetNoPermissionErrorMsg(permission) : GetErrorMsg(errCode);
    return CreateJsError(env, static_cast<int32_t>(errCode), errMsg);
}
} // namespace AbilityRuntime
} // namespace OHOS