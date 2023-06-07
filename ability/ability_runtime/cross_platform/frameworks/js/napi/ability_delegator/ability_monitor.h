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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_H

#include <memory>
#include "iability_monitor.h"
#include "js_ability_monitor.h"
#include "native_engine/native_reference.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AppExecFwk;
class AbilityMonitor : public IAbilityMonitor {
public:
    /**
     * A constructor used to create a AbilityMonitor instance with the input parameter passed.
     *
     * @param name Indicates the specified ability name.
     * @param jsAbilityMonitor Indicates the JSAbilityMonitor object.
     */
    AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~AbilityMonitor() = default;

    /**
     * Called when ability is started.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when ability is in foreground.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when ability is in background.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when ability is stopped.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when window stage is created.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when window stage is restored.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj) override;

    /**
     * Called when window stage is destroyed.
     *
     * @param abilityObj Indicates the ability object.
     */
    void OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj) override;

private:
    std::shared_ptr<JSAbilityMonitor> jsMonitor_;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_MONITOR_H
