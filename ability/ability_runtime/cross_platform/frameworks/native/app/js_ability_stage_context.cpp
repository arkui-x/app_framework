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

#include "js_ability_stage_context.h"

#include "hilog.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

void JsAbilityStageContext::Finalizer(napi_env env, void* data, void* hint)
{
    HILOG_INFO("JsAbilityStageContext::Finalizer is called");
    std::unique_ptr<JsAbilityStageContext>(static_cast<JsAbilityStageContext*>(data));
}

napi_value CreateJsAbilityStageContext(
    napi_env env, const std::shared_ptr<Context>& context, DetachCallback detach, NapiAttachCallback attach)
{
    HILOG_DEBUG("called.");
    napi_value objValue = CreateJsBaseContext(env, context);
    if (context == nullptr) {
        return objValue;
    }
    auto configuration = context->GetConfiguration();
    if (configuration != nullptr && objValue != nullptr) {
        napi_set_named_property(env, objValue, "config",
            CreateJsConfiguration(env, *configuration));
    }
    return objValue;
}

void JsAbilityStageContext::ConfigurationUpdated(
    napi_env env, std::shared_ptr<NativeReference>& jsContext, const std::shared_ptr<Configuration>& config)
{
    HILOG_INFO("ConfigurationUpdated called.");
    if ((jsContext == nullptr) || (config == nullptr)) {
        HILOG_ERROR("jsContext or config is nullptr.");
        return;
    }

    napi_value value = jsContext->GetNapiValue();
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr.");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, value, "onUpdateConfiguration", &method);
    if (!method) {
        HILOG_ERROR("Failed to get onUpdateConfiguration from object");
        return;
    }

    HILOG_INFO("JsAbilityStageContext call onUpdateConfiguration.");
    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_value callResult = nullptr;
    napi_call_function(env, value, method, 1, argv, &callResult);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS