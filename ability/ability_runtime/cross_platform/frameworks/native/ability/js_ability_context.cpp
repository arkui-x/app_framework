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

#include "js_ability_context.h"

#include "errors.h"
#include "hilog.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
} // namespace

void JsAbilityContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsAbilityContext::Finalizer is called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

NativeValue* JsAbilityContext::StartAbility(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("OnStartAbility is called.");
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnStartAbility(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnStartAbility is called.");

    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    decltype(info.argc) unwrapArgc = 1;

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    AsyncTask::ExecuteCallback execute = [weak = context_, cpWant = want, unwrapArgc, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        *innerErrorCode = context->StartAbility(cpWant, -1);
    };

    AsyncTask::CompleteCallback complete = [innerErrorCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsErrorByNativeErr(engine, *innerErrorCode));
        }
    };

    NativeValue* lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;

    AsyncTask::Schedule("JsAbilityContext::OnStartAbility", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));

    return result;
}

NativeValue* JsAbilityContext::TerminateSelf(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelf(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnTerminateSelf(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("OnTerminateSelf is called");

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    AsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->TerminateSelf();
    };

    AsyncTask::CompleteCallback complete = [innerErrorCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsErrorByNativeErr(engine, *innerErrorCode));
        }
    };

    NativeValue* lastParam = (info.argc > ARGC_ZERO) ? info.argv[ARGC_ZERO] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnTerminateSelf", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::StartAbilityForResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnStartAbilityForResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("called.");
    if (info.argc == ARGC_ZERO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    AAFwk::Want want;
    AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[0]), want);
    decltype(info.argc) unwrapArgc = 1;

    auto lastParam = (info.argc > unwrapArgc) ? info.argv[unwrapArgc] : nullptr;
    NativeValue* result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<AsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [&engine, asyncTask](int32_t resultCode, const AAFwk::Want& resultWant) {
        HILOG_INFO("OnStartAbilityForResult async callback is called");
        NativeValue* abilityResult = JsAbilityContext::WrapAbilityResult(engine, resultCode, resultWant);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(engine, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(engine, CreateJsError(engine, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        context->StartAbilityForResult(want, curRequestCode_, std::move(task));
    }
    return result;
}

NativeValue* JsAbilityContext::TerminateSelfWithResult(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(engine, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(*engine, *info) : nullptr;
}

NativeValue* JsAbilityContext::OnTerminateSelfWithResult(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("called.");
    if (info.argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(engine);
        return engine.CreateUndefined();
    }
    int32_t resultCode = 0;
    AAFwk::Want resultWant;
    if (!JsAbilityContext::UnWrapAbilityResult(engine, info.argv[0], resultCode, resultWant)) {
        HILOG_ERROR("Failed to parse ability result!");
        ThrowError(engine, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return engine.CreateUndefined();
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    AsyncTask::ExecuteCallback execute = [weak = context_, resultWant, resultCode, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->TerminateAbilityWithResult(resultWant, resultCode);
    };

    AsyncTask::CompleteCallback complete = [innerErrorCode](NativeEngine& engine, AsyncTask& task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsErrorByNativeErr(engine, *innerErrorCode));
        }
    };

    NativeValue* lastParam = (info.argc > ARGC_ONE) ? info.argv[ARGC_ONE] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsAbilityContext::OnTerminateSelfWithResult", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue* JsAbilityContext::WrapAbilityResult(
    NativeEngine& engine, int32_t resultCode, const AAFwk::Want& resultWant)
{
    NativeValue* jAbilityResult = engine.CreateObject();
    NativeObject* abilityResult = ConvertNativeValueTo<NativeObject>(jAbilityResult);
    if (abilityResult == nullptr) {
        HILOG_ERROR("abilityResult is nullptr");
        return jAbilityResult;
    }
    abilityResult->SetProperty("resultCode", engine.CreateNumber(resultCode));
    abilityResult->SetProperty("want", AppExecFwk::CreateJsWant(engine, resultWant));
    return jAbilityResult;
}

bool JsAbilityContext::UnWrapAbilityResult(
    NativeEngine& engine, NativeValue* argv, int32_t& resultCode, AAFwk::Want& resultWant)
{
    if (argv == nullptr) {
        HILOG_ERROR("argv is nullptr");
        return false;
    }
    if (argv->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("invalid type of abilityResult");
        return false;
    }
    NativeObject* jObj = ConvertNativeValueTo<NativeObject>(argv);
    if (jObj == nullptr) {
        HILOG_ERROR("jObj is nullptr");
        return false;
    }
    NativeValue* jResultCode = jObj->GetProperty("resultCode");
    if (jResultCode == nullptr) {
        HILOG_ERROR("jResultCode is nullptr");
        return false;
    }
    if (jResultCode->TypeOf() != NativeValueType::NATIVE_NUMBER) {
        HILOG_ERROR("invalid type of resultCode");
        return false;
    }
    resultCode = int32_t(*ConvertNativeValueTo<NativeNumber>(jObj->GetProperty("resultCode")));
    NativeValue* jWant = jObj->GetProperty("want");
    if (jWant == nullptr) {
        HILOG_ERROR("jWant is nullptr");
        return false;
    }
    if (jWant->TypeOf() == NativeValueType::NATIVE_UNDEFINED) {
        HILOG_ERROR("want is undefined");
        return true;
    }
    if (jWant->TypeOf() != NativeValueType::NATIVE_OBJECT) {
        HILOG_ERROR("invalid type of want");
        return false;
    }
    return AppExecFwk::UnwrapWant(reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(jWant), resultWant);
}

NativeValue* CreateJsAbilityContext(NativeEngine& engine, const std::shared_ptr<AbilityContext>& context)
{
    if (context == nullptr) {
        HILOG_ERROR("context is nullptr");
        return nullptr;
    }

    NativeValue* objValue = CreateJsBaseContext(engine, context);
    if (objValue == nullptr) {
        HILOG_ERROR("objValue is nullptr");
        return objValue;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return objValue;
    }

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    if (jsContext == nullptr) {
        HILOG_ERROR("jsContext is nullptr");
        return objValue;
    }
    object->SetNativePointer(jsContext.release(), JsAbilityContext::Finalizer, nullptr);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        object->SetProperty("abilityInfo", CreateJsAbilityInfo(engine, *abilityInfo));
    }
    auto config = context->GetConfiguration();
    if (config != nullptr) {
        object->SetProperty("config", CreateJsConfiguration(engine, *config));
    }

    const char* moduleName = "JsAbilityContext";
    BindNativeFunction(engine, *object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(engine, *object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(engine, *object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(
        engine, *object, "terminateSelfWithResult", moduleName, JsAbilityContext::TerminateSelfWithResult);
    return objValue;
}

void JsAbilityContext::ConfigurationUpdated(
    NativeEngine* engine, std::shared_ptr<NativeReference>& jsContext, const std::shared_ptr<Configuration>& config)
{
    HILOG_INFO("ConfigurationUpdated called.");
    if ((jsContext == nullptr) || (config == nullptr)) {
        HILOG_ERROR("jsContext or config is nullptr.");
        return;
    }

    NativeValue* value = jsContext->Get();
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return;
    }
    NativeObject* object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    NativeValue* method = object->GetProperty("onUpdateConfiguration");
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsAbilityContext call onUpdateConfiguration.");
    NativeValue* argv[] = { CreateJsConfiguration(*engine, *config) };
    engine->CallFunction(value, method, argv, 1);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
