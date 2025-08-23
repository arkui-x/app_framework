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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_H

#include <unordered_map>

#include "ability_stage.h"
#include "application_configuration_manager.h"
#include "application_context.h"
#include "bundle_container.h"
#include "configuration.h"
#include "runtime.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class Application : public std::enable_shared_from_this<Application> {
public:
    Application();
    ~Application();

    void SetRuntime(std::unique_ptr<AbilityRuntime::Runtime>&& runtime);
    const std::unique_ptr<AbilityRuntime::Runtime>& GetRuntime();
    void SetApplicationContext(const std::shared_ptr<ApplicationContext>& applicationContext);
    std::shared_ptr<ApplicationContext> GetApplicationContext()
    {
        return applicationContext_;
    };

    void SetBundleContainer(const std::shared_ptr<AppExecFwk::BundleContainer>& bundleContainer)
    {
        bundleContainer_ = bundleContainer;
    }

    void HandleAbilityStage(const AAFwk::Want& want);
    void DispatchOnNewWant(const AAFwk::Want& want);
    void DispatchOnForeground(const AAFwk::Want& want);
    void DispatchOnBackground(const AAFwk::Want& want);
    void DispatchOnDestroy(const AAFwk::Want& want);
    void OnConfigurationUpdate(Configuration& configuration, SetLevel level);
    void InitConfiguration(const Configuration& configuration);
    void DispatchOnAbilityResult(
        const AAFwk::Want& want, int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant);
    void NotifyApplicationForeground();
    void NotifyApplicationBackground();
    bool IsForegroud();

private:
    std::shared_ptr<AbilityStage> FindAbilityStage(const std::string& moduleName);
    bool IsUpdateColorNeeded(Configuration &config, SetLevel level);
    bool IsUpdateFontSizeNeeded(Configuration& config, SetLevel level);
    bool IsUpdateLanguageNeeded(Configuration& config, SetLevel level);

private:
    std::unique_ptr<AbilityRuntime::Runtime> runtime_;
    std::shared_ptr<ApplicationContext> applicationContext_ = nullptr;
    std::unordered_map<std::string, std::shared_ptr<AbilityStage>> abilityStages_;
    std::shared_ptr<AppExecFwk::BundleContainer> bundleContainer_ = nullptr;
    std::shared_ptr<Configuration> configuration_ = nullptr;
    bool isForeground_ = true;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_H