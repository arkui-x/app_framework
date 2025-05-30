/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_window_register_manager.h"

#include "js_window_listener.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
JsWindowRegisterManager::JsWindowRegisterManager()
{
    // white register list for window manager
    listenerProcess_[CaseType::CASE_WINDOW_MANAGER] = {
    };
    // white register list for window
    listenerProcess_[CaseType::CASE_WINDOW] = {
        { "windowSizeChange",              &JsWindowRegisterManager::ProcessSizeChangeRegister    },
        { "windowEvent",              &JsWindowRegisterManager::ProcessLifeCycleEventRegister    },
        {AVOID_AREA_CHANGE_CB,        &JsWindowRegisterManager::ProcessAvoidAreaChangeRegister    },
        {WINDOW_STATUS_CHANGE_CB,   &JsWindowRegisterManager::ProcessWindowStatusChangeRegister    }
    };
    // white register list for window stage
    listenerProcess_[CaseType::CASE_STAGE] = {
        {"windowStageEvent",         &JsWindowRegisterManager::ProcessLifeCycleEventRegister    },
    };
}

JsWindowRegisterManager& JsWindowRegisterManager::GetInstance() {
    static JsWindowRegisterManager instance;
    return instance;
}

JsWindowRegisterManager::~JsWindowRegisterManager()
{
}

WmErrorCode JsWindowRegisterManager::ProcessSizeChangeRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<Window> window, bool isRegister)
{
    if (window == nullptr) {
        WLOGE("JsWindowRegisterManager::ProcessSizeChangeRegister : [NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowChangeListener> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<OHOS::Rosen::Window> window, bool isRegister)
{
    if (window == nullptr) {
        WLOGE("[WindowRegister] window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IAvoidAreaChangedListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterAvoidAreaChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterAvoidAreaChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessWindowStatusChangeRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<OHOS::Rosen::Window> window, bool isRegister)
{
    if (window == nullptr) {
        WLOGE("[WindowRegister] window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowStatusChangeListener> thisListener(listener);
    WmErrorCode ret;
    if (isRegister) {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterWindowStatusChangeListener(thisListener));
    } else {
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterWindowStatusChangeListener(thisListener));
    }
    return ret;
}

WmErrorCode JsWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<Window> window, bool isRegister)
{
    WLOGI("JsWindowRegisterManager::ProcessLifeCycleEventRegister : Start...");
    if (window == nullptr) {
        WLOGE("JsWindowRegisterManager::ProcessLifeCycleEventRegister : [NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        WLOGI("JsWindowRegisterManager::ProcessLifeCycleEventRegister : RegisterLifeCycleListener");
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterLifeCycleListener(thisListener));
    } else {
        WLOGI("JsWindowRegisterManager::ProcessLifeCycleEventRegister : UnregisterLifeCycleListener");
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterLifeCycleListener(thisListener));
    }
    WLOGI("JsWindowRegisterManager::ProcessLifeCycleEventRegister : End!!! result=[%{public}d]", static_cast<int32_t>(ret));
    return ret;
}

bool JsWindowRegisterManager::IsEqualRegister(napi_env env, napi_value lhs, napi_ref rref)
{
    napi_value rhs;
    napi_status status = napi_get_reference_value(env, rref, &rhs);
    if (status != napi_ok) {
        return false;
    }
    bool result = false;
    status = napi_strict_equals(env, lhs, rhs, &result);
    return status == napi_ok && result;
}

bool JsWindowRegisterManager::IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGE("[NAPI]Method %{public}s has not been registerted", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        if (IsEqualRegister(env, jsListenerObject, iter->first)) {
            WLOGE("[NAPI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode JsWindowRegisterManager::RegisterListener(napi_env env, std::shared_ptr<Window> window, std::string type,
    CaseType caseType, napi_value value)
{
    WLOGE("JsWindowRegisterManager RegisterListener%p", this);
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(env, type, value)) {
        return WmErrorCode::WM_OK;
    }
    if (listenerProcess_[caseType].count(type) == 0) {
        WLOGE("JsWindowRegisterManager::RegisterListener : Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    napi_ref callbackRef;
    napi_status status = napi_create_reference(env, value, 1, &callbackRef);
    if (status != napi_ok) {
        WLOGE("JsWindowRegisterManager::RegisterListener : create ref fail");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }

    sptr<JsWindowListener> windowManagerListener = new(std::nothrow) JsWindowListener(type, env, callbackRef);
    if (windowManagerListener == nullptr) {
        WLOGE("JsWindowRegisterManager::RegisterListener : New JsWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = (this->*listenerProcess_[caseType][type])(windowManagerListener, window, true);
    WLOGE("JsWindowRegisterManager::RegisterListener : listenerProcess_[caseType] success");
    if (ret != WmErrorCode::WM_OK) {
        WLOGE("JsWindowRegisterManager::RegisterListener : Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = windowManagerListener;
    WLOGI("JsWindowRegisterManager::RegisterListener : Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::UnregisterListener(napi_env env, std::shared_ptr<Window> window, std::string type,
    CaseType caseType, napi_value value)
{
    WLOGE("JsWindowRegisterManager UnregisterListener%p", this);
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGE("JsWindowRegisterManager::RegisterListener : Type %{public}s was not registerted", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerProcess_[caseType].count(type) == 0) {
        WLOGE("JsWindowRegisterManager::RegisterListener : Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = (this->*listenerProcess_[caseType][type])(it->second, window, false);
            if (ret != WmErrorCode::WM_OK) {
                WLOGE("JsWindowRegisterManager::RegisterListener : Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); ++it) {
            if (!IsEqualRegister(env, value, it->first)) {
                continue;
            }
            findFlag = true;
            WmErrorCode ret = (this->*listenerProcess_[caseType][type])(it->second, window, false);
            if (ret != WmErrorCode::WM_OK) {
                WLOGE("JsWindowRegisterManager::UnregisterListener : Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            WLOGE("JsWindowRegisterManager::UnregisterListener : Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    WLOGI("JsWindowRegisterManager::UnregisterListener : Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}
} // namespace Rosen
} // namespace OHOS
