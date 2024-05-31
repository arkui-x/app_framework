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

#include "ability_lifecycle_callback.h"

#include "hilog.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
JsAbilityLifecycleCallback::JsAbilityLifecycleCallback(napi_env env) : env_(env) {}

int32_t JsAbilityLifecycleCallback::serialNumber_ = 0;

void JsAbilityLifecycleCallback::CallJsMethodInnerCommon(const std::string& methodName,
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage,
    const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks)
{
    if (env_ == nullptr) {
        HILOG_ERROR("CallJsMethodInnerCommon, env_ is nullptr");
        return;
    }

    auto nativeAbilityObj = CreateJsNull(env_);
    if (ability != nullptr) {
        nativeAbilityObj = ability->GetNapiValue();
    }

    bool isWindowStage = false;
    auto nativeWindowStageObj = CreateJsNull(env_);
    if (windowStage != nullptr) {
        nativeWindowStageObj = windowStage->GetNapiValue();
        isWindowStage = true;
    }

    for (auto& callback : callbacks) {
        if (!callback.second) {
            HILOG_ERROR("CallJsMethodInnerCommon, Invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            HILOG_ERROR("CallJsMethodInnerCommon, Failed to get object");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName.data(), &method);
        if (method == nullptr) {
            HILOG_ERROR("CallJsMethodInnerCommon, Failed to get %{public}s from object", methodName.data());
            return;
        }

        if (!isWindowStage) {
            napi_value argv[] = { nativeAbilityObj };
            napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
        } else {
            napi_value argv[] = { nativeAbilityObj, nativeWindowStageObj };
            napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
        }
    }
}

void JsAbilityLifecycleCallback::CallJsMethod(
    const std::string& methodName, const std::shared_ptr<NativeReference>& ability)
{
    HILOG_DEBUG("CallJsMethod methodName = %{public}s", methodName.c_str());
    if (!ability) {
        HILOG_ERROR("ability is nullptr");
        return;
    }
    std::weak_ptr<JsAbilityLifecycleCallback> thisWeakPtr(shared_from_this());
    
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, ability, callbacks = callbacks_](napi_env env, NapiAsyncTask &task, int32_t status)
        {
            std::shared_ptr<JsAbilityLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback) {
                jsCallback->CallJsMethodInnerCommon(methodName, ability, nullptr, callbacks);
            }
        }
    );
    
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAbilityLifecycleCallback::CallJsMethod:" + methodName, env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsAbilityLifecycleCallback::CallWindowStageJsMethod(const std::string& methodName,
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage)
{
    HILOG_DEBUG("CallWindowStageJsMethod methodName = %{public}s", methodName.c_str());
    if (!ability || !windowStage) {
        HILOG_ERROR("ability or windowStage is nullptr");
        return;
    }
    std::weak_ptr<JsAbilityLifecycleCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, ability, windowStage, callbacks = callbacks_](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsAbilityLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback) {
                jsCallback->CallJsMethodInnerCommon(methodName, ability, windowStage, callbacks);
            }
        });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsAbilityLifecycleCallback::CallWindowStageJsMethod:" + methodName, env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsAbilityLifecycleCallback::OnAbilityCreate(const std::shared_ptr<NativeReference>& ability)
{
    CallJsMethod("onAbilityCreate", ability);
}

void JsAbilityLifecycleCallback::OnWindowStageCreate(
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage)
{
    CallWindowStageJsMethod("onWindowStageCreate", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnWindowStageDestroy(
    const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage)
{
    CallWindowStageJsMethod("onWindowStageDestroy", ability, windowStage);
}

void JsAbilityLifecycleCallback::OnAbilityDestroy(const std::shared_ptr<NativeReference>& ability)
{
    CallJsMethod("onAbilityDestroy", ability);
}

void JsAbilityLifecycleCallback::OnAbilityForeground(const std::shared_ptr<NativeReference>& ability)
{
    CallJsMethod("onAbilityForeground", ability);
}

void JsAbilityLifecycleCallback::OnAbilityBackground(const std::shared_ptr<NativeReference>& ability)
{
    CallJsMethod("onAbilityBackground", ability);
}

int32_t JsAbilityLifecycleCallback::Register(napi_value value)
{
    if (env_ == nullptr) {
        return -1;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, value, 1, &ref);
    callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return callbackId;
}

bool JsAbilityLifecycleCallback::UnRegister(int32_t callbackId)
{
    HILOG_INFO("UnRegister called, callbackId : %{public}d", callbackId);
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        HILOG_ERROR("UnRegister callbackId: %{public}d is not in callbacks_", callbackId);
        return false;
    }
    return callbacks_.erase(callbackId) == 1;
}

bool JsAbilityLifecycleCallback::IsEmpty() const
{
    return callbacks_.empty();
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
