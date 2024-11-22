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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_CONTEXT_H

#include <native_engine/native_engine.h>

#include "ability_context.h"

namespace OHOS {
namespace AbilityRuntime {

namespace Platform {
class Context;
class JsAbilityStageContext final {
public:
    explicit JsAbilityStageContext(const std::shared_ptr<Context>& context) : context_(context) {}
    ~JsAbilityStageContext() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static void ConfigurationUpdated(napi_env env, std::shared_ptr<NativeReference>& jsContext,
        const std::shared_ptr<Configuration>& config);

private:
    std::weak_ptr<Context> context_;
};

napi_value CreateJsAbilityStageContext(
    napi_env env, const std::shared_ptr<Context>& context, DetachCallback detach, NapiAttachCallback attach);
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_CONTEXT_H