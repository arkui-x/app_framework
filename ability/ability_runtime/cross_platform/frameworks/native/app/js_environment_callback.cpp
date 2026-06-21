/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_environment_callback.h"

#include "hilog.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

int32_t JsEnvironmentCallback::serialNumber_ = 0;

JsEnvironmentCallback::JsEnvironmentCallback(napi_env env) : env_(env) {}

int32_t JsEnvironmentCallback::Register(napi_value value)
{
    if (env_ == nullptr) {
        HILOG_ERROR("env_ is nullptr");
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
    callbacks_.emplace(callbackId,
        std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    HILOG_INFO("Register environment callback, callbackId: %{public}d", callbackId);
    return callbackId;
}

bool JsEnvironmentCallback::UnRegister(int32_t callbackId)
{
    HILOG_INFO("UnRegister environment callback, callbackId: %{public}d", callbackId);
    auto it = callbacks_.find(callbackId);
    if (it == callbacks_.end()) {
        HILOG_ERROR("callbackId %{public}d not found", callbackId);
        return false;
    }
    return callbacks_.erase(callbackId) == 1;
}

bool JsEnvironmentCallback::IsEmpty() const
{
    return callbacks_.empty();
}

void JsEnvironmentCallback::CallConfigurationUpdatedInner(
    const Configuration& config,
    const std::map<int32_t, std::shared_ptr<NativeReference>>& callbacks)
{
    if (env_ == nullptr) {
        HILOG_ERROR("env_ is nullptr");
        return;
    }
    for (auto& callback : callbacks) {
        if (!callback.second) {
            HILOG_ERROR("Invalid jsCallback");
            continue;
        }
        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            HILOG_ERROR("Failed to get callback object");
            continue;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onConfigurationUpdated", &method);
        if (method == nullptr) {
            HILOG_DEBUG("onConfigurationUpdated not defined on callback");
            continue;
        }
        napi_value argv[] = { CreateJsConfiguration(env_, config) };
        napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    }
}

void JsEnvironmentCallback::CallMemoryLevelInner(
    int32_t level,
    const std::map<int32_t, std::shared_ptr<NativeReference>>& callbacks)
{
    if (env_ == nullptr) {
        HILOG_ERROR("env_ is nullptr");
        return;
    }
    for (auto& callback : callbacks) {
        if (!callback.second) {
            HILOG_ERROR("Invalid jsCallback");
            continue;
        }
        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            HILOG_ERROR("Failed to get callback object");
            continue;
        }
        napi_value method = nullptr;
        napi_get_named_property(env_, obj, "onMemoryLevel", &method);
        if (method == nullptr) {
            HILOG_DEBUG("onMemoryLevel not defined on callback");
            continue;
        }
        napi_value jsLevel = nullptr;
        napi_create_int32(env_, level, &jsLevel);
        napi_value argv[] = { jsLevel };
        napi_call_function(env_, obj, method, ArraySize(argv), argv, nullptr);
    }
}

void JsEnvironmentCallback::OnConfigurationUpdated(const Configuration& config)
{
    HILOG_INFO("OnConfigurationUpdated called");
    if (callbacks_.empty()) {
        return;
    }
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [thisWeakPtr, config, callbacks = callbacks_](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                std::shared_ptr<JsEnvironmentCallback> jsCallback = thisWeakPtr.lock();
                if (jsCallback) {
                    jsCallback->CallConfigurationUpdatedInner(config, callbacks);
                }
            });
    napi_ref callbackRef = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsEnvironmentCallback::OnConfigurationUpdated", env_,
        std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete)));
}

void JsEnvironmentCallback::OnMemoryLevel(int32_t level)
{
    HILOG_INFO("OnMemoryLevel called, level: %{public}d", level);
    if (callbacks_.empty()) {
        return;
    }
    std::weak_ptr<JsEnvironmentCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [thisWeakPtr, level, callbacks = callbacks_](
                napi_env env, NapiAsyncTask& task, int32_t status) {
                std::shared_ptr<JsEnvironmentCallback> jsCallback = thisWeakPtr.lock();
                if (jsCallback) {
                    jsCallback->CallMemoryLevelInner(level, callbacks);
                }
            });
    napi_ref callbackRef = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsEnvironmentCallback::OnMemoryLevel", env_,
        std::make_unique<NapiAsyncTask>(callbackRef, std::move(execute), std::move(complete)));
}

} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
