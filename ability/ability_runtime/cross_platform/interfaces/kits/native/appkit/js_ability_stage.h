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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_H

#include "ability_delegator_infos.h"
#include "ability_stage.h"
#include "native_engine/native_value.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
namespace Platform {
class Context;
class JsAbilityStage : public AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj);
    ~JsAbilityStage() override;

    void Init(const std::shared_ptr<Context>& context) override;
    void OnCreate() const override;
    void OnConfigurationUpdate(const Configuration& configuration) override;

    std::shared_ptr<OHOS::AppExecFwk::DelegatorAbilityStageProperty> CreateStageProperty() const;
    std::string GetHapModuleProp(const std::string &propName) const;
    
    static bool UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo);

    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsAbilityStageObj_;
    std::shared_ptr<NativeReference> shellContextRef_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ABILITY_STAGE_H
