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

#include "ability_runtime_error_util.h"
#include "hilog.h"
#include "js_application_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";
}

void JsBaseContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsBaseContext::Finalizer is called");
    std::unique_ptr<JsBaseContext>(static_cast<JsBaseContext*>(data));
}

napi_value JsBaseContext::GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsBaseContext::GetCacheDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetCacheDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetTempDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::GetTempDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetTempDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::GetFilesDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetFilesDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::GetDatabaseDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetDatabaseDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::GetPreferencesDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetPreferencesDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsBaseContext::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::GetBundleCodeDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetBundleCodeDir, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_WARN("context is already released");
        return CreateJsUndefined(env);
    }
    std::string path = context->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value AttachBaseContext(napi_env env, void* value, void* hint)
{
    HILOG_DEBUG("AttachBaseContext");
    if (value == nullptr || env == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<Context>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsBaseContext(env, ptr, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("AttachBaseContext, invalid systemModule.");
        return nullptr;
    }
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr base context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value AttachApplicationContext(napi_env env, void* value, void* hint)
{
    HILOG_INFO("AttachApplicationContext");
    if (value == nullptr || env == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<ApplicationContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }
    napi_value object = JsApplicationContextUtils::CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value JsBaseContext::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnGetApplicationContext, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("OnGetApplicationContext start");
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("context is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    auto applicationContext = Context::GetApplicationContext();
    if (applicationContext == nullptr) {
        HILOG_ERROR("applicationContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value value = JsApplicationContextUtils::CreateJsApplicationContext(env);
    if (systemModule_ != nullptr) {
        return systemModule_->GetNapiValue();
    }
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, object, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr, nullptr);
    return object;
}

napi_value JsBaseContext::CreateModuleContext(napi_env env, napi_callback_info info)
{
    HILOG_DEBUG("JsBaseContext::CreateModuleContext is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsBaseContext, OnCreateModuleContext, BASE_CONTEXT_NAME);
}

napi_value JsBaseContext::OnCreateModuleContext(napi_env env, NapiCallbackInfo& info)
{
    auto context = context_.lock();
    if (!context) {
        HILOG_ERROR("context is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::shared_ptr<Context> moduleContext = nullptr;
    std::string moduleName;

    HILOG_INFO("Parse inner module name.");
    if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
        HILOG_ERROR("Parse moduleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    moduleContext = context->CreateModuleContext(moduleName);
    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        return CreateJsUndefined(env);
    }

    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, object, napi_object)) {
        HILOG_ERROR("Failed to get object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return object;
}

napi_value CreateJsBaseContext(napi_env env, std::shared_ptr<Context> context, bool keepContext)
{
    HILOG_INFO("CreateJsBaseContext called");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_WARN("invalid object.");
        return nullptr;
    }
    
    auto jsContext = std::make_unique<JsBaseContext>(context);
    if (jsContext == nullptr) {
        HILOG_ERROR("jsContext is nullptr.");
        return object;
    }
    SetNamedNativePointer(env, object, BASE_CONTEXT_NAME, jsContext.release(), JsBaseContext::Finalizer);

    auto appInfo = context->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo != nullptr) {
        napi_set_named_property(env, object, "currentHapModuleInfo", CreateJsHapModuleInfo(env, *hapModuleInfo));
    }
    auto resMgr = context->GetResourceManager();
    if (resMgr != nullptr) {
        auto jsResourceManager = CreateJsResourceManager(env, resMgr, context);
        if (jsResourceManager != nullptr) {
            napi_set_named_property(env, object, "resourceManager", jsResourceManager);
        } else {
            HILOG_ERROR("jsResourceManager is nullptr");
        }
    }

    const char* moduleName = "JsBaseContext";
    BindNativeProperty(env, object, "cacheDir", JsBaseContext::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsBaseContext::GetTempDir);
    BindNativeProperty(env, object, "filesDir", JsBaseContext::GetFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsBaseContext::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsBaseContext::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsBaseContext::GetBundleCodeDir);
    BindNativeFunction(env, object, "getApplicationContext", moduleName, JsBaseContext::GetApplicationContext);
    BindNativeFunction(env, object, "createModuleContext", moduleName, JsBaseContext::CreateModuleContext);
    return object;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
