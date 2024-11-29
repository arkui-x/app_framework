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

#include "ability_delegator_registry.h"
#include "hilog.h"
#include "js_runtime_utils.h"
#include "runner_runtime/js_test_runner.h"

extern const char _binary_delegator_mgmt_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_delegator_mgmt_abc_end[];
#else
extern const char* _binary_delegator_mgmt_abc_end;
#endif

namespace OHOS {
namespace RunnerRuntime {
std::unique_ptr<TestRunner> JsTestRunner::Create(const std::unique_ptr<Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, 
    const std::shared_ptr<BundleInfo> &bundleInfo, bool isFaJsModel)
{
    HILOG_INFO("JsTestRunner: Create enter");
    if (!runtime) {
        HILOG_ERROR("AbilityDelegator:Invalid runtime");
        return nullptr;
    }

    if (!args) {
        HILOG_ERROR("Invalid ability delegator args");
        return nullptr;
    }

    auto pTestRunner = new (std::nothrow) JsTestRunner(static_cast<JsRuntime &>(*runtime), args, bundleInfo,
        isFaJsModel);
    if (!pTestRunner) {
        HILOG_ERROR("AbilityDelegator:Failed to create test runner");
        return nullptr;
    }

    return std::unique_ptr<JsTestRunner>(pTestRunner);
}

JsTestRunner::JsTestRunner(
    JsRuntime &jsRuntime, const std::shared_ptr<AbilityDelegatorArgs> &args, 
    const std::shared_ptr<BundleInfo> &bundleInfo, bool isFaJsModel)
    : jsRuntime_(jsRuntime), isFaJsModel_(isFaJsModel)
{
    HILOG_INFO("AbilityDelegator:JsTestRunner enter");
    if (bundleInfo == nullptr || args == nullptr) {
        HILOG_ERROR("bundleInfo or args is null.");
        return;
    }
    std::string moduleName = args->GetTestModuleName();
    std::string testRunnerName = args->GetTestRunnerClassName();
    std::string modulePath;
    auto hapModuleInfos = bundleInfo->hapModuleInfos;
    CompileMode compileMode = AppExecFwk::CompileMode::JS_BUNDLE;

    for (auto hapModuleInfo : hapModuleInfos) {
        if (hapModuleInfo.moduleName == moduleName) {
            compileMode = hapModuleInfo.compileMode;
            break;
        }
    }
    bool isEsModule = compileMode == AppExecFwk::CompileMode::ES_MODULE;
    auto abilityBuffer =
        AbilityRuntime::Platform::StageAssetManager::GetInstance()->GetModuleAbilityBuffer(moduleName, testRunnerName,
            modulePath, isEsModule);
    HILOG_INFO("modulePath: %{public}s , testRunnerName: %{public}s, moduleName: %{public}s", modulePath.c_str(),
        testRunnerName.c_str(), moduleName.c_str());
    srcPath_ = modulePath;

    if (abilityBuffer.empty()) {
        HILOG_WARN("AbilityDelegator abilityBuffer is null");
        return;
    }

    if (isFaJsModel) {
        HILOG_WARN("AbilityDelegator is FA model");
        return;
    }
    std::string srcEntrance;
    if (isEsModule) {
        HILOG_INFO("AbilityDelegator is isEsModule");
        srcEntrance.append("ets/TestRunner/").append(testRunnerName);
        modulePath = srcEntrance;
        srcEntrance.append(".ts");
        modulePath.append(".abc");
    } else {
        HILOG_INFO("AbilityDelegator is not isEsModule");
    }
    
    moduleName.append("::").append("TestRunner");
    jsTestRunnerObj_ = jsRuntime_.LoadModule(moduleName, modulePath, abilityBuffer, srcEntrance, isEsModule);
}

JsTestRunner::~JsTestRunner() = default;

bool JsTestRunner::Initialize()
{
    return true;
}

void JsTestRunner::Prepare()
{
    HILOG_INFO("Enter");
    TestRunner::Prepare();
    CallObjectMethod("onPrepare");
    HILOG_INFO("Exit");
}

void JsTestRunner::Run()
{
    HILOG_INFO("Enter");
    TestRunner::Run();
    CallObjectMethod("onRun");
    HILOG_INFO("Exit");
}

void JsTestRunner::CallObjectMethod(const char *name, napi_value const *argv, size_t argc)
{
    HILOG_INFO("JsTestRunner::CallObjectMethod(%{public}s)", name);
    auto env = jsRuntime_.GetNapiEnv();
    if (isFaJsModel_) {
        napi_value global = nullptr;
        napi_get_global(env, &global);
        napi_value exportObject = nullptr;
        napi_get_named_property(env, global, "exports", &exportObject);
        if (!CheckTypeForNapiValue(env, exportObject, napi_object)) {
            HILOG_ERROR("Failed to get exportObject");
            return;
        }

        napi_value defaultObject = nullptr;
        napi_get_named_property(env, exportObject, "default", &defaultObject);
        if (!CheckTypeForNapiValue(env, defaultObject, napi_object)) {
            HILOG_ERROR("Failed to get defaultObject");
            return;
        }

        napi_value func = nullptr;
        napi_get_named_property(env, defaultObject, name, &func);
        if (!CheckTypeForNapiValue(env, func, napi_function)) {
            HILOG_ERROR("CallRequest func is %{public}s", func == nullptr ? "nullptr" : "not func");
            return;
        }
        napi_call_function(env, CreateJsUndefined(env), func, argc, argv, nullptr);
        return;
    }

    HandleScope handleScope(jsRuntime_);
    napi_value obj = jsTestRunnerObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get Test Runner object");
        ReportFinished("Failed to get Test Runner object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, name, &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Test Runner object", name);
        ReportStatus("Failed to get " + std::string(name) + " from Test Runner object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, argc, argv, nullptr);
}

void JsTestRunner::ReportFinished(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }

    delegator->FinishUserTest(msg, -1);
}

void JsTestRunner::ReportStatus(const std::string &msg)
{
    HILOG_INFO("Enter");
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return;
    }

    delegator->Print(msg);
}
}  // namespace RunnerRuntime
}  // namespace OHOS
