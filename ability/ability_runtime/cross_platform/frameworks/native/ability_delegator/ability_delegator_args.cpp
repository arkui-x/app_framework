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

#include "ability_delegator_args.h"
#include "hilog.h"

namespace OHOS {
namespace AppExecFwk {
const std::string AbilityDelegatorArgs::KEY_TEST_BUNDLE_NAME {"-b"};
const std::string AbilityDelegatorArgs::KEY_TEST_PACKAGE_NAME {"-p"};
const std::string AbilityDelegatorArgs::KEY_TEST_MODULE_NAME {"-m"};
const std::string AbilityDelegatorArgs::KEY_TEST_RUNNER_CLASS {"-s unittest"};
const std::string AbilityDelegatorArgs::KEY_TEST_CASE {"-s class"};
const std::string AbilityDelegatorArgs::KEY_TEST_WAIT_TIMEOUT {"-w"};

const std::string AbilityDelegatorArgs::KEY_TEST_DEBUG {"-D"};
const std::string AbilityDelegatorArgs::VALUE_TEST_DEBUG {"true"};

AbilityDelegatorArgs::AbilityDelegatorArgs()
{}

AbilityDelegatorArgs::AbilityDelegatorArgs(const AAFwk::Want &want)
{
    bundleName_ = want.GetBundleName();
    moduleName_ = want.GetModuleName();
    abilityName_ = want.GetAbilityName();

    params_["bundleName"] = bundleName_;
    params_["moduleName"] = moduleName_;
    params_["abilityName"] = abilityName_;
    params_["-s timeout"] = want.GetStringParam("timeout");
}

AbilityDelegatorArgs::~AbilityDelegatorArgs()
{}

std::string AbilityDelegatorArgs::GetTestBundleName() const
{
    return bundleName_;
}

std::string AbilityDelegatorArgs::GetTestPackageName() const
{
    return GetParamValue(AbilityDelegatorArgs::KEY_TEST_PACKAGE_NAME);
}

std::string AbilityDelegatorArgs::GetTestModuleName() const
{
    return moduleName_;
}

std::string AbilityDelegatorArgs::GetTestRunnerClassName() const
{
    return abilityName_;
}

std::string AbilityDelegatorArgs::GetTestCaseName() const
{
    return "";
}

std::map<std::string, std::string> AbilityDelegatorArgs::GetTestParam() const
{
    return params_;
}

bool AbilityDelegatorArgs::FindDebugFlag() const
{
    return true;
}

std::string AbilityDelegatorArgs::GetParamValue(const std::string &key) const
{
    return "";
}
}  // namespace AppExecFwk
}  // namespace OHOS
