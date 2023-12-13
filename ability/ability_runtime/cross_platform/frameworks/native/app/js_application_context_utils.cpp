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
} // namespace

void JsApplicationContextUtils::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsApplicationContextUtils::Finalizer is called");
    std::unique_ptr<JsApplicationContextUtils>(static_cast<JsApplicationContextUtils*>(data));
}

NativeValue* JsApplicationContextUtils::GetCacheDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetCacheDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetCacheDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetCacheDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetCacheDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::GetTempDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetTempDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetTempDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetTempDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetTempDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::GetFilesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetFilesDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetFilesDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetFilesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetFilesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::GetDatabaseDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetDatabaseDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetDatabaseDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetDatabaseDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetDatabaseDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::GetPreferencesDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetPreferencesDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetPreferencesDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetPreferencesDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetPreferencesDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::GetBundleCodeDir(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsApplicationContextUtils::GetBundleCodeDir is called");
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetBundleCodeDir(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetBundleCodeDir(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        return engine.CreateUndefined();
    }
    std::string path = applicationContext->GetBundleCodeDir();
    return engine.CreateString(path.c_str(), path.length());
}

NativeValue* JsApplicationContextUtils::On(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOn(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::GetRunningProcessInformation(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetRunningProcessInformation(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnOn(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnOn is called");
    if (info.argc != ARGC_TWO) {
        HILOG_ERROR("Not enough params.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (type == "abilityLifecycle") {
        return OnOnAbilityLifecycle(engine, info);
    }

    HILOG_ERROR("on function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue* JsApplicationContextUtils::OnOnAbilityLifecycle(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnOnAbilityLifecycle is called");
    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (callback_ != nullptr) {
        HILOG_INFO("callback_ is not nullptr.");
        return engine.CreateNumber(callback_->Register(info.argv[1]));
    }
    callback_ = std::make_shared<JsAbilityLifecycleCallback>(&engine);
    if (callback_ == nullptr) {
        HILOG_INFO("callback_ is nullptr.");
        return engine.CreateUndefined();
    }
    int32_t callbackId = callback_->Register(info.argv[1]);
    applicationContext->RegisterAbilityLifecycleCallback(callback_);
    HILOG_INFO("OnOnAbilityLifecycle is end");
    return engine.CreateNumber(callbackId);
}

NativeValue* JsApplicationContextUtils::Off(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnOff(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnOff(NativeEngine& engine, const NativeCallbackInfo& info)
{
    HILOG_INFO("OnOff is called");

    if (info.argc != ARGC_TWO && info.argc != ARGC_THREE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[0]->TypeOf() != NATIVE_STRING) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    std::string type;
    if (!ConvertFromJsValue(engine, info.argv[0], type)) {
        HILOG_ERROR("convert type failed!");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    if (info.argv[1]->TypeOf() != NATIVE_NUMBER) {
        HILOG_ERROR("param0 is invalid");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    int32_t callbackId = -1;
    if (!ConvertFromJsValue(engine, info.argv[1], callbackId)) {
        HILOG_ERROR("convert type failed!");
        return engine.CreateUndefined();
    }

    if (type == "abilityLifecycle") {
        return OnOffAbilityLifecycle(engine, info, callbackId);
    }

    HILOG_ERROR("off function type not match.");
    AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
    return engine.CreateUndefined();
}

NativeValue* JsApplicationContextUtils::OnOffAbilityLifecycle(
    NativeEngine& engine, const NativeCallbackInfo& info, int32_t callbackId)
{
    HILOG_INFO("OnOffAbilityLifecycle is called");

    auto applicationContext = applicationContext_.lock();
    if (applicationContext == nullptr) {
        HILOG_ERROR("ApplicationContext is nullptr.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::weak_ptr<JsAbilityLifecycleCallback> callbackWeak(callback_);
    AsyncTask::CompleteCallback complete = [callbackWeak, callbackId](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto callback = callbackWeak.lock();
        if (callback == nullptr) {
            HILOG_ERROR("callback is nullptr");
            task.Reject(
                engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER, "callback is nullptr"));
            return;
        }

        HILOG_INFO("OnOffAbilityLifecycle begin");
        if (!callback->UnRegister(callbackId)) {
            HILOG_ERROR("call UnRegister failed!");
            task.Reject(engine,
                CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER, "call UnRegister failed!"));
            return;
        }

        task.ResolveWithNoError(engine, engine.CreateUndefined());
    };
    NativeValue* lastParam = (info.argc <= ARGC_TWO) ? nullptr : info.argv[INDEX_TWO];
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnOffAbilityLifecycle", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsApplicationContextUtils::GetApplicationContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils* me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnGetApplicationContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnGetApplicationContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("GetApplicationContext start");
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_ERROR("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    NativeValue* value = CreateJsApplicationContext(engine);
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return engine.CreateUndefined();
    }
    if (systemModule_ != nullptr) {
        return systemModule_->Get();
    }
    systemModule_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.ApplicationContext", &value, 1);
    if (systemModule_ == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule_->Get();
    if (contextObj == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, contextObj is nullptr");
        return engine.CreateUndefined();
    }
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnGetApplicationContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    nativeObj->ConvertToNativeBindingObject(
        &engine, DetachCallbackFunc, AttachApplicationContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr application context is called");
            delete static_cast<std::weak_ptr<ApplicationContext>*>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsApplicationContextUtils::CreateModuleContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsApplicationContextUtils *me =
        CheckParamsAndGetThis<JsApplicationContextUtils>(engine, info, APPLICATION_CONTEXT_NAME);
    return me != nullptr ? me->OnCreateModuleContext(*engine, *info) : nullptr;
}

NativeValue* JsApplicationContextUtils::OnCreateModuleContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    auto applicationContext = applicationContext_.lock();
    if (!applicationContext) {
        HILOG_WARN("applicationContext is already released");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }

    std::string moduleName;
    std::shared_ptr<Context> moduleContext = nullptr;
    HILOG_INFO("Parse inner module name.");
    if (!ConvertFromJsValue(engine, info.argv[0], moduleName)) {
        HILOG_ERROR("Parse moduleName failed");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    moduleContext = applicationContext->CreateModuleContext(moduleName);
    if (!moduleContext) {
        HILOG_ERROR("failed to create module context.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    NativeValue* value = CreateJsBaseContext(engine, moduleContext, true);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(&engine, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto contextObj = systemModule->Get();
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("OnCreateModuleContext, Failed to get context native object");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(moduleContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachBaseContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr module context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr);
    return contextObj;
}

NativeValue* JsApplicationContextUtils::CreateJsApplicationContext(NativeEngine& engine)
{
    HILOG_INFO("CreateJsApplicationContext start");

    NativeValue* objValue = engine.CreateObject();
    if (objValue == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext objValue is nullptr. ");
        return objValue;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext object is nullptr. ");
        return objValue;
    }

    std::shared_ptr<ApplicationContext> applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext applicationContext is nullptr. ");
        return objValue;
    }

    auto jsApplicationContextUtils = std::make_unique<JsApplicationContextUtils>(applicationContext);
    if (jsApplicationContextUtils == nullptr) {
        HILOG_ERROR("CreateJsApplicationContext jsApplicationContextUtils is nullptr. ");
        return objValue;
    }
    SetNamedNativePointer(engine, *object, APPLICATION_CONTEXT_NAME, jsApplicationContextUtils.release(),
        JsApplicationContextUtils::Finalizer);

    auto appInfo = applicationContext->GetApplicationInfo();
    if (appInfo != nullptr) {
        object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, *appInfo));
    }
    auto resourceManager = applicationContext->GetResourceManager();
    std::shared_ptr<Context> context = std::dynamic_pointer_cast<Context>(applicationContext);
    if (resourceManager != nullptr) {
        object->SetProperty("resourceManager", CreateJsResourceManager(engine, resourceManager, context));
    }

    BindNativeApplicationContext(engine, object);

    ApplicationContextManager::GetApplicationContextManager().AddGlobalObject(
        std::shared_ptr<NativeReference>(engine.CreateReference(objValue, 1)));

    return objValue;
}

NativeValue* JsApplicationContextUtils::OnGetRunningProcessInformation(NativeEngine& engine, NativeCallbackInfo& info)
{
    // only support 0 or 1 params
    if (info.argc != ARGC_ZERO && info.argc != ARGC_ONE) {
        HILOG_ERROR("Not enough params");
        AbilityRuntimeErrorUtil::Throw(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return engine.CreateUndefined();
    }
    auto complete = [applicationContext = applicationContext_](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto context = applicationContext.lock();
        if (!context) {
            task.Reject(engine, CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST,
                "applicationContext if already released."));
            return;
        }
        std::vector<RunningProcessInfo> processInfos;
        auto ret = context->GetProcessRunningInformation(processInfos);
        if (ret == 0) {
            task.Resolve(engine, CreateJsProcessRunningInfoArray(engine, processInfos));
        } else {
            task.Reject(engine,
                CreateJsError(engine, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR, "Get process infos failed."));
        }
    };

    NativeValue* lastParam = (info.argc == ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsApplicationContextUtils::OnGetRunningProcessInformation", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

void JsApplicationContextUtils::BindNativeApplicationContext(NativeEngine& engine, NativeObject* object)
{
    BindNativeProperty(*object, "cacheDir", JsApplicationContextUtils::GetCacheDir);
    BindNativeProperty(*object, "tempDir", JsApplicationContextUtils::GetTempDir);
    BindNativeProperty(*object, "filesDir", JsApplicationContextUtils::GetFilesDir);
    BindNativeProperty(*object, "databaseDir", JsApplicationContextUtils::GetDatabaseDir);
    BindNativeProperty(*object, "preferencesDir", JsApplicationContextUtils::GetPreferencesDir);
    BindNativeProperty(*object, "bundleCodeDir", JsApplicationContextUtils::GetBundleCodeDir);
    BindNativeFunction(engine, *object, "on", MD_NAME, JsApplicationContextUtils::On);
    BindNativeFunction(engine, *object, "off", MD_NAME, JsApplicationContextUtils::Off);
    BindNativeFunction(
        engine, *object, "getApplicationContext", MD_NAME, JsApplicationContextUtils::GetApplicationContext);
    BindNativeFunction(engine, *object, "getRunningProcessInformation", MD_NAME,
        JsApplicationContextUtils::GetRunningProcessInformation);
    BindNativeFunction(engine, *object, "createModuleContext", MD_NAME, JsApplicationContextUtils::CreateModuleContext);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
