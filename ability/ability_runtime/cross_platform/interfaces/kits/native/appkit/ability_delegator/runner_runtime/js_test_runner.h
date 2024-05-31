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

#ifndef OHOS_ABILITY_RUNTIME_JS_TEST_RUNNER_H
#define OHOS_ABILITY_RUNTIME_JS_TEST_RUNNER_H

#include "bundle_info.h"
#include "js_runtime.h"
#include "test_runner.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace RunnerRuntime {
using namespace AppExecFwk;
using namespace AbilityRuntime;

class JsTestRunner : public TestRunner {
public:
    /**
     * Creates a TestRunner instance with the input parameter passed.
     *
     * @param runtime Indicates the ability runtime.
     * @param args Indicates the AbilityDelegatorArgs object.
     * @param bundleInfo Indicates the bundle info.
     * @return the TestRunner object if JsTestRunner object is created successfully; returns null otherwise.
     */
    static std::unique_ptr<TestRunner> Create(const std::unique_ptr<Runtime> &runtime,
        const std::shared_ptr<AbilityDelegatorArgs> &args, const std::shared_ptr<BundleInfo> &bundleInfo,
        bool isFaJsModel);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~JsTestRunner() override;

    /**
     * Prepares the testing environment for running test code.
     */
    void Prepare() override;

    /**
     * Runs all test code.
     */
    void Run() override;

    bool Initialize() override;

private:
    JsTestRunner(JsRuntime &jsRuntime,
        const std::shared_ptr<AbilityDelegatorArgs> &args, const std::shared_ptr<BundleInfo> &bundleInfo,
        bool isFaJsModel);

    void CallObjectMethod(const char *name, napi_value const *argv = nullptr, size_t argc = 0);
    void ReportFinished(const std::string &msg);
    void ReportStatus(const std::string &msg);

    JsRuntime &jsRuntime_;
    std::unique_ptr<NativeReference> jsTestRunnerObj_;
    std::string srcPath_;
    std::string hapPath_;
    bool isFaJsModel_ = false;
};
}  // namespace RunnerRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_TEST_RUNNER_H
