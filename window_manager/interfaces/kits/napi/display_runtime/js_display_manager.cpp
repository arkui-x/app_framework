/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "js_display_manager.h"

#include <new>
#include <vector>
#include <cinttypes>

#include "display.h"
#include "display_manager.h"
#include "js_display.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "singleton_container.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_COUNT = 4;
constexpr int32_t INDEX_ONE = 1;

void JsDisplayManager::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("Finalizer is called");
    std::unique_ptr<JsDisplayManager>(static_cast<JsDisplayManager*>(data));
}

napi_value JsDisplayManager::GetDefaultDisplaySync(napi_env env, napi_callback_info info)
{
    WLOGD("JsDisplayManager::GetDefaultDisplaySync");
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (me != nullptr) ? me->OnGetDefaultDisplaySync(env, info) : nullptr;
}

napi_value JsDisplayManager::OnGetDefaultDisplaySync(napi_env env, napi_callback_info info)
{
    WLOGD("JsDisplayManager::OnGetDefaultDisplaySync : Start...");
    sptr<Display> display = DisplayManager::GetInstance().GetDefaultDisplaySync();
    if (display == nullptr) {
        WLOGE("OnGetDefaultDisplaySync, display is nullptr.");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN)));
        return CreateJsUndefined(env);
    }
    napi_value value = nullptr;
    value = CreateJsDisplayObject(env, display);
    if (value == nullptr) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));
        return CreateJsUndefined(env);
    }
    return value;
}

napi_value JsDisplayManager::RegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplayManager* displayManager = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (displayManager != nullptr) ? displayManager->OnRegisterDisplayManagerCallback(env, info) : nullptr;
}

napi_value JsDisplayManager::UnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    JsDisplayManager* displayManager = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (displayManager != nullptr) ? displayManager->OnUnregisterDisplayManagerCallback(env, info) : nullptr;
}

napi_value JsDisplayManager::IsFoldable(napi_env env, napi_callback_info info)
{
    auto* displayManager = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (displayManager != nullptr) ? displayManager->OnIsFoldable(env, info) : nullptr;
}

napi_value JsDisplayManager::GetFoldStatus(napi_env env, napi_callback_info info)
{
    auto* displayManager = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (displayManager != nullptr) ? displayManager->OnGetFoldStatus(env, info) : nullptr;
}

napi_value JsDisplayManager::GetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    auto* displayManager = CheckParamsAndGetThis<JsDisplayManager>(env, info);
    return (displayManager != nullptr) ? displayManager->OnGetFoldDisplayMode(env, info) : nullptr;
}

static napi_value InitDisplayState(napi_env env)
{
    WLOGD("InitDisplayState called");
    napi_value objValue = nullptr;
    napi_status status = napi_create_object(env, &objValue);
    if (status != napi_ok) {
        WLOGE("Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, objValue, "STATE_UNKNOWN",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_UNKNOWN)));
    napi_set_named_property(env, objValue, "STATE_OFF",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_OFF)));
    napi_set_named_property(env, objValue, "STATE_ON",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_ON)));
    napi_set_named_property(env, objValue, "STATE_DOZE",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_DOZE)));
    napi_set_named_property(env, objValue, "STATE_DOZE_SUSPEND",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_DOZE_SUSPEND)));
    napi_set_named_property(env, objValue, "STATE_VR",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_VR)));
    napi_set_named_property(env, objValue, "STATE_ON_SUSPEND",
        CreateJsValue(env, static_cast<int32_t>(DisplayStateMode::STATE_ON_SUSPEND)));
    return objValue;
}

static napi_value InitOrientation(napi_env env)
{
    WLOGD("InitOrientation called");
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        WLOGE("Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, obj, "PORTRAIT", CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::PORTRAIT)));
    napi_set_named_property(env, obj, "LANDSCAPE", CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE)));
    napi_set_named_property(env, obj, "PORTRAIT_INVERTED",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::PORTRAIT_INVERTED)));
    napi_set_named_property(env, obj, "LANDSCAPE_INVERTED",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE_INVERTED)));
    return obj;
}

static napi_value InitDisplayErrorCode(napi_env env)
{
    WLOGD("InitDisplayErrorCode called");
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        WLOGE("Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, obj, "DM_ERROR_NO_PERMISSION",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_NO_PERMISSION)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_PARAM",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, obj, "DM_ERROR_DEVICE_NOT_SUPPORT",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_SCREEN",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_CALLING",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, obj, "DM_ERROR_SYSTEM_INNORMAL",
        CreateJsValue(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));

    return obj;
}

static napi_value InitDisplayError(napi_env env)
{
    WLOGD("InitDisplayError called");
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        WLOGE("Failed to create object");
        return nullptr;
    }

    napi_set_named_property(env, obj, "DM_ERROR_INIT_DMS_PROXY_LOCKED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED)));
    napi_set_named_property(env, obj, "DM_ERROR_IPC_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_IPC_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_REMOTE_CREATE_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_REMOTE_CREATE_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_NULLPTR",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_NULLPTR)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_PARAM",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM)));
    napi_set_named_property(env, obj, "DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_DEATH_RECIPIENT",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_DEATH_RECIPIENT)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_MODE_ID",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_MODE_ID)));
    napi_set_named_property(env, obj, "DM_ERROR_WRITE_DATA_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_WRITE_DATA_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_RENDER_SERVICE_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_RENDER_SERVICE_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_UNREGISTER_AGENT_FAILED",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_UNREGISTER_AGENT_FAILED)));
    napi_set_named_property(env, obj, "DM_ERROR_INVALID_CALLING",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_INVALID_CALLING)));
    napi_set_named_property(env, obj, "DM_ERROR_UNKNOWN",
        CreateJsValue(env, static_cast<int32_t>(DMError::DM_ERROR_UNKNOWN)));
    return obj;
}

napi_value InitFoldStatus(napi_env env)
{
    WLOGD("InitFoldStatus called");
    if (env == nullptr) {
        WLOGE("env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "FOLD_STATUS_UNKNOWN",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::UNKNOWN)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_EXPANDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::EXPAND)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::FOLDED)));
    napi_set_named_property(env, objValue, "FOLD_STATUS_HALF_FOLDED",
        CreateJsValue(env, static_cast<uint32_t>(FoldStatus::HALF_FOLD)));
    return objValue;
}

napi_value InitFoldDisplayMode(napi_env env)
{
    WLOGD("IniFoldDisplayMode called");
    if (env == nullptr) {
        WLOGE("env is nullptr");
        return nullptr;
    }
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGE("Failed to get object");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_UNKNOWN",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::UNKNOWN)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_FULL",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::FULL)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_MAIN",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::MAIN)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_SUB",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::SUB)));
    napi_set_named_property(env, objValue, "FOLD_DISPLAY_MODE_COORDINATION",
        CreateJsValue(env, static_cast<uint32_t>(FoldDisplayMode::COORDINATION)));
    return objValue;
}

bool JsDisplayManager::IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("IsCallbackRegistered methodName %{public}s not registered!", type.c_str());
        return false;
    }

    for (auto& iter : jsCbMap_[type]) {
        bool isEquals = false;
        napi_strict_equals(env, jsListenerObject, iter.first->GetNapiValue(), &isEquals);
        if (isEquals) {
            WLOGE("IsCallbackRegistered callback already registered!");
            return true;
        }
    }
    return false;
}

DMError JsDisplayManager::RegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (IsCallbackRegistered(env, type, value)) {
        WLOGE("RegisterDisplayListenerWithType callback already registered!");
        return DMError::DM_OK;
    }
    std::unique_ptr<NativeReference> callbackRef;
    napi_ref result = nullptr;
    napi_create_reference(env, value, 1, &result);
    callbackRef.reset(reinterpret_cast<NativeReference*>(result));
    sptr<JsDisplayListener> displayListener = new(std::nothrow) JsDisplayListener(env);
    DMError ret = DMError::DM_OK;
    if (displayListener == nullptr) {
        WLOGE("displayListener is nullptr");
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayListener(displayListener);
    } else if (type == EVENT_FOLD_STATUS_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterFoldStatusListener(displayListener);
    } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
        ret = SingletonContainer::Get<DisplayManager>().RegisterDisplayModeListener(displayListener);
    } else {
        WLOGE("RegisterDisplayListenerWithType failed, %{public}s not support", type.c_str());
        return DMError::DM_ERROR_INVALID_PARAM;
    }
    if (ret != DMError::DM_OK) {
        WLOGE("RegisterDisplayListenerWithType failed, ret: %{public}u", ret);
        return ret;
    }
    displayListener->AddCallback(type, value);
    jsCbMap_[type][std::move(callbackRef)] = displayListener;
    return DMError::DM_OK;
}

DMError JsDisplayManager::UnregisterAllDisplayListenerWithType(const std::string& type)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("UnregisterAllDisplayListenerWithType methodName %{public}s not registered!",
            type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        it->second->RemoveAllCallback();
        if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
            sptr<DisplayManager::DisplayListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
        } else if (type == EVENT_FOLD_STATUS_CHANGED) {
            sptr<DisplayManager::FoldStatusListener> thisListener(it->second);
            ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
        } else {
            ret = DMError::DM_ERROR_INVALID_PARAM;
        }
        jsCbMap_[type].erase(it++);
        WLOGD("unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
    }
    jsCbMap_.erase(type);
    return ret;
}

DMError JsDisplayManager::UnRegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value)
{
    if (jsCbMap_.empty() || jsCbMap_.find(type) == jsCbMap_.end()) {
        WLOGI("UnRegisterDisplayListenerWithType methodName %{public}s not registered!", type.c_str());
        return DMError::DM_OK;
    }
    DMError ret = DMError::DM_OK;
    for (auto it = jsCbMap_[type].begin(); it != jsCbMap_[type].end();) {
        bool isEquals = false;
        napi_strict_equals(env, value, it->first->GetNapiValue(), &isEquals);
        if (isEquals) {
            it->second->RemoveCallback(env, type, value);
            if (type == EVENT_ADD || type == EVENT_REMOVE || type == EVENT_CHANGE) {
                sptr<DisplayManager::DisplayListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayListener(thisListener);
            } else if (type == EVENT_FOLD_STATUS_CHANGED) {
                sptr<DisplayManager::FoldStatusListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterFoldStatusListener(thisListener);
            } else if (type == EVENT_DISPLAY_MODE_CHANGED) {
                sptr<DisplayManager::DisplayModeListener> thisListener(it->second);
                ret = SingletonContainer::Get<DisplayManager>().UnregisterDisplayModeListener(thisListener);
            } else {
                ret = DMError::DM_ERROR_INVALID_PARAM;
            }
            jsCbMap_[type].erase(it++);
            WLOGI("unregister display listener with type %{public}s  ret: %{public}u", type.c_str(), ret);
            break;
        } else {
            it++;
        }
    }
    if (jsCbMap_[type].empty()) {
        jsCbMap_.erase(type);
    }
    return ret;
}

bool JsDisplayManager::NapiIsCallable(napi_env env, napi_value value)
{
    bool result = false;
    napi_is_callable(env, value, &result);
    return result;
}

napi_value JsDisplayManager::OnRegisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("OnRegisterDisplayManagerCallback is called");
    size_t argc = ARGC_COUNT;
    napi_value argv[ARGC_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < ARGC_TWO) {
        WLOGE("JsDisplayManager Params not match: %{public}zu", argc);
        std::string errMsg = "Invalid args count, need 2 args";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return CreateJsUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        std::string errMsg = "Failed to convert parameter to callbackType";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        WLOGE("Failed to convert parameter to callbackType");
        return CreateJsUndefined(env);
    }
    napi_value value = argv[INDEX_ONE];
    if (value == nullptr) {
        WLOGI("OnRegisterDisplayManagerCallback info->argv[1] is nullptr");
        std::string errMsg = "OnRegisterDisplayManagerCallback is nullptr";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return CreateJsUndefined(env);
    }
    if (!NapiIsCallable(env, value)) {
        WLOGI("OnRegisterDisplayManagerCallback info->argv[1] is not callable");
        std::string errMsg = "OnRegisterDisplayManagerCallback is not callable";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return CreateJsUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret = DM_JS_TO_ERROR_CODE_MAP.at(RegisterDisplayListenerWithType(env, cbType, value));
    if (ret != DmErrorCode::DM_OK) {
        DmErrorCode errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        if (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            errCode = ret;
        }
        WLOGE("Failed to register display listener with type");
        std::string errMsg = "Failed to register display listener with type";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(errCode), errMsg));
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsDisplayManager::OnIsFoldable(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_COUNT;
    napi_value argv[ARGC_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc >= ARGC_ONE) {
        WLOGE("Params not match %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }
    bool foldable = SingletonContainer::Get<DisplayManager>().IsFoldable();
    WLOGD("[NAPI], isFoldable = %{public}u", foldable);
    napi_value result;
    napi_get_boolean(env, foldable, &result);
    return result;
}

napi_value JsDisplayManager::OnGetFoldStatus(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_COUNT;
    napi_value argv[ARGC_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc >= ARGC_ONE) {
        WLOGE("Params not match %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }
    FoldStatus status = SingletonContainer::Get<DisplayManager>().GetFoldStatus();
    WLOGD("[NAPI], getFoldStatus = %{public}u", status);
    return CreateJsValue(env, status);
}

napi_value JsDisplayManager::OnGetFoldDisplayMode(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_COUNT;
    napi_value argv[ARGC_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc >= ARGC_ONE) {
        WLOGE("Params not match %{public}zu", argc);
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }
    FoldDisplayMode mode = SingletonContainer::Get<DisplayManager>().GetFoldDisplayMode();
    WLOGD("[NAPI], getFoldDisplayMode = %{public}u", mode);
    return CreateJsValue(env, mode);
}

napi_value JsDisplayManager::OnUnregisterDisplayManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGI("OnUnregisterDisplayCallback is called");
    size_t argc = ARGC_COUNT;
    napi_value argv[ARGC_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc < ARGC_ONE) {
        WLOGE("JsDisplayManager Params not match %{public}zu", argc);
        std::string errMsg = "Invalid args count, need one arg at least!";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return CreateJsUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGE("Failed to convert parameter to callbackType");
        std::string errMsg = "Failed to convert parameter to string";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM), errMsg));
        return CreateJsUndefined(env);
    }
    std::lock_guard<std::mutex> lock(mtx_);
    DmErrorCode ret;
    if (argc == ARGC_ONE) {
        ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
    } else {
        napi_value value = argv[INDEX_ONE];
        if ((value == nullptr) || (!NapiIsCallable(env, value))) {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnregisterAllDisplayListenerWithType(cbType));
        } else {
            ret = DM_JS_TO_ERROR_CODE_MAP.at(UnRegisterDisplayListenerWithType(env, cbType, value));
        }
    }
    if (ret != DmErrorCode::DM_OK) {
        DmErrorCode errCode = DmErrorCode::DM_ERROR_INVALID_PARAM;
        if (ret == DmErrorCode::DM_ERROR_NOT_SYSTEM_APP) {
            errCode = ret;
        }
        WLOGE("failed to unregister display listener with type");
        std::string errMsg = "failed to unregister display listener with type";
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(errCode), errMsg));
        return CreateJsUndefined(env);
    }
    return CreateJsUndefined(env);
}

napi_value JsDisplayManagerInit(napi_env env, napi_value exportObj)
{
    WLOGI("JsDisplayManagerInit is called");
    if (env == nullptr || exportObj == nullptr) {
        WLOGE("JsDisplayManagerInit env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsDisplayManager> jsDisplayManager = std::make_unique<JsDisplayManager>(env);
    napi_wrap(env, exportObj, jsDisplayManager.release(), JsDisplayManager::Finalizer, nullptr, nullptr);

    napi_set_named_property(env, exportObj, "DisplayState", InitDisplayState(env));
    napi_set_named_property(env, exportObj, "Orientation", InitOrientation(env));
    napi_set_named_property(env, exportObj, "DmErrorCode", InitDisplayErrorCode(env));
    napi_set_named_property(env, exportObj, "DMError", InitDisplayError(env));
    napi_set_named_property(env, exportObj, "FoldStatus", InitFoldStatus(env));
    napi_set_named_property(env, exportObj, "FoldDisplayMode", InitFoldDisplayMode(env));

    const char *moduleName = "JsDisplayManager";
    BindNativeFunction(env, exportObj, "getDefaultDisplaySync", moduleName, JsDisplayManager::GetDefaultDisplaySync);
    BindNativeFunction(env, exportObj, "isFoldable", moduleName, JsDisplayManager::IsFoldable);
    BindNativeFunction(env, exportObj, "getFoldStatus", moduleName, JsDisplayManager::GetFoldStatus);
    BindNativeFunction(env, exportObj, "getFoldDisplayMode", moduleName, JsDisplayManager::GetFoldDisplayMode);
    BindNativeFunction(env, exportObj, "on", moduleName, JsDisplayManager::RegisterDisplayManagerCallback);
    BindNativeFunction(env, exportObj, "off", moduleName, JsDisplayManager::UnregisterDisplayManagerCallback);
    return CreateJsUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS