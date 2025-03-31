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
#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H

#include "ability_business_error.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
void ThrowError(napi_env env, int32_t errCode, const std::string& errorMsg = "");
void ThrowError(napi_env env, const AbilityErrorCode& err);
void ThrowInvalidCallerError(napi_env env);
void ThrowTooFewParametersError(napi_env env);
void ThrowInvalidNumParametersError(napi_env env);
void ThrowNoPermissionError(napi_env env, const std::string& permission);
void ThrowErrorByNativeErr(napi_env env, int32_t err);
void ThrowInvalidParamError(napi_env env, const std::string& message);

napi_value CreateJsError(napi_env env, const AbilityErrorCode& err);
napi_value CreateNoPermissionError(napi_env env, const std::string& permission);
napi_value CreateJsErrorByNativeErr(napi_env env, int32_t err, const std::string& permission = "");
napi_value CreateInvalidParamJsError(napi_env env, const std::string& message);
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ERROR_UTILS_H
