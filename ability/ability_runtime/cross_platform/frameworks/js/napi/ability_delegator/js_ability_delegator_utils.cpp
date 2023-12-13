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

#include "js_ability_delegator_utils.h"

#include <map>
#include "foundation/appframework/ability/ability_runtime/cross_platform/interfaces/kits/native/ability/js_ability.h"
#include "hilog_wrapper.h"
#include "js_ability_monitor.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
NativeValue *CreateJsAbilityDelegator(NativeEngine &engine)
{
    HILOG_INFO("enter");

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return engine.CreateNull();
    }

    std::unique_ptr<JSAbilityDelegator> jsAbilityDelegator = std::make_unique<JSAbilityDelegator>();
    object->SetNativePointer(jsAbilityDelegator.release(), JSAbilityDelegator::Finalizer, nullptr);

    const char *moduleName = "JSAbilityDelegator";
    BindNativeFunction(engine, *object, "addAbilityMonitor", moduleName, JSAbilityDelegator::AddAbilityMonitor);
    BindNativeFunction(engine, *object, "addAbilityStageMonitor",
        moduleName, JSAbilityDelegator::AddAbilityStageMonitor);
    BindNativeFunction(engine, *object, "removeAbilityMonitor", moduleName, JSAbilityDelegator::RemoveAbilityMonitor);
    BindNativeFunction(engine, *object, "removeAbilityStageMonitor",
        moduleName, JSAbilityDelegator::RemoveAbilityStageMonitor);
    BindNativeFunction(engine, *object, "waitAbilityMonitor", moduleName, JSAbilityDelegator::WaitAbilityMonitor);
    BindNativeFunction(engine, *object, "waitAbilityStageMonitor",
        moduleName, JSAbilityDelegator::WaitAbilityStageMonitor);
    BindNativeFunction(engine, *object, "getAppContext", moduleName, JSAbilityDelegator::GetAppContext);
    BindNativeFunction(engine, *object, "getAbilityState", moduleName, JSAbilityDelegator::GetAbilityState);
    BindNativeFunction(engine, *object, "getCurrentTopAbility", moduleName, JSAbilityDelegator::GetCurrentTopAbility);
    BindNativeFunction(engine, *object, "startAbility", moduleName, JSAbilityDelegator::StartAbility);
    BindNativeFunction(engine, *object, "doAbilityForeground", moduleName, JSAbilityDelegator::DoAbilityForeground);
    BindNativeFunction(engine, *object, "doAbilityBackground", moduleName, JSAbilityDelegator::DoAbilityBackground);
    BindNativeFunction(engine, *object, "print", moduleName, JSAbilityDelegator::Print);
    BindNativeFunction(engine, *object, "printSync", moduleName, JSAbilityDelegator::PrintSync);
    BindNativeFunction(engine, *object, "finishTest", moduleName, JSAbilityDelegator::FinishTest);
    return objValue;
}

NativeValue *SetAbilityDelegatorArgumentsPara(NativeEngine &engine, const std::map<std::string, std::string> &paras)
{
    HILOG_INFO("enter");
    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto iter = paras.begin();
    for (; iter != paras.end(); ++iter) {
        object->SetProperty(iter->first.c_str(), CreateJsValue(engine, iter->second));
    }
    return objValue;
}

NativeValue *CreateJsAbilityDelegatorArguments(
    NativeEngine &engine, const std::shared_ptr<AbilityDelegatorArgs> &abilityDelegatorArgs)
{
    HILOG_INFO("enter");

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return engine.CreateNull();
    }

    object->SetProperty("bundleName", CreateJsValue(engine, abilityDelegatorArgs->GetTestBundleName()));
    object->SetProperty("parameters",
        SetAbilityDelegatorArgumentsPara(engine, abilityDelegatorArgs->GetTestParam()));
    object->SetProperty("testCaseNames", CreateJsValue(engine, abilityDelegatorArgs->GetTestCaseName()));
    object->SetProperty("testRunnerClassName", CreateJsValue(engine, abilityDelegatorArgs->GetTestRunnerClassName()));

    return objValue;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
