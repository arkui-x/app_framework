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

#include "application_context.h"
#include "ability_stage_context.h"

#include <algorithm>

#include "application_context_adapter.h"
#include "hilog.h"
#include "stage_application_info_adapter.h"
#include "unicode/locid.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::vector<std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::shared_ptr<ApplicationContext> Context::applicationContext_ = nullptr;
std::mutex Context::contextMutex_;
const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {}("Context"));
std::vector<std::shared_ptr<ApplicationStateChangeCallback>> ApplicationContext::applicationStateCallback_;

std::shared_ptr<ApplicationContext> ApplicationContext::GetInstance()
{
    if (applicationContext_ == nullptr) {
        std::lock_guard<std::mutex> lock_l(Context::contextMutex_);
        if (applicationContext_ == nullptr) {
            applicationContext_ = std::make_shared<ApplicationContext>();
        }
    }
    return applicationContext_;
}

std::string ApplicationContext::GetBundleName() const
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->bundleName : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> ApplicationContext::GetApplicationInfo() const
{
    return applicationInfo_;
}

std::shared_ptr<ApplicationContext> Context::GetApplicationContext()
{
    std::lock_guard<std::mutex> lock(contextMutex_);
    return applicationContext_;
}

void ApplicationContext::SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo>& info)
{
    if (info == nullptr) {
        HILOG_ERROR("SetApplicationInfo failed, info is empty");
        return;
    }
    applicationInfo_ = info;
}

std::shared_ptr<Global::Resource::ResourceManager> ApplicationContext::GetResourceManager() const
{
    return resourceMgr_;
}

void ApplicationContext::SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr)
{
    resourceMgr_ = resMgr;
}

std::string ApplicationContext::GetBundleCodePath() const
{
    return (applicationInfo_ != nullptr) ? applicationInfo_->codePath : "";
}

std::string ApplicationContext::GetBundleCodeDir()
{
    return StageAssetManager::GetInstance()->GetBundleCodeDir();
}

std::string ApplicationContext::GetCacheDir()
{
    return StageAssetManager::GetInstance()->GetCacheDir();
}

std::string ApplicationContext::GetTempDir()
{
    return StageAssetManager::GetInstance()->GetTempDir();
}

std::string ApplicationContext::GetFilesDir()
{
    return StageAssetManager::GetInstance()->GetFilesDir();
}

std::string ApplicationContext::GetDatabaseDir()
{
    return StageAssetManager::GetInstance()->GetDatabaseDir();
}

std::string ApplicationContext::GetPreferencesDir()
{
    return StageAssetManager::GetInstance()->GetPreferencesDir();
}

int ApplicationContext::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    std::string dir;
    if (groupId.empty()) {
        databaseDir = GetDatabaseDir();
    } else {
        HILOG_ERROR("GetSystemDatabaseDir doesn't support cross-platform");
        return -1;
    }
    HILOG_DEBUG("ApplicationContext::GetSystemDatabaseDir:%{public}s", dir.c_str());
    return 0;
}

int ApplicationContext::GetSystemPreferencesDir(
    const std::string &groupId, bool checkExist, std::string &preferencesDir)
{
    std::string dir;
    if (groupId.empty()) {
        preferencesDir = GetPreferencesDir();
    } else {
        HILOG_ERROR("GetSystemPreferencesDir doesn't support cross-platform");
        return -1;
    }
    HILOG_DEBUG("ApplicationContext::GetSystemPreferencesDir:%{public}s", dir.c_str());
    return 0;
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return nullptr;
}

void ApplicationContext::GetResourcePaths(std::string& hapResPath, std::string& sysResPath) {}

void ApplicationContext::GetResourcePaths(std::vector<std::string>& appResourcePaths, std::string& sysResourcePath) {}

std::shared_ptr<Configuration> ApplicationContext::GetConfiguration()
{
    return configuration_;
}

void ApplicationContext::SetConfiguration(const std::shared_ptr<Configuration>& configuration)
{
    configuration_ = configuration;
}

std::shared_ptr<AppExecFwk::BundleContainer> ApplicationContext::GetBundleContainer()
{
    return bundleContainer_;
}

void ApplicationContext::SetBundleContainer(const std::shared_ptr<AppExecFwk::BundleContainer>& bundleContainer)
{
    bundleContainer_ = bundleContainer;
}

std::shared_ptr<Context> ApplicationContext::CreateModuleContext(const std::string &moduleName)
{
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    if (abilityStageContext == nullptr) {
        HILOG_ERROR("abilityStageContext is nullptr");
        return nullptr;
    }
    abilityStageContext->SetApplicationContext(applicationContext_);

    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return nullptr;
    }
    auto hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
    if (hapModuleInfo == nullptr) {
        HILOG_ERROR("hapModuleInfo is nullptr, moduleName: %{public}s", moduleName.c_str());
        return nullptr;
    }

    abilityStageContext->SetHapModuleInfo(hapModuleInfo);
    abilityStageContext->SetConfiguration(GetConfiguration());
    abilityStageContext->InitResourceManeger();
    return abilityStageContext;
}

void ApplicationContext::RegisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback>& abilityLifecycleCallback)
{
    HILOG_INFO("ApplicationContext RegisterAbilityLifecycleCallback");
    if (abilityLifecycleCallback == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    callbacks_.push_back(abilityLifecycleCallback);
}

void ApplicationContext::UnregisterAbilityLifecycleCallback(
    const std::shared_ptr<AbilityLifecycleCallback>& abilityLifecycleCallback)
{
    HILOG_INFO("ApplicationContext UnregisterAbilityLifecycleCallback");
    std::lock_guard<std::mutex> lock(callbackLock_);
    auto it = std::find(callbacks_.begin(), callbacks_.end(), abilityLifecycleCallback);
    if (it != callbacks_.end()) {
        callbacks_.erase(it);
    }
}

void ApplicationContext::RegisterApplicationStateChangeCallback(
    const std::shared_ptr<ApplicationStateChangeCallback>& applicationStateChangeCallback)
{
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    applicationStateCallback_.push_back(applicationStateChangeCallback);
}

void ApplicationContext::UnRegisterApplicationStateChangeCallback(
    const std::shared_ptr<ApplicationStateChangeCallback>& applicationStateChangeCallback)
{
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    auto it = std::find(applicationStateCallback_.begin(), applicationStateCallback_.end(), applicationStateChangeCallback);
    if (it != applicationStateCallback_.end()) {
        applicationStateCallback_.erase(it);
    }
}

void ApplicationContext::RegisterAppConfigUpdateCallback(AppConfigUpdateCallback appConfigChangeCallback)
{
    appConfigChangeCallback_ = appConfigChangeCallback;
}

void ApplicationContext::DispatchOnAbilityCreate(const std::shared_ptr<NativeReference>& ability)
{
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityCreate(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference>& ability)
{
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityDestroy(ability);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageCreate(
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage)
{
    if (!ability || !windowStage) {
        HILOG_ERROR("ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageCreate(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnWindowStageDestroy(
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage)
{
    if (!ability || !windowStage) {
        HILOG_ERROR("ability or windowStage is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnWindowStageDestroy(ability, windowStage);
        }
    }
}

void ApplicationContext::DispatchOnAbilityForeground(const std::shared_ptr<NativeReference>& ability)
{
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityForeground(ability);
        }
    }
}

void ApplicationContext::DispatchOnAbilityBackground(const std::shared_ptr<NativeReference>& ability)
{
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(callbackLock_);
    for (auto callback : callbacks_) {
        if (callback != nullptr) {
            callback->OnAbilityBackground(ability);
        }
    }
}

void ApplicationContext::NotifyApplicationForeground()
{
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        if (callback != nullptr) {
            callback->NotifyApplicationForeground();
        }
    }
}

void ApplicationContext::NotifyApplicationBackground()
{
    std::lock_guard<std::mutex> lock(applicationStateCallbackLock_);
    for (auto callback : applicationStateCallback_) {
        if (callback != nullptr) {
            callback->NotifyApplicationBackground();
        }
    }
}

int32_t ApplicationContext::GetProcessRunningInformation(std::vector<RunningProcessInfo>& processInfos)
{
    processInfos = ApplicationContextAdapter::GetInstance()->GetRunningProcessInformation();
    return 0;
}

void ApplicationContext::SetColorMode(int32_t colorMode)
{
    HILOG_DEBUG("colorMode:%{public}d", colorMode);
    if (colorMode < -1 || colorMode > 1) {
        HILOG_ERROR("colorMode is invalid");
        return;
    }
    Configuration config;
    config.AddItem(ConfigurationInner::SYSTEM_COLORMODE, config.GetColorModeStr(colorMode));
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetLanguage(const std::string &language)
{
    HILOG_DEBUG("language:%{public}s", language.c_str());
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale::forLanguageTag(icu::StringPiece(language), status);
    if (status != U_ZERO_ERROR) {
        HILOG_ERROR("language is invalid");
        return;
    }
    Configuration config;
    config.AddItem(ConfigurationInner::APPLICATION_LANGUAGE, language);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

void ApplicationContext::SetFont(const std::string &font)
{
    HILOG_DEBUG("SetFont:%{public}s", font.c_str());
    Configuration config;
    config.AddItem(ConfigurationInner::APPLICATION_FONT, font);
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
}

bool ApplicationContext::SetFontSizeScale(double fontSizeScale)
{
    Configuration config;
    config.AddItem(ConfigurationInner::SYSTEM_FONT_SIZE_SCALE, std::to_string(fontSizeScale));
    if (appConfigChangeCallback_ != nullptr) {
        appConfigChangeCallback_(config);
    }
    return true;
}

void ApplicationContext::SetAppRunningUniqueId(const std::string& appRunningUniqueId)
{
    appRunningUniqueId_ = appRunningUniqueId;
}

std::string ApplicationContext::GetAppRunningUniqueId() const
{
    return appRunningUniqueId_;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
