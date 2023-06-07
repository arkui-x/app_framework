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

namespace OHOS {
namespace AppExecFwk {
std::shared_ptr<AbilityDelegator> AbilityDelegatorRegistry::abilityDelegator_ {};
std::shared_ptr<AbilityDelegatorArgs> AbilityDelegatorRegistry::abilityDelegatorArgs_ {};

std::shared_ptr<AbilityDelegator> AbilityDelegatorRegistry::GetAbilityDelegator()
{
    return abilityDelegator_;
}

std::shared_ptr<AbilityDelegatorArgs> AbilityDelegatorRegistry::GetArguments()
{
    return abilityDelegatorArgs_;
}

void AbilityDelegatorRegistry::RegisterInstance(
    const std::shared_ptr<AbilityDelegator> &delegator, const std::shared_ptr<AbilityDelegatorArgs> &args)
{
    abilityDelegator_ = delegator;
    abilityDelegatorArgs_ = args;
}
}  // namespace AppExecFwk
}  // namespace OHOS
