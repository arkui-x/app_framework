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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H

#include <memory>

#include "ability_lifecycle_callback.h"
#include "application_context.h"
#include "application_state_change_callback.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class JsApplicationContextUtils {
public:
    explicit JsApplicationContextUtils(std::weak_ptr<ApplicationContext>&& applicationContext)
        : applicationContext_(std::move(applicationContext))
    {}
    virtual ~JsApplicationContextUtils() = default;
    static void Finalizer(napi_env env, void* data, void* hint);

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value GetApplicationContext(napi_env env, napi_callback_info info);
    static napi_value GetCacheDir(napi_env env, napi_callback_info info);
    static napi_value GetTempDir(napi_env env, napi_callback_info info);
    static napi_value GetFilesDir(napi_env env, napi_callback_info info);
    static napi_value GetDatabaseDir(napi_env env, napi_callback_info info);
    static napi_value GetPreferencesDir(napi_env env, napi_callback_info info);
    static napi_value GetBundleCodeDir(napi_env env, napi_callback_info info);
    static napi_value GetRunningProcessInformation(napi_env env, napi_callback_info info);
    static napi_value CreateJsApplicationContext(napi_env env);
    static napi_value CreateModuleContext(napi_env env, napi_callback_info info);
    static napi_value SetColorMode(napi_env env, napi_callback_info info);
    static napi_value SetFont(napi_env env, napi_callback_info info);
    static napi_value SetFontSizeScale(napi_env env, napi_callback_info info);
    static napi_value SetLanguage(napi_env env, napi_callback_info info);

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    napi_value OnOn(napi_env env, NapiCallbackInfo& info);
    napi_value OnOff(napi_env env, const NapiCallbackInfo& info);
    napi_value OnOnAbilityLifecycle(napi_env env, NapiCallbackInfo& info);
    napi_value OnOnApplicationStateChange(napi_env env, NapiCallbackInfo& info);
    napi_value OnOffApplicationStateChange(napi_env env, const NapiCallbackInfo& info);
    napi_value OnOffAbilityLifecycle(napi_env env, const NapiCallbackInfo& info, int32_t callbackId);
    napi_value OnGetApplicationContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetCacheDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetTempDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetFilesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetRunningProcessInformation(napi_env env, NapiCallbackInfo& info);
    napi_value OnCreateModuleContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetColorMode(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetFont(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetFontSizeScale(napi_env env, NapiCallbackInfo& info);
    napi_value OnSetLanguage(napi_env env, NapiCallbackInfo& info);

    static void BindNativeApplicationContext(napi_env env, napi_value object);

    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
    std::unique_ptr<NativeReference> systemModule_;
    static std::shared_ptr<JsApplicationStateChangeCallback> applicationStateCallback_;
    std::mutex applicationStateCallbackLock_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H
