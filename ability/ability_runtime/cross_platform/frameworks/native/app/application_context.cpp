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

#include "hilog.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
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
    return StageAssetManager::GetInstance().GetBundleCodeDir();
}

std::string ApplicationContext::GetCacheDir()
{
    return StageAssetManager::GetInstance().GetCacheDir();
}

std::string ApplicationContext::GetTempDir()
{
    return StageAssetManager::GetInstance().GetTempDir();
}

std::string ApplicationContext::GetFilesDir()
{
    return StageAssetManager::GetInstance().GetFilesDir();
}

std::string ApplicationContext::GetDatabaseDir()
{
    return StageAssetManager::GetInstance().GetDatabaseDir();
}

std::string ApplicationContext::GetPreferencesDir()
{
    return StageAssetManager::GetInstance().GetPreferencesDir();
}

std::shared_ptr<AppExecFwk::HapModuleInfo> ApplicationContext::GetHapModuleInfo() const
{
    return nullptr;
}

std::shared_ptr<StageAssetManager> ApplicationContext::GetAssetManager()
{
    return std::shared_ptr<StageAssetManager>(&StageAssetManager::GetInstance());
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
