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
#include "js_want_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
constexpr size_t ARGC_ZERO = 0;
}

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
    UnwrapJsWant(engine, info.argv[0], want);
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
