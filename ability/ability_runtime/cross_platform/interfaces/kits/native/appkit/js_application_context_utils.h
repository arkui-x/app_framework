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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H

#include <memory>

#include "ability_lifecycle_callback.h"
#include "application_context.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class JsApplicationContextUtils {
public:
    explicit JsApplicationContextUtils(std::weak_ptr<ApplicationContext>&& applicationContext)
        : applicationContext_(std::move(applicationContext))
    {}
    virtual ~JsApplicationContextUtils() = default;
    static void Finalizer(NativeEngine* engine, void* data, void* hint);

    static NativeValue* On(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* Off(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetTempDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetRunningProcessInformation(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* CreateJsApplicationContext(NativeEngine& engine);
    static NativeValue* CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    std::weak_ptr<ApplicationContext> applicationContext_;

private:
    NativeValue* OnOn(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnOff(NativeEngine& engine, const NativeCallbackInfo& info);
    NativeValue* OnOnAbilityLifecycle(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnOffAbilityLifecycle(NativeEngine& engine, const NativeCallbackInfo& info, int32_t callbackId);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetRunningProcessInformation(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info);

    static void BindNativeApplicationContext(NativeEngine& engine, NativeObject* object);

    std::shared_ptr<JsAbilityLifecycleCallback> callback_;
    std::unique_ptr<NativeReference> systemModule_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_APPLICATION_CONTEXT_UTILS_H
