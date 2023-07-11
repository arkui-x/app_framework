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

#include "ability_stage_context.h"

#include "hilog.h"
#include "stage_application_info_adapter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::string AbilityStageContext::GetBundleName() const
{
    return applicationContext_ ? applicationContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityStageContext::GetApplicationInfo() const
{
    return applicationContext_ ? applicationContext_->GetApplicationInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityStageContext::GetResourceManager() const
{
    return resourceManager_;
}

std::string AbilityStageContext::GetBundleCodePath() const
{
    return applicationContext_ ? applicationContext_->GetBundleCodePath() : "";
}

std::string AbilityStageContext::GetBundleCodeDir()
{
    return applicationContext_ ? applicationContext_->GetBundleCodeDir() : "";
}

std::string AbilityStageContext::GetCacheDir()
{
    return applicationContext_ ? applicationContext_->GetCacheDir() : "";
}

std::string AbilityStageContext::GetTempDir()
{
    return applicationContext_ ? applicationContext_->GetTempDir() : "";
}

std::string AbilityStageContext::GetFilesDir()
{
    return applicationContext_ ? applicationContext_->GetFilesDir() : "";
}

std::string AbilityStageContext::GetDatabaseDir()
{
    return applicationContext_ ? applicationContext_->GetDatabaseDir() : "";
}

std::string AbilityStageContext::GetPreferencesDir()
{
    return applicationContext_ ? applicationContext_->GetPreferencesDir() : "";
}

int AbilityStageContext::GetSystemDatabaseDir(std::string groupId, std::string &databaseDir)
{
    return applicationContext_ ?
        applicationContext_->GetSystemDatabaseDir(groupId, databaseDir) : -1;
}

int AbilityStageContext::GetSystemPreferencesDir(std::string groupId, std::string &preferencesDir)
{
    return applicationContext_ ?
        applicationContext_->GetSystemPreferencesDir(groupId, preferencesDir) : -1;
}

std::shared_ptr<Context> AbilityStageContext::CreateModuleContext(const std::string &moduleName)
{
    return applicationContext_ ? applicationContext_->CreateModuleContext(moduleName) : nullptr;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityStageContext::GetHapModuleInfo() const
{
    return hapModuleInfo_;
}

void AbilityStageContext::SetHapModuleInfo(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo)
{
    hapModuleInfo_ = hapModuleInfo;
}

void AbilityStageContext::SetApplicationContext(const std::shared_ptr<Context>& applicationContext)
{
    applicationContext_ = applicationContext;
}

void AbilityStageContext::SetConfiguration(const std::shared_ptr<Configuration>& configuration)
{
    configuration_ = configuration;
}

std::shared_ptr<Configuration> AbilityStageContext::GetConfiguration()
{
    return configuration_;
}

void AbilityStageContext::InitResourceManeger()
{
    resourceManager_ = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    if (resourceManager_ == nullptr) {
        HILOG_ERROR("resourceManager_ is nullptr");
        return;
    }
    if (hapModuleInfo_ == nullptr) {
        HILOG_ERROR("hapModuleInfo_ is nullptr");
        return;
    }
    std::string moduleName = hapModuleInfo_->moduleName;
    StageAssetManager::GetInstance()->GetResIndexPath(moduleName, appResourcePath_, sysResourcePath_);
    auto appResRet = resourceManager_->AddResource(appResourcePath_.c_str());
    if (!appResRet) {
        HILOG_ERROR("Add app resource failed");
    }
    auto sysResRet = resourceManager_->AddResource(sysResourcePath_.c_str());
    if (!sysResRet) {
        HILOG_ERROR("Add system resource failed");
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
    if (configuration_ != nullptr) {
        auto colorMode = configuration_->GetItem(ConfigurationInner::SYSTEM_COLORMODE);
        Global::Resource::ColorMode mode;
        if (colorMode == ConfigurationInner::COLOR_MODE_LIGHT) {
            mode = Global::Resource::ColorMode::LIGHT;
        } else if (colorMode == ConfigurationInner::COLOR_MODE_DARK) {
            mode = Global::Resource::ColorMode::DARK;
        } else {
            mode = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;
        }
        resConfig->SetColorMode(mode);
        auto direction = configuration_->GetItem(ConfigurationInner::APPLICATION_DIRECTION);
        if (direction == ConfigurationInner::DIRECTION_VERTICAL) {
            resConfig->SetDirection(Global::Resource::Direction::DIRECTION_VERTICAL);
        } else if (direction == ConfigurationInner::DIRECTION_HORIZONTAL) {
            resConfig->SetDirection(Global::Resource::Direction::DIRECTION_HORIZONTAL);
        }
        auto densityDpi = configuration_->GetItem(
            OHOS::AbilityRuntime::Platform::ConfigurationInner::APPLICATION_DENSITYDPI);
        if (!densityDpi.empty()) {
            double density = std::stoi(densityDpi) / 160.0f;
            resConfig->SetScreenDensity(density);
        }
    }
    resourceManager_->UpdateResConfig(*resConfig);
}

void AbilityStageContext::GetResourcePaths(std::string& appResourcePath, std::string& sysResourcePath)
{
    HILOG_INFO("Get resource paths");
    auto appFisrtPos = appResourcePath_.find_last_of('/');
    appResourcePath = appResourcePath_.substr(0, appFisrtPos);

    auto sysFisrtPos = sysResourcePath_.find_last_of('/');
    sysResourcePath = sysResourcePath_.substr(0, sysFisrtPos);
    HILOG_INFO(
        "appResourcePath: %{public}s, sysResourcePath: %{public}s", appResourcePath.c_str(), sysResourcePath.c_str());
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
