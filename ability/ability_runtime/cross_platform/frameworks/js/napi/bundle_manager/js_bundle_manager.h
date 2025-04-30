/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef OHOS_ABILITY_RUNTIME_JS_BUNDLE_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_BUNDLE_MANAGER_H

#include "bundle_info.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AAFwk {
class JsBundleManager {
public:
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetBundleInfoForSelfSync(napi_env env, napi_callback_info info);
    static napi_value GetBundleInfoForSelf(napi_env env, napi_callback_info info);

private:
    napi_value OnGetBundleInfoForSelfSync(napi_env env, AbilityRuntime::NapiCallbackInfo& info);
    napi_value OnGetBundleInfoForSelf(napi_env env, AbilityRuntime::NapiCallbackInfo& info);
}; // JsBundleManager
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_BUNDLE_MANAGER_H