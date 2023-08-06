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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_CONTEXT_H

#include "ability_lifecycle_callback.h"
#include "bundle_container.h"
#include "context.h"
#include "running_process_info.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class ApplicationContext : public Context, public std::enable_shared_from_this<ApplicationContext> {
public:
    ApplicationContext() = default;
    ~ApplicationContext() = default;

    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo>& info);
    void SetConfiguration(const std::shared_ptr<Configuration>& configuration);
    void SetBundleContainer(const std::shared_ptr<AppExecFwk::BundleContainer>& bundleContainer);
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
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
    void GetResourcePaths(std::string& hapResPath, std::string& sysResPath) override;
    std::shared_ptr<Context> CreateModuleContext(const std::string &moduleName) override;
    std::shared_ptr<Configuration> GetConfiguration() override;
    static std::shared_ptr<ApplicationContext> GetInstance();
    void RegisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback>& abilityLifecycleCallback);
    void UnregisterAbilityLifecycleCallback(const std::shared_ptr<AbilityLifecycleCallback>& abilityLifecycleCallback);

    void DispatchOnAbilityCreate(const std::shared_ptr<NativeReference>& ability);
    void DispatchOnAbilityDestroy(const std::shared_ptr<NativeReference>& ability);
    void DispatchOnWindowStageCreate(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage);
    void DispatchOnWindowStageDestroy(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage);
    void DispatchOnAbilityForeground(const std::shared_ptr<NativeReference>& ability);
    void DispatchOnAbilityBackground(const std::shared_ptr<NativeReference>& ability);
    int32_t GetProcessRunningInformation(std::vector<RunningProcessInfo>& processInfos);
    void SetResourceManager(const std::shared_ptr<Global::Resource::ResourceManager> &resMgr);

private:
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = nullptr;
    std::shared_ptr<Configuration> configuration_ = nullptr;
    std::shared_ptr<AppExecFwk::BundleContainer> bundleContainer_ = nullptr;
    static std::vector<std::shared_ptr<AbilityLifecycleCallback>> callbacks_;
    std::mutex callbackLock_;
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_CONTEXT_H