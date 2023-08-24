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

#include "app_main.h"

#include <string>

#include "ability.h"
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "ability_delegator_args.h"
#include "application_context.h"
#include "ability_context_adapter.h"
#include "hilog.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "runtime.h"

#include "base/log/ace_trace.h"
#include "base/utils/string_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<AppMain> AppMain::instance_ = nullptr;
std::mutex AppMain::mutex_;
AppMain::AppMain()
{
    runner_ = AppExecFwk::EventRunner::Current();
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
}

AppMain::~AppMain() {}

std::shared_ptr<AppMain> AppMain::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<AppMain>();
        }
    }

    return instance_;
}

void AppMain::LaunchApplication()
{
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    auto task = []() { AppMain::GetInstance()->ScheduleLaunchApplication(); };
    eventHandler_->PostTask(task);
}

void AppMain::ScheduleLaunchApplication()
{
    HILOG_INFO("AppMain schedule launch application.");
    Ace::AceScopedTrace trace("ScheduleLaunchApplication");

    bundleContainer_ = std::make_shared<AppExecFwk::BundleContainer>();
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }

    auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
    HILOG_INFO("module list size: %{public}d", static_cast<int32_t>(moduleList.size()));
    bundleContainer_->LoadBundleInfos(moduleList);
    bundleContainer_->SetAppCodePath(StageAssetManager::GetInstance()->GetBundleCodeDir());
    bundleContainer_->SetPidAndUid(pid_, uid_);
    application_ = std::make_shared<Application>();
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    application_->SetBundleContainer(bundleContainer_);

    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("applicationContext is nullptr");
        return;
    }
    applicationContext->SetBundleContainer(bundleContainer_);
    auto applicationInfo = bundleContainer_->GetApplicationInfo();
    if (applicationInfo == nullptr) {
        HILOG_ERROR("applicationInfo is nullptr");
        return;
    }
    applicationContext->SetApplicationInfo(applicationInfo);
    application_->SetApplicationContext(applicationContext);

    auto bundleInfo = bundleContainer_->GetBundleInfo();
    if (!CreateRuntime(applicationInfo->bundleName, bundleInfo->hapModuleInfos.back().compileMode
        != AppExecFwk::CompileMode::ES_MODULE)) {
        HILOG_ERROR("runtime create failed.");
        return;
    }

    HILOG_INFO("Launch application success.");
}

void AppMain::PrepareAbilityDelegator(const std::string& bundleName, const std::string& moduleName,
    const std::string& testRunerName, const std::string& timeout)
{
    auto task = [bundleName, moduleName, testRunerName, timeout]() {
        AppMain::GetInstance()->CreateAbilityDelegator(bundleName, moduleName, testRunerName, timeout);
    };
    eventHandler_->PostTask(task);
}

void AppMain::CreateAbilityDelegator(const std::string& bundleName, const std::string& moduleName,
    const std::string& testRunerName, const std::string& timeout)
{
    HILOG_INFO("PrepareAbilityDelegator for Stage model.");
    Want want;
    want.SetBundleName(bundleName);
    want.SetModuleName(moduleName);
    want.SetAbilityName(testRunerName);
    want.SetParam("timeout", timeout);
    auto args = std::make_shared<AppExecFwk::AbilityDelegatorArgs>(want);
    auto applicationContext = ApplicationContext::GetInstance();
    std::unique_ptr<AppExecFwk::TestRunner> testRunner = AppExecFwk::TestRunner::Create(application_->GetRuntime(),
        args, false, bundleContainer_->GetBundleInfo());
    HILOG_INFO("PrepareAbilityDelegator create TestRunner.");
    if (testRunner == nullptr) {
        HILOG_WARN("AbilityDelegator: testRunner is null.");
        return;
    }
    auto delegator = std::make_shared<AppExecFwk::AbilityDelegator>(applicationContext, std::move(testRunner));
    if (delegator == nullptr) {
        HILOG_WARN("AbilityDelegator: delegator is null.");
        return;
    }
    AppExecFwk::AbilityDelegatorRegistry::RegisterInstance(delegator, args);
    delegator->Prepare();
}

bool AppMain::CreateRuntime(const std::string& bundleName, bool isBundle)
{
    OHOS::AbilityRuntime::Runtime::Options options;
    options.loadAce = true;
    options.eventRunner = runner_;
    options.codePath = StageAssetManager::GetInstance()->GetBundleCodeDir();
    options.bundleName = bundleName;
    options.isBundle = isBundle;
    options.appLibPath = StageAssetManager::GetInstance()->GetAppLibDir();
#ifdef ANDROID_PLATFORM
    options.appDataLibPath = StageAssetManager::GetInstance()->GetAppDataLibDir();
#endif
    auto runtime = AbilityRuntime::Runtime::Create(options);
    if (runtime == nullptr) {
        return false;
    }

    application_->SetRuntime(std::move(runtime));
    return true;
}

void AppMain::DispatchOnCreate(const std::string& instanceName, const std::string& params)
{
    HILOG_INFO("DispatchOnCreate called");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName, params]() { AppMain::GetInstance()->HandleDispatchOnCreate(instanceName, params); };
    eventHandler_->PostTask(task);
}

void AppMain::DispatchOnNewWant(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnNewWant called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName]() { AppMain::GetInstance()->HandleDispatchOnNewWant(instanceName); };
    eventHandler_->PostTask(task);
}

void AppMain::DispatchOnForeground(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnForeground called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName]() { AppMain::GetInstance()->HandleDispatchOnForeground(instanceName); };
    eventHandler_->PostTask(task);
}

void AppMain::DispatchOnBackground(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnBackground called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName]() { AppMain::GetInstance()->HandleDispatchOnBackground(instanceName); };
    eventHandler_->PostSyncTask(task);
}

void AppMain::DispatchOnDestroy(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnDestroy called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName]() { AppMain::GetInstance()->HandleDispatchOnDestroy(instanceName); };
    eventHandler_->PostSyncTask(task);
}

void AppMain::OnConfigurationUpdate(const std::string& jsonConfiguration)
{
    HILOG_INFO("OnConfigurationUpdate called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [jsonConfiguration]() {
        AppMain::GetInstance()->HandleOnConfigurationUpdate(jsonConfiguration);
    };
    eventHandler_->PostTask(task);
}

void AppMain::InitConfiguration(const std::string& jsonConfiguration)
{
    HILOG_INFO("InitConfiguration called.");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [jsonConfiguration]() {
        AppMain::GetInstance()->HandleInitConfiguration(jsonConfiguration);
    };
    eventHandler_->PostTask(task);
}

void AppMain::HandleDispatchOnCreate(const std::string& instanceName, const std::string& params)
{
    HILOG_INFO("HandleDispatchOnCreate called, instanceName: %{public}s", instanceName.c_str());
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    auto want = TransformToWant(instanceName);
    std::string moduleName = want.GetModuleName();
    auto hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
    if (hapModuleInfo == nullptr) {        
        auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
        auto jsonFile = StageAssetManager::GetInstance()->GetAppDataModuleDir() + '/' + moduleName + "/module.json";
        auto dynamicModuleJson = StageAssetManager::GetInstance()->GetBufferByAppDataPath(jsonFile);
        if (dynamicModuleJson.size() > 0) {
            moduleList.emplace_back(dynamicModuleJson);
            HILOG_INFO("stage dynamic module list size: %{public}d", static_cast<int32_t>(moduleList.size()));
            bundleContainer_->LoadBundleInfos(moduleList);
        } 
    }

    application_->HandleAbilityStage(TransformToWant(instanceName, params));
}

void AppMain::HandleDispatchOnNewWant(const std::string& instanceName)
{
    HILOG_INFO("HandleDispatchOnNewWant called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnNewWant(TransformToWant(instanceName));
}

void AppMain::HandleDispatchOnForeground(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnForeground called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnForeground(TransformToWant(instanceName));
}

void AppMain::HandleDispatchOnBackground(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnBackground called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnBackground(TransformToWant(instanceName));
}

void AppMain::HandleDispatchOnDestroy(const std::string& instanceName)
{
    HILOG_INFO("DispatchOnDestroy called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnDestroy(TransformToWant(instanceName));
}

void AppMain::HandleOnConfigurationUpdate(const std::string& jsonConfiguration)
{
    HILOG_INFO("OnConfigurationUpdate called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    Configuration configuration;
    configuration.ReadFromJsonConfiguration(jsonConfiguration);
    application_->OnConfigurationUpdate(configuration);
}

void AppMain::HandleInitConfiguration(const std::string& jsonConfiguration)
{
    HILOG_INFO("InitConfiguration called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    Configuration configuration;
    configuration.ReadFromJsonConfiguration(jsonConfiguration);
    application_->InitConfiguration(configuration);
}

Want AppMain::TransformToWant(const std::string& instanceName, const std::string& params)
{
    std::vector<std::string> nameStrs;
    Ace::StringUtils::StringSplitter(instanceName, ':', nameStrs);
    for (auto str : nameStrs) {
        HILOG_INFO("TransformToWant::str : %{public}s", str.c_str());
    }

    Want want;
    if (nameStrs.size() == 4) {
        want.SetBundleName(nameStrs[0]);
        want.SetModuleName(nameStrs[1]);
        want.SetAbilityName(nameStrs[2]);
        want.SetParam(Want::ABILITY_ID, nameStrs[3]);
        want.SetParam(Want::INSTANCE_NAME, instanceName);
    }
    want.ParseJson(params);
    return want;
}

void AppMain::SetPidAndUid(int32_t pid, int32_t uid)
{
    HILOG_INFO("set pid: %{public}d, uid: %{public}d", pid, uid);
    pid_ = pid;
    uid_ = uid;
}

bool AppMain::IsSingleton(const std::string& moduleName, const std::string& abilityName)
{
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return false;
    }
    auto abilityInfo = bundleContainer_->GetAbilityInfo(moduleName, abilityName);
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return false;
    }
    return (abilityInfo->launchMode == AppExecFwk::LaunchMode::SINGLETON);
}

void AppMain::DispatchOnAbilityResult(
    const std::string& instanceName, int32_t requestCode, int32_t resultCode, const std::string& resultWant)
{
    HILOG_INFO("called");
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [instanceName, requestCode, resultCode, resultWant]() {
        AppMain::GetInstance()->HandleDispatchOnAbilityResult(instanceName, requestCode, resultCode, resultWant);
    };
    eventHandler_->PostTask(task);
}

void AppMain::HandleDispatchOnAbilityResult(
    const std::string& instanceName, int32_t requestCode, int32_t resultCode, const std::string& resultWant)
{
    HILOG_INFO("called, instanceName: %{public}s, resultWant: %{public}s", instanceName.c_str(), resultWant.c_str());
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    Want abilityResultWant;
    abilityResultWant.ParseJson(resultWant);
    application_->DispatchOnAbilityResult(
        TransformToWant(instanceName), requestCode, resultCode, abilityResultWant);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
