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

#include "stage_asset_manager.h"

#include "hilog.h"
#include "stage_asset_provider.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<StageAssetManager> StageAssetManager::instance_ = nullptr;
std::mutex StageAssetManager::mutex_;
std::shared_ptr<StageAssetManager> StageAssetManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<StageAssetManager>();
        }
    }

    return instance_;
}
std::list<std::vector<uint8_t>> StageAssetManager::GetModuleJsonBufferList()
{
    return StageAssetProvider::GetInstance()->GetModuleJsonBufferList();
}
std::vector<uint8_t> StageAssetManager::GetModuleBuffer(const std::string& moduleName, std::string& modulePath)
{
    return StageAssetProvider::GetInstance()->GetModuleBuffer(moduleName, modulePath);
}

std::vector<uint8_t> StageAssetManager::GetModuleAbilityBuffer(
    const std::string& moduleName, const std::string& abilityName, std::string& modulePath)
{
    return StageAssetProvider::GetInstance()->GetModuleAbilityBuffer(moduleName, abilityName, modulePath);
}

std::string StageAssetManager::GetBundleCodeDir() const
{
    return StageAssetProvider::GetInstance()->GetBundleCodeDir();
}

std::string StageAssetManager::GetCacheDir() const
{
    return StageAssetProvider::GetInstance()->GetCacheDir();
}

std::string StageAssetManager::GetTempDir() const
{
    return StageAssetProvider::GetInstance()->GetTempDir();
}

std::string StageAssetManager::GetFilesDir() const
{
    return StageAssetProvider::GetInstance()->GetFilesDir();
}

std::string StageAssetManager::GetDatabaseDir() const
{
    return StageAssetProvider::GetInstance()->GetDatabaseDir();
}

std::string StageAssetManager::GetPreferencesDir() const
{
    return StageAssetProvider::GetInstance()->GetPreferencesDir();
}

void StageAssetManager::GetResIndexPath(const std::string& moduleName, 
    std::string& appResIndexPath, std::string& sysResIndexPath)
{
    return StageAssetProvider::GetInstance()->GetResIndexPath(moduleName, appResIndexPath, sysResIndexPath);
}

jobject StageAssetManager::GetJavaAssetManager()
{
    return StageAssetProvider::GetInstance()->GetAssetManager();
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS