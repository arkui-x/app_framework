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

#include "ability_context_impl.h"

#include "ability_context_adapter.h"
#include "hilog.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
const size_t AbilityContext::CONTEXT_TYPE_ID(std::hash<const char*> {}("AbilityContext"));
std::string AbilityContextImpl::GetBundleCodeDir()
{
    return stageContext_ ? stageContext_->GetBundleCodeDir() : "";
}

std::string AbilityContextImpl::GetCacheDir()
{
    return stageContext_ ? stageContext_->GetCacheDir() : "";
}

std::string AbilityContextImpl::GetDatabaseDir()
{
    return stageContext_ ? stageContext_->GetDatabaseDir() : "";
}

std::string AbilityContextImpl::GetPreferencesDir()
{
    return stageContext_ ? stageContext_->GetPreferencesDir() : "";
}

std::string AbilityContextImpl::GetTempDir()
{
    return stageContext_ ? stageContext_->GetTempDir() : "";
}

std::string AbilityContextImpl::GetFilesDir()
{
    return stageContext_ ? stageContext_->GetFilesDir() : "";
}

ErrCode AbilityContextImpl::StartAbility(const AAFwk::Want& want, int requestCode)
{
    HILOG_INFO("Start ability");
    Platform::AbilityContextAdapter::GetInstance()->StartAbility(instanceName_, want);
    return 0;
}

std::string AbilityContextImpl::GetBundleName() const
{
    return stageContext_ ? stageContext_->GetBundleName() : "";
}

std::shared_ptr<AppExecFwk::ApplicationInfo> AbilityContextImpl::GetApplicationInfo() const
{
    return stageContext_ ? stageContext_->GetApplicationInfo() : nullptr;
}

std::string AbilityContextImpl::GetBundleCodePath() const
{
    return stageContext_ ? stageContext_->GetBundleCodePath() : "";
}

std::shared_ptr<AppExecFwk::HapModuleInfo> AbilityContextImpl::GetHapModuleInfo() const
{
    return stageContext_ ? stageContext_->GetHapModuleInfo() : nullptr;
}

std::shared_ptr<Global::Resource::ResourceManager> AbilityContextImpl::GetResourceManager() const
{
    return stageContext_ ? stageContext_->GetResourceManager() : nullptr;
}

void AbilityContextImpl::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

std::shared_ptr<AppExecFwk::AbilityInfo> AbilityContextImpl::GetAbilityInfo() const
{
    return abilityInfo_;
}

void AbilityContextImpl::SetAbilityStageContext(const std::shared_ptr<Context>& abilityStageContext)
{
    stageContext_ = abilityStageContext;
}

ErrCode AbilityContextImpl::TerminateSelf()
{
    HILOG_INFO("Terminate self");
    Platform::AbilityContextAdapter::GetInstance()->TerminateSelf(instanceName_);
    return 0;
}

ErrCode AbilityContextImpl::CloseAbility()
{
    return 0;
}

void AbilityContextImpl::GetResourcePaths(std::string& hapResPath, std::string& sysResPath)
{
    if (stageContext_ == nullptr) {
        HILOG_INFO("stageContext_ is nullptr");
        return;
    }
    stageContext_->GetResourcePaths(hapResPath, sysResPath);
}

std::shared_ptr<Configuration> AbilityContextImpl::GetConfiguration()
{
    return stageContext_ ? stageContext_->GetConfiguration() : nullptr;
}

void AbilityContextImpl::SetInstanceName(const std::string& instanceName)
{
    instanceName_ = instanceName;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
