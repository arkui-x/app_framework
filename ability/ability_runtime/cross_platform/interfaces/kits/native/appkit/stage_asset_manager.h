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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H

#include <list>
#include <mutex>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class StageAssetManager {
public:
    StageAssetManager() = default;
    ~StageAssetManager() = default;

    static std::shared_ptr<StageAssetManager> GetInstance();
    std::vector<uint8_t> GetPkgJsonBuffer(const std::string& moduleName);
    std::list<std::vector<uint8_t>> GetModuleJsonBufferList();
    std::vector<uint8_t> GetFontConfigJsonBuffer(const std::string& moduleName);
    std::vector<uint8_t> GetModuleBuffer(const std::string& moduleName, std::string& modulePath, bool esmodule);
    std::vector<uint8_t> GetModuleAbilityBuffer(
        const std::string& moduleName, const std::string& abilityName, std::string& modulePath, bool esmodule);
    std::string GetBundleCodeDir() const;
    std::string GetCacheDir() const;
    std::string GetTempDir() const;
    std::string GetFilesDir() const;
    std::string GetDatabaseDir() const;
    std::string GetPreferencesDir() const;
    std::string GetAppLibDir() const;
#ifdef IOS_PLATFORM
    std::string GetResourceFilePrefixPath();
#endif
#ifdef ANDROID_PLATFORM
    std::string GetAppDataLibDir() const;
    void CopyHspResourcePath(const std::string& moduleName);
    void CopyNativeLibToAppDataModuleDir(const std::string& bundleName);
    void SetNativeLibPaths(const std::string& bundleName, const std::vector<std::string>& moduleNames);
    void RemoveModuleFilePath(const std::string& moduleName);
    bool IsDynamicLoadLibs();
#endif
    void GetResIndexPath(const std::string& moduleName, std::string& appResIndexPath, std::string& sysResIndexPath);
    std::string GetAppDataModuleDir() const;
    bool GetAppDataModuleAssetList(const std::string& path, std::vector<std::string>& fileFullPaths, bool onlyChild);
    std::vector<uint8_t> GetBufferByAppDataPath(const std::string& fileFullPath);
    void InitModuleVersionCode();
    void isDynamicModule(const std::string& moduleName, bool needUpdate);
    void SetBundleName(const std::string& bundleName);
    std::string GetSplicingModuleName(const std::string& moduleName);
    bool IsDynamicUpdateModule(const std::string& moduleName);

private:
    static std::shared_ptr<StageAssetManager> instance_;
    static std::mutex mutex_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H