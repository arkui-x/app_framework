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

#include "js_window_manager.h"

#include <ability.h>

#include "js_window.h"
#include "js_window_utils.h"
#include "window_option.h"
#include "window_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

JsWindowManager::JsWindowManager() {}

JsWindowManager::~JsWindowManager() {}

static void CreateSubWindowTask(
    void* contextPtr, std::shared_ptr<WindowOption> windowOption, napi_env env, NapiAsyncTask& task)
{
    if (windowOption == nullptr) {
        task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY, "New window option failed"));
        WLOGE("CreateSubWindowTask : New window option failed");
        return;
    }
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (windowOption->GetParentId() == INVALID_WINDOW_ID) {
        WLOGE("CreateSubWindowTask : subwindow option parentid error");
        return;
    }
    auto context = static_cast<std::weak_ptr<OHOS::AbilityRuntime::Platform::AbilityContext>*>(contextPtr);
    std::shared_ptr<Window> window = Window::CreateSubWindow(context->lock(), windowOption);
    if (window != nullptr) {
        task.Resolve(env, CreateJsWindowObject(env, window));
    } else {
        WLOGE("CreateSubWindowTask : Create window failed");
        task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Create window failed"));
    }
}

void JsWindowManager::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGD("JsWindowManager::Finalizer");
    std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
}

napi_value JsWindowManager::CreateWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::CreateWindow");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnCreateWindow(env, info) : nullptr;
}

napi_value JsWindowManager::FindWindowSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::FindWindowSync");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnFindWindowSync(env, info) : nullptr;
}

napi_value JsWindowManager::GetLastWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::GetLastWindow");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(env, info);
    return (me != nullptr) ? me->OnGetLastWindow(env, info) : nullptr;
}

napi_value JsWindowManager::OnCreateWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::OnCreateWindow : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindowStage::OnCreateWindow : argc error![%{public}zu]", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }

    WindowOption option;
    void* contextPtr = nullptr;
    if (!ParseConfigOption(env, argv[0], option, contextPtr)) {
        WLOGE("JsWindowManager::OnCreateWindow : Failed to parse config");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }
    if (option.GetWindowType() != WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
        WLOGE("JsWindowManager::OnCreateWindow : Invalid Window Type!");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_NOT_SYSTEM_APP));
        return nullptr;
    }

    NapiAsyncTask::CompleteCallback complete = [=](napi_env env, NapiAsyncTask& task, int32_t status) {
        std::shared_ptr<WindowOption> windowOption = std::make_shared<WindowOption>(option);
        return CreateSubWindowTask(contextPtr, windowOption, env, task);
    };
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowManager::OnCreateWindow", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

template <class T>
static bool ParseJsValue(napi_env env, napi_value jsObject, const std::string &name, T &data)
{
    napi_value value = nullptr;
    napi_status status = napi_get_named_property(env, jsObject, name.c_str(), &value);
    if (status != napi_ok) {
        return false;
    }
    if (!ConvertFromJsValue(env, value, data)) {
        return false;
    }
    return true;
}

bool JsWindowManager::ParseConfigOption(napi_env env, napi_value jsObject, WindowOption& option, void*& contextPtr)
{
    std::string windowName;
    if (ParseJsValue(env, jsObject, "name", windowName)) {
        option.SetWindowName(windowName);
    } else {
        WLOGE("JsWindowManager::ParseConfigOption : Failed to convert parameter to windowName");
        return false;
    }

    uint32_t winType;
    if (ParseJsValue(env, jsObject, "windowType", winType)) {
        if (winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
            winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
            option.SetWindowType(JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(winType)));
        } else {
            option.SetWindowType(static_cast<WindowType>(winType));
        }
    } else {
        WLOGE("JsWindowManager::ParseConfigOption : Failed to convert parameter to winType");
        return false;
    }

    napi_value value = nullptr;
    napi_status status = napi_get_named_property(env, jsObject, "ctx", &value);
    if (status != napi_ok) {
        return false;
    }
    status = napi_unwrap(env, value, &contextPtr);
    if (status != napi_ok) {
        return false;
    }

    int64_t parentId = -1;
    if (ParseJsValue(env, jsObject, "parentId", parentId)) {
        option.SetParentId(parentId);
    } else {
        return false;
    }
    return true;
}

napi_value JsWindowManager::OnFindWindowSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::OnFindWindowSync : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindowStage::OnFindWindowSync : argc error![%{public}zu]", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }

    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGE("JsWindowManager::OnFindWindowSync : Failed to convert parameter to windowName");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }
    napi_value object = nullptr;
    napi_ref ref = FindJsWindowObject(windowName);
    if (ref != nullptr) {
        napi_status status = napi_get_reference_value(env, ref, &object);
        if (status != napi_ok) {
            return nullptr;
        }
        WLOGD("JsWindowManager::OnFindWindowSync : Find window: %{public}s, use exist js window", windowName.c_str());
        return object;
    }

    WLOGD("JsWindowManager::OnFindWindowSync : could not find window from js window, call Window::FindWindow");
    std::shared_ptr<Window> window = Window::FindWindow(windowName);
    if (window == nullptr) {
        WLOGE("JsWindowManager::OnFindWindowSync : call Window::FindWindow return failed!");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return nullptr;
    } else {
        WLOGE("JsWindowManager::OnFindWindowSync : call Window::FindWindow return success!");
        return CreateJsWindowObject(env, window);
    }
}

static void GetTopWindowTask(void* contextPtr, napi_env env, NapiAsyncTask& task, bool newApi)
{
    WLOGD("WindowManager:GetTopWindowTask : Start...");
    std::string windowName;
    std::shared_ptr<Rosen::Window> window = nullptr;
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Platform::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        WLOGE("WindowManager:GetTopWindowTask: Stage mode without context");
    }

    window = Window::GetTopWindow(context == nullptr ? nullptr : context->lock());
    WLOGD("WindowManager:GetTopWindowTask: GetTopWindow[%{public}p]", window.get());
    if (window == nullptr) {
        task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
            "Get top window failed"));
        WLOGE("WindowManager:GetTopWindowTask : Get top window failed");
        return;
    }
    windowName = window->GetWindowName();
    // get and create jsWindow
    task.Resolve(env, CreateJsWindowObject(env, window));
    WLOGD("WindowManager:GetTopWindowTask : Get top window %{public}s success", windowName.c_str());
    return;
}

napi_value JsWindowManager::OnGetLastWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowManager::OnGetLastWindow : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1 || argc > 2) {
        WLOGE("JsWindowStage::OnGetLastWindow : argc error![%{public}zu]", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }

    WLOGD("JsWindowManager::OnGetLastWindow : check parameter, argc = %{public}zu", argc);
    napi_value nativeCallback = nullptr;
    void* contextPtr = nullptr;
    status = napi_unwrap(env, argv[0], &contextPtr);
    if (status != napi_ok) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return nullptr;
    }
    if (argc > 1 && IsFunction(env, argv[1])) {
        nativeCallback = argv[1];
    }

    WLOGD("JsWindowManager::OnGetLastWindow : processing...");
    NapiAsyncTask::CompleteCallback complete = [=](napi_env env, NapiAsyncTask& task, int32_t status) {
        return GetTopWindowTask(contextPtr, env, task, true);
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowManager::OnGetTopWindow", env,
        CreateAsyncTaskWithLastParam(env, nativeCallback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowManagerInit(napi_env env, napi_value exportObj)
{
    WLOGD("JsWindowManagerInit");
    if (env == nullptr || exportObj == nullptr) {
        WLOGE("JsWindowManagerInit : env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowManager> jsWinManager = std::make_unique<JsWindowManager>();
    napi_wrap(env, exportObj, jsWinManager.release(), JsWindowManager::Finalizer, nullptr, nullptr);

    const napi_property_descriptor props[] = {
        DECLARE_NAPI_PROPERTY("WindowType", WindowTypeInit(env)),
        DECLARE_NAPI_PROPERTY("WindowMode", WindowModeInit(env)),
        DECLARE_NAPI_PROPERTY("Orientation", OrientationInit(env)),
        DECLARE_NAPI_PROPERTY("WindowEventType", WindowEventTypeInit(env)),
        DECLARE_NAPI_PROPERTY("WindowStageEventType", WindowStageEventTypeInit(env)),
        DECLARE_NAPI_PROPERTY("WmErrorCode", WindowErrorCodeInit(env)),
        DECLARE_NAPI_PROPERTY("WMError", WindowErrorInit(env)),
        DECLARE_NAPI_PROPERTY("WindowStatusType", WindowStatusTypeInit(env)),
        DECLARE_NAPI_PROPERTY("ColorSpace", WindowColorSpaceInit(env)),
        DECLARE_NAPI_FUNCTION("createWindow", JsWindowManager::CreateWindow),
        DECLARE_NAPI_FUNCTION("findWindow", JsWindowManager::FindWindowSync),
        DECLARE_NAPI_FUNCTION("getLastWindow", JsWindowManager::GetLastWindow),
        DECLARE_NAPI_PROPERTY("AvoidAreaType", AvoidAreaTypeInit(env)),
    };
    SetWindowObjectProperties(env, exportObj, "JsWindowManager", props, sizeof(props) / sizeof((props)[0]));
    return CreateUndefined(env);
}
} // namespace Rosen
} // namespace OHOS
