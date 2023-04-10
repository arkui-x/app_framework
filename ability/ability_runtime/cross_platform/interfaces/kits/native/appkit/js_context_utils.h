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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_CONTEXT_UTILS_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_CONTEXT_UTILS_H

#include <native_engine/native_engine.h>

#include "context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class JsBaseContext {
public:
    explicit JsBaseContext(std::weak_ptr<Context>&& context) : context_(std::move(context)) {}
    virtual ~JsBaseContext() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);

    static NativeValue* GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetTempDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info);

protected:
    NativeValue* OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info);
    std::weak_ptr<Context> context_;
};

NativeValue* CreateJsBaseContext(
    NativeEngine& engine, const std::shared_ptr<Context>& context, bool keepContext = false);
NativeValue* AttachApplicationContext(NativeEngine* engine, void* value, void* hint);
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_CONTEXT_UTILS_H
