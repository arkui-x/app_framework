/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <regex>
#include "native_engine/native_engine.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr int32_t RGB_LENGTH = 6;
constexpr int32_t RGBA_LENGTH = 8;

napi_value WindowTypeInit(napi_env env)
{
    WLOGD("WindowTypeInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("TYPE_APP",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_APP))),
        DECLARE_NAPI_PROPERTY("TYPE_SYSTEM_ALERT",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_SYSTEM_ALERT))),
        DECLARE_NAPI_PROPERTY("TYPE_INPUT_METHOD",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_INPUT_METHOD))),
        DECLARE_NAPI_PROPERTY("TYPE_STATUS_BAR",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_STATUS_BAR))),
        DECLARE_NAPI_PROPERTY("TYPE_PANEL",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_PANEL))),
        DECLARE_NAPI_PROPERTY("TYPE_KEYGUARD",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_KEYGUARD))),
        DECLARE_NAPI_PROPERTY("TYPE_VOLUME_OVERLAY",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_VOLUME_OVERLAY))),
        DECLARE_NAPI_PROPERTY("TYPE_NAVIGATION_BAR",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_NAVIGATION_BAR))),
        DECLARE_NAPI_PROPERTY("TYPE_FLOAT",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_FLOAT))),
        DECLARE_NAPI_PROPERTY("TYPE_FLOAT_CAMERA",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_FLOAT_CAMERA))),
        DECLARE_NAPI_PROPERTY("TYPE_WALLPAPER",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_WALLPAPER))),
        DECLARE_NAPI_PROPERTY("TYPE_DESKTOP",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_DESKTOP))),
        DECLARE_NAPI_PROPERTY("TYPE_LAUNCHER_RECENT",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_RECENT))),
        DECLARE_NAPI_PROPERTY("TYPE_LAUNCHER_DOCK",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_LAUNCHER_DOCK))),
        DECLARE_NAPI_PROPERTY("TYPE_VOICE_INTERACTION",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_VOICE_INTERACTION))),
        DECLARE_NAPI_PROPERTY("TYPE_DIALOG",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_DIALOG))),
        DECLARE_NAPI_PROPERTY("TYPE_POINTER",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_POINTER))),
        DECLARE_NAPI_PROPERTY("TYPE_SCREENSHOT",
            CreateJsValue(env, static_cast<int32_t>(ApiWindowType::TYPE_SCREENSHOT))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowModeInit(napi_env env)
{
    WLOGD("WindowModeInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("UNDEFINED", CreateJsValue(env,
            static_cast<int32_t>(ApiWindowMode::UNDEFINED))),
        DECLARE_NAPI_PROPERTY("FULLSCREEN", CreateJsValue(env,
            static_cast<int32_t>(ApiWindowMode::FULLSCREEN))),
        DECLARE_NAPI_PROPERTY("PRIMARY", CreateJsValue(env,
            static_cast<int32_t>(ApiWindowMode::PRIMARY))),
        DECLARE_NAPI_PROPERTY("SECONDARY", CreateJsValue(env,
            static_cast<int32_t>(ApiWindowMode::SECONDARY))),
        DECLARE_NAPI_PROPERTY("FLOATING", CreateJsValue(env,
            static_cast<int32_t>(ApiWindowMode::FLOATING))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value OrientationInit(napi_env env)
{
    WLOGD("OrientationInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("UNSPECIFIED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::UNSPECIFIED))),
        DECLARE_NAPI_PROPERTY("PORTRAIT", CreateJsValue(env,
            static_cast<int32_t>(Orientation::VERTICAL))),
        DECLARE_NAPI_PROPERTY("LANDSCAPE", CreateJsValue(env,
            static_cast<int32_t>(Orientation::HORIZONTAL))),
        DECLARE_NAPI_PROPERTY("PORTRAIT_INVERTED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::REVERSE_VERTICAL))),
        DECLARE_NAPI_PROPERTY("LANDSCAPE_INVERTED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::REVERSE_HORIZONTAL))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION", CreateJsValue(env,
            static_cast<int32_t>(Orientation::SENSOR))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION_PORTRAIT", CreateJsValue(env,
            static_cast<int32_t>(Orientation::SENSOR_VERTICAL))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION_LANDSCAPE", CreateJsValue(env,
            static_cast<int32_t>(Orientation::SENSOR_HORIZONTAL))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION_RESTRICTED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::AUTO_ROTATION_RESTRICTED))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION_PORTRAIT_RESTRICTED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::AUTO_ROTATION_PORTRAIT_RESTRICTED))),
        DECLARE_NAPI_PROPERTY("AUTO_ROTATION_LANDSCAPE_RESTRICTED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::AUTO_ROTATION_LANDSCAPE_RESTRICTED))),
        DECLARE_NAPI_PROPERTY("LOCKED", CreateJsValue(env,
            static_cast<int32_t>(Orientation::LOCKED))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowEventTypeInit(napi_env env)
{
    WLOGD("WindowEventTypeInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("WINDOW_SHOWN", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::FOREGROUND))),
        DECLARE_NAPI_PROPERTY("WINDOW_ACTIVE", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::ACTIVE))),
        DECLARE_NAPI_PROPERTY("WINDOW_INACTIVE", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::INACTIVE))),
        DECLARE_NAPI_PROPERTY("WINDOW_HIDDEN", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::BACKGROUND))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowStageEventTypeInit(napi_env env)
{
    WLOGD("WindowStageEventTypeInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("SHOWN", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::FOREGROUND))),
        DECLARE_NAPI_PROPERTY("ACTIVE", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::ACTIVE))),
        DECLARE_NAPI_PROPERTY("INACTIVE", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::INACTIVE))),
        DECLARE_NAPI_PROPERTY("HIDDEN", CreateJsValue(env, static_cast<int32_t>(LifeCycleEventType::BACKGROUND))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowErrorInit(napi_env env)
{
    WLOGD("WindowErrorInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("WM_DO_NOTHING", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_DO_NOTHING))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_NO_MEM", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_NO_MEM))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_DESTROYED_OBJECT", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_DESTROYED_OBJECT))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_WINDOW", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_OPERATION", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_OPERATION))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_PERMISSION", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_PERMISSION))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_NO_REMOTE_ANIMATION", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_NO_REMOTE_ANIMATION))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_DEVICE_NOT_SUPPORT))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_NULLPTR", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_NULLPTR))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_TYPE", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_TYPE))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_PARAM", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_INVALID_PARAM))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_SAMGR", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_SAMGR))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_IPC_FAILED", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_IPC_FAILED))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
            static_cast<int32_t>(WMError::WM_ERROR_START_ABILITY_FAILED))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowErrorCodeInit(napi_env env)
{
    WLOGD("WindowErrorCodeInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("WM_ERROR_NO_PERMISSION", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_NO_PERMISSION))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_NOT_SYSTEM_APP", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_PARAM", CreateJsValue(env,
            WmErrorCode::WM_ERROR_INVALID_PARAM)),
        DECLARE_NAPI_PROPERTY("WM_ERROR_DEVICE_NOT_SUPPORT", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_DEVICE_NOT_SUPPORT))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_REPEAT_OPERATION", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_REPEAT_OPERATION))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_STATE_ABNORMALLY", CreateJsValue(env,
            WmErrorCode::WM_ERROR_STATE_ABNORMALLY)),
        DECLARE_NAPI_PROPERTY("WM_ERROR_SYSTEM_ABNORMALLY", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_INVALID_CALLING", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_CALLING))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_STAGE_ABNORMALLY", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_STAGE_ABNORMALLY))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_CONTEXT_ABNORMALLY", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_CONTEXT_ABNORMALLY))),
        DECLARE_NAPI_PROPERTY("WM_ERROR_START_ABILITY_FAILED", CreateJsValue(env,
            static_cast<int32_t>(WmErrorCode::WM_ERROR_START_ABILITY_FAILED))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value WindowColorSpaceInit(napi_env env)
{
    WLOGD("WindowColorSpaceInit");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("DEFAULT", CreateJsValue(env,
            static_cast<int32_t>(ColorSpace::COLOR_SPACE_DEFAULT))),
        DECLARE_NAPI_PROPERTY("WIDE_GAMUT", CreateJsValue(env,
            static_cast<int32_t>(ColorSpace::COLOR_SPACE_WIDE_GAMUT))),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value GetRectAndConvertToJsValue(napi_env env, const Rect& rect)
{
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("left", CreateJsValue(env, rect.posX_)),
        DECLARE_NAPI_PROPERTY("top", CreateJsValue(env, rect.posY_)),
        DECLARE_NAPI_PROPERTY("width", CreateJsValue(env, rect.width_)),
        DECLARE_NAPI_PROPERTY("height", CreateJsValue(env, rect.height_)),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
}

napi_value CreateJsWindowPropertiesObject(napi_env env, std::shared_ptr<Window>& window)
{
    WLOGD("CreateJsWindowPropertiesObject");
    Rect rect = window->GetRect();
    napi_value rectObj = GetRectAndConvertToJsValue(env, rect);
    if (rectObj == nullptr) {
        WLOGE("GetRect failed!");
        return nullptr;
    }

    napi_value typeObj;
    WindowType type = window->GetType();
    if (NATIVE_JS_TO_WINDOW_TYPE_MAP.count(type) != 0) {
        typeObj = CreateJsValue(env, NATIVE_JS_TO_WINDOW_TYPE_MAP.at(type));
    } else {
        typeObj = CreateJsValue(env, type);
    }
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("windowRect", rectObj),
        DECLARE_NAPI_PROPERTY("type", typeObj),
        DECLARE_NAPI_PROPERTY("name", CreateJsValue(env, window->GetWindowName())),
        DECLARE_NAPI_PROPERTY("isKeepScreenOn", CreateJsValue(env, window->IsKeepScreenOn())),
        DECLARE_NAPI_PROPERTY("brightness", CreateJsValue(env, window->GetBrightness())),
        DECLARE_NAPI_PROPERTY("id", CreateJsValue(env, window->GetWindowId())),
    };
    return CreateObject(env, nullptr, props, sizeof(props) / sizeof(props[0]));
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
                        napi_env env, size_t argc, const napi_value arg, std::shared_ptr<Window>& window)
{
    bool isArray;
    napi_status status = napi_is_array(env, arg, &isArray);
    if (status != napi_ok || !isArray) {
        WLOGE("Failed to convert parameter to SystemBarArray");
        return false;
    }
    uint32_t size = 0;
    status = napi_get_array_length(env, arg, &size);
    if (status != napi_ok) {
        WLOGE("Failed to convert parameter to SystemBarArray");
        return false;
    }

    auto statusProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
    auto navProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
    statusProperty.enable_ = false;
    navProperty.enable_ = false;
    systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
    systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
    for (uint32_t i = 0; i < size; i++) {
        std::string name;
        napi_value ele = nullptr;
        status = napi_get_element(env, arg, i, &ele);
        if (status != napi_ok) {
            WLOGE("Failed to get element %{public}u", i);
            continue;
        }
        if (!ConvertFromJsValue(env, ele, name)) {
            WLOGE("Failed to convert parameter to SystemBarName");
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

bool GetSpecificBarStatus(std::map<WindowType, SystemBarProperty>& systemBarProperties,
                          napi_env env, napi_callback_info info, std::shared_ptr<Window>& window)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string name;
    if (!ConvertFromJsValue(env, argv[0], name)) {
        WLOGE("Failed to convert parameter to SystemBarName");
        return false;
    }
    bool enable = false;
    if (!ConvertFromJsValue(env, argv[1], enable)) {
        WLOGE("Failed to convert parameter to bool");
        return CreateUndefined(env);
    }
    if (name.compare("status") == 0) {
        auto statusProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_STATUS_BAR);
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR] = statusProperty;
        systemBarProperties[WindowType::WINDOW_TYPE_STATUS_BAR].enable_ = enable;
    } else if (name.compare("navigation") == 0) {
        auto navProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_BAR);
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR] = navProperty;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enable_ = enable;
    } else if (name.compare("navigationIndicator") == 0) {
        auto navIndicatorProperty = window->GetSystemBarPropertyByType(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR);
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR] = navIndicatorProperty;
        systemBarProperties[WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR].enable_ = enable;
    } else {
        WLOGE("Unknown SystemBarName parameter");
        return false;
    }
    return true;
}

napi_value ConvertAvoidAreaToJsValue(napi_env env, const AvoidArea& avoidArea, AvoidAreaType type)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        WLOGE("Failed to convert avoidArea to jsObject");
        return nullptr;
    }
    napi_set_named_property(env, objValue, "visible",
        CreateJsValue(env, type == AvoidAreaType::TYPE_CUTOUT ? false : true));
    napi_set_named_property(env, objValue, "leftRect", GetRectAndConvertToJsValue(env, avoidArea.leftRect_));
    napi_set_named_property(env, objValue, "topRect", GetRectAndConvertToJsValue(env, avoidArea.topRect_));
    napi_set_named_property(env, objValue, "rightRect", GetRectAndConvertToJsValue(env, avoidArea.rightRect_));
    napi_set_named_property(env, objValue, "bottomRect", GetRectAndConvertToJsValue(env, avoidArea.bottomRect_));
    return objValue;
}

void LoadContentTask(napi_env env, napi_ref storageRef, const std::string &contextUrl,
    std::shared_ptr<Window> weakWindow, NapiAsyncTask& task, bool isLoadedByName)
{
    napi_value nativeStorage = nullptr;
    if (storageRef != nullptr) {
        napi_status status = napi_get_reference_value(env, storageRef, &nativeStorage);
        // delete ref
        napi_delete_reference(env, storageRef);
        if (status != napi_ok) {
            WLOGE("LoadContentTask : get value fail %{public}d", static_cast<int>(status));
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
    }
    WLOGI("LoadContentTask : contextUrl %{public}s", contextUrl.c_str());
    WMError ret = weakWindow->SetUIContent(contextUrl,
        reinterpret_cast<NativeEngine*>(env), nativeStorage, false, nullptr, isLoadedByName);
    if (ret == WMError::WM_OK) {
        task.Resolve(env, CreateUndefined(env));
    } else {
        task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window load content failed"));
    }
}

bool GetContentArg(napi_env env,
    napi_callback_info info, std::string &contextUrl, napi_value &storage, napi_value &callback)
{
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1 || argc > 3) { // 3 max arg
        WLOGE("OnLoadContent : invalid param");
        return false;
    }
    storage = nullptr;
    callback = nullptr;
    if (!ConvertFromJsValue(env, argv[0], contextUrl)) {
        WLOGE("OnLoadContent : ConvertFromJsValue to contextUrl fail");
        return false;
    }

    napi_valuetype type;
    if (argc > 2) { // 2 func
        napi_typeof(env, argv[2], &type); // 2 func
        if (type == napi_function) {
            callback = argv[2];
        }
    }
    if (argc > 1) {
        napi_typeof(env, argv[1], &type);
        if (type == napi_function) {
            callback = argv[1];
        } else if (type == napi_object) {
            storage = argv[1];
        }
    }
    return true;
}

bool SetWindowObjectProperties(napi_env env,
    napi_value object, const char *moduleName, const napi_property_descriptor *props, size_t size)
{
    napi_status status = napi_ok;
    for (size_t i = 0; i < size; i++) {
        if (props[i].value) {
            napi_set_named_property(env, object, props[i].utf8name, props[i].value);
        } else if (props[i].getter) {
            BindNativeFunction(env, object, props[i].utf8name, moduleName, props[i].getter);
        } else if (props[i].setter) {
            BindNativeFunction(env, object, props[i].utf8name, moduleName, props[i].setter);
        } else if (props[i].method) {
            BindNativeFunction(env, object, props[i].utf8name, moduleName, props[i].method);
        }
        if (status != napi_ok) {
            return false;
        }
    }
    return true;
}

napi_value CreateObject(napi_env env, const char *moduleName, const napi_property_descriptor *props, size_t size)
{
    napi_value obj = nullptr;
    napi_status status = napi_create_object(env, &obj);
    if (status != napi_ok) {
        return nullptr;
    }
    if (!SetWindowObjectProperties(env, obj, moduleName, props, size)) {
        return nullptr;
    }
    return obj;
}

napi_value AvoidAreaTypeInit(napi_env env)
{
    WLOGD("AvoidAreaTypeInit");

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

    napi_set_named_property(env, objValue, "TYPE_SYSTEM", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM)));
    napi_set_named_property(env, objValue, "TYPE_CUTOUT", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_CUTOUT)));
    napi_set_named_property(env, objValue, "TYPE_SYSTEM_GESTURE", CreateJsValue(env,
        static_cast<int32_t>(AvoidAreaType::TYPE_SYSTEM_GESTURE)));
    napi_set_named_property(env, objValue, "TYPE_KEYBOARD",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_KEYBOARD)));
    napi_set_named_property(env, objValue, "TYPE_NAVIGATION_INDICATOR",
        CreateJsValue(env, static_cast<int32_t>(AvoidAreaType::TYPE_NAVIGATION_INDICATOR)));
    return objValue;
}

template<class T>
bool ParseJsValue(napi_value jsObject, napi_env env, const std::string& name, T& data)
{
    napi_value value = nullptr;
    napi_get_named_property(env, jsObject, name.c_str(), &value);
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type != napi_undefined) {
        if (!AbilityRuntime::ConvertFromJsValue(env, value, data)) {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

napi_valuetype IsUndefined(napi_env env, napi_value value)
{
    napi_valuetype res = napi_undefined;
    napi_typeof(env, value, &res);
    return res;
}

static uint32_t GetColorFromJs(napi_env env, napi_value jsObject,
    const char* name, uint32_t defaultColor, bool& flag)
{
    napi_value jsColor = nullptr;
    napi_get_named_property(env, jsObject, name, &jsColor);
    if (IsUndefined(env, jsColor) != napi_undefined) {
        std::string colorStr;
        if (!ConvertFromJsValue(env, jsColor, colorStr)) {
            WLOGE("Failed to convert parameter to color");
            return defaultColor;
        }
        std::regex pattern("^#([A-Fa-f0-9]{6}|[A-Fa-f0-9]{8})$");
        if (!std::regex_match(colorStr, pattern)) {
            WLOGD("Invalid color input");
            return defaultColor;
        }
        std::string color = colorStr.substr(1);
        if (color.length() == RGB_LENGTH) {
            color = "FF" + color; // ARGB
        }
        flag = true;
        std::stringstream ss;
        uint32_t hexColor;
        ss << std::hex << color;
        ss >> hexColor;
        return hexColor;
    }
    return defaultColor;
}

bool SetWindowStatusBarContentColor(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto statusProperty = properties[WindowType::WINDOW_TYPE_STATUS_BAR];
    napi_value jsStatusContentColor = nullptr;
    napi_get_named_property(env, jsObject, "statusBarContentColor", &jsStatusContentColor);
    napi_value jsStatusIcon = nullptr;
    napi_get_named_property(env, jsObject, "isStatusBarLightIcon", &jsStatusIcon);
    if (IsUndefined(env, jsStatusContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ =  GetColorFromJs(env,
            jsObject, "statusBarContentColor", statusProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag);
        WLOGE("ww statusBarContentColor not run");
    } else if (IsUndefined(env, jsStatusIcon) != napi_undefined) {
        bool isStatusBarLightIcon;
        if (!ConvertFromJsValue(env, jsStatusIcon, isStatusBarLightIcon)) {
            WLOGE("Convert status icon value failed");
            return false;
        }
        if (isStatusBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].contentColorFlag = true;
    }
    return true;
}

bool SetWindowNavigationBarContentColor(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    auto navProperty = properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR];
    napi_value jsNavigationContentColor = nullptr;
    napi_get_named_property(env, jsObject, "navigationBarContentColor", &jsNavigationContentColor);
    napi_value jsNavigationIcon = nullptr;
    napi_get_named_property(env, jsObject, "isNavigationBarLightIcon", &jsNavigationIcon);
    if (IsUndefined(env, jsNavigationContentColor) != napi_undefined) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = GetColorFromJs(env,
            jsObject, "navigationBarContentColor", navProperty.contentColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag);
    } else if (IsUndefined(env, jsNavigationIcon) != napi_undefined) {
        bool isNavigationBarLightIcon;
        if (!ConvertFromJsValue(env, jsNavigationIcon, isNavigationBarLightIcon)) {
            WLOGE("Convert navigation icon value failed");
            return false;
        }
        if (isNavigationBarLightIcon) {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_WHITE;
        } else {
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColor_ = SYSTEM_COLOR_BLACK;
        }
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].contentColorFlag = true;
    }
    return true;
}

bool GetSystemBarPropertiesFromJs(napi_env env, napi_value jsObject,
    std::unordered_map<WindowType, SystemBarProperty>& properties,
    std::unordered_map<WindowType, SystemBarPropertyFlag>& propertyFlags)
{
    properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "statusBarColor",
            properties[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].backgroundColorFlag);
    properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_ =
        GetColorFromJs(env, jsObject, "navigationBarColor",
            properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColor_,
            propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].backgroundColorFlag);

    if (!SetWindowStatusBarContentColor(env, jsObject, properties, propertyFlags) ||
        !SetWindowNavigationBarContentColor(env, jsObject, properties, propertyFlags)) {
        return false;
    }
    bool enableStatusBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableStatusBarAnimation", enableStatusBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimation_ = enableStatusBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_STATUS_BAR].enableAnimationFlag = true;
    }
    bool enableNavigationBarAnimation = false;
    if (ParseJsValue(jsObject, env, "enableNavigationBarAnimation", enableNavigationBarAnimation)) {
        properties[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimation_ = enableNavigationBarAnimation;
        propertyFlags[WindowType::WINDOW_TYPE_NAVIGATION_BAR].enableAnimationFlag = true;
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS
