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
#include "hilog.h"

namespace OHOS {
namespace Rosen {
JsWindowRegisterManager::JsWindowRegisterManager()
{
    // white register list for window manager
    listenerProcess_[CaseType::CASE_WINDOW_MANAGER] = {
    };
    // white register list for window
    listenerProcess_[CaseType::CASE_WINDOW] = {
        { "windowEvent",              &JsWindowRegisterManager::ProcessLifeCycleEventRegister    },
    };
    // white register list for window stage
    listenerProcess_[CaseType::CASE_STAGE] = {
        {WINDOW_STAGE_EVENT_CB,         &JsWindowRegisterManager::ProcessLifeCycleEventRegister    }
    };
}

JsWindowRegisterManager::~JsWindowRegisterManager()
{
}

WmErrorCode JsWindowRegisterManager::ProcessLifeCycleEventRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<Window> window, bool isRegister)
{
    HILOG_INFO("JsWindowRegisterManager::ProcessLifeCycleEventRegister : Start...");
    if (window == nullptr) {
        HILOG_ERROR("JsWindowRegisterManager::ProcessLifeCycleEventRegister : [NAPI]Window is nullptr");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    sptr<IWindowLifeCycle> thisListener(listener);
    WmErrorCode ret = WmErrorCode::WM_OK;
    if (isRegister) {
        HILOG_INFO("JsWindowRegisterManager::ProcessLifeCycleEventRegister : RegisterLifeCycleListener");
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->RegisterLifeCycleListener(thisListener));
    } else {
        HILOG_INFO("JsWindowRegisterManager::ProcessLifeCycleEventRegister : UnregisterLifeCycleListener");
        ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UnregisterLifeCycleListener(thisListener));
    }
    HILOG_INFO("JsWindowRegisterManager::ProcessLifeCycleEventRegister : End!!! result=[%{public}d]", static_cast<int32_t>(ret));
    return ret;
}

bool JsWindowRegisterManager::IsCallbackRegistered(std::string type, NativeValue* jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        HILOG_ERROR("[NAPI]Method %{public}s has not been registerted", type.c_str());
        return false;
    }

    for (auto iter = jsCbMap_[type].begin(); iter != jsCbMap_[type].end(); ++iter) {
        if (jsListenerObject->StrictEquals(iter->first->Get())) {
            HILOG_ERROR("[NAPI]Method %{public}s has already been registered", type.c_str());
            return true;
        }
    }
    return false;
}

WmErrorCode JsWindowRegisterManager::RegisterListener(std::shared_ptr<Window> window, std::string type,
    CaseType caseType, NativeEngine& engine, NativeValue* value)
{
    HILOG_ERROR("JsWindowRegisterManager RegisterListener%p", this);
    std::lock_guard<std::mutex> lock(mtx_);
    if (IsCallbackRegistered(type, value)) {
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerProcess_[caseType].count(type) == 0) {
        HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    std::shared_ptr<NativeReference> callbackRef;
    callbackRef.reset(engine.CreateReference(value, 1));
    sptr<JsWindowListener> windowManagerListener = new(std::nothrow) JsWindowListener(type, &engine, callbackRef);
    if (windowManagerListener == nullptr) {
        HILOG_ERROR("JsWindowRegisterManager::RegisterListener : New JsWindowListener failed");
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    WmErrorCode ret = (this->*listenerProcess_[caseType][type])(windowManagerListener, window, true);
    HILOG_ERROR("JsWindowRegisterManager::RegisterListener : listenerProcess_[caseType] success");
    if (ret != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Register type %{public}s failed", type.c_str());
        return ret;
    }
    jsCbMap_[type][callbackRef] = windowManagerListener;
    HILOG_INFO("JsWindowRegisterManager::RegisterListener : Register type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    return WmErrorCode::WM_OK;
}

WmErrorCode JsWindowRegisterManager::UnregisterListener(std::shared_ptr<Window> window, std::string type,
    CaseType caseType, NativeValue* value)
{
    HILOG_ERROR("JsWindowRegisterManager UnregisterListener%p", this);
    std::lock_guard<std::mutex> lock(mtx_);
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Type %{public}s was not registerted", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (listenerProcess_[caseType].count(type) == 0) {
        HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Type %{public}s is not supported", type.c_str());
        return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
    }
    if (value == nullptr) {
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
            WmErrorCode ret = (this->*listenerProcess_[caseType][type])(it->second, window, false);
            if (ret != WmErrorCode::WM_OK) {
                HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Unregister type %{public}s failed, no value", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it++);
        }
    } else {
        bool findFlag = false;
        for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end(); ++it) {
            if (!value->StrictEquals(it->first->Get())) {
                continue;
            }
            findFlag = true;
            WmErrorCode ret = (this->*listenerProcess_[caseType][type])(it->second, window, false);
            if (ret != WmErrorCode::WM_OK) {
                HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Unregister type %{public}s failed", type.c_str());
                return ret;
            }
            jsCbMap_[type].erase(it);
            break;
        }
        if (!findFlag) {
            HILOG_ERROR("JsWindowRegisterManager::RegisterListener : Unregister type %{public}s failed because not found callback!", type.c_str());
            return WmErrorCode::WM_ERROR_STATE_ABNORMALLY;
        }
    }
    HILOG_INFO("JsWindowRegisterManager::RegisterListener : Unregister type %{public}s success! callback map size: %{public}zu",
        type.c_str(), jsCbMap_[type].size());
    // erase type when there is no callback in one type
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return WmErrorCode::WM_OK;
}
} // namespace Rosen
} // namespace OHOS
