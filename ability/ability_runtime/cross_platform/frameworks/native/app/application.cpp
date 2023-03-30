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

#include "application.h"

#include "ability_stage_context.h"
#include "hilog.h"

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
    abilityStageContext->InitResourceManeger();

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

void Application::OnConfigurationUpdate(const Configuration& configuration)
{
    HILOG_INFO("Application::OnConfigurationUpdate");
    if (configuration_ == nullptr) {
        HILOG_ERROR("configuration_ is nullptr");
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
}

std::shared_ptr<AbilityStage> Application::FindAbilityStage(const std::string& moduleName)
{
    auto iter = abilityStages_.find(moduleName);
    if (iter != abilityStages_.end()) {
        return iter->second;
    }
    return nullptr;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS