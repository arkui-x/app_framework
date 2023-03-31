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
#include "js_application_context_utils.h"
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
        HILOG_ERROR("context is already released");
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
        HILOG_ERROR("context is already released");
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
        HILOG_ERROR("context is already released");
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
        HILOG_ERROR("context is already released");
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
        HILOG_ERROR("context is already released");
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
        HILOG_ERROR("context is already released");
        return engine.CreateUndefined();
    }
    std::string path = context->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* AttachApplicationContext(NativeEngine* engine, void* value, void* hint)
{
    HILOG_INFO("AttachApplicationContext");
    if (value == nullptr || engine == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ApplicationContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    NativeValue* object = JsApplicationContextUtils::CreateJsApplicationContext(*engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.ApplicationContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_ERROR("invalid nObject.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachApplicationContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(ptr);
    nObject->SetNativePointer(workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        }, nullptr);
    return contextObj;
}

NativeValue* JsBaseContext::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("GetApplicationContext start");
    JsBaseContext* me = CheckParamsAndGetThis<JsBaseContext>(engine, info, BASE_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsBaseContext::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnGetApplicationContext start");
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("context is already released");
        return engine.CreateUndefined();
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        HILOG_ERROR("applicationContext is nullptr");
        return engine.CreateUndefined();
    }

    NativeValue* value = JsApplicationContextUtils::CreateJsApplicationContext(engine);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, invalid systemModule.");
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, Failed to get context native object");
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachApplicationContext,
        workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* CreateJsBaseContext(NativeEngine& engine, std::shared_ptr<Context> context, bool keepContext)
{
    HILOG_INFO("CreateJsBaseContext called");
    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("CreateJsBaseContext objValue is nullptr. ");
        return objValue;
    }
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("invalid object.");
        return objValue;
    }
    auto jsContext = std::make_unique<JsBaseContext>(context);
    if (jsContext == nullptr) {
        HILOG_ERROR("jsContext is nullptr.");
        return objValue;
    }
    SetNamedNativePointer(engine, *object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        object->SetProperty("currentHapModuleInfo", CreateJsHapModuleInfo(engine, *hapModuleInfo));
    }

    const char *moduleName = "JsBaseContext";
    BindNativeProperty(*object, "cacheDir", JsBaseContext::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsBaseContext::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsBaseContext::GetFilesDir);
    BindNativeProperty(*object, "databaseDir", JsBaseContext::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsBaseContext::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsBaseContext::GetBundleCodeDir);
    BindNativeFunction(engine, *object, "getApplicationContext", moduleName, JsBaseContext::GetApplicationContext);
    return objValue;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
