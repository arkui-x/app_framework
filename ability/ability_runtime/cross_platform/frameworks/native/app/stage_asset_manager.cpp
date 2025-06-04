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

#include "stage_asset_manager.h"

#include <string>

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

std::vector<uint8_t> StageAssetManager::GetPkgJsonBuffer(const std::string& moduleName)
{
    return StageAssetProvider::GetInstance()->GetPkgJsonBuffer(moduleName);
}

std::list<std::vector<uint8_t>> StageAssetManager::GetModuleJsonBufferList()
{
    return StageAssetProvider::GetInstance()->GetModuleJsonBufferList();
}

std::vector<uint8_t> StageAssetManager::GetFontConfigJsonBuffer(const std::string& moduleName)
{
    return StageAssetProvider::GetInstance()->GetFontConfigJsonBuffer(moduleName);
}

std::vector<uint8_t> StageAssetManager::GetModuleBuffer(
    const std::string& moduleName, std::string& modulePath, bool esmodule)
{
    return StageAssetProvider::GetInstance()->GetModuleBuffer(moduleName, modulePath, esmodule);
}

std::vector<uint8_t> StageAssetManager::GetModuleAbilityBuffer(
    const std::string& moduleName, const std::string& abilityName, std::string& modulePath, bool esmodule)
{
    return StageAssetProvider::GetInstance()->GetModuleAbilityBuffer(moduleName, abilityName, modulePath, esmodule);
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

std::string StageAssetManager::GetAppLibDir() const
{
    return StageAssetProvider::GetInstance()->GetAppLibDir();
}

#ifdef ANDROID_PLATFORM
std::string StageAssetManager::GetAppDataLibDir() const
{
    return StageAssetProvider::GetInstance()->GetAppDataLibDir();
}

void StageAssetManager::CopyHspResourcePath(const std::string& moduleName)
{
    StageAssetProvider::GetInstance()->CopyHspResourcePath(moduleName);
}

void StageAssetManager::CopyNativeLibToAppDataModuleDir(const std::string& bundleName)
{
    StageAssetProvider::GetInstance()->CopyNativeLibToAppDataModuleDir(bundleName);
}

void StageAssetManager::SetNativeLibPaths(
    const std::string& bundleName, const std::vector<std::string>& moduleNames)
{
    StageAssetProvider::GetInstance()->SetNativeLibPaths(bundleName, moduleNames);
}

void StageAssetManager::RemoveModuleFilePath(const std::string& moduleName)
{
    StageAssetProvider::GetInstance()->RemoveModuleFilePath(moduleName);
}

bool StageAssetManager::IsDynamicLoadLibs()
{
    return StageAssetProvider::GetInstance()->IsDynamicLoadLibs();
}
#endif

void StageAssetManager::GetResIndexPath(
    const std::string& moduleName, std::string& appResIndexPath, std::string& sysResIndexPath)
{
    return StageAssetProvider::GetInstance()->GetResIndexPath(moduleName, appResIndexPath, sysResIndexPath);
}

std::string StageAssetManager::GetAppDataModuleDir() const
{
    return StageAssetProvider::GetInstance()->GetAppDataModuleDir();
}

bool GetAppDataModuleAssetList(const std::string& path, std::vector<std::string>& fileFullPaths, bool onlyChild)
{
    return StageAssetProvider::GetInstance()->GetAppDataModuleAssetList(path, fileFullPaths, onlyChild);
}

#ifdef IOS_PLATFORM
std::string StageAssetManager::GetResourceFilePrefixPath()
{
    return StageAssetProvider::GetInstance()->GetResourceFilePrefixPath();
}
#endif

std::vector<uint8_t> StageAssetManager::GetBufferByAppDataPath(const std::string& fileFullPath)
{
    return StageAssetProvider::GetInstance()->GetBufferByAppDataPath(fileFullPath);
}

std::vector<uint8_t> StageAssetManager::GetAotBuffer(const std::string &fileName)
{
    return StageAssetProvider::GetInstance()->GetAotBuffer(fileName);
}

void StageAssetManager::isDynamicModule(const std::string& moduleName, bool needUpdate)
{
    StageAssetProvider::GetInstance()->UpdateVersionCode(moduleName, needUpdate);
}

void StageAssetManager::InitModuleVersionCode()
{
    StageAssetProvider::GetInstance()->InitModuleVersionCode();
}

bool StageAssetManager::IsDynamicUpdateModule(const std::string& moduleName)
{
    return StageAssetProvider::GetInstance()->IsDynamicUpdateModule(moduleName);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS