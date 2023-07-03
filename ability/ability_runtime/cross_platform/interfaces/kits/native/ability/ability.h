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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_ABILITY_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_ABILITY_H

#include <memory>

#include "ability_context.h"
#include "ability_info.h"
#include "ability_lifecycle_executor.h"
#include "application.h"
#include "launch_param.h"
#include "want.h"
#include "window_stage.h"

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
namespace Platform {

using Want = AAFwk::Want;
class Ability {
public:
    static std::shared_ptr<Ability> Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime);

    Ability() = default;
    virtual ~Ability() = default;

    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);

    /**
     * @brief Obtains the AbilityContext object of the ability.
     *
     * @return Returns the AbilityContext object of the ability.
     */
    std::shared_ptr<AbilityContext> GetAbilityContext()
    {
        return abilityContext_;
    }

    void SetAbilityContext(const std::shared_ptr<AbilityContext>& abilityContext)
    {
        abilityContext_ = abilityContext;
    }

    /**
     * @brief Called when this ability is started. You must override this function if you want to perform some
     *        initialization operations during ability startup.
     *
     * This function can be called only once in the entire lifecycle of an ability.
     * @param Want Indicates the {@link Want} structure containing startup information about the ability.
     */
    virtual void OnCreate(const Want& want);

    /**
     * @brief Called when this ability enters the <b>STATE_STOP</b> state.
     *
     * The ability in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnDestory();

    /**
     * @brief Called when the launch mode of an ability is set to singleInstance. This happens when you re-launch an
     * ability that has been at the top of the ability stack.
     *
     * @param want Indicates the new Want containing information about the ability.
     */
    virtual void OnNewWant(const Want& want);

    /**
     * @brief Called when this ability enters the <b>STATE_FOREGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_FOREGROUND</b> state is visible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnForeground(const Want& want);

    /**
     * @brief Called when this ability enters the <b>STATE_BACKGROUND</b> state.
     *
     *
     * The ability in the <b>STATE_BACKGROUND</b> state is invisible.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnBackground();

    /**
     * @brief Obtains the class name in this ability name, without the prefixed bundle name.
     *
     * @return Returns the class name of this ability.
     */
    std::string GetAbilityName();

    /**
     * @brief Obtains the lifecycle state of this ability.
     *
     * @return Returns the lifecycle state of this ability.
     */
    virtual OHOS::AppExecFwk::AbilityLifecycleExecutor::LifecycleState GetState() final;

    virtual void OnWindowStageCreated();
    virtual void OnWindowStageDestroy();
    void SetWant(const AAFwk::Want& want);
    std::shared_ptr<AAFwk::Want> GetWant();

    void SetInstanceName(const std::string& instanceName)
    {
        instanceName_ = instanceName;
    }

    std::string GetInstanceName() const
    {
        return instanceName_;
    }

    virtual void OnConfigurationUpdate(const Configuration& configuration);
    void DispatchLifecycleOnForeground(const Want &want);

    const LaunchParam& GetLaunchParam() const;
    virtual void OnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant);

public:
    std::shared_ptr<Rosen::WindowStage> windowStage_ = nullptr;

protected:
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo_ = nullptr;

private:
    std::shared_ptr<AbilityContext> abilityContext_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::AbilityLifecycleExecutor> abilityLifecycleExecutor_ = nullptr;
    std::shared_ptr<AAFwk::Want> want_ = nullptr;
    std::string instanceName_ { "" };
    LaunchParam launchParam_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_ABILITY_H