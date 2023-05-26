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

#include <vector>
#include <new>

// #include <hitrace_meter.h>
#include "js_runtime_utils.h"
// #include "native_engine/native_reference.h"
#include "display.h"
#include "display_manager.h"
// #include "window_manager_hilog.h"
#include "singleton_container.h"
// #include "js_display_listener.h"
// #include "abstract_display_controller.h"
#include "js_display.h"
#include "js_display_manager.h"
#include "hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr int32_t INDEX_ONE = 1;

void JsDisplayManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("Finalizer is called");
    std::unique_ptr<JsDisplayManager>(static_cast<JsDisplayManager*>(data));
}

NativeValue* JsDisplayManager::GetDefaultDisplaySync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsDisplayManager::GetDefaultDisplaySync");
    JsDisplayManager* me = CheckParamsAndGetThis<JsDisplayManager>(engine, info);
    return (me != nullptr) ? me->OnGetDefaultDisplaySync(*engine, *info) : nullptr;
}

NativeValue* JsDisplayManager::OnGetDefaultDisplaySync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsDisplayManager::OnGetDefaultDisplaySync : Start...");
    sptr<Display> display = DisplayManager::GetInstance().GetDefaultDisplaySync();//displayManager_->GetDefaultDisplaySync();
    if (display == nullptr) {
        HILOG_ERROR("OnGetDefaultDisplaySync, display is nullptr.");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN)));
        return engine.CreateUndefined();
    }
    return CreateJsDisplayObject(engine, display);
}

NativeValue* InitDisplayState(NativeEngine* engine)
{
    HILOG_INFO("InitDisplayState called");

    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("STATE_UNKNOWN", CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_UNKNOWN)));
    object->SetProperty("STATE_OFF", CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_OFF)));
    object->SetProperty("STATE_ON", CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_ON)));
    object->SetProperty("STATE_DOZE",
        CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_DOZE)));
    object->SetProperty("STATE_DOZE_SUSPEND",
        CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_DOZE_SUSPEND)));
    object->SetProperty("STATE_VR",
        CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_VR)));
    object->SetProperty("STATE_ON_SUSPEND",
        CreateJsValue(*engine, static_cast<int32_t>(DisplayStateMode::STATE_ON_SUSPEND)));
    return objValue;
}

NativeValue* InitOrientation(NativeEngine* engine)
{
    HILOG_INFO("InitOrientation called");

    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("PORTRAIT", CreateJsValue(*engine, static_cast<uint32_t>(DisplayOrientation::PORTRAIT)));
    object->SetProperty("LANDSCAPE", CreateJsValue(*engine, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE)));
    object->SetProperty("PORTRAIT_INVERTED",
        CreateJsValue(*engine, static_cast<uint32_t>(DisplayOrientation::PORTRAIT_INVERTED)));
    object->SetProperty("LANDSCAPE_INVERTED",
        CreateJsValue(*engine, static_cast<uint32_t>(DisplayOrientation::LANDSCAPE_INVERTED)));
    return objValue;
}

NativeValue* InitDisplayErrorCode(NativeEngine* engine)
{
    HILOG_INFO("InitDisplayErrorCode called");

    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("DM_ERROR_NO_PERMISSION",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_NO_PERMISSION)));
    object->SetProperty("DM_ERROR_INVALID_PARAM",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
    object->SetProperty("DM_ERROR_DEVICE_NOT_SUPPORT",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_DEVICE_NOT_SUPPORT)));
    object->SetProperty("DM_ERROR_INVALID_SCREEN",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_SCREEN)));
    object->SetProperty("DM_ERROR_INVALID_CALLING",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_CALLING)));
    object->SetProperty("DM_ERROR_SYSTEM_INNORMAL",
        CreateJsValue(*engine, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));

    return objValue;
}

NativeValue* InitDisplayError(NativeEngine* engine)
{
    HILOG_INFO("InitDisplayError called");

    if (engine == nullptr) {
        HILOG_ERROR("engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("DM_ERROR_INIT_DMS_PROXY_LOCKED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_INIT_DMS_PROXY_LOCKED)));
    object->SetProperty("DM_ERROR_IPC_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_IPC_FAILED)));
    object->SetProperty("DM_ERROR_REMOTE_CREATE_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_REMOTE_CREATE_FAILED)));
    object->SetProperty("DM_ERROR_NULLPTR",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_NULLPTR)));
    object->SetProperty("DM_ERROR_INVALID_PARAM",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_INVALID_PARAM)));
    object->SetProperty("DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_WRITE_INTERFACE_TOKEN_FAILED)));
    object->SetProperty("DM_ERROR_DEATH_RECIPIENT",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_DEATH_RECIPIENT)));
    object->SetProperty("DM_ERROR_INVALID_MODE_ID",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_INVALID_MODE_ID)));
    object->SetProperty("DM_ERROR_WRITE_DATA_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_WRITE_DATA_FAILED)));
    object->SetProperty("DM_ERROR_RENDER_SERVICE_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_RENDER_SERVICE_FAILED)));
    object->SetProperty("DM_ERROR_UNREGISTER_AGENT_FAILED",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_UNREGISTER_AGENT_FAILED)));
    object->SetProperty("DM_ERROR_INVALID_CALLING",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_INVALID_CALLING)));
    object->SetProperty("DM_ERROR_UNKNOWN",
        CreateJsValue(*engine, static_cast<int32_t>(DMError::DM_ERROR_UNKNOWN)));

    return objValue;
}

NativeValue* JsDisplayManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsDisplayManagerInit is called");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("JsDisplayManagerInit engine or exportObj is nullptr");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("JsDisplayManagerInit object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsDisplayManager> jsDisplayManager = std::make_unique<JsDisplayManager>(engine);
    object->SetNativePointer(jsDisplayManager.release(), JsDisplayManager::Finalizer, nullptr);

    object->SetProperty("DisplayState", InitDisplayState(engine));
    object->SetProperty("Orientation", InitOrientation(engine));
    object->SetProperty("DmErrorCode", InitDisplayErrorCode(engine));
    object->SetProperty("DMError", InitDisplayError(engine));

    const char *moduleName = "JsDisplayManager";
    BindNativeFunction(*engine, *object, "getDefaultDisplaySync", moduleName, JsDisplayManager::GetDefaultDisplaySync);
    return engine->CreateUndefined();
}
}  // namespace Rosen
}  // namespace OHOS