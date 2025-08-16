/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

int AbilityContextImpl::GetSystemDatabaseDir(const std::string &groupId, bool checkExist, std::string &databaseDir)
{
    return stageContext_ ?
        stageContext_->GetSystemDatabaseDir(groupId, checkExist, databaseDir) : -1;
}

int AbilityContextImpl::GetSystemPreferencesDir(
    const std::string &groupId, bool checkExist, std::string &preferencesDir)
{
    return stageContext_ ?
        stageContext_->GetSystemPreferencesDir(groupId, checkExist, preferencesDir) : -1;
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
    return Platform::AbilityContextAdapter::GetInstance()->StartAbility(instanceName_, want);
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
    auto platformBundleName = Platform::AbilityContextAdapter::GetInstance()->GetPlatformBundleName();
    if (!platformBundleName.empty() && abilityInfo_ != nullptr) {
        abilityInfo_->bundleName = platformBundleName;
    }
    return abilityInfo_;
}

void AbilityContextImpl::SetJsWindowStage(const std::shared_ptr<NativeReference> jsWindowStage)
{
    jsWindowStage_ = jsWindowStage;
}

std::shared_ptr<NativeReference> AbilityContextImpl::GetJsWindowStage()
{
    return jsWindowStage_;
}

void AbilityContextImpl::SetAbilityStageContext(const std::shared_ptr<Context>& abilityStageContext)
{
    stageContext_ = abilityStageContext;
}

std::shared_ptr<Context> AbilityContextImpl::CreateModuleContext(const std::string& moduleName)
{
    return stageContext_ ? stageContext_->CreateModuleContext(moduleName) : nullptr;
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

void AbilityContextImpl::GetResourcePaths(std::vector<std::string>& appResourcePaths, std::string& sysResourcePath)
{
    if (stageContext_ == nullptr) {
        HILOG_ERROR("stageContext_ is nullptr");
        return;
    }
    stageContext_->GetResourcePaths(appResourcePaths, sysResourcePath);
}

std::shared_ptr<Configuration> AbilityContextImpl::GetConfiguration()
{
    return stageContext_ ? stageContext_->GetConfiguration() : nullptr;
}

void AbilityContextImpl::SetInstanceName(const std::string& instanceName)
{
    instanceName_ = instanceName;
}

ErrCode AbilityContextImpl::StartAbilityForResult(const AAFwk::Want& want, int32_t requestCode, RuntimeTask&& task)
{
    HILOG_INFO("called.");
    resultCallbacks_.insert(std::make_pair(requestCode, std::move(task)));
    ErrCode err =
        Platform::AbilityContextAdapter::GetInstance()->StartAbilityForResult(instanceName_, want, requestCode);
    if (err != 0) {
        HILOG_ERROR("failed, ret = %{public}d", err);
        HandleOnAbilityResult(requestCode, err, want);
    }
    return err;
}

ErrCode AbilityContextImpl::TerminateAbilityWithResult(const AAFwk::Want& resultWant, int32_t resultCode)
{
    HILOG_INFO("called.");
    return Platform::AbilityContextAdapter::GetInstance()->TerminateAbilityWithResult(
        instanceName_, resultWant, resultCode);
}

void AbilityContextImpl::OnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant)
{
    HILOG_INFO("called.");
    HandleOnAbilityResult(requestCode, resultCode, resultWant);
}

void AbilityContextImpl::HandleOnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& want)
{
    auto callback = resultCallbacks_.find(requestCode);
    if (callback != resultCallbacks_.end()) {
        if (callback->second) {
            callback->second(resultCode, want);
        }
        resultCallbacks_.erase(requestCode);
    }
}

ErrCode AbilityContextImpl::ReportDrawnCompleted()
{
    HILOG_INFO("Called.");
    return Platform::AbilityContextAdapter::GetInstance()->ReportDrawnCompleted(instanceName_);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
