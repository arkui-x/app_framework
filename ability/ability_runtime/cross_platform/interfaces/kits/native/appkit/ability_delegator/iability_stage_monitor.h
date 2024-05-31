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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_STAGE_MONITORE_H
#define OHOS_ABILITY_RUNTIME_IABILITY_STAGE_MONITORE_H

#include <condition_variable>
#include <memory>
#include <string>

#include "ability_delegator_infos.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityStageMonitor {
public:
    /**
     * Indicates that the default timeout is 5 seconds.
     */
    static constexpr int64_t MAX_TIME_OUT {5000};

public:
    /**
     * A constructor used to create a IAbilityStageMonitor instance with the input parameter passed.
     *
     * @param moduleName Indicates the specified module name for monitoring the lifecycle state changes
     * of the abilityStage.
     * @param srcEntrance Indicates the path of the abilityStage.
     */
    explicit IAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance);

    /**
     * Default deconstructor used to deconstruct.
     */
    virtual ~IAbilityStageMonitor() = default;

    /**
     * Match the monitored AbilityStage objects when newAbility objects are started.
     *
     * @param abilityStage Indicates the abilityStage.
     * @param isNotify Indicates whether to notify the matched abilityStage to the object who waited.
     * @return true if match is successful; returns false otherwise.
     */
    virtual bool Match(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage, bool isNotify = false);

    /**
     * Waits for and returns the started AbilityStage object that matches the conditions specified in this monitor
     * within the specified time.
     * The current thread will be blocked until the timer specified by timeoutMillisecond expires.
     *
     * @param timeoutMs Indicates the maximum amount of time to wait, in milliseconds.
     * The value must be a positive integer.
     * @return the AbilityStage object if any object has started is matched within the specified time;
     * returns null otherwise.
     */
    virtual std::shared_ptr<DelegatorAbilityStageProperty> WaitForAbilityStage(const int64_t timeoutMs);

    /**
     * Waits for and returns the started AbilityStage object that matches the conditions specified in this monitor
     * within 5 seconds.
     * The current thread will be blocked until the 5-second default timer expires.
     *
     * @return the AbilityStage object if any object has started is matched within 5 seconds; returns null otherwise.
     */
    virtual std::shared_ptr<DelegatorAbilityStageProperty> WaitForAbilityStage();

private:
    std::string moduleName_;
    std::string srcEntrance_;
    std::shared_ptr<DelegatorAbilityStageProperty> matchedAbilityStage_;

    std::condition_variable cvMatch_;
    std::mutex mtxMatch_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_IABILITY_STAGE_MONITORE_H
