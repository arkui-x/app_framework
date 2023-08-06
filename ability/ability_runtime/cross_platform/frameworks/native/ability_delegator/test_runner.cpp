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

#include "test_runner.h"

#include "hilog.h"
#include "runtime.h"
#include "runner_runtime/js_test_runner.h"

namespace OHOS {
namespace AppExecFwk {
std::unique_ptr<TestRunner> TestRunner::Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
    const std::shared_ptr<AbilityDelegatorArgs> &args, bool isFaJsModel, const std::shared_ptr<BundleInfo> bundleInfo)
{
    HILOG_INFO("AbilityDelegator TestRunner::Create");
    if (!runtime) {
        HILOG_ERROR("AbilityDelegator: fail to runtime");
        return std::make_unique<TestRunner>();
    }

    if (!bundleInfo) {
        HILOG_ERROR("AbilityDelegator:Failed to GetBundleInfo");
        return nullptr;
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return RunnerRuntime::JsTestRunner::Create(runtime, args, bundleInfo, isFaJsModel);
        default:
            return std::make_unique<TestRunner>();
    }
}

void TestRunner::Prepare()
{
    HILOG_INFO("AbilityDelegator TestRunner::Prepare");
}

void TestRunner::Run()
{
    HILOG_INFO("AbilityDelegator TestRunner::Run");
}

bool TestRunner::Initialize()
{
    HILOG_INFO("AbilityDelegator TestRunner::Initialize");
    return true;
}
}  // namespace AppExecFwk
}  // namespace OHOS
