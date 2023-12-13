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

#include "js_ability_monitor.h"

#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
JSAbilityMonitor::JSAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

JSAbilityMonitor::JSAbilityMonitor(const std::string &abilityName, const std::string &moduleName)
    : abilityName_(abilityName), moduleName_(moduleName)
{}

void JSAbilityMonitor::OnAbilityCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onAbilityCreate", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onAbilityForeground", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onAbilityBackground", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onWindowStageCreate", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onWindowStageRestore", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    CallLifecycleCBFunction("onWindowStageDestroy", abilityObj);

    HILOG_INFO("exit");
}

void JSAbilityMonitor::SetJsAbilityMonitor(NativeValue *jsAbilityMonitor)
{
    HILOG_INFO("enter");

    jsAbilityMonitor_ = std::unique_ptr<NativeReference>(engine_->CreateReference(jsAbilityMonitor, 1));
}

NativeValue *JSAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<NativeReference> &abilityObj)
{
    if (functionName.empty()) {
        HILOG_ERROR("Invalid function name");
        return nullptr;
    }

    if (!jsAbilityMonitor_) {
        HILOG_ERROR("Invalid jsAbilityMonitor");
        return nullptr;
    }

    auto value = jsAbilityMonitor_->Get();
    auto obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto method = obj->GetProperty(functionName.data());
    if (method == nullptr) {
        HILOG_ERROR("Failed to get %{public}s from object", functionName.data());
        return nullptr;
    }

    auto nativeAbilityObj = engine_->CreateNull();
    if (!abilityObj.expired()) {
        nativeAbilityObj = abilityObj.lock()->Get();
    }

    NativeValue* argv[] = { nativeAbilityObj };
    return engine_->CallFunction(value, method, argv, ArraySize(argv));
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
