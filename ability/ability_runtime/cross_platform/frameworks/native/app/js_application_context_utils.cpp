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

#include "js_application_context_utils.h"

#include <map>

#include "ability_runtime_error_util.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "hilog.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<JsApplicationStateChangeCallback> JsApplicationContextUtils::applicationStateCallback_ = nullptr;
namespace {
constexpr char APPLICATION_CONTEXT_NAME[] = "__application_context_ptr__";
const char* MD_NAME = "JsApplicationContextUtils";
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr double FONT_SIZE = 0.0;
} // namespace

void JsApplicationContextUtils::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsApplicationContextUtils::Finalizer is called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils*>(data));
}

napi_value JsApplicationContextUtils::GetCacheDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetCacheDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetCacheDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetCacheDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetCacheDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetTempDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetTempDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetTempDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetTempDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetTempDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetFilesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetFilesDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetFilesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetFilesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetFilesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetDatabaseDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDatabaseDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnGetDatabaseDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetDatabaseDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetDatabaseDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetPreferencesDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetPreferencesDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetPreferencesDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetPreferencesDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetPreferencesDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::GetBundleCodeDir(napi_env env, napi_callback_info info)
{
    HILOG_INFO("JsApplicationContextUtils::GetBundleCodeDir is called");
    GET_NAPI_INFO_WITH_NAME_AND_CALL(
        env, info, JsApplicationContextUtils, OnGetBundleCodeDir, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetBundleCodeDir(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return CreateJsValue(env, path);
}

napi_value JsApplicationContextUtils::On(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOn, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::GetRunningProcessInformation(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetRunningProcessInformation, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnOn(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("OnOn is called");
    if (info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(env, info);
    }

    if (type == "applicationStateChange") {
        return OnOnApplicationStateChange(env, info);
    }

    HILOG_ERROR("on function type not match.");
    AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOnAbilityLifecycle(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("OnOnAbilityLifecycle is called");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (callback_ != nullptr) {
        HILOG_INFO("callback_ is not nullptr.");
        return CreateJsValue(env, callback_->Register(info.argv[1]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(env);
    if (callback_ == nullptr) {
        HILOG_INFO("callback_ is nullptr.");
        return CreateJsUndefined(env);
    }
    int32_t callbackId = callback_->Register(info.argv[1]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnOnAbilityLifecycle is end");
    return CreateJsValue(env, callbackId);
}

napi_value JsApplicationContextUtils::Off(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnOff, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnOff(napi_env env, const NapiCallbackInfo& info)
{
    HILOG_INFO("OnOff is called");

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (!CheckTypeForNapiValue(env, info.argv[0], napi_string)) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    std::string type;
    if (!ConvertFromJsValue(env, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    int32_t callbackId = -1;
    if (CheckTypeForNapiValue(env, info.argv[1], napi_number)) {
        napi_get_value_int32(env, info.argv[1], &callbackId);
        HILOG_DEBUG("callbackId is %{public}d.", callbackId);
    }
    
    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(env, info, callbackId);
    }

    if (type == "applicationStateChange") {
        return OnOffApplicationStateChange(env, info);
    }

    HILOG_ERROR("off function type not match.");
    AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffAbilityLifecycle(
    napi_env env, const NapiCallbackInfo& info, int32_t callbackId)
{
    HILOG_INFO("OnOffAbilityLifecycle is called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    NapiAsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
                                               napi_env env, NapiAsyncTask &task, int32_t status) {
        auto callback = callbackWeak.lock();
        if (callback == nullptr) {
            HILOG_ERROR("callback is nullptr");
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                "callback is nullptr"));
            return;
        }

        HILOG_INFO("OnOffAbilityLifecycle begin");
        if (!callback->UnRegister(callbackId)) {
            HILOG_ERROR("call UnRegister failed!");
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER,
                "call UnRegister failed!"));
            return;
        }

        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::OnOnApplicationStateChange(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ != nullptr) {
        applicationStateCallback_->Register(info.argv[INDEX_ONE]);
        return CreateJsUndefined(env);
    }

    applicationStateCallback_ = std::make_shared<JsApplicationStateChangeCallback>(env);
    if (applicationStateCallback_ == nullptr) {
        HILOG_ERROR("applicationStateCallback_ is nullptr.");
        return CreateJsUndefined(env);
    }
    applicationStateCallback_->Register(info.argv[INDEX_ONE]);
    applicationContext->RegisterApplicationStateChangeCallback(applicationStateCallback_);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::OnOffApplicationStateChange(napi_env env, const NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    if (applicationStateCallback_ == nullptr) {
        HILOG_ERROR("ApplicationStateCallback_ is nullptr");
        ThrowInvalidParamError(
            env, "Parse applicationStateCallback failed, applicationStateCallback must be function.");
        return CreateJsUndefined(env);
    }

    if (info.argc == ARGC_ONE || !CheckTypeForNapiValue(env, info.argv[INDEX_ONE], napi_object)) {
        applicationStateCallback_->UnRegister();
    } else if (!applicationStateCallback_->UnRegister(info.argv[INDEX_ONE])) {
        HILOG_ERROR("call UnRegister failed");
        ThrowInvalidParamError(env, "Parse param call UnRegister failed, call UnRegister must be function.");
        return CreateJsUndefined(env);
    }

    if (applicationStateCallback_->IsEmpty()) {
        applicationContext->UnRegisterApplicationStateChangeCallback(applicationStateCallback_);
        applicationStateCallback_.reset();
    }
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::GetApplicationContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnGetApplicationContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnGetApplicationContext(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("GetApplicationContext start");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_ERROR("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value value = CreateJsApplicationContext(env);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.ApplicationContext", &value, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::CreateModuleContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils,
        OnCreateModuleContext, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnCreateModuleContext(napi_env env, NapiCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    HILOG_INFO("Parse inner module name.");
    if (!ConvertFromJsValue(env, info.argv[0], moduleName)) {
        HILOG_ERROR("Parse moduleName failed");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return CreateJsUndefined(env);
    }
    moduleContext = applicationContext->CreateModuleContext(moduleName);
    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
            return CreateJsUndefined(env);
    }
    
    napi_value value = CreateJsBaseContext(env, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    
    napi_value contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

napi_value JsApplicationContextUtils::CreateJsApplicationContext(napi_env env)
{
    HILOG_INFO("CreateJsApplicationContext start");

    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext object is nullptr. ");
        return object;
    }

    std::shared_ptr<ApplicationContext> applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext applicationContext is nullptr. ");
        return object;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    if (jsApplicationContextUtils == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext jsApplicationContextUtils is nullptr. ");
        return object;
    }
    SetNamedNativePointer(env, object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        napi_set_named_property(env, object, "applicationInfo", CreateJsApplicationInfo(env, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        napi_set_named_property(env, object, "resourceManager", CreateJsResourceManager(env, resourceManager, context));
    }

    BindNativeApplicationContext(env, object);
    return object;
}

napi_value JsApplicationContextUtils::OnGetRunningProcessInformation(napi_env env, NapiCallbackInfo& info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return CreateJsUndefined(env);
    }
    auto complete = [applicationContext = applicationContext_](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(env, CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        std::vector<RunningProcessInfo> processInfos;
        auto ret = context->GetProcessRunningInformation(processInfos);
        if (ret == 0) {
            task.Resolve(env, CreateJsProcessRunningInfoArray(env, processInfos));
        } else {
            task.Reject(env,
                CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR, "Get process infos failed."));
        }
    };

    napi_value lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsApplicationContextUtils::OnGetRunningProcessInformation",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsApplicationContextUtils::SetColorMode(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetColorMode, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetColorMode(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        HILOG_WARN("Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_WARN("applicationContext is already released");
        return CreateJsUndefined(env);
    }

    int32_t colorMode = 0;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], colorMode)) {
        ThrowInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        HILOG_ERROR("Parse colorMode failed");
        return CreateJsUndefined(env);
    }
    applicationContext->SetColorMode(colorMode);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetFont(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetFont, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetFont(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    std::string fontName;
    if (!ConvertFromJsValue(env, info.argv[0], fontName)) {
        HILOG_ERROR("Parse fontName failed");
        ThrowInvalidParamError(env, "Parse param font failed, font must be string.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is already released");
        CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST, "ApplicationContext if already released.");
        return CreateJsUndefined(env);
    }
    applicationContext->SetFont(fontName);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetFontSizeScale(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetFontSizeScale,
        APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetFontSizeScale(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }

    double fontSizeScale = 1.0;
    if (!ConvertFromJsValue(env, info.argv[0], fontSizeScale)) {
        HILOG_ERROR("Parse fontSizeScale failed");
        ThrowInvalidParamError(env, "Parse fontSizeScale failed, fontSizeScale must be number.");
        return CreateJsUndefined(env);
    }
    if (fontSizeScale < FONT_SIZE) {
        HILOG_ERROR("Invalid font size");
        ThrowInvalidParamError(env, "Invalid font size.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is already released");
        return CreateJsUndefined(env);
    }
    applicationContext->SetFontSizeScale(fontSizeScale);
    return CreateJsUndefined(env);
}

napi_value JsApplicationContextUtils::SetLanguage(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_WITH_NAME_AND_CALL(env, info, JsApplicationContextUtils, OnSetLanguage, APPLICATION_CONTEXT_NAME);
}

napi_value JsApplicationContextUtils::OnSetLanguage(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowInvalidParamError(env, "Not enough params.");
        return CreateJsUndefined(env);
    }
    std::string language;
    if (!ConvertFromJsValue(env, info.argv[0], language)) {
        HILOG_ERROR("Parse language failed");
        ThrowInvalidParamError(env, "Parse param language failed, language must be string.");
        return CreateJsUndefined(env);
    }
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is already released");
        CreateJsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST, "ApplicationContext if already released.");
        return CreateJsUndefined(env);
    }
    applicationContext->SetLanguage(language);
    return CreateJsUndefined(env);
}

void JsApplicationContextUtils::BindNativeApplicationContext(napi_env env, napi_value object)
{
    BindNativeProperty(env, object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(env, object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(env, object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(env, object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(env, object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(env, object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(env, object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(env, object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(env, object, "getApplicationContext", MD_NAME,
        JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(env, object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(env, object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
    BindNativeFunction(env, object, "setColorMode", MD_NAME, JsApplicationContextUtils::SetColorMode);
    BindNativeFunction(env, object, "setFont", MD_NAME, JsApplicationContextUtils::SetFont);
    BindNativeFunction(env, object, "setFontSizeScale", MD_NAME, JsApplicationContextUtils::SetFontSizeScale);
    BindNativeFunction(env, object, "setLanguage", MD_NAME, JsApplicationContextUtils::SetLanguage);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
