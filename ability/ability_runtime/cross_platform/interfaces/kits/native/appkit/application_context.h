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

#include "context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class ApplicationContext : public Context, public std::enable_shared_from_this<ApplicationContext> {
public:
    ApplicationContext() = default;
    ~ApplicationContext() = default;

    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    void SetApplicationInfo(const std::shared_ptr<AppExecFwk::ApplicationInfo> &info);
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;
    std::string GetBundleCodePath() const override;
    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;
    std::shared_ptr<StageAssetManager> GetAssetManager() override;

    static std::shared_ptr<ApplicationContext> GetInstance();

private:
    std::shared_ptr<AppExecFwk::ApplicationInfo> applicationInfo_ = nullptr;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_APPLICATION_CONTEXT_H