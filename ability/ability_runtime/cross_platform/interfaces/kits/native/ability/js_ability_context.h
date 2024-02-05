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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_CONTEXT_H

#include <native_engine/native_engine.h>

#include "ability_business_error.h"
#include "ability_context.h"

class NativeObject;
class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class JsAbilityContext final {
public:
    explicit JsAbilityContext(const std::shared_ptr<AbilityContext>& context) : context_(context) {}
    ~JsAbilityContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value TerminateSelf(napi_env env, napi_callback_info info);
    static void ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference>& jsContext,
        const std::shared_ptr<Configuration>& config);
    static napi_value StartAbilityForResult(napi_env env, napi_callback_info info);
    static napi_value TerminateSelfWithResult(napi_env env, napi_callback_info info);
    static napi_value ReportDrawnCompleted(napi_env env, napi_callback_info info);

private:
    napi_value OnStartAbility(napi_env env, napi_callback_info info);
    napi_value OnTerminateSelf(napi_env env, napi_callback_info info);
    napi_value OnStartAbilityForResult(napi_env env, napi_callback_info info);
    napi_value OnTerminateSelfWithResult(napi_env env, napi_callback_info info);
    napi_value OnReportDrawnCompleted(napi_env env, napi_callback_info info);
    static napi_value WrapAbilityResult(napi_env env, int32_t resultCode, const AAFwk::Want& resultWant);
    bool UnWrapAbilityResult(napi_env env, napi_value argv, int32_t& resultCode, AAFwk::Want& want);
    std::weak_ptr<AbilityContext> context_;
    int32_t curRequestCode_ = 0;
};
napi_value CreateJsAbilityContext(napi_env env, const std::shared_ptr<AbilityContext>& context);
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_CONTEXT_H
