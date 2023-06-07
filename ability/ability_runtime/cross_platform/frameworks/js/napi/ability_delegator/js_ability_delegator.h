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
     * @param engine Indicates the native engine.
     * @param data Indicates the object to be destroyed.
     * @param hint Indicates the hint.
     */
    static void Finalizer(NativeEngine *engine, void *data, void *hint);

    /**
     * Adds ability monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *AddAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Adds abilityStage monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *AddAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Removes ability monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *RemoveAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Removes abilityStage monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *RemoveAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Waits for the specified monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *WaitAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Waits for the specified stage monitor.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *WaitAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Prints log information to the console.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *Print(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Prints log information to the console.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *PrintSync(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Obtains the application context.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *GetAppContext(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Obtains the lifecycle state of the specified ability.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *GetAbilityState(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Obtains the ability that is currently being displayed.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *GetCurrentTopAbility(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Starts an ability based on the given parameters.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *StartAbility(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Transits the specified ability to foreground.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *DoAbilityForeground(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Transits the specified ability to background.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *DoAbilityBackground(NativeEngine *engine, NativeCallbackInfo *info);

    /**
     * Finishes user test.
     *
     * @param engine Indicates the native engine.
     * @param info Indicates the parameters from js.
     * @return exec result.
     */
    static NativeValue *FinishTest(NativeEngine *engine, NativeCallbackInfo *info);

private:
    NativeValue *OnAddAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnAddAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnRemoveAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnRemoveAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnWaitAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnWaitAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnPrint(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnPrintSync(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetAppContext(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetAbilityState(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnGetCurrentTopAbility(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnStartAbility(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnDoAbilityForeground(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnDoAbilityBackground(NativeEngine &engine, NativeCallbackInfo &info);
    NativeValue *OnFinishTest(NativeEngine &engine, NativeCallbackInfo &info);

private:
    NativeValue *CreateAbilityObject(NativeEngine &engine, const sptr<IRemoteObject> &remoteObject);
    NativeValue *ParseMonitorPara(NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityMonitor> &monitor);
    NativeValue *ParseStageMonitorPara(
        NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted);
    NativeValue *ParseAbilityParaInfo(NativeEngine &engine, NativeValue *value, std::string &fullname);
    void AbilityLifecycleStateToJs(
        const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState);
    NativeValue *ParseAbilityMonitorPara(
        NativeEngine &engine, NativeCallbackInfo &info, std::shared_ptr<AbilityMonitor> &monitor);
    NativeValue *ParseAbilityStageMonitorPara(
        NativeEngine &engine, NativeCallbackInfo &info, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted);
    NativeValue *ParseWaitAbilityMonitorPara(NativeEngine &engine, NativeCallbackInfo &info,
        std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout);
    NativeValue *ParseWaitAbilityStageMonitorPara(NativeEngine &engine, NativeCallbackInfo &info,
        std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout);
    NativeValue *ParseTimeoutCallbackPara(
        NativeEngine &engine, NativeCallbackInfo &info, TimeoutCallback &opt, int64_t &timeout);
    NativeValue *ParsePrintPara(NativeEngine &engine, NativeCallbackInfo &info, std::string &msg);
    NativeValue *ParseAbilityCommonPara(
        NativeEngine &engine, NativeCallbackInfo &info, std::string &fullame);
    NativeValue *ParseStartAbilityPara(
        NativeEngine &engine, NativeCallbackInfo &info, AAFwk::Want &want);
    NativeValue *ParseFinishTestPara(NativeEngine &engine, NativeCallbackInfo &info, std::string &msg, int64_t &code);
    void AddStageMonitorRecord(
        NativeEngine &engine, NativeValue *value, const std::shared_ptr<AbilityStageMonitor> &monitor);
    void RemoveStageMonitorRecord(NativeValue *value);
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_ABILITY_DELEGATOR_H
