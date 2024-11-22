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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_REGISTRY_H
#define OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_REGISTRY_H

#include <memory>
#include "ability_delegator.h"
#include "ability_delegator_args.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityDelegatorRegistry {
public:
    /**
     * Obtains the AbilityDelegator object of the application.
     *
     * @return the AbilityDelegator object initialized when the application is started.
     */
    static std::shared_ptr<AbilityDelegator> GetAbilityDelegator();

    /**
     * Obtains test parameters stored in the AbilityDelegatorArgs object.
     *
     * @return the previously registered AbilityDelegatorArgs object.
     */
    static std::shared_ptr<AbilityDelegatorArgs> GetArguments();

    /**
     * Registers the instances of AbilityDelegator and AbilityDelegatorArgs as globally unique instances.
     * This method is called during application startup to initialize the test environment.
     *
     * @param delegator, Indicates the AbilityDelegator object.
     * @param args, Indicates the AbilityDelegatorArgs object.
     */
    static void RegisterInstance(
        const std::shared_ptr<AbilityDelegator> &delegator, const std::shared_ptr<AbilityDelegatorArgs> &args);

private:
    static std::shared_ptr<AbilityDelegator> abilityDelegator_;
    static std::shared_ptr<AbilityDelegatorArgs> abilityDelegatorArgs_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_ABILITY_DELEGATOR_REGISTRY_H
