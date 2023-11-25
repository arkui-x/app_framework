/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_window_utils.h"

#include <iomanip>
#include "hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr int32_t RGB_LENGTH = 6;
constexpr int32_t RGBA_LENGTH = 8;

NativeValue* WindowTypeInit(NativeEngine* engine)
{
    HILOG_INFO("WindowTypeInit");

    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("TYPE_APP",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_APP)));
    object->SetProperty("TYPE_SYSTEM_ALERT",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_ALERT)));
    object->SetProperty("TYPE_INPUT_METHOD",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_INPUT_METHOD)));
    object->SetProperty("TYPE_STATUS_BAR",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_STATUS_BAR)));
    object->SetProperty("TYPE_PANEL",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_PANEL)));
    object->SetProperty("TYPE_KEYGUARD",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_KEYGUARD)));
    object->SetProperty("TYPE_VOLUME_OVERLAY",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_VOLUME_OVERLAY)));
    object->SetProperty("TYPE_NAVIGATION_BAR",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_NAVIGATION_BAR)));
    object->SetProperty("TYPE_FLOAT",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_FLOAT)));
    object->SetProperty("TYPE_FLOAT_CAMERA",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_FLOAT_CAMERA)));
    object->SetProperty("TYPE_WALLPAPER",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_WALLPAPER)));
    object->SetProperty("TYPE_DESKTOP",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_DESKTOP)));
    object->SetProperty("TYPE_LAUNCHER_RECENT",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_RECENT)));
    object->SetProperty("TYPE_LAUNCHER_DOCK",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_DOCK)));
    object->SetProperty("TYPE_VOICE_INTERACTION",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_VOICE_INTERACTION)));
    object->SetProperty("TYPE_DIALOG",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_DIALOG)));
    object->SetProperty("TYPE_POINTER",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_POINTER)));
    object->SetProperty("TYPE_SCREENSHOT",
        CreateJsValue(*engine, static_cast<int32_t>(ApiWindowType::TYPE_SCREENSHOT)));

    return objValue;
}

NativeValue* WindowModeInit(NativeEngine* engine)
{
    HILOG_INFO("WindowModeInit");

    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("UNDEFINED", CreateJsValue(*engine,
        static_cast<int32_t>(ApiWindowMode::UNDEFINED)));
    object->SetProperty("FULLSCREEN", CreateJsValue(*engine,
        static_cast<int32_t>(ApiWindowMode::FULLSCREEN)));
    object->SetProperty("PRIMARY", CreateJsValue(*engine,
        static_cast<int32_t>(ApiWindowMode::PRIMARY)));
    object->SetProperty("SECONDARY", CreateJsValue(*engine,
        static_cast<int32_t>(ApiWindowMode::SECONDARY)));
    object->SetProperty("FLOATING", CreateJsValue(*engine,
        static_cast<int32_t>(ApiWindowMode::FLOATING)));
    return objValue;
}

NativeValue* OrientationInit(NativeEngine* engine)
{
    HILOG_INFO("OrientationInit");

    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("UNSPECIFIED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::UNSPECIFIED)));
    object->SetProperty("PORTRAIT", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::VERTICAL)));
    object->SetProperty("LANDSCAPE", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::HORIZONTAL)));
    object->SetProperty("PORTRAIT_INVERTED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::REVERSE_VERTICAL)));
    object->SetProperty("LANDSCAPE_INVERTED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::REVERSE_HORIZONTAL)));
    object->SetProperty("AUTO_ROTATION", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::SENSOR)));
    object->SetProperty("AUTO_ROTATION_PORTRAIT", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::SENSOR_VERTICAL)));
    object->SetProperty("AUTO_ROTATION_LANDSCAPE", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::SENSOR_HORIZONTAL)));
    object->SetProperty("AUTO_ROTATION_RESTRICTED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::AUTO_ROTATION_RESTRICTED)));
    object->SetProperty("AUTO_ROTATION_PORTRAIT_RESTRICTED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED)));
    object->SetProperty("AUTO_ROTATION_LANDSCAPE_RESTRICTED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED)));
    object->SetProperty("LOCKED", CreateJsValue(*engine,
        static_cast<int32_t>(Orientation::LOCKED)));
    return objValue;
}

NativeValue* WindowEventTypeInit(NativeEngine* engine)
{
    HILOG_INFO("WindowEventTypeInit");

    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    object->SetProperty("WINDOW_SHOWN", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    object->SetProperty("WINDOW_ACTIVE", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    object->SetProperty("WINDOW_INACTIVE", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    object->SetProperty("WINDOW_HIDDEN", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    return objValue;
}

NativeValue* WindowStageEventTypeInit(NativeEngine* engine)
{
    HILOG_INFO("WindowStageEventTypeInit");

    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("SHOWN", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::FOREGROUND)));
    object->SetProperty("ACTIVE", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::ACTIVE)));
    object->SetProperty("INACTIVE", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::INACTIVE)));
    object->SetProperty("HIDDEN", CreateJsValue(*engine, static_cast<int32_t>(LifeCycleEventType::BACKGROUND)));
    return objValue;
}

NativeValue* WindowErrorInit(NativeEngine* engine)
{
    HILOG_INFO("WindowErrorInit");
    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("WM_DO_NOTHING", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_DO_NOTHING)));
    object->SetProperty("WM_ERROR_NO_MEM", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_NO_MEM)));
    object->SetProperty("WM_ERROR_DESTROYED_OBJECT", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_DESTROYED_OBJECT)));
    object->SetProperty("WM_ERROR_INVALID_WINDOW", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW)));
    object->SetProperty("WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE)));
    object->SetProperty("WM_ERROR_INVALID_OPERATION", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_OPERATION)));
    object->SetProperty("WM_ERROR_INVALID_PERMISSION", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION)));
    object->SetProperty("WM_ERROR_NO_REMOTE_ANIMATION", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_NO_REMOTE_ANIMATION)));
    object->SetProperty("WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_DEVICE_NOT_SUPPORT)));
    object->SetProperty("WM_ERROR_NULLPTR", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_NULLPTR)));
    object->SetProperty("WM_ERROR_INVALID_TYPE", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_TYPE)));
    object->SetProperty("WM_ERROR_INVALID_PARAM", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_INVALID_PARAM)));
    object->SetProperty("WM_ERROR_SAMGR", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_SAMGR)));
    object->SetProperty("WM_ERROR_IPC_FAILED", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED)));
    object->SetProperty("WM_ERROR_START_ABILITY_FAILED", CreateJsValue(*engine,
        static_cast<int32_t>(WMError::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

NativeValue* WindowErrorCodeInit(NativeEngine* engine)
{
    HILOG_INFO("WindowErrorCodeInit");
    if (engine == nullptr) {
        HILOG_ERROR("Engine is nullptr");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    object->SetProperty("WM_ERROR_NO_PERMISSION", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION)));
    object->SetProperty("WM_ERROR_NOT_SYSTEM_APP", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
    object->SetProperty("WM_ERROR_INVALID_PARAM", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
    object->SetProperty("WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT)));
    object->SetProperty("WM_ERROR_REPEAT_OPERATION", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_REPEAT_OPERATION)));
    object->SetProperty("WM_ERROR_STATE_ABNORMALLY", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
    object->SetProperty("WM_ERROR_SYSTEM_ABNORMALLY", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY)));
    object->SetProperty("WM_ERROR_INVALID_CALLING", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING)));
    object->SetProperty("WM_ERROR_STAGE_ABNORMALLY", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY)));
    object->SetProperty("WM_ERROR_CONTEXT_ABNORMALLY", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY)));
    object->SetProperty("WM_ERROR_START_ABILITY_FAILED", CreateJsValue(*engine,
        static_cast<int32_t>(WmErrorCode::WM_ERROR_START_ABILITY_FAILED)));
    return objValue;
}

NativeValue* GetRectAndConvertToJsValue(NativeEngine& engine, const Rect& rect)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to convert rect to jsObject");
        return nullptr;
    }
    object->SetProperty("left", CreateJsValue(engine, rect.posX_));
    object->SetProperty("top", CreateJsValue(engine, rect.posY_));
    object->SetProperty("width", CreateJsValue(engine, rect.width_));
    object->SetProperty("height", CreateJsValue(engine, rect.height_));
    return objValue;
}

NativeValue* CreateJsWindowPropertiesObject(NativeEngine& engine, std::shared_ptr<Window>& window)
{
    HILOG_INFO("CreateJsWindowPropertiesObject");
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to convert windowProperties to jsObject");
        return nullptr;
    }

    Rect rect = window->GetRect();
    NativeValue* rectObj = GetRectAndConvertToJsValue(engine, rect);
    if (rectObj == nullptr) {
        HILOG_ERROR("GetRect failed!");
    }
    object->SetProperty("windowRect", rectObj);
    WindowType type = window->GetType();
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        object->SetProperty("type", CreateJsValue(engine, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(type)));
    } else {
        object->SetProperty("type", CreateJsValue(engine, type));
    }
    object->SetProperty("name", CreateJsValue(engine, window->GetWindowName()));
    object->SetProperty("isKeepScreenOn", CreateJsValue(engine, window->IsKeepScreenOn()));
    object->SetProperty("brightness", CreateJsValue(engine, window->GetBrightness()));
    object->SetProperty("id", CreateJsValue(engine, window->GetWindowId()));
    return objValue;
}
static std::string GetHexColor(uint32_t color)
{
    std::stringstream ioss;
    std::string temp;
    ioss << std::setiosflags(std::ios::uppercase) << std::hex << color;
    ioss >> temp;
    int count = RGBA_LENGTH - static_cast<int>(temp.length());
    std::string tmpColor(count, '0');
    tmpColor += temp;
    std::string finalColor("#");
    finalColor += tmpColor;
    return finalColor;
}

bool GetSystemBarStatus(std::map<WindowType, SystemBarProperty>& systemBarProperties,
                        NativeEngine& engine, NativeCallbackInfo& info, std::shared_ptr<Window>& window)
{
    NativeArray* nativeArray = nullptr;
    uint32_t size = 0;
    if (info.argc > 0 && info.argv[0]->TypeOf() != NATIVE_FUNCTION) {
        nativeArray = ConvertNativeValueTo<NativeArray>(info.argv[0]);
        if (nativeArray == nullptr) {
            HILOG_ERROR("Failed to convert parameter to SystemBarArray");
            return false;
        }
        size = nativeArray->GetLength();
    }
    auto statusProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    statusProperty.enable_ = false;
    navProperty.enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    for (uint32_t i = 0; i < size; i++) {
        std::string name;
        if (!ConvertFromJsValue(engine, nativeArray->GetElement(i), name)) {
            HILOG_ERROR("Failed to convert parameter to SystemBarName");
            return false;
        }
        if (name.compare("status") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = true;
        } else if (name.compare("navigation") == 0) {
            systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = true;
        }
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS
