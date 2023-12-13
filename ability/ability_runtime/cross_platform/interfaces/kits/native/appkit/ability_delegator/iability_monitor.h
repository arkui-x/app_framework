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

#ifndef OHOS_ABILITY_RUNTIME_IABILITY_MONITORE_H
#define OHOS_ABILITY_RUNTIME_IABILITY_MONITORE_H

#include <condition_variable>
#include <memory>
#include <string>

#include "ability_delegator_infos.h"

namespace OHOS {
namespace AppExecFwk {
class IAbilityMonitor {
public:
    /**
     * Indicates that the default timeout is 5 seconds.
     */
    static constexpr int64_t MAX_TIME_OUT {5000};

public:
    /**
     * A constructor used to create a IAbilityMonitor instance with the input parameter passed.
     *
     * @param abilityName Indicates the specified ability name for monitoring the lifecycle state changes
     * of the ability.
     */
    explicit IAbilityMonitor(const std::string &abilityName);

    /**
     * Default deconstructor used to deconstruct.
     */
    virtual ~IAbilityMonitor() = default;

    /**
     * Match the monitored Ability objects when newAbility objects are started or
     * the lifecycle states of monitored abilities have changed.
     *
     * @param ability Indicates the ability.
     * @param isNotify Indicates whether to notify the matched ability to the object who waited.
     * @return true if match is successful; returns false otherwise.
     */
    virtual bool Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify = false);

    /**
     * Waits for and returns the started Ability object that matches the conditions specified in this monitor
     * within 5 seconds.
     * The current thread will be blocked until the 5-second default timer expires.
     *
     * @return the Ability object if any object has started is matched within 5 seconds; returns null otherwise.
     */
    virtual std::shared_ptr<ADelegatorAbilityProperty> WaitForAbility();

    /**
     * Waits for and returns the started Ability object that matches the conditions specified in this monitor
     * within the specified time.
     * The current thread will be blocked until the timer specified by timeoutMillisecond expires.
     *
     * @param timeoutMs Indicates the maximum amount of time to wait, in milliseconds.
     * The value must be a positive integer.
     * @return the Ability object if any object has started is matched within the specified time;
     * returns null otherwise.
     */
    virtual std::shared_ptr<ADelegatorAbilityProperty> WaitForAbility(const int64_t timeoutMs);

    /**
     * Called when ability is started.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when ability is in foreground.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when ability is in background.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when ability is stopped.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when window stage is created.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when window stage is restored.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj);

    /**
     * Called when window stage is destroyed.
     *
     * @param abilityObj Indicates the ability object.
     */
    virtual void OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj);

private:
    std::string abilityName_;
    std::shared_ptr<ADelegatorAbilityProperty> matchedAbility_;

    std::condition_variable cvMatch_;
    std::mutex mMatch_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_IABILITY_MONITORE_H
