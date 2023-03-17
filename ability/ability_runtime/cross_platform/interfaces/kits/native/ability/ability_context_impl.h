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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
#define OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H

#include "ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class AbilityContextImpl : public AbilityContext {
public:
    AbilityContextImpl() = default;
    virtual ~AbilityContextImpl() = default;

    std::string GetBundleCodeDir() override;
    std::string GetCacheDir() override;
    std::string GetTempDir() override;
    std::string GetFilesDir() override;
    std::string GetDatabaseDir() override;
    std::string GetPreferencesDir() override;
    std::string GetBundleName() const override;
    std::shared_ptr<AppExecFwk::ApplicationInfo> GetApplicationInfo() const override;
    std::shared_ptr<Global::Resource::ResourceManager> GetResourceManager() const override;

    std::string GetBundleCodePath() const override;
    ErrCode StartAbility(const AAFwk::Want& want, int requestCode) override;
    ErrCode TerminateSelf() override;
    ErrCode CloseAbility() override;
    std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const override;
    std::shared_ptr<AppExecFwk::HapModuleInfo> GetHapModuleInfo() const override;

    std::shared_ptr<StageAssetManager> GetAssetManager() override
    {
        return nullptr;
    }

    void GetPlatformResourcePaths(const std::string& hapResPath, const std::string& sysResPath) override {}

    /**
     * @brief Set the Ability Info object
     *
     * set ability info to ability context
     */
    void SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);

    void SetAbilityStageContext(const std::shared_ptr<Context>& abilityStageContext);
private:
    std::shared_ptr<AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;
    std::shared_ptr<Context> stageContext_ = nullptr;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_ABILITY_CONTEXT_IMPL_H
