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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APP_MAIN_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APP_MAIN_H

#include <memory>
#include <mutex>

#include "application.h"
#include "bundle_container.h"
#include "configuration.h"
#include "event_handler.h"

class NativeReference;
namespace OHOS {
namespace JsEnv {
    struct UncaughtExceptionInfo;
}

namespace AbilityRuntime {
namespace Platform {
using Want = AAFwk::Want;
class AppMain final : public std::enable_shared_from_this<AppMain> {
public:
    AppMain();
    ~AppMain();
    static std::shared_ptr<AppMain> GetInstance();
    void LaunchApplication(bool isCopyNativeLibs = true);
    void DispatchOnCreate(const std::string& instanceName, const std::string& params);
    void DispatchOnNewWant(const std::string& instanceName, const std::string& params = "");
    void DispatchOnForeground(const std::string& instanceName);
    void DispatchOnBackground(const std::string& instanceName);
    void DispatchOnDestroy(const std::string& instanceName);
    void SetPidAndUid(int32_t pid, int32_t uid);
    void OnConfigurationUpdate(const std::string& jsonConfiguration);
    void InitConfiguration(const std::string& jsonConfiguration);
    bool IsSingleton(const std::string& moduleName, const std::string& abilityName);
    void PrepareAbilityDelegator(const std::string& bundleName, const std::string& moduleName,
        const std::string& testRunerName, const std::string& timeout);
    void DispatchOnAbilityResult(const std::string& instanceName, int32_t requestCode, int32_t resultCode,
        const std::string& wantParams);
    void ParseHspModuleJson(const std::string& moduleName);
    void NotifyApplicationForeground();
    void NotifyApplicationBackground();
    void PreloadModule(const std::string& moduleName, const std::string& abilityName);
#ifdef IOS_PLATFORM
    void SetResourceFilePrefixPath();
#endif

private:
    void ScheduleLaunchApplication(bool isCopyNativeLibs);
    bool CreateRuntime(const std::string& bundleName, bool isBundle);
    void ParseBundleComplete();
    void HandleDispatchOnCreate(const std::string& instanceName, const std::string& params);
    void UpdateAbilityBundleName(const std::string& bundleName);
    void HandleDispatchOnNewWant(const std::string& instanceName, const std::string& params = "");
    void HandleDispatchOnForeground(const std::string& instanceName);
    void HandleDispatchOnBackground(const std::string& instanceName);
    void HandleDispatchOnDestroy(const std::string& instanceName);
    void HandleOnConfigurationUpdate(const std::string& jsonConfiguration);
    void HandleInitConfiguration(const std::string& jsonConfiguration);
    Want TransformToWant(const std::string& instanceName, const std::string& params = "");
    void CreateAbilityDelegator(const std::string& bundleName, const std::string& moduleName,
        const std::string& testRunerName, const std::string& timeout);
    void HandleDispatchOnAbilityResult(
        const std::string& instanceName, int32_t requestCode, int32_t resultCode, const std::string& resultWant);
    void HandleApplicationForeground();
    void HandleApplicationBackground();
    void LoadIcuData();
    void HandlePreloadModule(const std::string& moduleName, const std::string& abilityName);
    void RegisterUncaughtExceptionHandler(Runtime* runtime);
    void FillUncaughtExceptionInfo(JsEnv::UncaughtExceptionInfo& info, const std::string& hapPath);

private:
    std::shared_ptr<Application> application_ = nullptr;
    std::shared_ptr<AppExecFwk::BundleContainer> bundleContainer_ = nullptr;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner_ = nullptr;
    int32_t pid_ { 0 };
    int32_t uid_ { 0 };
    static std::shared_ptr<AppMain> instance_;
    static std::mutex mutex_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APP_MAIN_H