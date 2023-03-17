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

#include "ability_stage_context.h"

#include "hilog.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::string AbilityStageContext::GetBundleName() const
{
    return applicationContext_ ? applicationContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityStageContext::GetApplicationInfo() const
{
    return applicationContext_ ? applicationContext_->GetApplicationInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityStageContext::GetResourceManager() const
{
    return nullptr;
}

std::string AbilityStageContext::GetBundleCodePath() const
{
     return applicationContext_ ? applicationContext_->GetBundleCodePath() : "";
}

std::string AbilityStageContext::GetBundleCodeDir()
{
    return applicationContext_ ? applicationContext_->GetBundleCodeDir() : "";
}

std::string AbilityStageContext::GetCacheDir()
{
    return applicationContext_ ? applicationContext_->GetCacheDir() : "";
}

std::string AbilityStageContext::GetTempDir()
{
    return applicationContext_ ? applicationContext_->GetTempDir() : "";
}

std::string AbilityStageContext::GetFilesDir()
{
    return applicationContext_ ? applicationContext_->GetFilesDir() : "";
}

std::string AbilityStageContext::GetDatabaseDir()
{
    return applicationContext_ ? applicationContext_->GetDatabaseDir() : "";
}

std::string AbilityStageContext::GetPreferencesDir()
{
    return applicationContext_ ? applicationContext_->GetPreferencesDir() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityStageContext::GetHapModuleInfo() const
{
    return hapModuleInfo_;
}

std::shared_ptr<StageAssetManager> AbilityStageContext::GetAssetManager()
{
    return applicationContext_ ? applicationContext_->GetAssetManager() : nullptr;
}

void AbilityStageContext::SetHapModuleInfo(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo)
{
    hapModuleInfo_ = hapModuleInfo;
}

void AbilityStageContext::SetApplicationContext(const std::shared_ptr<Context>& applicationContext)
{
    applicationContext_ = applicationContext;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
