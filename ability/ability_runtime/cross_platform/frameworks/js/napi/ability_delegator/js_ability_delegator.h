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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_DELEGATOR_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_DELEGATOR_H

#include <string>
#include "ability_delegator.h"
#include "ability_monitor.h"
#include "ability_stage_monitor.h"
#include "js_ability_delegator_registry.h"
#include "js_ability_monitor.h"
#include "want.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
class JSAbilityDelegator {
public:
    struct TimeoutCallback {
        bool hasTimeoutPara;
        bool hasCallbackPara;
    };

    /**
     * Default constructor used to create a JSAbilityDelegator instance.
     */
    JSAbilityDelegator();

    /**
     * Default deconstructor used to deconstruct.
     */
    ~JSAbilityDelegator() = default;

    /**
     * Destory object.
     *
     * @param env Indicates the native env.
     * @param data Indicates the object to be destroyed.
     * @param hint Indicates the hint.
     */
    static void Finalizer(napi_env env, void *data, void *hint);

    /**
     * Adds ability monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value AddAbilityMonitor(napi_env env, napi_callback_info info);

    /**
     * Adds abilityStage monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value AddAbilityStageMonitor(napi_env env, napi_callback_info info);

    /**
     * Removes ability monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value RemoveAbilityMonitor(napi_env env, napi_callback_info info);

    /**
     * Removes abilityStage monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value RemoveAbilityStageMonitor(napi_env env, napi_callback_info info);

    /**
     * Waits for the specified monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value WaitAbilityMonitor(napi_env env, napi_callback_info info);

    /**
     * Waits for the specified stage monitor.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value WaitAbilityStageMonitor(napi_env env, napi_callback_info info);

    /**
     * Prints log information to the console.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value Print(napi_env env, napi_callback_info info);

    /**
     * Prints log information to the console.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value PrintSync(napi_env env, napi_callback_info info);

    /**
     * Obtains the application context.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value GetAppContext(napi_env env, napi_callback_info info);

    /**
     * Obtains the lifecycle state of the specified ability.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value GetAbilityState(napi_env env, napi_callback_info info);

    /**
     * Obtains the ability that is currently being displayed.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value GetCurrentTopAbility(napi_env env, napi_callback_info info);

    /**
     * Starts an ability based on the given parameters.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value StartAbility(napi_env env, napi_callback_info info);

    /**
     * Transits the specified ability to foreground.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value DoAbilityForeground(napi_env env, napi_callback_info info);

    /**
     * Transits the specified ability to background.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value DoAbilityBackground(napi_env env, napi_callback_info info);

    /**
     * Finishes user test.
     *
     * @param env Indicates the native env.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static napi_value FinishTest(napi_env env, napi_callback_info info);

private:
    napi_value OnAddAbilityMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnAddAbilityStageMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnRemoveAbilityMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnRemoveAbilityStageMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnWaitAbilityMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnWaitAbilityStageMonitor(napi_env env, NapiCallbackInfo& info);
    napi_value OnPrint(napi_env env, NapiCallbackInfo& info);
    napi_value OnPrintSync(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetAppContext(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetAbilityState(napi_env env, NapiCallbackInfo& info);
    napi_value OnGetCurrentTopAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnDoAbilityForeground(napi_env env, NapiCallbackInfo& info);
    napi_value OnDoAbilityBackground(napi_env env, NapiCallbackInfo& info);
    napi_value OnFinishTest(napi_env env, NapiCallbackInfo& info);

private:
    napi_value ParseMonitorPara(napi_env env, napi_value value, std::shared_ptr<AbilityMonitor> &monitor);
    napi_value ParseStageMonitorPara(
        napi_env env, napi_value value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted);
    napi_value ParseAbilityParaInfo(napi_env env, napi_value value, std::string &fullName);
    void AbilityLifecycleStateToJs(
        const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState);
    napi_value ParseAbilityMonitorPara(
        napi_env env, NapiCallbackInfo& info, std::shared_ptr<AbilityMonitor> &monitor);
    napi_value ParseAbilityStageMonitorPara(
        napi_env env, NapiCallbackInfo& info,
        std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted);
    napi_value ParseWaitAbilityMonitorPara(napi_env env, NapiCallbackInfo& info,
        std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout);
    napi_value ParseWaitAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
        std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout);
    napi_value ParseTimeoutCallbackPara(
        napi_env env, NapiCallbackInfo& info, TimeoutCallback &opt, int64_t &timeout);
    napi_value ParsePrintPara(napi_env env, NapiCallbackInfo& info, std::string &msg);
    napi_value ParseAbilityCommonPara(
        napi_env env, NapiCallbackInfo& info, std::string &fullName);
    napi_value ParseStartAbilityPara(
        napi_env env, NapiCallbackInfo& info, AAFwk::Want &want);
    napi_value ParseFinishTestPara(napi_env env, NapiCallbackInfo& info, std::string &msg, int64_t &code);
    void AddStageMonitorRecord(
        napi_env env, napi_value value, const std::shared_ptr<AbilityStageMonitor> &monitor);
    void RemoveStageMonitorRecord(napi_env env, napi_value value);
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_DELEGATOR_H
