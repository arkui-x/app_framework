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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_CONTEXT_H

#include "configuration.h"
#include "context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class AbilityStageContext : public Context {
public:
    AbilityStageContext() = default;
    ~AbilityStageContext() = default;

    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    void SetResourceManager(std::shared_ptr<Global::Resource::ResourceManager> rmg);
    std::string GetBundleCodePath() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    int GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir) override;
    int GetSystemPreferencesDir(const std::string &groupId, bool checkExist, std::string &preferencesDir) override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<Configuration> GetConfiguration() override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    void SetHapModuleInfo(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo);
    void SetApplicationContext(const std::shared_ptr<Context>& applicationContext);
    void SetConfiguration(const std::shared_ptr<Configuration>& configuration);
    void InitResourceManeger();
    void GetResourcePaths(std::string& appResourcePath, std::string& sysResourcePath) override;
    void GetResourcePaths(std::vector<std::string>& appResourcePaths, std::string& sysResourcePath) override;

private:
    std::shared_ptr<Context> applicationContext_ = nullptr;
    std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo_ = nullptr;
    std::shared_ptr<Configuration> configuration_ = nullptr;
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager_ = nullptr;
    std::string appResourcePath_ { "" };
    std::string sysResourcePath_ { "" };
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_STAGE_CONTEXT_H