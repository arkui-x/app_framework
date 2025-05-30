/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_window.h"

#include <memory>
#include <string>
#include "color_parser.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "js_window_register_manager.h"
#include "js_window_utils.h"
#include "virtual_rs_window.h"
#include "window_hilog.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

static thread_local std::map<std::string, napi_ref> g_jsWindowMap;
#ifdef IOS_PLATFORM
static bool g_willTerminate;
#endif
std::recursive_mutex g_jsWindowMutex;
static constexpr Rect EMPTY_RECT = {0, 0, 0, 0};

JsWindow::JsWindow(std::shared_ptr<Rosen::Window>& window)
    : windowToken_(window), registerManager_(std::make_unique<JsWindowRegisterManager>())
{
    WLOGI("JsWindow::JsWindow");
}

JsWindow::~JsWindow()
{
    WLOGI("JsWindow::~JsWindow");
}

napi_value JsWindow::RegisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::On");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnRegisterWindowManagerCallback(env, info) : nullptr;
}

napi_value JsWindow::UnregisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnUnregisterWindowManagerCallback");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnUnregisterWindowManagerCallback(env, info) : nullptr;
}

napi_value JsWindow::SetWindowColorSpace(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowColorSpace(env, info) : nullptr;
}

napi_value JsWindow::GetWindowColorSpace(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::getWindowColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowColorSpace(env, info) : nullptr;
}

napi_value JsWindow::ShowWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::ShowWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnShowWindow(env, info) : nullptr;
}

napi_value JsWindow::DestroyWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::DestroyWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnDestroyWindow(env, info) : nullptr;
}

napi_value JsWindow::MoveWindowTo(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::MoveWindowTo");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnMoveWindowTo(env, info) : nullptr;
}

napi_value JsWindow::Resize(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::Resize");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnResize(env, info) : nullptr;
}

napi_value JsWindow::GetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::GetWindowPropertiesSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowPropertiesSync(env, info) : nullptr;
}

napi_value JsWindow::SetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowSystemBarEnable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarEnable(env, info) : nullptr;
}

napi_value JsWindow::SetPreferredOrientation(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetPreferredOrientation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetPreferredOrientation(env, info) : nullptr;
}

napi_value JsWindow::LoadContent(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::LoadContent");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsWindow::LoadContentByName(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::LoadContentByName");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsWindow::SetUIContent(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetUIContent");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetUIContent(env, info) : nullptr;
}

napi_value JsWindow::IsWindowShowingSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::IsWindowShowingSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnIsWindowShowingSync(env, info) : nullptr;
}

napi_value JsWindow::SetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowBackgroundColorSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColorSync(env, info) : nullptr;
}

napi_value JsWindow::SetWindowBrightness(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowBrightness");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowBrightness(env, info) : nullptr;
}

napi_value JsWindow::SetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowKeepScreenOn");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowKeepScreenOn(env, info) : nullptr;
}

napi_value JsWindow::SetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::SetWindowPrivacyMode");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowPrivacyMode(env, info) : nullptr;
}

napi_value JsWindow::SetWindowFocusable(napi_env env, napi_callback_info info)
{
    WLOGD("SetWindowFocusable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowFocusable(env, info) : nullptr;
}

napi_value JsWindow::OnShowWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnShowWindow : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        WLOGE("JsWindowStage::OnShowWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            WLOGE("JsWindow::OnShowWindow : window is nullptr or get invalid param");
            return;
        }
        WMError ret = weakWindow->ShowWindow();
        if (ret == WMError::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env,
                CreateWindowsJsError(env, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(ret)), "Window show failed"));
        }
        WLOGI("JsWindow::OnShowWindow : Window [%{public}u, %{public}s] show end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 1 && IsFunction(env, argv[0])) {
        callback = argv[0];
    }
    NapiAsyncTask::Schedule("JsWindow::OnShowWindow", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnDestroyWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnDestroyWindow : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        WLOGE("JsWindowStage::OnShowWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [this, weakToken](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            WLOGE("window is nullptr or get invalid param");
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        RemoveJsWindowObject(env, weakWindow->GetWindowName());
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Destroy());
        WLOGI("JsWindow::OnDestroyWindow : Window [%{public}u, %{public}s] destroy end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        if (ret != WmErrorCode::WM_OK) {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window destroy failed"));
            return;
        }
        windowToken_ = nullptr; // ensure window dtor when finalizer invalid
        task.Resolve(env, CreateUndefined(env));
    };
    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 1 && IsFunction(env, argv[0])) {
        callback = argv[0];
    }
    NapiAsyncTask::Schedule("JsWindow::OnDestroyWindow", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnMoveWindowTo(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnMoveWindowTo : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 2) {
        WLOGE("JsWindowStage::OnShowWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t x = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], x)) {
        WLOGE("JsWindow::OnMoveWindowTo : Failed to convert parameter to x");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t y = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[1], y)) {
        WLOGE("JsWindow::OnMoveWindowTo : Failed to convert parameter to y");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, x, y](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            WLOGE("JsWindow::OnMoveWindowTo : window is nullptr");
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->MoveWindowTo(x, y));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window move failed"));
        }
        WLOGI("JsWindow::OnMoveWindowTo : Window [%{public}u, %{public}s] move end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 2 && IsFunction(env, argv[2])) {
        callback = argv[2];
    }
    NapiAsyncTask::Schedule("JsWindow::OnMoveWindowTo", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnResize(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnResize : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 2 || argc > 3) {
        WLOGE("JsWindow::OnResize : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t width = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[0], width)) {
        WLOGE("JsWindow::OnResize : Failed to convert parameter to width");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t height = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(env, argv[1], height)) {
        WLOGE("JsWindow::OnResize : Failed to convert parameter to height");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (width <= 0 || height <= 0) {
        WLOGE("JsWindow::OnResize : width or height should greater than 0!");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    WLOGE("OnResize %p", windowToken_.get());
    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, width, height](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            WLOGE("JsWindow::OnResize : window is nullptr");
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->ResizeWindowTo(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window resize failed"));
        }
        WLOGI("JsWindow::OnResize : Window [%{public}u, %{public}s] resize end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 2 && IsFunction(env, argv[2])) {
        callback = argv[2];
    }
    NapiAsyncTask::Schedule("JsWindow::OnResize", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnGetWindowPropertiesSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnGetWindowPropertiesSync : Start...");
    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        WLOGE("JsWindow::OnGetWindowPropertiesSync : window is nullptr or get invalid param");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    auto objValue = CreateJsWindowPropertiesObject(env, window);
    WLOGI("JsWindow::OnGetWindowPropertiesSync : Window [%{public}u, %{public}s] get properties end",
        window->GetWindowId(), window->GetWindowName().c_str());
    if (objValue == nullptr) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    return objValue;
}

napi_value JsWindow::OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetWindowSystemBarEnable : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetWindowSystemBarEnable : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (!GetSystemBarStatus(systemBarProperties, env, argc, argv[0], windowToken_)) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, systemBarProperties](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSystemBarProperty(
            WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
        if (ret != WmErrorCode::WM_OK) {
            task.Reject(env,
                CreateWindowsJsError(env, static_cast<int32_t>(ret), "JsWindow::OnSetWindowSystemBarEnable failed"));
        }
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSystemBarProperty(
            WindowType::WINDOW_TYPE_NAVIGATION_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env,
                CreateWindowsJsError(env, static_cast<int32_t>(ret), "JsWindow::OnSetWindowSystemBarEnable failed"));
        }
    };
    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 1 && IsFunction(env, argv[0])) {
        callback = argv[0];
    } else if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowSystemBarEnable", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetPreferredOrientation(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetPreferredOrientation : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetPreferredOrientation : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    uint32_t type = 0;
    if (!ConvertFromJsValue(env, argv[0], type) ||
        !JS_TO_NATIVE_ORIENTATION_MAP.count(static_cast<ApiOrientation>(type))) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        WLOGE("JsWindow::OnSetPreferredOrientation : Orientation %{public}u invalid!",
                static_cast<uint32_t>(type));
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    Orientation requestedOrientation = JS_TO_NATIVE_ORIENTATION_MAP.at(static_cast<ApiOrientation>(type));
    if (requestedOrientation < Orientation::BEGIN || requestedOrientation > Orientation::END) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        WLOGE("JsWindow::OnSetPreferredOrientation : Orientation %{public}u invalid!",
                static_cast<uint32_t>(requestedOrientation));
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, requestedOrientation](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            WLOGE("JsWindow::OnSetPreferredOrientation : Window is nullptr");
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                                    "OnSetPreferredOrientation failed"));
            return;
        }
        weakWindow->SetRequestedOrientation(requestedOrientation);
        task.Resolve(env, CreateUndefined(env));
        WLOGI("JsWindow::OnSetPreferredOrientation : Window [%{public}u, %{public}s] "
                   "OnSetPreferredOrientation end, orientation = %{public}u",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            static_cast<uint32_t>(requestedOrientation));
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetPreferredOrientation", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WLOGD("JsWindow::OnLoadContent : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    napi_value storage = nullptr;
    napi_value callback = nullptr;
    std::string contextUrl;
    if (!GetContentArg(env, info, contextUrl, storage, callback)) {
        WLOGE("JsWindow::OnSetPreferredOrientation : Argc is invalid");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    WLOGD("JsWindow::OnLoadContent : Processing...");
    napi_ref contentStorage = nullptr;
    if ((errCode == WmErrorCode::WM_OK) && (storage != nullptr)) {
        napi_status status = napi_create_reference(env, storage, 1, &contentStorage);
        errCode = (status == napi_ok) ? WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Rosen::Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete = [weakToken, contentStorage, contextUrl, isLoadedByName](
        napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            WLOGE("JsWindow::OnLoadContent : Window is nullptr or get invalid param");
            task.Reject(env, CreateWindowsJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        LoadContentTask(env, contentStorage, contextUrl, weakWindow, task, isLoadedByName);
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnLoadContent", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetUIContent(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetUIContent : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    napi_value storage = nullptr;
    napi_value callback = nullptr;
    std::string contextUrl;
    if (!GetContentArg(env, info, contextUrl, storage, callback)) {
        WLOGE("JsWindow::OnSetUIContent : Argc is invalid");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    WLOGD("JsWindow::OnSetUIContent : Processing...");
    napi_ref contentStorage = nullptr;
    if ((errCode == WmErrorCode::WM_OK) && (storage != nullptr)) {
        napi_status status = napi_create_reference(env, storage, 1, &contentStorage);
        errCode = (status == napi_ok) ? WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, contentStorage, contextUrl](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            WLOGE("JsWindow::OnSetUIContent : Window is nullptr");
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        LoadContentTask(env, contentStorage, contextUrl, weakWindow, task, false);
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetUIContent", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnIsWindowShowingSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnIsWindowShowingSync : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc > 1) {
        WLOGE("JsWindow::OnIsWindowShowingSync : Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    std::weak_ptr<Rosen::Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    if (weakWindow == nullptr) {
        WLOGE("JsWindow::OnIsWindowShowingSync : window is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    bool state = weakWindow->IsWindowShow();
    WLOGD("JsWindow::OnIsWindowShowingSync : Window [%{public}u, %{public}s] get show state = %{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), state);
    return CreateJsValue(env, state);
}

napi_value JsWindow::OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetWindowBackgroundColorSync : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc != 1) {
        WLOGE("JsWindow::OnIsWindowShowingSync : Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::string color;
    uint32_t colorValue;
    if (!ConvertFromJsValue(env, argv[0], color) || !ColorParser::Parse(color, colorValue)) {
        WLOGE("JsWindow::OnSetWindowBackgroundColorSync : Failed to convert parameter to background color");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        WLOGE("JsWindow::OnSetWindowBackgroundColorSync : window is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    WLOGI("JsWindow::OnSetWindowBackgroundColorSync set color %{public}s, %{public}u", color.c_str(), colorValue);
    WMError err = window->SetBackgroundColor(colorValue);
    WLOGI("JsWindow::OnSetWindowBackgroundColorSync : Window [%{public}u, %{public}s] result %{public}u",
        window->GetWindowId(), window->GetWindowName().c_str(), static_cast<uint32_t>(err));
    errCode = WM_JS_TO_ERROR_CODE_MAP.at(err);
    if (errCode == WmErrorCode::WM_OK) {
        return CreateUndefined(env);
    } else {
        napi_throw(env, CreateWindowsJsError(env, errCode));
        return CreateUndefined(env);
    }
}

napi_value JsWindow::OnSetWindowBrightness(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetWindowBrightness : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetWindowBrightness : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    float brightness = UNDEFINED_BRIGHTNESS;
    if (!ConvertFromJsValue(env, argv[0], brightness)) {
        WLOGE("JsWindow::OnSetWindowBrightness : Failed to convert parameter to brightness");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, brightness](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                                    "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetBrightness(brightness));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window set brightness failed"));
        }
        WLOGI("JsWindow::OnSetWindowBrightness : Window [%{public}u, %{public}s] set brightness end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowBrightness", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetWindowPrivacyMode(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetWindowPrivacyMode : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetWindowPrivacyMode : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    bool isPrivacyMode = true;
    if (!ConvertFromJsValue(env, argv[0], isPrivacyMode)) {
        WLOGE("JsWindow::OnSetWindowPrivacyMode : Failed to convert parameter to isPrivacyMode");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, isPrivacyMode](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetWindowPrivacyMode(isPrivacyMode));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret),
                                                "Window set privacy mode failed"));
        }
        WLOGI("JsWindow::OnSetWindowPrivacyMode : Window [%{public}u, %{public}s] set keep screen on end",
              weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowPrivacyMode", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnSetWindowKeepScreenOn : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetWindowKeepScreenOn : Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    bool keepScreenOn = true;
    if (!ConvertFromJsValue(env, argv[0], keepScreenOn)) {
        WLOGE("JsWindow::OnSetWindowKeepScreenOn : Failed to convert parameter to keepScreenOn");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, keepScreenOn](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                                    "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetKeepScreenOn(keepScreenOn));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(ret), "Window set keep screen on failed"));
        }
        WLOGI("JsWindow::OnSetWindowKeepScreenOn : Window [%{public}u, %{public}s] set keep screen on end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowKeepScreenOn", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnSetWindowColorSpace(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindow::OnSetWindowColorSpace, argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    uint32_t resultValue = 0;
    if (!ConvertFromJsValue(env, argv[0], resultValue) ||
        resultValue >static_cast<uint32_t>(ColorSpace::COLOR_SPACE_WIDE_GAMUT)) {
        WLOGE("Failed to convert parameter to callbackType");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    ColorSpace colorSpace = static_cast<ColorSpace>(resultValue);
    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, colorSpace](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Invalidate params."));
            return;
        }
        WLOGI("JsWindow::OnSetWindowColorSpace colorSpace..., %{public}d",  colorSpace);
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetColorSpace(colorSpace));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, ret, "Window SetColorSpace failed"));
        }
        WLOGI("JsWindow::SetColorSpace : Window [%{public}u, %{public}s] set colorSpace end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowColorSpace", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::OnGetWindowColorSpace(napi_env env, napi_callback_info info)
{
    std::weak_ptr<Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    if (weakWindow == nullptr) {
        WLOGE("JsWindow::OnGetWindowColorSpace is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    ColorSpace colorSpace = weakWindow->GetColorSpace();
    WLOGI("JsWindow [%{public}u, %{public}s] OnGetColorSpace end, colorSpace = %{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));
    return CreateJsValue(env, static_cast<uint32_t>(colorSpace));
}

napi_value JsWindow::OnRegisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnRegisterWindowManagerCallback : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 2 || !IsFunction(env, argv[1])) {
        WLOGE("JsWindow::OnRegisterWindowManagerCallback, argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGE("Failed to convert parameter to callbackType");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    WmErrorCode ret = registerManager_->RegisterListener(env, windowToken_, cbType, CaseType::CASE_WINDOW, argv[1]);
    if (ret != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    WLOGD("Register end, type = %{public}s", cbType.c_str());
    return nullptr;
}

napi_value JsWindow::OnUnregisterWindowManagerCallback(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindow::OnUnregisterWindowManagerCallback : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("Argc is invalid: %{public}zu", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    std::string cbType;
    if (!ConvertFromJsValue(env, argv[0], cbType)) {
        WLOGE("Failed to convert parameter to callbackType");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    
    if (argc > 1 && IsFunction(env, argv[1])) {
        registerManager_->UnregisterListener(env, windowToken_, cbType, CaseType::CASE_WINDOW, argv[1]);
    } else {
        registerManager_->UnregisterListener(env, windowToken_, cbType, CaseType::CASE_WINDOW, nullptr);
    }
    WLOGD("Unregister end, type = %{public}s", cbType.c_str());
    return nullptr;
}

napi_value JsWindow::OnSetWindowFocusable(napi_env env, napi_callback_info info)
{
    WLOGI("JsWindow::OnSetWindowFocusable : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindowStage::OnSetWindowFocusable : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    bool isFocusable = false;
    if (!ConvertFromJsValue(env, argv[0], isFocusable)) {
        WLOGE("JsWindow::OnSetWindowFocusable : Failed to convert parameter to isFocusable");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, isFocusable](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            WLOGE("JsWindow::OnSetWindowFocusable : window is nullptr or get invalid param");
            return;
        }
        WMError ret = weakWindow->SetFocusable(isFocusable);
        if (ret == WMError::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env,
                CreateWindowsJsError(env, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(ret)), "SetFocusable"));
        }
        WLOGI("JsWindow::OnSetWindowFocusable : SetWindowFocusable [%{public}u, %{public}s] end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    napi_value result = nullptr;
    napi_value callback = nullptr;
    if (argc >= 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowFocusable", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_ref FindJsWindowObject(const std::string &windowName)
{
    WLOGD("Try to find window %{public}s in g_jsWindowMap", windowName.c_str());
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    if (g_jsWindowMap.find(windowName) == g_jsWindowMap.end()) {
        WLOGI("Can not find window %{public}s in g_jsWindowMap", windowName.c_str());
        return nullptr;
    }
    return g_jsWindowMap[windowName];
}

void RemoveJsWindowObject(napi_env env, const std::string &windowName)
{
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    auto iter = g_jsWindowMap.find(windowName);
    if (iter != g_jsWindowMap.end()) {
        WLOGI("RemoveJsWindowObject %{public}s", windowName.c_str());
        napi_delete_reference(env, iter->second);
        g_jsWindowMap.erase(iter);
    }
}

std::string JsWindow::GetWindowName()
{
    std::weak_ptr<Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    return (weakWindow == nullptr) ? "" : weakWindow->GetWindowName();
}

void JsWindow::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("JsWindow::Finalizer");
#ifdef IOS_PLATFORM
    if (g_willTerminate) {
        return;
    }
#endif
    auto jsWin = std::unique_ptr<JsWindow>(static_cast<JsWindow*>(data));
    if (jsWin == nullptr) {
        WLOGE("jsWin is nullptr");
        return;
    }
    std::string windowName = jsWin->GetWindowName();
    RemoveJsWindowObject(env, windowName);
    WLOGI("Remove window %{public}s from g_jsWindowMap", windowName.c_str());
}

napi_value JsWindow::GetUIContext(napi_env env, napi_callback_info info)
{
    WLOGD("GetUIContext");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetUIContext(env, info) : nullptr;
}

napi_value JsWindow::OnGetUIContext(napi_env env, napi_callback_info info)
{
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc >= 1) {
        WLOGE("Argc is invalid: %{public}zu, expect zero params", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        WLOGE("window is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }

    auto uicontent = window->GetUIContent();
    if (uicontent == nullptr) {
        WLOGE("uicontent is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }

    napi_value uiContext = uicontent->GetUINapiContext();
    if (uiContext == nullptr) {
        WLOGE("uiContext obtained from jsEngine is nullptr");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    return uiContext;
}

napi_value JsWindow::SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    WLOGI("SetSystemBarEnable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetSpecificSystemBarEnabled(env, info) : nullptr;
}

napi_value JsWindow::OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info)
{
    WmErrorCode err = (windowToken_ == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    std::string name;
    if (!ConvertFromJsValue(env, argv[0], name)) {
        WLOGE("Failed to convert parameter to SystemBarName");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (err == WmErrorCode::WM_OK && (argc < 1 || // 1: params num
        !GetSpecificBarStatus(systemBarProperties, env, info, windowToken_))) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete = [weakToken, systemBarProperties, name, err]
            (napi_env env, NapiAsyncTask& task, int32_t status) mutable {
        auto weakWindow = weakToken.lock();
        err = (weakWindow == nullptr) ? WmErrorCode::WM_ERROR_STATE_ABNORMALLY : err;
        if (err != WmErrorCode::WM_OK) {
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(err)));
            return;
        }
        if (name.compare("status") == 0) {
            err = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(
                WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
        } else if (name.compare("navigation") == 0) {
            err = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(WindowType::WINDOW_TYPE_NAVIGATION_BAR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
        } else if (name.compare("navigationIndicator") == 0) {
            err = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSpecificBarProperty(
                WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR,
                systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_INDICATOR)));
        }
        if (err == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            task.Reject(env, CreateWindowsJsError(env, static_cast<int32_t>(err),
                "JsWindow::OnSetSpecificSystemBarEnabled failed"));
        }
    };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetSpecificSystemBarEnabled",
        env, CreateAsyncTaskWithLastParam(env, nullptr, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::SetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
{
    WLOGD("SetLayoutFullScreen");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowLayoutFullScreen(env, info) : nullptr;
}

napi_value JsWindow::OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        WLOGE("Argc is invalid: %{public}zu", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    bool isLayoutFullScreen = false;
    if (errCode == WmErrorCode::WM_OK) {
        napi_value nativeVal = argv[0];
        if (nativeVal == nullptr) {
            WLOGE("Failed to convert parameter to isLayoutFullScreen");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            napi_get_value_bool(env, nativeVal, &isLayoutFullScreen);
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
        [weakToken, isLayoutFullScreen](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakWindow = weakToken.lock();
            if (weakWindow == nullptr) {
                task.Reject(env,
                    CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                    "Invalidate params."));
                return;
            }
            WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetLayoutFullScreen(isLayoutFullScreen));
            if (ret == WmErrorCode::WM_OK) {
                task.Resolve(env, CreateUndefined(env));
            } else {
                task.Reject(env, CreateWindowsJsError(env,
                    static_cast<int32_t>(ret), "Window OnSetLayoutFullScreen failed."));
            }
            WLOGI("Window [%{public}u, %{public}s] set layout full screen end, ret = %{public}d",
                weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        };

    napi_value lastParam = (argc <= 1) ? nullptr :
        ((argv[1] != nullptr && IsFunction(env, argv[1]) == napi_function) ? argv[1] : nullptr);
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowLayoutFullScreen",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindow::GetWindowAvoidAreaSync(napi_env env, napi_callback_info info)
{
    WLOGD("GetWindowAvoidAreaSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnGetWindowAvoidAreaSync(env, info) : nullptr;
}

static void ParseAvoidAreaParam(napi_env env, napi_callback_info info, WMError& errCode, AvoidAreaType& avoidAreaType)
{
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1 || argc > 2) {
        WLOGE("Argc is invalid: %{public}zu", argc);
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    }
    
    if (errCode == WMError::WM_OK) {
        napi_value nativeMode = argv[0];
        if (nativeMode == nullptr) {
            WLOGE("Failed to convert parameter to AvoidAreaType");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
        } else {
            uint32_t resultValue = 0;
            napi_status status = napi_get_value_uint32(env, nativeMode, &resultValue);
            if (status != napi_ok) {
                WLOGE("napi_get_value_uint32 failed, status = %{public}d", status);
                errCode = WMError::WM_ERROR_INVALID_PARAM;
            }
            errCode = WMError::WM_OK;
        }
    }
}

napi_value JsWindow::OnGetWindowAvoidAreaSync(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = 4;
    napi_value argv[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < 1) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    AvoidAreaType avoidAreaType = AvoidAreaType::TYPE_SYSTEM;
    napi_value nativeMode = argv[0];
    if (nativeMode == nullptr) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        uint32_t resultValue = 0;
        napi_get_value_uint32(env, nativeMode, &resultValue);
        avoidAreaType = static_cast<AvoidAreaType>(resultValue);
        errCode = ((avoidAreaType > AvoidAreaType::TYPE_NAVIGATION_INDICATOR) ||
                   (avoidAreaType < AvoidAreaType::TYPE_SYSTEM)) ?
            WmErrorCode::WM_ERROR_INVALID_PARAM : WmErrorCode::WM_OK;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    // getAvoidRect by avoidAreaType
    AvoidArea avoidArea;
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->GetAvoidAreaByType(avoidAreaType, avoidArea));
    if (ret != WmErrorCode::WM_OK) {
        avoidArea.topRect_ = EMPTY_RECT;
        avoidArea.leftRect_ = EMPTY_RECT;
        avoidArea.rightRect_ = EMPTY_RECT;
        avoidArea.bottomRect_ = EMPTY_RECT;
    }
    WLOGI("Window [%{public}u, %{public}s] get avoid area type %{public}d end, ret %{public}d "
          "top{%{public}d,%{public}d,%{public}d,%{public}d}, down{%{public}d,%{public}d,%{public}d,%{public}d}",
          window->GetWindowId(), window->GetWindowName().c_str(), avoidAreaType, ret,
          avoidArea.topRect_.posX_, avoidArea.topRect_.posY_, avoidArea.topRect_.width_, avoidArea.topRect_.height_,
          avoidArea.bottomRect_.posX_, avoidArea.bottomRect_.posY_, avoidArea.bottomRect_.width_,
          avoidArea.bottomRect_.height_);
    napi_value avoidAreaObj = ConvertAvoidAreaToJsValue(env, avoidArea, avoidAreaType);
    if (avoidAreaObj != nullptr) {
        return avoidAreaObj;
    } else {
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
}

napi_value JsWindow::SetWindowSystemBarProperties(napi_env env, napi_callback_info info)
{
    WLOGD("SetWindowSystemBarProperties");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(env, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarProperties(env, info) : nullptr;
}

napi_value JsWindow::OnSetWindowSystemBarProperties(napi_env env, napi_callback_info info)
{
    size_t argc = 4;
    napi_value argv[4] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    napi_value lastParam = (argc <= 1) ? nullptr :
        (IsFunction(env, argv[1]) == napi_function ? argv[1] : nullptr);
    napi_value result = nullptr;
    std::unordered_map<WindowType, SystemBarProperty> systemBarProperties;
    std::unordered_map<WindowType, SystemBarPropertyFlag> systemBarPropertyFlags;
    if (argc < 1 || argv[0] == nullptr ||
        !GetSystemBarPropertiesFromJs(env, argv[0], systemBarProperties, systemBarPropertyFlags)) {
        WLOGE("argc is invalid or failed to convert parameter ");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM, "Invalid params."));
        return CreateJsUndefined(env);
    }
    std::weak_ptr<Window> weakToken(windowToken_);
    NapiAsyncTask::CompleteCallback complete =
       [weakToken, systemBarProperties = std::move(systemBarProperties), systemBarPropertyFlags =
        std::move(systemBarPropertyFlags)](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto window = weakToken.lock();
        if (window == nullptr) {
            task.Reject(env,
                CreateJsError(env, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(window->UpdateSystemBarProperties(systemBarProperties,
            systemBarPropertyFlags));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(env, CreateUndefined(env));
        } else {
            WLOGE("set system bar properties failed, errcode: %{public}d", ret);
            task.Reject(env, CreateWindowsJsError(env, ret,
                "SetWindowSystemBarProperties"));
        }
        WLOGD("JsWindow::OnSetWindowSystemBarProperties : SetWindowSystemBarProperties"
            "[%{public}u, %{public}s] end, ret = %{public}d",
            window->GetWindowId(), window->GetWindowName().c_str(), ret);
    };

    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    NapiAsyncTask::Schedule("JsWindow::OnSetWindowSystemBarProperties", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

static const napi_property_descriptor g_props[] = {
    DECLARE_NAPI_FUNCTION("showWindow", JsWindow::ShowWindow),
    DECLARE_NAPI_FUNCTION("destroyWindow", JsWindow::DestroyWindow),
    DECLARE_NAPI_FUNCTION("moveWindowTo", JsWindow::MoveWindowTo),
    DECLARE_NAPI_FUNCTION("resize", JsWindow::Resize),
    DECLARE_NAPI_FUNCTION("getWindowProperties", JsWindow::GetWindowPropertiesSync),
    DECLARE_NAPI_FUNCTION("setWindowSystemBarEnable", JsWindow::SetWindowSystemBarEnable),
    DECLARE_NAPI_FUNCTION("setPreferredOrientation", JsWindow::SetPreferredOrientation),
    DECLARE_NAPI_FUNCTION("loadContent", JsWindow::LoadContent),
    DECLARE_NAPI_FUNCTION("loadContentByName", JsWindow::LoadContentByName),
    DECLARE_NAPI_FUNCTION("getUIContext", JsWindow::GetUIContext),
    DECLARE_NAPI_FUNCTION("setUIContent", JsWindow::SetUIContent),
    DECLARE_NAPI_FUNCTION("isWindowShowing", JsWindow::IsWindowShowingSync),
    DECLARE_NAPI_FUNCTION("setWindowBackgroundColor", JsWindow::SetWindowBackgroundColorSync),
    DECLARE_NAPI_FUNCTION("setWindowBrightness", JsWindow::SetWindowBrightness),
    DECLARE_NAPI_FUNCTION("setWindowKeepScreenOn", JsWindow::SetWindowKeepScreenOn),
    DECLARE_NAPI_FUNCTION("setWindowPrivacyMode", JsWindow::SetWindowPrivacyMode),
    DECLARE_NAPI_FUNCTION("on", JsWindow::RegisterWindowManagerCallback),
    DECLARE_NAPI_FUNCTION("off", JsWindow::UnregisterWindowManagerCallback),
    DECLARE_NAPI_FUNCTION("setWindowColorSpace", JsWindow::SetWindowColorSpace),
    DECLARE_NAPI_FUNCTION("getWindowColorSpace", JsWindow::GetWindowColorSpace),
    DECLARE_NAPI_FUNCTION("setSpecificSystemBarEnabled", JsWindow::SetSpecificSystemBarEnabled),
    DECLARE_NAPI_FUNCTION("setWindowLayoutFullScreen", JsWindow::SetWindowLayoutFullScreen),
    DECLARE_NAPI_FUNCTION("getWindowAvoidArea", JsWindow::GetWindowAvoidAreaSync),
    DECLARE_NAPI_FUNCTION("setWindowFocusable", JsWindow::SetWindowFocusable),
    DECLARE_NAPI_FUNCTION("setWindowSystemBarProperties", JsWindow::SetWindowSystemBarProperties),
};

napi_value CreateJsWindowObject(napi_env env, std::shared_ptr<Rosen::Window>& window)
{
    std::string windowName = window->GetWindowName();
    WLOGD("CreateJsWindowObject windowName %{public}s", windowName.c_str());
    // avoid repeatedly create js window when getWindow
    napi_value object = nullptr;
    napi_ref ref = FindJsWindowObject(windowName);
    if (ref != nullptr) {
        napi_status status = napi_get_reference_value(env, ref, &object);
        if (status != napi_ok) {
            WLOGE("Failed to get object for %{public}s", windowName.c_str());
            return nullptr;
        }
        return object;
    }

    std::unique_ptr<JsWindow> jsWindow = std::make_unique<JsWindow>(window);
    object = CreateObject(env, "JsWindow", g_props, sizeof(g_props) / sizeof(g_props[0]));
    if (object == nullptr) {
        WLOGE("Create jsWindow fail");
        return nullptr;
    }
    napi_wrap(env, object, jsWindow.release(), JsWindow::Finalizer, nullptr, nullptr);
    NotifyNativeWinDestroyFunc func = [env](std::string windowName) {
        WLOGI("Destroy window %{public}s in js window", windowName.c_str());
        RemoveJsWindowObject(env, windowName);
    };
    window->RegisterWindowDestroyedListener(func);

#ifdef IOS_PLATFORM
    NotifyWillTerminateFunc terminamteFunc = []() {
        g_willTerminate = true;
    };
    window->RegisterWillTerminateListener(terminamteFunc);
#endif
    napi_status status = napi_create_reference(env, object, 1, &ref);
    if (status != napi_ok) {
        WLOGE("Failed to reference for %{public}s", windowName.c_str());
        return nullptr;
    }
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    g_jsWindowMap[windowName] = ref;
    return object;
}
} // namespace Rosen
} // namespace OHOS
