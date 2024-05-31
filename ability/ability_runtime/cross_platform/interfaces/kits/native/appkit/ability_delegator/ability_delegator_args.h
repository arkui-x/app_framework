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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_ARGS_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_ARGS_H

#include <map>
#include <string>

#include "want.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegatorArgs {
public:
    /**
     * Indicates the key of test bundle name.
     */
    static const std::string KEY_TEST_BUNDLE_NAME;
    /**
     * Indicates the key of test package name.
     */
    static const std::string KEY_TEST_PACKAGE_NAME;
    /**
     * Indicates the key of test module name.
     */
    static const std::string KEY_TEST_MODULE_NAME;
    /**
     * Indicates the key of test runner class name.
     */
    static const std::string KEY_TEST_RUNNER_CLASS;
    /**
     * Indicates the key of test case name.
     */
    static const std::string KEY_TEST_CASE;
    /**
     * Indicates the key of test wait timeout.
     */
    static const std::string KEY_TEST_WAIT_TIMEOUT;
    /**
     * Indicates the key of debug flag.
     */
    static const std::string KEY_TEST_DEBUG;
    /**
     * Indicates the value of debug flag.
     */
    static const std::string VALUE_TEST_DEBUG;

public:
    /**
     * Default constructor used to create a AbilityDelegatorArgs instance.
     */
    AbilityDelegatorArgs();

    /**
     * A constructor used to create a AbilityDelegatorArgs instance with the input parameter passed.
     *
     * @param want Indicates the Want that contains parameters.
     */
    explicit AbilityDelegatorArgs(const AAFwk::Want &want);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~AbilityDelegatorArgs();

    /**
     * Obtains the bundle name of the application being tested.
     *
     * @return the application bundle name.
     */
    std::string GetTestBundleName() const;

    /**
     * Obtains the package name of the application being tested.
     *
     * @return the application package name.
     */
    std::string GetTestPackageName() const;

    /**
     * Obtains the module name of the application being tested.
     *
     * @return the application module name.
     */
    std::string GetTestModuleName() const;

    /**
     * Obtains the class name of the test runner used to execute test cases.
     *
     * @return the class name of the test runner.
     */
    std::string GetTestRunnerClassName() const;

    /**
     * Obtains the test cases name.
     *
     * @return the test cases name.
     */
    std::string GetTestCaseName() const;

    /**
     * Obtains the test parameters.
     *
     * @return the test parameters.
     */
    std::map<std::string, std::string> GetTestParam() const;

    /**
     * Find debug flag.
     *
     * @return true if found, false otherwise.
     */
    bool FindDebugFlag() const;

private:
    std::string GetParamValue(const std::string &key) const;

private:
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::map<std::string, std::string> params_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_ARGS_H
