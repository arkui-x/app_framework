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

#include "app_main.h"

#include <fstream>
#include <string>
#include <unistd.h>

#include "ability.h"
#include "ability_context_adapter.h"
#include "ability_delegator.h"
#include "ability_delegator_args.h"
#include "ability_delegator_registry.h"
#include "application_configuration_manager.h"
#include "application_context.h"
#include "application_data_manager.h"
#include "base/log/ace_trace.h"
#include "base/utils/string_utils.h"
#include "hilog.h"
#include "js_error_logger.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "preload_manager.h"
#include "runtime.h"

#include "base/log/ace_trace.h"
#include "base/utils/string_utils.h"
#include "include/core/SkFontMgr.h"
#include "ohos/init_data.h"
#include "runtime.h"
#include "uncaught_exception_callback.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
const std::string CONFIGURATION = "configuration";
const std::string FONT_SIZE_SCALE = "fontSizeScale";
const std::string FONT_SIZE_MAX_SCALE = "fontSizeMaxScale";
constexpr int64_t NANOSECONDS = 1000000000;
constexpr int64_t MICROSECONDS = 1000000;
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

void AppMain::LaunchApplication(bool isCopyNativeLibs)
{
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }

    auto task = [isCopyNativeLibs]() { AppMain::GetInstance()->ScheduleLaunchApplication(isCopyNativeLibs); };
    eventHandler_->PostTask(task);
}

void AppMain::ScheduleLaunchApplication(bool isCopyNativeLibs)
{
    HILOG_INFO("AppMain schedule launch application.");
    Ace::AceScopedTrace trace("ScheduleLaunchApplication");

    LoadIcuData();

    bundleContainer_ = std::make_shared<AppExecFwk::BundleContainer>();
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }
    StageAssetManager::GetInstance()->InitModuleVersionCode();
    auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
    HILOG_INFO("module list size: %{public}d", static_cast<int32_t>(moduleList.size()));
    if (!moduleList.empty()) {
        bundleContainer_->LoadBundleInfos(moduleList);
    }
    bundleContainer_->SetAppCodePath(StageAssetManager::GetInstance()->GetBundleCodeDir());
    bundleContainer_->SetPidAndUid(pid_, uid_);
#ifdef ANDROID_PLATFORM
    auto bundleName = bundleContainer_->GetBundleName();
    if (isCopyNativeLibs && !bundleName.empty()) {
        StageAssetManager::GetInstance()->CopyNativeLibToAppDataModuleDir(bundleName);
    }
#endif
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
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    applicationContext->SetAppRunningUniqueId(
        std::to_string(static_cast<int64_t>((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS));
    applicationContext->SetBundleContainer(bundleContainer_);
    ParseBundleComplete();

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
    auto bundleinfo = bundleContainer_->GetBundleInfo();
    if (bundleinfo != nullptr) {
        for (const auto& hapModuleInfo : bundleinfo->hapModuleInfos) {
            auto moduleName = hapModuleInfo.moduleName;
            std::vector<uint8_t> buffer = StageAssetManager::GetInstance()->GetPkgJsonBuffer(moduleName);
            if (buffer.empty()) {
                continue;
            }
            buffer.push_back('\0');
            options.pkgContextInfoJsonBufferMap[moduleName] = buffer;
            options.packageNameList[moduleName] = hapModuleInfo.packageName;
        }
    }

    auto runtime = AbilityRuntime::Runtime::Create(options);
    if (runtime == nullptr) {
        return false;
    }

    RegisterUncaughtExceptionHandler(runtime.get());
    application_->SetRuntime(std::move(runtime));
    return true;
}

void AppMain::ParseBundleComplete()
{
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }

    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("applicationContext is nullptr");
        return;
    }

    auto applicationInfo = bundleContainer_->GetApplicationInfo();
    if (applicationInfo != nullptr) {
        applicationContext->SetApplicationInfo(applicationInfo);
    }
    application_->SetApplicationContext(applicationContext);

    auto bundleInfo = bundleContainer_->GetBundleInfo();
    if (applicationInfo == nullptr || bundleInfo == nullptr) {
        HILOG_ERROR("applicationInfo or bundleInfo is nullptr.");
        return;
    }

    if (!CreateRuntime(applicationInfo->bundleName,
            bundleInfo->hapModuleInfos.back().compileMode != AppExecFwk::CompileMode::ES_MODULE)) {
        HILOG_ERROR("runtime create failed.");
        return;
    }
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

void AddConfigurationData(Configuration& configuration, const std::string& moduleName = "")
{
    auto fontConfigBuffer = StageAssetManager::GetInstance()->GetFontConfigJsonBuffer(moduleName);
    if (fontConfigBuffer.empty()) {
        return;
    }
    std::string fontConfigContent(fontConfigBuffer.begin(), fontConfigBuffer.end());
    nlohmann::json fontConfigJson = nlohmann::json::parse(fontConfigContent, nullptr, false);
    if (fontConfigJson.is_discarded() || !fontConfigJson.contains(CONFIGURATION)) {
        return;
    }
    auto fontConfig = fontConfigJson[CONFIGURATION];
    if (fontConfig.is_discarded() || !fontConfig.is_object()) {
        return;
    }
    if (fontConfig.contains(FONT_SIZE_SCALE)) {
        std::string fontSizeScaleStr = fontConfig[FONT_SIZE_SCALE].get<std::string>();
        configuration.AddItem(ConfigurationInner::APP_FONT_SIZE_SCALE, fontSizeScaleStr);
    }
    if (fontConfig.contains(FONT_SIZE_MAX_SCALE)) {
        std::string fontMaxScaleStr = fontConfig[FONT_SIZE_MAX_SCALE].get<std::string>();
        configuration.AddItem(ConfigurationInner::APP_FONT_MAX_SCALE, fontMaxScaleStr);
    }
}

void UpdateModuleConfiguration(const std::string& moduleName)
{
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        return;
    }
    auto configuration = applicationContext->GetConfiguration();
    if (configuration == nullptr) {
        return;
    }
    AddConfigurationData(*configuration, moduleName);
}

void AppMain::HandleDispatchOnCreate(const std::string& instanceName, const std::string& params)
{
    HILOG_INFO("HandleDispatchOnCreate called, instanceName: %{public}s", instanceName.c_str());
    if (application_ == nullptr || bundleContainer_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    auto want = TransformToWant(instanceName);
    std::string moduleName = want.GetModuleName();
    StageAssetManager::GetInstance()->isDynamicModule(moduleName, true);
    bool isDynamicModule = StageAssetManager::GetInstance()->IsDynamicUpdateModule(moduleName);
    if (isDynamicModule) {
#ifdef ANDROID_PLATFORM
        StageAssetManager::GetInstance()->RemoveModuleFilePath(moduleName);
#endif
        bundleContainer_->RemoveModuleInfo(moduleName);
    }
    UpdateModuleConfiguration(moduleName);
    auto hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
    if (hapModuleInfo == nullptr) {
        auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
        auto jsonFile = StageAssetManager::GetInstance()->GetAppDataModuleDir() + '/' + moduleName + "/module.json";
        auto dynamicModuleJson = StageAssetManager::GetInstance()->GetBufferByAppDataPath(jsonFile);
        if (dynamicModuleJson.size() > 0) {
            moduleList.emplace_back(dynamicModuleJson);
            HILOG_INFO("stage dynamic module list size: %{public}d", static_cast<int32_t>(moduleList.size()));
            bundleContainer_->LoadBundleInfos(moduleList);
            if (application_->GetRuntime() == nullptr) {
                ParseBundleComplete();
                hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
            }
        }
    }

#ifdef ANDROID_PLATFORM
    std::vector<std::string> moduleNames { moduleName };
    if (hapModuleInfo != nullptr) {
        auto dependencies = hapModuleInfo->dependencies;
        if (!dependencies.empty()) {
            for (const auto& dependency : dependencies) {
                StageAssetManager::GetInstance()->CopyHspResourcePath(dependency);
                moduleNames.emplace_back(dependency);
            }
        }
    }
    StageAssetManager::GetInstance()->SetNativeLibPaths(hapModuleInfo->bundleName, moduleNames);
#endif
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
    HILOG_INFO("HandleDispatchOnForeground called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnForeground(TransformToWant(instanceName));
}

void AppMain::HandleDispatchOnBackground(const std::string& instanceName)
{
    HILOG_INFO("HandleDispatchOnBackground called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnBackground(TransformToWant(instanceName));
}

void AppMain::HandleDispatchOnDestroy(const std::string& instanceName)
{
    HILOG_INFO("HandleDispatchOnDestroy called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }

    application_->DispatchOnDestroy(TransformToWant(instanceName));
}

void AppMain::HandleOnConfigurationUpdate(const std::string& jsonConfiguration)
{
    HILOG_INFO("HandleOnConfigurationUpdate called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    Configuration configuration;
    configuration.ReadFromJsonConfiguration(jsonConfiguration);
    AddConfigurationData(configuration);
    application_->OnConfigurationUpdate(configuration, SetLevel::System);
}

void AppMain::HandleInitConfiguration(const std::string& jsonConfiguration)
{
    HILOG_INFO("HandleInitConfiguration called.");
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    Configuration configuration;
    configuration.ReadFromJsonConfiguration(jsonConfiguration);
    AddConfigurationData(configuration);
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

void AppMain::RegisterUncaughtExceptionHandler(Runtime* runtime)
{
    if (runtime == nullptr || bundleContainer_ == nullptr) {
        HILOG_ERROR("AppMain RegisterUncaughtExceptionHandler failed, runtime or bundleContainer_ is nullptr");
        return;
    }
    auto bundleInfo = bundleContainer_->GetBundleInfo();
    if (bundleInfo == nullptr) {
        HILOG_ERROR("bundleInfo is nullptr");
        return;
    }
    bool findEntryHapModuleInfo = false;
    AppExecFwk::HapModuleInfo entryHapModuleInfo;
    if (!bundleInfo->hapModuleInfos.empty()) {
        for (auto hapModuleInfo : bundleInfo->hapModuleInfos) {
            if (hapModuleInfo.moduleType == AppExecFwk::ModuleType::ENTRY) {
                findEntryHapModuleInfo = true;
                entryHapModuleInfo = hapModuleInfo;
                break;
            }
        }
        if (!findEntryHapModuleInfo) {
            entryHapModuleInfo = bundleInfo->hapModuleInfos.back();
        }
    }

    JsEnv::UncaughtExceptionInfo uncaughtExceptionInfo;
    FillUncaughtExceptionInfo(uncaughtExceptionInfo, entryHapModuleInfo.hapPath);
    (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).RegisterUncaughtExceptionHandler(uncaughtExceptionInfo);
}

void AppMain::FillUncaughtExceptionInfo(JsEnv::UncaughtExceptionInfo& info, const std::string& hapPath)
{
    if (application_ == nullptr || bundleContainer_ == nullptr) {
        HILOG_ERROR("application_ or bundleContainer_ is nullptr");
        return;
    }
    auto applicationContext = application_->GetApplicationContext();
    auto applicationInfo = bundleContainer_->GetApplicationInfo();
    if (applicationContext == nullptr || applicationInfo == nullptr) {
        HILOG_ERROR("applicationContext or applicationInfo is nullptr");
        return;
    }
    info.hapPath = hapPath;
    JsErrorInfo jsInfo;
    jsInfo.appRunningId = applicationContext->GetAppRunningUniqueId();
    jsInfo.bundleName = applicationInfo->bundleName;
    jsInfo.versionName = applicationInfo->versionName;
    jsInfo.pid = pid_;
    jsInfo.uid = uid_;
    std::weak_ptr<AppMain> weakThis = shared_from_this();
    info.uncaughtTask = [weakThis, jsInfo] (std::string summary, const JsEnv::ErrorObject errorObj) {
        auto sharedThis = weakThis.lock();
        if (sharedThis == nullptr) {
            HILOG_ERROR("null AppMain");
            return;
        }
        AppExecFwk::ErrorObject appExecErrorObj = {
            .name = errorObj.name,
            .message = errorObj.message,
            .stack = errorObj.stack
        };
        if (sharedThis->application_ == nullptr) {
            return;
        }
        JsErrorLogger::SendExceptionToJsError(jsInfo, appExecErrorObj, sharedThis->application_->IsForegroud());
        auto& runtime = sharedThis->application_->GetRuntime();
        if (runtime == nullptr) {
            return;
        }
        auto napiEnv = (static_cast<AbilityRuntime::JsRuntime&>(*runtime)).GetNapiEnv();
        if (NapiErrorManager::GetInstance()->NotifyUncaughtException(
            napiEnv, summary, appExecErrorObj.name,
            appExecErrorObj.message, appExecErrorObj.stack)) {
            return;
        }
        if (AppExecFwk::ApplicationDataManager::GetInstance().NotifyUnhandledException(summary)) {
            AppExecFwk::ApplicationDataManager::GetInstance().NotifyExceptionObject(appExecErrorObj);
        }
    };
}

void AppMain::ParseHspModuleJson(const std::string& moduleName)
{
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }
    bool isDynamicModule = StageAssetManager::GetInstance()->IsDynamicUpdateModule(moduleName);
    if (isDynamicModule) {
        bundleContainer_->RemoveModuleInfo(moduleName);
    }
    auto hapModuleInfo = bundleContainer_->GetHapModuleInfo(moduleName);
    if (hapModuleInfo != nullptr) {
        HILOG_WARN("Module has been parsed");
        return;
    }
    auto jsonFile = StageAssetManager::GetInstance()->GetAppDataModuleDir() + '/' + moduleName + "/module.json";
    auto dynamicModuleJson = StageAssetManager::GetInstance()->GetBufferByAppDataPath(jsonFile);
    if (dynamicModuleJson.empty()) {
        HILOG_ERROR("Get buffer by module path failed.");
        return;
    }
    std::list<std::vector<uint8_t>> moduleList { dynamicModuleJson };
    bundleContainer_->LoadBundleInfos(moduleList);
}

void AppMain::NotifyApplicationForeground()
{
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = []() {
        AppMain::GetInstance()->HandleApplicationForeground();
    };
    eventHandler_->PostTask(task);
}

void AppMain::NotifyApplicationBackground()
{
    if (eventHandler_ == nullptr) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = []() {
        AppMain::GetInstance()->HandleApplicationBackground();
    };
    eventHandler_->PostTask(task);
}

void AppMain::HandleApplicationForeground()
{
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    application_->NotifyApplicationForeground();
}

void AppMain::HandleApplicationBackground()
{
    if (application_ == nullptr) {
        HILOG_ERROR("application_ is nullptr");
        return;
    }
    application_->NotifyApplicationBackground();
}

void AppMain::LoadIcuData()
{
    #ifdef ANDROID_PLATFORM
        std::string appDataModuleDir = StageAssetManager::GetInstance()->GetAppDataModuleDir();
    #else
        std::string appDataModuleDir = StageAssetManager::GetInstance()->GetBundleCodeDir();
    #endif
    appDataModuleDir = appDataModuleDir + "/" + "systemres";
    SetArkuiXIcuDirectory(appDataModuleDir.c_str());
}

void AppMain::PreloadModule(const std::string& moduleName, const std::string& abilityName)
{
    if (!eventHandler_) {
        HILOG_ERROR("eventHandler_ is nullptr");
        return;
    }
    auto task = [moduleName, abilityName]() { AppMain::GetInstance()->HandlePreloadModule(moduleName, abilityName); };
    eventHandler_->PostTask(task);
}

void AppMain::HandlePreloadModule(const std::string& moduleName, const std::string& abilityName)
{
    if (bundleContainer_ == nullptr) {
        HILOG_ERROR("bundleContainer_ is nullptr");
        return;
    }

    auto abilityInfo = bundleContainer_->GetAbilityInfo(moduleName, abilityName);
    if (abilityInfo == nullptr) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    if (application_->GetRuntime() == nullptr) {
        HILOG_ERROR("application_->GetRuntime() is nullptr");
        return;
    }

    if (PreloadManager::GetInstance() != nullptr) {
        abilityInfo->hapPath = "arkui-x/" + abilityInfo->moduleName + "/";
        PreloadManager::GetInstance()->PreloadModule(application_->GetRuntime(), abilityInfo);
    } else {
        HILOG_ERROR("PreloadManager::GetInstance() is nullptr");
        return;
    }
}

#ifdef IOS_PLATFORM
void AppMain::SetResourceFilePrefixPath()
{
    std::string resourcesFilePrefixPath("");
    std::string runtimeOS = "IOS";
    resourcesFilePrefixPath = StageAssetManager::GetInstance()->GetResourceFilePrefixPath();
    SkFontMgr::SetFontMgrConfig(runtimeOS, resourcesFilePrefixPath);
}
#endif
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
