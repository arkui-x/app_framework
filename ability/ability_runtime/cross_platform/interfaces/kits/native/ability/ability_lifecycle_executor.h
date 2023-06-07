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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_LIFECYCLE_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_ABILITY_LIFECYCLE_EXECUTOR_H

namespace OHOS {
namespace AppExecFwk {
class AbilityLifecycleExecutor {
public:
    AbilityLifecycleExecutor() = default;
    virtual ~AbilityLifecycleExecutor() = default;
    enum LifecycleState {
        ACTIVE,
        BACKGROUND,
        INACTIVE,
        INITIAL,
        UNINITIALIZED,
        STARTED_NEW,
        FOREGROUND_NEW,
        BACKGROUND_NEW,
        STOPED_NEW
    };

    /**
     * @brief While Ability's lifecycle changes, dispatch lifecycle state.
     *
     * @param state  Lifecycle state.
     */
    void DispatchLifecycleState(const AbilityLifecycleExecutor::LifecycleState &state);

    /**
     * @brief Obtains the int value of the ability lifecycle state represented by the
     * AbilityLifecycleExecutor.LifecycleState enum constant.
     *
     * @return return  Returns the int value of the ability lifecycle state represented
     * by the AbilityLifecycleExecutor.LifecycleState enum constant.
     */
    int GetState() const;

private:
    AbilityLifecycleExecutor::LifecycleState state_ = UNINITIALIZED;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_LIFECYCLE_EXECUTOR_H
