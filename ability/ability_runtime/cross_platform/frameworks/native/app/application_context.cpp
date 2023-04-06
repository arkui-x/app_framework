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

#include "application_context.h"

#include <algorithm>

#include "application_context_adapter.h"
#include "hilog.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::vector<std::shared_ptr<AbilityLifecycleCallback>> ApplicationContext::callbacks_;
std::shared_ptr<ApplicationContext> Context::applicationContext_ = nullptr;
std::mutex Context::contextMutex_;
const size_t Context::CONTEXT_TYPE_ID(std::hash<const char*> {}("Context"));

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
    return nullptr;
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

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return nullptr;
}

std::shared_ptr<StageAssetManager> ApplicationContext::GetAssetManager()
{
    return StageAssetManager::GetInstance();
}

void ApplicationContext::GetResourcePaths(std::string& hapResPath, std::string& sysResPath) {}

std::shared_ptr<Configuration> ApplicationContext::GetConfiguration()
{
    return nullptr;
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

int32_t ApplicationContext::GetProcessRunningInformation(std::vector<RunningProcessInfo>& processInfos)
{
    processInfos = ApplicationContextAdapter::GetInstance()->GetRunningProcessInformation();
    return 0;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
