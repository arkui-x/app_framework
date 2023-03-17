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

#include "js_context_utils.h"

#include "hilog.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";
}

void JsBaseContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsBaseContext::Finalizer is called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

NativeValue* JsBaseContext::GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetCacheDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetTempDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetTempDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetFilesDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetDatabaseDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetPreferencesDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsBaseContext::GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsBaseContext::GetBundleCodeDir is called");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* CreateJsBaseContext(NativeEngine& engine, std::shared_ptr<Context> context, bool keepContext)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_WARN("invalid object.");
        return objValue;
    }
    auto jsContext = std::make_unique<JsBaseContext>(context);
    SetNamedNativePointer(engine, *object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        object->SetProperty("currentHapModuleInfo", CreateJsHapModuleInfo(engine, *hapModuleInfo));
    }

    BindNativeProperty(*object, "cacheDir", JsBaseContext::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsBaseContext::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsBaseContext::GetFilesDir);
    BindNativeProperty(*object, "databaseDir", JsBaseContext::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsBaseContext::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsBaseContext::GetBundleCodeDir);

    return objValue;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
