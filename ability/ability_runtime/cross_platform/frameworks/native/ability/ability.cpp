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

#include "ability.h"

#include "hilog.h"
#include "js_ability.h"
#include "runtime.h"
#include "window_view_adapter.h"
#include "stage_application_info_adapter.h"

using namespace OHOS::AppExecFwk;
namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<Ability> Ability::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    HILOG_INFO("Ability create.");
    if (!runtime) {
        return std::make_shared<Ability>();
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return AbilityRuntime::Platform::JsAbility::Create(runtime);

        default:
            return std::make_shared<Ability>();
    }
}

void Ability::Init(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    HILOG_INFO("Init begin.");
    abilityInfo_ = abilityInfo;

    abilityLifecycleExecutor_ = std::make_shared<AbilityLifecycleExecutor>();
    if (abilityLifecycleExecutor_ != nullptr) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INITIAL);
    } else {
        HILOG_ERROR("abilityLifecycleExecutor_ make failed.");
    }
}

void Ability::OnCreate(const Want& want)
{
    HILOG_INFO("OnCreate begin.");
    SetWant(want);

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnStart error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (!abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::STARTED_NEW);
    }
}

void Ability::OnNewWant(const Want& want)
{
    HILOG_INFO("OnNewWant begin.");
}

void Ability::OnForeground(const Want& want)
{
    HILOG_INFO("OnForeground begin.");
    DispatchLifecycleOnForeground(want);
}

void Ability::DispatchLifecycleOnForeground(const Want &want)
{
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnForeground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    if (abilityInfo_ != nullptr && abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::INACTIVE);
    }
}

void Ability::OnBackground()
{
    HILOG_INFO("OnBackground begin.");
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("abilityInfo_ is nullptr.");
        return;
    }
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnBackground error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }

    if (abilityInfo_->isStageBasedModel) {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW);
    } else {
        abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::BACKGROUND);
    }
}

void Ability::OnDestory()
{
    HILOG_INFO("OnDestory begin.");
    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::OnStop error. abilityLifecycleExecutor_ == nullptr.");
        return;
    }
    abilityLifecycleExecutor_->DispatchLifecycleState(AbilityLifecycleExecutor::LifecycleState::STOPED_NEW);
}

void Ability::OnWindowStageCreated()
{
    HILOG_INFO("OnWindowStageCreated begin.");
}

void Ability::OnWindowStageDestroy()
{
    HILOG_INFO("onWindowStageDestroy begin.");
    WindowViewAdapter::GetInstance()->RemoveWindowView(instanceName_);
}

void Ability::SetWant(const AAFwk::Want& want)
{
    want_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> Ability::GetWant()
{
    return want_;
}

std::string Ability::GetAbilityName()
{
    if (abilityInfo_ == nullptr) {
        HILOG_ERROR("Ability::GetAbilityName abilityInfo_ is nullptr");
        return "";
    }

    return abilityInfo_->name;
}

AbilityLifecycleExecutor::LifecycleState Ability::GetState()
{
    HILOG_DEBUG("Ability::GetState called");

    if (abilityLifecycleExecutor_ == nullptr) {
        HILOG_ERROR("Ability::GetState error. abilityLifecycleExecutor_ == nullptr.");
        return AbilityLifecycleExecutor::LifecycleState::UNINITIALIZED;
    }

    return (AbilityLifecycleExecutor::LifecycleState)abilityLifecycleExecutor_->GetState();
}

void Ability::OnConfigurationUpdate(const Configuration& configuration)
{
    HILOG_INFO("OnConfigurationUpdate begin.");
    if (abilityContext_ == nullptr) {
        HILOG_ERROR("abilityContext_ is nullptr");
        return;
    }
    auto resourceManager = abilityContext_->GetResourceManager();
    if (resourceManager == nullptr) {
        HILOG_ERROR("resourceManager is nullptr");
        return;
    }
    std::unique_ptr<Global::Resource::ResConfig> resConfig(Global::Resource::CreateResConfig());
    if (resConfig == nullptr) {
        HILOG_ERROR("resConfig is nullptr");
        return;
    }
    std::string language { "" };
    std::string country { "" };
    std::string script { "" };
    StageApplicationInfoAdapter::GetInstance()->GetLocale(language, country, script);
    resConfig->SetLocaleInfo(language.c_str(), script.c_str(), country.c_str());
    auto colorMode = configuration.GetItem(ConfigurationInner::SYSTEM_COLORMODE);
    if (colorMode != "") {
        Global::Resource::ColorMode mode;
        if (colorMode == ConfigurationInner::COLOR_MODE_LIGHT) {
            mode = Global::Resource::ColorMode::LIGHT;
        } else if (colorMode == ConfigurationInner::COLOR_MODE_DARK) {
            mode = Global::Resource::ColorMode::DARK;
        } else {
            mode = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;
        }
        resConfig->SetColorMode(mode);
    }
    resourceManager->UpdateResConfig(*resConfig);
}

const LaunchParam& Ability::GetLaunchParam() const
{
    return launchParam_;
}

void Ability::OnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant)
{
    HILOG_INFO("OnAbilityResult begin.");
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS