/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "application.h"

#include "ability_stage_context.h"
#include "hilog.h"
#include "base/log/ace_trace.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
Application::Application() {}

Application::~Application() {}

void Application::SetRuntime(std::unique_ptr<AbilityRuntime::Runtime>&& runtime)
{
    runtime_ = std::move(runtime);
}

const std::unique_ptr<AbilityRuntime::Runtime>& Application::GetRuntime()
{
    return runtime_;
}

void Application::SetApplicationContext(const std::shared_ptr<ApplicationContext>& applicationContext)
{
    HILOG_INFO("Application::SetApplicationContext");
    if (applicationContext == nullptr) {
        HILOG_ERROR("SetApplicationContext failed, context is empty");
        return;
    }
    applicationContext_ = applicationContext;
}

void Application::HandleAbilityStage(const AAFwk::Want& want)
{
    HILOG_INFO("Application::HandleAbilityStage");
    Ace::AceScopedTrace trace("HandleAbilityStage");
    if (applicationContext_ == nullptr) {
        HILOG_ERROR("abilityRuntimeContext_ is nullptr.");
        return;
    }

    if (runtime_ == nullptr) {
        HILOG_ERROR("runtime_ is nullptr.");
        return;
    }

    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage != nullptr) {
        stage->LaunchAbility(want, runtime_);
        return;
    }

    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    if (abilityStageContext == nullptr) {
        HILOG_ERROR("abilityStageContext is nullptr");
        return;
    }
    abilityStageContext->SetApplicationContext(applicationContext_);

    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }
    auto hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
    if (hapModuleInfo == nullptr) {
        HILOG_ERROR("hapModuleInfo is nullptr, moduleName: %{public}s", moduleName.c_str());
        return;
    }
    abilityStageContext->SetHapModuleInfo(hapModuleInfo);
    abilityStageContext->SetConfiguration(configuration_);
    auto rmg = applicationContext_->GetResourceManager();
    if (rmg != nullptr) {
        abilityStageContext->SetResourceManager(rmg);
    }
    abilityStageContext->InitResourceManeger();
    applicationContext_->SetResourceManager(abilityStageContext->GetResourceManager());

    auto abilityStage = AbilityRuntime::Platform::AbilityStage::Create(runtime_, *hapModuleInfo);
    abilityStage->Init(abilityStageContext);
    abilityStage->SetBundleContainer(bundleContainer_);
    abilityStage->OnCreate();
    abilityStage->LaunchAbility(want, runtime_);

    abilityStages_.emplace(moduleName, abilityStage);
}

void Application::DispatchOnNewWant(const AAFwk::Want& want)
{
    HILOG_INFO("Application::DispatchOnNewWant");
    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    stage->DispatchOnNewWant(want);
}

void Application::DispatchOnForeground(const AAFwk::Want& want)
{
    HILOG_INFO("Application::DispatchOnForeground");
    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    stage->DispatchOnForeground(want);
}

void Application::DispatchOnBackground(const AAFwk::Want& want)
{
    HILOG_INFO("Application::DispatchOnBackground");
    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    stage->DispatchOnBackground(want);
}

void Application::DispatchOnDestroy(const AAFwk::Want& want)
{
    HILOG_INFO("Application::DispatchOnDestroy");
    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    stage->DispatchOnDestroy(want);

    if (stage->IsEmpty()) {
        HILOG_INFO("ability stage is empty");
        abilityStages_.erase(moduleName);
    }
}

void Application::OnConfigurationUpdate(Configuration& configuration, SetLevel level)
{
    HILOG_INFO("Application::OnConfigurationUpdate");
    if (configuration_ == nullptr) {
        HILOG_ERROR("configuration_ is nullptr");
        return;
    }

    bool isUpdateAppColor = IsUpdateColorNeeded(configuration, level);
    bool isUpdateAppFontSize = IsUpdateFontSizeNeeded(configuration, level);
    bool isUpdateAppLanguage = IsUpdateLanguageNeeded(configuration, level);
    if (!isUpdateAppColor && !isUpdateAppFontSize && !isUpdateAppLanguage && configuration.GetItemSize() == 0) {
        HILOG_DEBUG("configuration need not updated");
        return;
    }

    configuration_->UpdateConfigurationInfo(configuration);

    for (auto stage : abilityStages_) {
        if (stage.second == nullptr) {
            HILOG_ERROR("stage is nullptr");
            continue;
        }
        stage.second->OnConfigurationUpdate(configuration);
    }
}

void Application::InitConfiguration(const Configuration& configuration)
{
    if (configuration_ != nullptr) {
        HILOG_INFO("configuration_ is exist");
        return;
    }
    configuration_ = std::make_shared<Configuration>(configuration);
    auto colorMode = configuration.GetItem(ConfigurationInner::SYSTEM_COLORMODE);
    ApplicationConfigurationManager::GetInstance().SetColorModeSetLevel(SetLevel::System, colorMode);

    auto applicationContext = ApplicationContext::GetInstance();
    applicationContext->SetConfiguration(configuration_);

    auto application = std::static_pointer_cast<Application>(shared_from_this());
    std::weak_ptr<Application> applicationWeak = application;
    auto appConfigUpdateCallback = [applicationWeak](Configuration& configuration) {
        auto application = applicationWeak.lock();
        if (application == nullptr) {
            HILOG_ERROR("application is nullptr.");
            return;
        }
        application->OnConfigurationUpdate(configuration, SetLevel::Application);
    };
    applicationContext->RegisterAppConfigUpdateCallback(appConfigUpdateCallback);
}

std::shared_ptr<AbilityStage> Application::FindAbilityStage(const std::string& moduleName)
{
    auto iter = abilityStages_.find(moduleName);
    if (iter != abilityStages_.end()) {
        return iter->second;
    }
    return nullptr;
}

void Application::DispatchOnAbilityResult(
    const AAFwk::Want& want, int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant)
{
    HILOG_INFO("called");
    std::string moduleName = want.GetModuleName();
    auto stage = FindAbilityStage(moduleName);
    if (stage == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    stage->DispatchOnAbilityResult(want, requestCode, resultCode, resultWant);
}

void Application::NotifyApplicationForeground()
{
    if (applicationContext_ == nullptr) {
        HILOG_ERROR("stage is nullptr");
        return;
    }
    isForeground_ = true;
    applicationContext_->NotifyApplicationForeground();
}

void Application::NotifyApplicationBackground()
{
    if (applicationContext_ == nullptr) {
        HILOG_ERROR("applicationContext is nullptr");
        return;
    }
    isForeground_ = false;
    applicationContext_->NotifyApplicationBackground();
}

bool Application::IsForegroud()
{
    return isForeground_;
}

bool Application::IsUpdateColorNeeded(Configuration& config, SetLevel level)
{
    std::string colorMode = config.GetItem(ConfigurationInner::SYSTEM_COLORMODE);
    std::string colorModeIsSetBySa = config.GetItem(ConfigurationInner::COLORMODE_IS_SET_BY_SA);
    if (level < SetLevel::SA && !colorModeIsSetBySa.empty()) {
        level = SetLevel::SA;
    }

    HILOG_DEBUG("current %{public}d, pre %{public}d", static_cast<uint8_t>(level),
        static_cast<uint8_t>(ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel()));

    bool needUpdate = true;

    if (level < ApplicationConfigurationManager::GetInstance().GetColorModeSetLevel() || colorMode.empty()) {
        config.RemoveItem(ConfigurationInner::SYSTEM_COLORMODE);
        config.RemoveItem(ConfigurationInner::COLORMODE_IS_SET_BY_SA);
        HILOG_DEBUG("color remove");
        needUpdate = false;
    }

    if (!colorMode.empty()) {
        config.AddItem(ConfigurationInner::SYSTEM_COLORMODE,
            ApplicationConfigurationManager::GetInstance().SetColorModeSetLevel(level, colorMode));

        if (level > SetLevel::System) {
            config.AddItem(ConfigurationInner::COLORMODE_IS_SET_BY_APP, ConfigurationInner::IS_SET_BY_APP);
        }
    }

    return needUpdate;
}

bool Application::IsUpdateFontSizeNeeded(Configuration &config, SetLevel level)
{
    std::string fontSize = config.GetItem(ConfigurationInner::SYSTEM_FONT_SIZE_SCALE);
    if (fontSize.empty()) {
        return false;
    }

    auto preLevel = ApplicationConfigurationManager::GetInstance().GetFontSizeSetLevel();
    if (level < preLevel) {
        config.RemoveItem(ConfigurationInner::SYSTEM_FONT_SIZE_SCALE);
        return false;
    }

    std::string fontFollowSystem = configuration_->GetItem(ConfigurationInner::APP_FONT_SIZE_SCALE);
    if (level == preLevel && !fontFollowSystem.empty()) {
        if (fontFollowSystem.compare(ConfigurationInner::IS_APP_FONT_FOLLOW_SYSTEM) == 0) {
            return true;
        }
        config.RemoveItem(ConfigurationInner::SYSTEM_FONT_SIZE_SCALE);
        return false;
    }
    configuration_->RemoveItem(ConfigurationInner::APP_FONT_SIZE_SCALE);
    ApplicationConfigurationManager::GetInstance().SetFontSizeSetLevel(level);
    return true;
}

bool Application::IsUpdateLanguageNeeded(Configuration &config, SetLevel level)
{
    std::string language = config.GetItem(ConfigurationInner::APPLICATION_LANGUAGE);
    if (language.empty()) {
        return false;
    }
    auto preLevel = ApplicationConfigurationManager::GetInstance().GetLanguageSetLevel();
    if (level < preLevel) {
        config.RemoveItem(ConfigurationInner::APPLICATION_LANGUAGE);
        return false;
    }
    ApplicationConfigurationManager::GetInstance().SetLanguageSetLevel(level);
    return true;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS