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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H

#include <list>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class StageAssetManager {
public:
    StageAssetManager() = default;
    ~StageAssetManager() = default;

    static StageAssetManager& GetInstance()
    {
        static StageAssetManager instance;
        return instance;
    }

    std::list<std::vector<uint8_t>> GetModuleJsonBufferList();
    std::vector<uint8_t> GetModuleBuffer(const std::string& moduleName, std::string& modulePath);
    std::vector<uint8_t> GetModuleAbilityBuffer(
        const std::string& moduleName, const std::string& abilityName, std::string& modulePath);
    std::string GetBundleCodeDir() const;
    std::string GetCacheDir() const;
    std::string GetTempDir() const;
    std::string GetFilesDir() const;
    std::string GetDatabaseDir() const;
    std::string GetPreferencesDir() const;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_STAGE_ASSET_MANAGER_H