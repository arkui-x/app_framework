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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_H

#include <memory>

#include "ability.h"
#include "ability_delegator_infos.h"
#include "ability_info.h"
#include "application.h"
#include "napi/native_api.h"

class NativeReference;
class NativeValue;
namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
namespace Platform {

using Want = AAFwk::Want;

class JsAbility : public Ability {
public:
    static std::shared_ptr<Ability> Create(const std::unique_ptr<Runtime>& runtime);

    explicit JsAbility(JsRuntime& jsRuntime);
    ~JsAbility() override;

    void Init(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo) override;
    void OnCreate(const Want& want) override;
    void OnDestory() override;
    void OnNewWant(const Want& want) override;
    void OnForeground(const Want& want) override;
    void OnBackground() override;
    void OnWindowStageCreated() override;
    void OnWindowStageDestroy() override;
    std::unique_ptr<NativeReference> CreateJsWindowStage();
    void OnConfigurationUpdate(const Configuration& configuration) override;
    void OnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant) override;

private:
    void CallObjectMethod(const char* name, napi_value const* argv = nullptr, size_t argc = 0);
    void CallPostPerformStart();
    std::shared_ptr<AppExecFwk::ADelegatorAbilityProperty> CreateADelegatorAbilityProperty();
    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsAbilityObj_;
    std::shared_ptr<NativeReference> shellContextRef_;
    std::shared_ptr<NativeReference> jsWindowStageObj_;
    bool isDispatchOnWindowStageCreated_ = false;
    std::string moduleName_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_JS_ABILITY_H
