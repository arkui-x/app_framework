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

#ifndef OHOS_ABILITY_RUNTIME_TEST_RUNNER_H
#define OHOS_ABILITY_RUNTIME_TEST_RUNNER_H

#include <memory>
#include "ability_delegator_args.h"
#include "bundle_info.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
}

namespace AppExecFwk {
class TestRunner {
public:
    /**
     * Default constructor used to create a TestRunner instance.
     */
    TestRunner() = default;

    /**
     * Default deconstructor used to deconstruct.
     */
    virtual ~TestRunner() = default;

    /**
     * Creates a TestRunner instance with the input parameter passed.
     *
     * @param runtime Indicates the ability runtime.
     * @param args Indicates the AbilityDelegatorArgs object.
     * @return the TestRunner object if TestRunner object is created successfully; returns null otherwise.
     */
    static std::unique_ptr<TestRunner> Create(const std::unique_ptr<AbilityRuntime::Runtime> &runtime,
        const std::shared_ptr<AbilityDelegatorArgs> &args, bool isFaJsModel,
        const std::shared_ptr<BundleInfo> bundleInfo);
    /**
     * Prepares the testing environment for running test code.
     */
    virtual void Prepare();

    /**
     * Runs all test code.
     */
    virtual void Run();

    /**
     * Initialize runtime only fo ace 1.0.
     */
    virtual bool Initialize();
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_TEST_RUNNER_H
