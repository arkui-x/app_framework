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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_H

#include <map>
#include <memory>
#include <string>

#include "bundle_container.h"
#include "configuration.h"
#include "hap_module_info.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
namespace Platform {
class Ability;
class Context;

class AbilityStage {
public:
    static std::shared_ptr<AbilityStage> Create(
        const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo);

    AbilityStage() = default;
    virtual ~AbilityStage() = default;
    virtual void OnCreate() const;
    virtual void OnDestroy() const;
    virtual void OnConfigurationUpdate(const Configuration& configuration);
    virtual void Init(const std::shared_ptr<Context>& context);
    std::shared_ptr<Context> GetContext() const;
    void LaunchAbility(const AAFwk::Want& want, const std::unique_ptr<Runtime>& runtime);
    void SetBundleContainer(const std::shared_ptr<AppExecFwk::BundleContainer>& bundleContainer)
    {
        bundleContainer_ = bundleContainer;
    }

    void DispatchOnNewWant(const AAFwk::Want& want);
    void DispatchOnForeground(const AAFwk::Want& want);
    void DispatchOnBackground(const AAFwk::Want& want);
    void DispatchOnDestroy(const AAFwk::Want& want);
    bool IsEmpty();
    void DispatchOnAbilityResult(
        const AAFwk::Want& want, int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant);

private:
    std::shared_ptr<Ability> FindAbility(const std::string& abilityName);

private:
    std::shared_ptr<Context> stageContext_;
    std::shared_ptr<AppExecFwk::BundleContainer> bundleContainer_ = nullptr;
    std::map<std::string, std::shared_ptr<Ability>> abilities_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_H
