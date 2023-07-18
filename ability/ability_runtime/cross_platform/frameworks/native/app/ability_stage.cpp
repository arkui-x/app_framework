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

#include "ability_stage.h"

#include "ability.h"
#include "ability_context_impl.h"
#include "ability_info.h"
#include "base/log/ace_trace.h"
#include "bundle_container.h"
#include "context.h"
#include "hilog.h"
#include "js_ability_stage.h"
#include "runtime.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AbilityRuntime {
class Runtime;

namespace Platform {
std::shared_ptr<AbilityStage> AbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (!runtime) {
        return std::make_shared<AbilityStage>();
    }

    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsAbilityStage::Create(runtime, hapModuleInfo);

        default:
            return std::make_shared<AbilityStage>();
    }
}

void AbilityStage::OnCreate() const
{
    HILOG_INFO("AbilityStage OnCreate");
}

void AbilityStage::OnDestroy() const
{
    HILOG_INFO("AbilityStage::OnDestroy come");
}

void AbilityStage::OnConfigurationUpdate(const Configuration& configuration)
{
    HILOG_INFO("AbilityStage::OnConfigurationUpdate called.");
    for (auto ability : abilities_) {
        if (ability.second == nullptr) {
            HILOG_ERROR("stage is nullptr");
            continue;
        }
        ability.second->OnConfigurationUpdate(configuration);
    }
}

void AbilityStage::Init(const std::shared_ptr<Context>& context)
{
    stageContext_ = context;
}

std::shared_ptr<Context> AbilityStage::GetContext() const
{
    return stageContext_;
}

void AbilityStage::LaunchAbility(const AAFwk::Want& want, const std::unique_ptr<Runtime>& runtime)
{
    HILOG_INFO("AbilityStage::LaunchAbility called.");
    Ace::AceScopedTrace trace("LaunchAbility");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability != nullptr) {
        HILOG_ERROR("ability is already exist");
        return;
    }

    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }

    auto abilityInfo = bundleContainer_->GetAbilityInfo(want.GetModuleName(), want.GetAbilityName());
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }
    abilityInfo->hapPath = "arkui-x/" + abilityInfo->moduleName + "/";
    HILOG_INFO("LaunchAbility ability name: %{public}s, hapPath: %{public}s", abilityInfo->name.c_str(),
        abilityInfo->hapPath.c_str());
    abilityInfo->instanceId = std::stoi(want.GetStringParam(Want::ABILITY_ID));
    HILOG_INFO("LaunchAbility ability instanceId: %{public}d", abilityInfo->instanceId);

    auto abilityContext = std::make_shared<AbilityContextImpl>();
    if (abilityContext == nullptr) {
        HILOG_ERROR("abilityContext is nullptr");
        return;
    }
    abilityContext->SetAbilityStageContext(stageContext_);
    abilityContext->SetAbilityInfo(abilityInfo);
    abilityContext->SetInstanceName(want.GetStringParam(Want::INSTANCE_NAME));

    auto newAbility = Ability::Create(runtime);
    if (newAbility == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    newAbility->SetAbilityContext(abilityContext);
    newAbility->Init(abilityInfo);
    newAbility->SetInstanceName(want.GetStringParam(Want::INSTANCE_NAME));
    newAbility->OnCreate(want);

    abilities_.emplace(abilityName, newAbility);
}

void AbilityStage::DispatchOnNewWant(const AAFwk::Want& want)
{
    HILOG_INFO("AbilityStage::DispatchOnNewWant called.");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    auto getWant = ability->GetWant();
    if (getWant == nullptr) {
        HILOG_ERROR("want is nullptr");
        return;
    }
    ability->OnNewWant(*getWant);
}

void AbilityStage::DispatchOnForeground(const AAFwk::Want& want)
{
    HILOG_INFO("AbilityStage::DispatchOnForeground called.");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }

    auto getWant = ability->GetWant();
    if (getWant == nullptr) {
        HILOG_ERROR("want is nullptr");
        return;
    }
    ability->OnForeground(*getWant);
}

void AbilityStage::DispatchOnBackground(const AAFwk::Want& want)
{
    HILOG_INFO("AbilityStage::DispatchOnBackground called.");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    ability->OnBackground();
}

void AbilityStage::DispatchOnDestroy(const AAFwk::Want& want)
{
    HILOG_INFO("AbilityStage::DispatchOnDestroy called.");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    ability->OnDestory();
    abilities_.erase(abilityName);
}

std::shared_ptr<Ability> AbilityStage::FindAbility(const std::string& abilityName)
{
    HILOG_INFO("AbilityStage::FindAbility, abilityName: %{public}s.", abilityName.c_str());
    auto finder = abilities_.find(abilityName);
    if (finder != abilities_.end()) {
        return finder->second;
    }
    return nullptr;
}

bool AbilityStage::IsEmpty()
{
    return abilities_.empty();
}

void AbilityStage::DispatchOnAbilityResult(
    const AAFwk::Want& want, int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant)
{
    HILOG_INFO("called.");
    auto abilityName = want.GetAbilityName() + want.GetStringParam(Want::ABILITY_ID);
    auto ability = FindAbility(abilityName);
    if (ability == nullptr) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    ability->OnAbilityResult(requestCode, resultCode, resultWant);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
