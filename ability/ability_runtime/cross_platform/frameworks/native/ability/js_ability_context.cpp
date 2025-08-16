/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "errors.h"
#include "hilog.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_error_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
constexpr size_t ARGC_ZERO = 0;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace

void JsAbilityContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsAbilityContext::Finalizer is called");
    std::unique_ptr<JsAbilityContext>(static_cast<JsAbilityContext*>(data));
}

napi_value JsAbilityContext::StartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("OnStartAbility is called.");
    NapiCallbackInfo napiInfo;
    JsAbilityContext* me = static_cast<JsAbilityContext*>(GetNapiCallbackInfoAndThis(env, info, napiInfo, nullptr));
    return (me != nullptr) ? me->OnStartAbility(env, info) : nullptr;  
}

napi_value JsAbilityContext::OnStartAbility(napi_env env, napi_callback_info info)
{
    HILOG_INFO("OnStartAbility is called.");
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc < ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    AAFwk::Want want;
    OHOS::AppExecFwk::UnwrapWant(env, argv[0], want);
    decltype(argc) unwrapArgc = 1;

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, cpWant = want, unwrapArgc, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        if (cpWant.IsEmpty()) {
            *innerErrorCode = AAFwk::ERR_IMPLICIT_START_ABILITY_FAIL;
            return;
        }
        if (cpWant.GetBundleName().empty()) {
            *innerErrorCode = AAFwk::RESOLVE_ABILITY_ERR;
            return;
        }
        if (cpWant.GetAbilityName().empty()) {
            *innerErrorCode = AAFwk::ERR_IMPLICIT_START_ABILITY_FAIL;
            return;
        }
        *innerErrorCode = context->StartAbility(cpWant, -1);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_valuetype paramsType[argc];
    for (size_t i = ARGC_ZERO; i < argc; i++) {
        NAPI_CALL(env, napi_typeof(env, argv[i], &paramsType[i]));
    }
    if (paramsType[argc - ARGC_TWO] == napi_object && paramsType[argc - ARGC_ONE] == napi_function) {
        unwrapArgc = ARGC_TWO;
    }
    napi_value lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;

    NapiAsyncTask::Schedule("JsAbilityContext::OnStartAbility", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));

    return result;
}

napi_value JsAbilityContext::TerminateSelf(napi_env env, napi_callback_info info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(env, info);
    return (me != nullptr) ? me->OnTerminateSelf(env, info) : nullptr;
}

napi_value JsAbilityContext::OnTerminateSelf(napi_env env, napi_callback_info info)
{
    HILOG_INFO("OnTerminateSelf is called");

    auto innerErrorCode = std::make_shared<int>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->TerminateSelf();
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode == 0) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };
    
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    napi_value lastParam = (argc > ARGC_ZERO) ? argv[ARGC_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnTerminateSelf", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::StartAbilityForResult(napi_env env, napi_callback_info info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(env, info);
    return (me != nullptr) ? me->OnStartAbilityForResult(env, info) : nullptr;
}

napi_value JsAbilityContext::OnStartAbilityForResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called.");
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < ARGC_TWO) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    AppExecFwk::UnwrapWant(env, argv[ARGC_ZERO], want);
    decltype(argc) unwrapArgc = 1;

    napi_valuetype paramsType[argc];
    for (size_t i = ARGC_ZERO; i < argc; i++) {
        NAPI_CALL(env, napi_typeof(env, argv[i], &paramsType[i]));
    }
    if (paramsType[argc - ARGC_TWO] == napi_object && paramsType[argc - ARGC_ONE] == napi_function) {
        unwrapArgc = ARGC_TWO;
    }
    auto lastParam = (argc > unwrapArgc) ? argv[unwrapArgc] : nullptr;
    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);
    RuntimeTask task = [env, asyncTask](int32_t resultCode, const AAFwk::Want& resultWant) {
        HILOG_INFO("OnStartAbilityForResult async callback is called");
        napi_value abilityResult = JsAbilityContext::WrapAbilityResult(env, resultCode, resultWant);
        if (abilityResult == nullptr) {
            HILOG_WARN("wrap abilityResult failed");
            asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INNER));
        } else {
            asyncTask->Resolve(env, abilityResult);
        }
    };
    auto context = context_.lock();
    if (context == nullptr) {
        HILOG_WARN("context is released");
        asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    } else {
        curRequestCode_ = (curRequestCode_ == INT_MAX) ? 0 : (curRequestCode_ + 1);
        context->StartAbilityForResult(want, curRequestCode_, std::move(task));
    }
    return result;
}

napi_value JsAbilityContext::TerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(env, info);
    return (me != nullptr) ? me->OnTerminateSelfWithResult(env, info) : nullptr;
}

napi_value JsAbilityContext::OnTerminateSelfWithResult(napi_env env, napi_callback_info info)
{
    HILOG_INFO("called.");
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc == 0) {
        HILOG_ERROR("Not enough params");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int32_t resultCode = 0;
    AAFwk::Want resultWant;
    if (!JsAbilityContext::UnWrapAbilityResult(env, argv[0], resultCode, resultWant)) {
        HILOG_ERROR("Failed to parse ability result!");
        ThrowError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return CreateJsUndefined(env);
    }

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, resultWant, resultCode, innerErrorCode]() {
        auto context = weak.lock();
        if (!context) {
            HILOG_WARN("context is released");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->TerminateAbilityWithResult(resultWant, resultCode);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask &task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(env,CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = (argc > ARGC_ONE) ? argv[ARGC_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnTerminateSelfWithResult", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAbilityContext::WrapAbilityResult(
    napi_env env, int32_t resultCode, const AAFwk::Want& resultWant)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        HILOG_ERROR("abilityResult is nullptr");
        return objValue;
    }
    napi_set_named_property(env, objValue, "resultCode", CreateJsValue(env, resultCode));
    napi_set_named_property(env, objValue, "want", AppExecFwk::CreateJsWant(env, resultWant));
    return objValue;
}

bool JsAbilityContext::UnWrapAbilityResult(
    napi_env env, napi_value jObj, int32_t& resultCode, AAFwk::Want& resultWant)
{

    if (jObj == nullptr) {
        HILOG_ERROR("jObj is nullptr");
        return false;
    }
    napi_value jResultCode = nullptr;
    napi_get_named_property(env, jObj, "resultCode", &jResultCode);
    if (jResultCode == nullptr) {
        HILOG_ERROR("jResultCode is nullptr");
        return false;
    }

    napi_value jsProNameList = nullptr;
    napi_get_named_property(env, jObj, "resultCode", &jsProNameList);
    ConvertFromJsNumber(env, jsProNameList, resultCode);
    
    
    napi_value jWant = nullptr;
    napi_get_named_property(env, jObj, "want", &jWant);
    if (jWant == nullptr) {
        HILOG_ERROR("jWant is nullptr");
        return false;
    }

    return AppExecFwk::UnwrapWant(env, jWant, resultWant);
}

napi_value CreateJsAbilityContext(napi_env env, const std::shared_ptr<AbilityContext> &context)
{
    if (context == nullptr) {
        HILOG_ERROR("context is nullptr");
        return nullptr;
    }

    napi_value object = CreateJsBaseContext(env, context);

    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return object;
    }

    std::unique_ptr<JsAbilityContext> jsContext = std::make_unique<JsAbilityContext>(context);
    if (jsContext == nullptr) {
        HILOG_ERROR("jsContext is nullptr");
        return object;
    }

    napi_wrap(env, object, jsContext.release(), JsAbilityContext::Finalizer, nullptr, nullptr);

    auto abilityInfo = context->GetAbilityInfo();
    if (abilityInfo != nullptr) {
        napi_set_named_property(env, object, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo));
    }
    auto config = context->GetConfiguration();
    if (config != nullptr) {
        napi_set_named_property(env, object, "config", CreateJsConfiguration(env, *config));
    }
    auto windowStage = context->GetJsWindowStage();
    if (windowStage != nullptr) {
        napi_value stageValue = windowStage->GetNapiValue();
        napi_set_named_property(env, object, "windowStage", stageValue);
    }

    const char* moduleName = "JsAbilityContext";
    BindNativeFunction(env, object, "startAbility", moduleName, JsAbilityContext::StartAbility);
    BindNativeFunction(env, object, "terminateSelf", moduleName, JsAbilityContext::TerminateSelf);
    BindNativeFunction(env, object, "startAbilityForResult", moduleName, JsAbilityContext::StartAbilityForResult);
    BindNativeFunction(
        env, object, "terminateSelfWithResult", moduleName, JsAbilityContext::TerminateSelfWithResult);
    BindNativeFunction(env, object, "reportDrawnCompleted", moduleName, JsAbilityContext::ReportDrawnCompleted);
    return object;
}

void JsAbilityContext::ConfigurationUpdated(
    napi_env env, std::shared_ptr<NativeReference>& jsContext, const std::shared_ptr<Configuration>& config)
{
    HILOG_INFO("ConfigurationUpdated called.");
    if ((jsContext == nullptr) || (config == nullptr)) {
        HILOG_ERROR("jsContext or config is nullptr.");
        return;
    }

    napi_value object = jsContext->GetNapiValue();
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, object, "onUpdateConfiguration", &method);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsAbilityContext call onUpdateConfiguration.");
    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_call_function(env, object, method, 1, argv, nullptr);
}

napi_value JsAbilityContext::ReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    JsAbilityContext* me = CheckParamsAndGetThis<JsAbilityContext>(env, info);
    return (me != nullptr) ? me->OnReportDrawnCompleted(env, info) : nullptr;
}

napi_value JsAbilityContext::OnReportDrawnCompleted(napi_env env, napi_callback_info info)
{
    HILOG_INFO("Called.");
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, innerErrorCode]() {
        auto context = weak.lock();
        if (context == nullptr) {
            HILOG_ERROR("Context is released.");
            *innerErrorCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }

        *innerErrorCode = context->ReportDrawnCompleted();
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrorCode](napi_env env, NapiAsyncTask& task, int32_t status) {
        if (*innerErrorCode == ERR_OK) {
            task.Resolve(env, CreateJsUndefined(env));
        } else {
            task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrorCode));
        }
    };

    napi_value lastParam = (argc > ARGC_ZERO) ? argv[ARGC_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAbilityContext::OnReportDrawnCompleted", env,
        CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
