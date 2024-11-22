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
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onAbilityCreate", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onAbilityForeground", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onAbilityBackground", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnAbilityDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onAbilityDestroy", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onWindowStageCreate", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onWindowStageRestore", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    CallLifecycleCBFunction("onWindowStageDestroy", abilityObj);

    RESMGR_HILOGI(RESMGR_TAG, "exit");
}

void JSAbilityMonitor::SetJsAbilityMonitor(napi_value jsAbilityMonitor)
{
    RESMGR_HILOGI(RESMGR_TAG, "enter");

    napi_ref ref = nullptr;
    napi_create_reference(env_, jsAbilityMonitor, 1, &ref);
    jsAbilityMonitor_ = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

napi_value JSAbilityMonitor::CallLifecycleCBFunction(const std::string &functionName,
    const std::weak_ptr<NativeReference> &abilityObj)
{
    if (functionName.empty()) {
        RESMGR_HILOGE(RESMGR_TAG, "Invalid function name");
        return nullptr;
    }

    if (!jsAbilityMonitor_) {
        RESMGR_HILOGE(RESMGR_TAG, "Invalid jsAbilityMonitor");
        return nullptr;
    }

    napi_value obj = jsAbilityMonitor_->GetNapiValue();
    if (obj == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "Failed to get object");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env_, obj, functionName.data(), &method);
    if (method == nullptr) {
        RESMGR_HILOGE(RESMGR_TAG, "Failed to get %{public}s from object", functionName.data());
        return nullptr;
    }

    auto nativeAbilityObj = CreateJsNull(env_);
    if (!abilityObj.expired()) {
        nativeAbilityObj = abilityObj.lock()->GetNapiValue();
    }

    napi_value argv[] = { nativeAbilityObj };
    napi_value callResult = nullptr;
    napi_call_function(env_, obj, method, ArraySize(argv), argv, &callResult);
    return callResult;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
