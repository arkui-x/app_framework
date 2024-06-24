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

#include "js_display_manager.h"

#include <new>
#include <vector>

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

    napi_set_named_property(env, obj, "PORTRAIT",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::PORTRAIT)));
    napi_set_named_property(env, obj, "LANDSCAPE",
        CreateJsValue(env, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE)));
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
    BindNativeFunction(env, exportObj, "getDefaultDisplaySync",
        "JsDisplayManager", JsDisplayManager::GetDefaultDisplaySync);
    return CreateJsUndefined(env);
}
}  // namespace Rosen
}  // namespace OHOS