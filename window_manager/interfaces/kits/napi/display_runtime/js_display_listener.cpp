/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "js_display_listener.h"

#include "dm_common.h"
#include "js_runtime_utils.h"
#include "window_hilog.h"
#include "js_display.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
// namespace {
// constexpr HiviewDFX::HiLogLabel LABEL = {LOG_CORE, HILOG_DOMAIN_DISPLAY, "JsDisplayListener"};
// }

JsDisplayListener::JsDisplayListener(napi_env env) : env_(env), weakRef_(wptr<JsDisplayListener> (this))
{
    napi_add_env_cleanup_hook(env_, CleanEnv, this);
}

JsDisplayListener::~JsDisplayListener()
{
    napi_remove_env_cleanup_hook(env_, CleanEnv, this);
    env_ = nullptr;
}

void JsDisplayListener::CleanEnv(void* obj)
{
    JsDisplayListener* thisObj = reinterpret_cast<JsDisplayListener*>(obj);
    if (!thisObj) {
        WLOGE("obj is nullptr");
        return;
    }
    thisObj->env_ = nullptr;
}

void JsDisplayListener::AddCallback(const std::string& type, napi_value jsListenerObject)
{
    WLOGD("JsDisplayListener::AddCallback is called");
    std::unique_ptr<NativeReference> callbackRef;
    if (env_ == nullptr) {
        WLOGE("env_ nullptr");
        return;
    }
    napi_ref result = nullptr;
    napi_create_reference(env_, jsListenerObject, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    std::lock_guard<std::mutex> lock(mtx_);
    jsCallBack_[type].emplace_back(std::move(callbackRef));
    WLOGD("JsDisplayListener::AddCallback success jsCallBack_ size: %{public}u!",
        static_cast<uint32_t>(jsCallBack_[type].size()));
}

void JsDisplayListener::RemoveAllCallback()
{
    std::lock_guard<std::mutex> lock(mtx_);
    jsCallBack_.clear();
}

void JsDisplayListener::RemoveCallback(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = jsCallBack_.find(type);
    if (it == jsCallBack_.end()) {
        WLOGE("JsDisplayListener::RemoveCallback no callback to remove");
        return;
    }
    auto& listeners = it->second;
    for (auto iter = listeners.begin(); iter != listeners.end();) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, (*iter)->GetNapiValue(), &isEquals);
        if (isEquals) {
            iter = listeners.erase(iter);
        } else {
            iter++;
        }
    }
    WLOGI("JsDisplayListener::RemoveCallback success jsCallBack_ size: %{public}u!",
        static_cast<uint32_t>(listeners.size()));
}

void JsDisplayListener::CallJsMethod(const std::string& methodName, napi_value const * argv, size_t argc)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (methodName.empty()) {
        WLOGE("empty method name str, call method failed");
        return;
    }
    WLOGD("CallJsMethod methodName = %{public}s", methodName.c_str());
    if (env_ == nullptr) {
        WLOGE("env_ nullptr");
        return;
    }
    for (auto& callback : jsCallBack_[methodName]) {
        napi_value method = callback->GetNapiValue();
        if (method == nullptr) {
            WLOGE("Failed to get method callback from object");
            continue;
        }
        napi_call_function(env_, CreateJsUndefined(env_), method, argc, argv, nullptr);
    }
}

void JsDisplayListener::OnCreate(DisplayId id)
{
    DispatchDisplayEvent(EVENT_ADD, id);
}

void JsDisplayListener::OnDestroy(DisplayId id)
{
    DispatchDisplayEvent(EVENT_REMOVE, id);
}

void JsDisplayListener::OnChange(DisplayId id)
{
    DispatchDisplayEvent(EVENT_CHANGE, id);
}

void JsDisplayListener::DispatchDisplayEvent(const std::string& eventType, DisplayId id)
{
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCallBack_.empty()) {
        WLOGE("JsDisplayListener::DispatchDisplayEvent not register!");
        return;
    }
    if (jsCallBack_.find(eventType) == jsCallBack_.end()) {
        WLOGE("JsDisplayListener::%{public}s not this event", eventType.c_str());
        return;
    }
    auto napiTask = [self = weakRef_, id, env = env_, eventType]() {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            WLOGE("[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, static_cast<uint32_t>(id))};
        thisListener->CallJsMethod(eventType, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate);
        if (ret != napi_status::napi_ok) {
            WLOGE("DispatchDisplayEvent: Failed to SendEvent.");
        }
    } else {
        WLOGE("DispatchDisplayEvent: env is nullptr");
    }
}

void JsDisplayListener::OnFoldStatusChanged(FoldStatus foldStatus)
{
    std::lock_guard<std::mutex> lock(mtx_);
    WLOGI("OnFoldStatusChanged is called, foldStatus: %{public}u", static_cast<uint32_t>(foldStatus));
    if (jsCallBack_.empty()) {
        WLOGE("OnFoldStatusChanged not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_FOLD_STATUS_CHANGED) == jsCallBack_.end()) {
        WLOGE("OnFoldStatusChanged not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, foldStatus, env = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            WLOGE("[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, foldStatus)};
        thisListener->CallJsMethod(EVENT_FOLD_STATUS_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate);
        if (ret != napi_status::napi_ok) {
            WLOGE("OnFoldStatusChanged: Failed to SendEvent.");
        }
    } else {
        WLOGE("OnFoldStatusChanged: env is nullptr");
    }
}

void JsDisplayListener::OnDisplayModeChanged(FoldDisplayMode displayMode)
{
    std::lock_guard<std::mutex> lock(mtx_);
    WLOGI("OnDisplayModeChanged is called, displayMode: %{public}u", static_cast<uint32_t>(displayMode));
    if (jsCallBack_.empty()) {
        WLOGE("OnDisplayModeChanged not register!");
        return;
    }
    if (jsCallBack_.find(EVENT_DISPLAY_MODE_CHANGED) == jsCallBack_.end()) {
        WLOGE("OnDisplayModeChanged not this event, return");
        return;
    }
    auto napiTask = [self = weakRef_, displayMode, env = env_] () {
        auto thisListener = self.promote();
        if (thisListener == nullptr || env == nullptr) {
            WLOGE("[NAPI]this listener or env is nullptr");
            return;
        }
        napi_value argv[] = {CreateJsValue(env, displayMode)};
        thisListener->CallJsMethod(EVENT_DISPLAY_MODE_CHANGED, argv, ArraySize(argv));
    };

    if (env_ != nullptr) {
        napi_status ret = napi_send_event(env_, napiTask, napi_eprio_immediate);
        if (ret != napi_status::napi_ok) {
            WLOGE("OnDisplayModeChanged: Failed to SendEvent.");
        }
    } else {
        WLOGE("OnDisplayModeChanged: env is nullptr");
    }
}

} // namespace Rosen
} // namespace OHOS
