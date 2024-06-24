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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H

#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "want.h"
#include "want_params.h"

namespace OHOS {
namespace AppExecFwk {
using Want = OHOS::AAFwk::Want;
napi_value WrapWant(napi_env env, const Want &want);
bool UnwrapWant(napi_env env, napi_value param, Want& want);
bool UnwrapWantParams(napi_env env, napi_value param, Want& want);
void InnerUnwrapWantParamsNumber(napi_env env, const std::string& key, napi_value param, Want& want);
napi_value CreateJsWant(napi_env env, const Want& want);
napi_value CreateJsWantParams(napi_env env, const Want& want);

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams& wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams& wantParams);
napi_value WantStringToJSValue(napi_env env, const std::string &value);
napi_value WantBoolToJSValue(napi_env env, bool value);
napi_value WantDoubleToJSValue(napi_env env, double value);
napi_value WantInt32ToJSValue(napi_env env, int32_t value);
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_NAPI_COMMON_WANT_H
