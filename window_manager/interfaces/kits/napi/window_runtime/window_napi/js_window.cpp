/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "hilog.h"
#include "js_runtime_utils.h"
#include "js_window_utils.h"
#include "virtual_rs_window.h"
#include "wm_common.h"
#include "js_window_register_manager.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

static thread_local std::map<std::string, std::shared_ptr<NativeReference>> g_jsWindowMap;
std::unique_ptr<JsWindowRegisterManager> g_registerManager =  std::make_unique<JsWindowRegisterManager>();
#ifdef IOS_PLATFORM
static bool g_willTerminate;
#endif
std::recursive_mutex g_jsWindowMutex;

JsWindow::JsWindow(std::shared_ptr<Rosen::Window>& window) : windowToken_(window)
{
    HILOG_INFO("JsWindow::JsWindow");
    NotifyNativeWinDestroyFunc func = [](std::string windowName) {
        std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
        if (windowName.empty() || g_jsWindowMap.count(windowName) == 0) {
            HILOG_ERROR("Can not find window %{public}s ", windowName.c_str());
            return;
        }
        g_jsWindowMap.erase(windowName);
        HILOG_INFO("Destroy window %{public}s in js window", windowName.c_str());
    };
    windowToken_->RegisterWindowDestroyedListener(func);
#ifdef IOS_PLATFORM
    NotifyWillTerminateFunc terminamteFunc = []() {
        g_willTerminate = true;
    };
    windowToken_->RegisterWillTerminateListener(terminamteFunc);
#endif
}

JsWindow::~JsWindow()
{
    HILOG_INFO("JsWindow::~JsWindow");
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    std::shared_ptr<Window> weakWindow, NativeEngine& engine, AsyncTask& task)
{
    NativeValue* nativeStorage = (contentStorage == nullptr) ? nullptr : contentStorage->Get();
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
        weakWindow->SetUIContent(contextUrl, &engine, nativeStorage, false, nullptr));
    if (ret == WmErrorCode::WM_OK) {
        task.Resolve(engine, engine.CreateUndefined());
    } else {
        task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window load content failed"));
    }
}

NativeValue* JsWindow::RegisterWindowManagerCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::On");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnRegisterWindowManagerCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindow::UnregisterWindowManagerCallback(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::OnUnregisterWindowManagerCallback");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnUnregisterWindowManagerCallback(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowColorSpace(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetWindowColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowColorSpace(*engine, *info) : nullptr;
}

NativeValue* JsWindow::GetWindowColorSpace(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::getWindowColorSpace");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetWindowColorSpace(*engine, *info) : nullptr;
}

NativeValue* JsWindow::ShowWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::ShowWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnShowWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindow::DestroyWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::DestroyWindow");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnDestroyWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindow::MoveWindowTo(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsWindow::MoveWindowTo");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnMoveWindowTo(*engine, *info) : nullptr;
}

NativeValue* JsWindow::Resize(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_DEBUG("JsWindow::Resize");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnResize(*engine, *info) : nullptr;
}

NativeValue* JsWindow::GetWindowPropertiesSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::GetWindowPropertiesSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetWindowPropertiesSync(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowSystemBarEnable(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetWindowSystemBarEnable");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowSystemBarEnable(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetPreferredOrientation(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetPreferredOrientation");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetPreferredOrientation(*engine, *info) : nullptr;
}

NativeValue* JsWindow::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::LoadContent");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetUIContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetUIContent");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetUIContent(*engine, *info) : nullptr;
}

NativeValue* JsWindow::IsWindowShowingSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::IsWindowShowingSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnIsWindowShowingSync(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowBackgroundColorSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetWindowBackgroundColorSync");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowBackgroundColorSync(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowBrightness(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetWindowBrightness");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowBrightness(*engine, *info) : nullptr;
}

NativeValue* JsWindow::SetWindowKeepScreenOn(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindow::SetWindowKeepScreenOn");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnSetWindowKeepScreenOn(*engine, *info) : nullptr;
}

NativeValue* JsWindow::OnShowWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnShowWindow : Start...");
    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            HILOG_ERROR("JsWindow::OnShowWindow : window is nullptr or get invalid param");
            return;
        }
        WMError ret = weakWindow->ShowWindow();
        if (ret == WMError::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(WM_JS_TO_ERROR_CODE_MAP.at(ret)), "Window show failed"));
        }
        HILOG_INFO("JsWindow::OnShowWindow : Window [%{public}u, %{public}s] show end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };
    NativeValue* result = nullptr;
    NativeValue* lastParam =
        (info.argc == 0)
            ? nullptr
            : ((info.argv[0] != nullptr && info.argv[0]->TypeOf() == NATIVE_FUNCTION) ? info.argv[0] : nullptr);
    AsyncTask::Schedule("JsWindow::OnShowWindow", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnDestroyWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnDestroyWindow : Start...");
    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [this, weakToken](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            HILOG_ERROR("window is nullptr or get invalid param");
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        RemoveJsWindowObject(weakWindow->GetWindowName());
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->Destroy());
        HILOG_INFO("JsWindow::OnDestroyWindow : Window [%{public}u, %{public}s] destroy end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
        if (ret != WmErrorCode::WM_OK) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window destroy failed"));
            return;
        }
        windowToken_ = nullptr; // ensure window dtor when finalizer invalid
        task.Resolve(engine, engine.CreateUndefined());
    };

    NativeValue* lastParam =
        (info.argc == 0)
            ? nullptr
            : ((info.argv[0] != nullptr && info.argv[0]->TypeOf() == NATIVE_FUNCTION) ? info.argv[0] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnDestroyWindow", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnMoveWindowTo(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnMoveWindowTo : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 2) { // 2:minimum param num
        HILOG_ERROR("JsWindow::OnMoveWindowTo : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t x = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[0], x)) {
        HILOG_ERROR("JsWindow::OnMoveWindowTo : Failed to convert parameter to x");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t y = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[1], y)) {
        HILOG_ERROR("JsWindow::OnMoveWindowTo : Failed to convert parameter to y");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnMoveWindowTo : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, x, y](NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            HILOG_ERROR("JsWindow::OnMoveWindowTo : window is nullptr");
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->MoveWindowTo(x, y));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window move failed"));
        }
        HILOG_INFO("JsWindow::OnMoveWindowTo : Window [%{public}u, %{public}s] move end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };

    // 2: params num; 2: index of callback
    NativeValue* lastParam =
        (info.argc <= 2)
            ? nullptr
            : ((info.argv[2] != nullptr && info.argv[2]->TypeOf() == NATIVE_FUNCTION) ? info.argv[2] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnMoveWindowTo", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnResize(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnResize : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 2 || info.argc > 3) {
        HILOG_ERROR("JsWindow::OnResize : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t width = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[0], width)) {
        HILOG_ERROR("JsWindow::OnResize : Failed to convert parameter to width");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    int32_t height = 0;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[1], height)) {
        HILOG_ERROR("JsWindow::OnResize : Failed to convert parameter to height");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (width <= 0 || height <= 0) {
        HILOG_ERROR("JsWindow::OnResize : width or height should greater than 0!");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnResize : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }
    HILOG_ERROR("OnResize %p", windowToken_.get());
    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, width, height](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            HILOG_ERROR("JsWindow::OnResize : window is nullptr");
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(
            weakWindow->ResizeWindowTo(static_cast<uint32_t>(width), static_cast<uint32_t>(height)));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window resize failed"));
        }
        HILOG_INFO("JsWindow::OnResize : Window [%{public}u, %{public}s] resize end, ret = %{public}d",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), ret);
    };

    // 2: params num; 2: index of callback
    NativeValue* lastParam =
        (info.argc <= 2)
            ? nullptr
            : ((info.argv[2] != nullptr && info.argv[2]->TypeOf() == NATIVE_FUNCTION) ? info.argv[2] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnResize", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnGetWindowPropertiesSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnGetWindowPropertiesSync : Start...");
    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        HILOG_ERROR("JsWindow::OnGetWindowPropertiesSync : window is nullptr or get invalid param");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    auto objValue = CreateJsWindowPropertiesObject(engine, window);
    HILOG_INFO("JsWindow::OnGetWindowPropertiesSync : Window [%{public}u, %{public}s] get properties end",
        window->GetWindowId(), window->GetWindowName().c_str());
    if (objValue != nullptr) {
        return objValue;
    } else {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
}

NativeValue* JsWindow::OnSetWindowSystemBarEnable(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetWindowSystemBarEnable : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::map<WindowType, SystemBarProperty> systemBarProperties;
    if (info.argc < 1 || windowToken_ == nullptr || // 1: params num
        !GetSystemBarStatus(systemBarProperties, engine, info, windowToken_)) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_INFO("JsWindow::OnSetWindowSystemBarEnable : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }
    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, systemBarProperties](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSystemBarProperty(
            WindowType::WINDOW_TYPE_STATUS_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_STATUS_BAR)));
        if (ret != WmErrorCode::WM_OK) {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnSetWindowSystemBarEnable failed"));
        }
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetSystemBarProperty(
            WindowType::WINDOW_TYPE_NAVIGATION_BAR, systemBarProperties.at(WindowType::WINDOW_TYPE_NAVIGATION_BAR)));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine,
                CreateJsError(engine, static_cast<int32_t>(ret), "JsWindow::OnSetWindowSystemBarEnable failed"));
        }
    };
    NativeValue* lastParam = nullptr;
    if (info.argc >= 1 && info.argv[0] != nullptr && info.argv[0]->TypeOf() == NATIVE_FUNCTION) {
        lastParam = info.argv[0];
    } else if (info.argc >= 2 && // 2 arg count
               info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) {
        lastParam = info.argv[1];
    }
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetWindowSystemBarEnable", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetPreferredOrientation(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetPreferredOrientation : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    Orientation requestedOrientation = Orientation::UNSPECIFIED;
    if (info.argc < 1) { // 1: params num
        HILOG_ERROR("JsWindow::OnSetPreferredOrientation : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        NativeNumber* nativeType = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
        if (nativeType == nullptr) {
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            HILOG_ERROR("JsWindow::OnSetPreferredOrientation : Failed to convert parameter to Orientation");
        } else {
            requestedOrientation =
                JS_TO_NATIVE_ORIENTATION_MAP.at(static_cast<ApiOrientation>(static_cast<uint32_t>(*nativeType)));
            if (requestedOrientation < Orientation::BEGIN || requestedOrientation > Orientation::END) {
                HILOG_ERROR("JsWindow::OnSetPreferredOrientation : Orientation %{public}u invalid!",
                    static_cast<uint32_t>(requestedOrientation));
                errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
            }
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnSetPreferredOrientation : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, requestedOrientation](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            HILOG_ERROR("JsWindow::OnSetPreferredOrientation : Window is nullptr");
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                                    "OnSetPreferredOrientation failed"));
            return;
        }
        weakWindow->SetRequestedOrientation(requestedOrientation);
        task.Resolve(engine, engine.CreateUndefined());
        HILOG_INFO("JsWindow::OnSetPreferredOrientation : Window [%{public}u, %{public}s] "
                   "OnSetPreferredOrientation end, orientation = %{public}u",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(),
            static_cast<uint32_t>(requestedOrientation));
    };

    NativeValue* lastParam =
        (info.argc <= 1)
            ? nullptr
            : ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ? info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetPreferredOrientation", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnLoadContent : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    NativeValue* storage = nullptr;
    NativeValue* callback = nullptr;

    HILOG_INFO("JsWindow::OnLoadContent : Check Parameter");
    if (info.argc == 2) {
        if (info.argv[1]->TypeOf() == NATIVE_OBJECT) {
            storage = info.argv[1];
        } else if (info.argv[1]->TypeOf() == NATIVE_FUNCTION) {
            callback = info.argv[1];
        } else {
            HILOG_INFO(
                "JsWindow::OnLoadContent : info.argv[1] TypeOf %{public}d errorCode = -1", info.argv[1]->TypeOf());
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    } else if (info.argc == 3) {
        if (info.argv[1]->TypeOf() == NATIVE_OBJECT && info.argv[2]->TypeOf() == NATIVE_FUNCTION) {
            storage = info.argv[1];
            callback = info.argv[2];
        } else {
            HILOG_INFO("JsWindow::OnLoadContent : info.argv[1]&info.argv[2] TypeOf errorCode = -1");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    } else {
        HILOG_INFO("JsWindow::OnLoadContent : info.argc errorCode = -1");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    HILOG_INFO("JsWindow::OnLoadContent : Parse Parameter");
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK) {
        if (!ConvertFromJsValue(engine, info.argv[0], contextUrl)) {
            HILOG_INFO("JsWindow::OnLoadContent : ConvertFromJsValue errorCode = -1");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_INFO("JsWindow::OnLoadContent : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    HILOG_INFO("JsWindow::OnLoadContent : Processing...");
    std::shared_ptr<NativeReference> contentStorage =
        (storage == nullptr) ? nullptr : std::shared_ptr<NativeReference>(engine.CreateReference(storage, 1));
    std::weak_ptr<Rosen::Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, contentStorage, contextUrl](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            HILOG_ERROR("JsWindow::OnLoadContent : Window is nullptr or get invalid param");
            task.Reject(engine, CreateJsError(engine,
                static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, weakWindow, engine, task);
    };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnLoadContent", engine,
        CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetUIContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetUIContent : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) { // 2 maximum param num
        HILOG_ERROR("JsWindow::OnSetUIContent : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (errCode == WmErrorCode::WM_OK && !ConvertFromJsValue(engine, info.argv[0], contextUrl)) {
        HILOG_ERROR("JsWindow::OnSetUIContent : Failed to convert parameter to context url");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_INFO("JsWindow::OnSetUIContent : errorCode != 0");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }
    NativeValue* storage = nullptr;
    NativeValue* callBack = nullptr;
    if (info.argc >= 2) { // 2 param num
        callBack = info.argv[1];
    }
    std::shared_ptr<NativeReference> contentStorage =
        (storage == nullptr) ? nullptr : std::shared_ptr<NativeReference>(engine.CreateReference(storage, 1));
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnSetUIContent : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, contentStorage, contextUrl](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            HILOG_ERROR("JsWindow::OnSetUIContent : Window is nullptr");
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return;
        }
        LoadContentTask(contentStorage, contextUrl, weakWindow, engine, task);
    };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetUIContent", engine,
        CreateAsyncTaskWithLastParam(engine, callBack, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnIsWindowShowingSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnIsWindowShowingSync : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc > 1) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_INFO("JsWindow::OnIsWindowShowingSync : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }
    std::weak_ptr<Rosen::Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    if (weakWindow == nullptr) {
        HILOG_ERROR("JsWindow::OnIsWindowShowingSync : window is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    bool state = weakWindow->IsWindowShow();
    HILOG_INFO("JsWindow::OnIsWindowShowingSync : Window [%{public}u, %{public}s] get show state = %{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), state);
    return CreateJsValue(engine, state);
}

NativeValue* JsWindow::OnSetWindowBackgroundColorSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetWindowBackgroundColorSync : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::string color;
    if (info.argc != 1) {
        HILOG_ERROR("JsWindow::OnSetWindowBackgroundColorSync : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(engine, info.argv[0], color)) {
            HILOG_ERROR("JsWindow::OnSetWindowBackgroundColorSync : Failed to convert parameter to background color");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnSetWindowBackgroundColorSync : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        HILOG_ERROR("JsWindow::OnSetWindowBackgroundColorSync : window is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    uint32_t colorValue;
    if (!ColorParser::Parse(color, colorValue)) {
        HILOG_ERROR("JsWindow::OnSetWindowBackgroundColorSync : color is error");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    HILOG_INFO(
        "JsWindow::OnSetWindowBackgroundColorSync : set color %{public}s, %{public}u", color.c_str(), colorValue);
    WMError err = window->SetBackgroundColor(colorValue);
    WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(err);
    if (ret == WmErrorCode::WM_OK) {
        HILOG_INFO("JsWindow::OnSetWindowBackgroundColorSync : Window [%{public}u, %{public}s] "
                   "set background color end",
            window->GetWindowId(), window->GetWindowName().c_str());
        return engine.CreateUndefined();
    } else {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(ret)));
        return engine.CreateUndefined();
    }
}

NativeValue* JsWindow::OnSetWindowBrightness(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetWindowBrightness : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) {
        HILOG_ERROR("JsWindow::OnSetWindowBrightness : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    float brightness = UNDEFINED_BRIGHTNESS;
    if (errCode == WmErrorCode::WM_OK) {
        NativeNumber* nativeVal = ConvertNativeValueTo<NativeNumber>(info.argv[0]);
        if (nativeVal == nullptr) {
            HILOG_ERROR("JsWindow::OnSetWindowBrightness : Failed to convert parameter to brightness");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            brightness = static_cast<double>(*nativeVal);
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnSetWindowBrightness : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, brightness](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                                    "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetBrightness(brightness));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window set brightness failed"));
        }
        HILOG_INFO("JsWindow::OnSetWindowBrightness : Window [%{public}u, %{public}s] set brightness end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    NativeValue* lastParam =
        (info.argc <= 1)
            ? nullptr
            : ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ? info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetWindowBrightness", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetWindowKeepScreenOn(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetWindowKeepScreenOn : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) {
        HILOG_ERROR("JsWindow::OnSetWindowKeepScreenOn : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    bool keepScreenOn = true;
    if (errCode == WmErrorCode::WM_OK) {
        NativeBoolean* nativeVal = ConvertNativeValueTo<NativeBoolean>(info.argv[0]);
        if (nativeVal == nullptr) {
            HILOG_ERROR("JsWindow::OnSetWindowKeepScreenOn : Failed to convert parameter to keepScreenOn");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        } else {
            keepScreenOn = static_cast<bool>(*nativeVal);
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindow::OnSetWindowKeepScreenOn : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, keepScreenOn](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                                    "Invalidate params."));
            return;
        }
        WmErrorCode ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetKeepScreenOn(keepScreenOn));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window set keep screen on failed"));
        }
        HILOG_INFO("JsWindow::OnSetWindowKeepScreenOn : Window [%{public}u, %{public}s] set keep screen on end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    NativeValue* lastParam =
        (info.argc <= 1)
            ? nullptr
            : ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ? info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetWindowKeepScreenOn", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnSetWindowColorSpace(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnSetWindowColorSpace start...");
    ColorSpace colorSpace = ColorSpace::COLOR_SPACE_DEFAULT;
    if (info.argc < 1) { // 1: params num
        HILOG_ERROR("JsWindow::OnSetWindowColorSpace, argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    uint32_t resultValue = 0;
    if (!ConvertFromJsValue(engine, info.argv[0], resultValue)) {
        HILOG_ERROR("Failed to convert parameter to callbackType");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    colorSpace = static_cast<ColorSpace>(resultValue);
    if (colorSpace > ColorSpace::COLOR_SPACE_WIDE_GAMUT) {
        HILOG_ERROR("ColorSpace %{public}u invalid!", static_cast<uint32_t>(colorSpace));
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    std::weak_ptr<Window> weakToken(windowToken_);
    AsyncTask::CompleteCallback complete = [weakToken, colorSpace](
                                               NativeEngine& engine, AsyncTask& task, int32_t status) {
        auto weakWindow = weakToken.lock();
        if (weakWindow == nullptr) {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY),
                "Invalidate params."));
            return;
        }
        HILOG_INFO("JsWindow::OnSetWindowColorSpace colorSpace..., %{public}d",  colorSpace);
        WmErrorCode ret = WmErrorCode::WM_OK;
        ret = WM_JS_TO_ERROR_CODE_MAP.at(weakWindow->SetColorSpace(colorSpace));
        if (ret == WmErrorCode::WM_OK) {
            task.Resolve(engine, engine.CreateUndefined());
        } else {
            task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window SetColorSpace failed"));
        }
        HILOG_INFO("JsWindow::SetColorSpace : Window [%{public}u, %{public}s] set colorSpace end",
            weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str());
    };

    NativeValue* lastParam =
        (info.argc <= 1)
            ? nullptr
            : ((info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ? info.argv[1] : nullptr);
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindow::OnSetWindowColorSpace", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindow::OnGetWindowColorSpace(NativeEngine& engine, NativeCallbackInfo& info)
{
    std::weak_ptr<Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    if (weakWindow == nullptr) {
        HILOG_ERROR("JsWindow::OnGetWindowColorSpace is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    ColorSpace colorSpace = weakWindow->GetColorSpace();
    HILOG_INFO("JsWindow [%{public}u, %{public}s] OnGetColorSpace end, colorSpace = %{public}u",
        weakWindow->GetWindowId(), weakWindow->GetWindowName().c_str(), static_cast<uint32_t>(colorSpace));

    return CreateJsValue(engine, static_cast<uint32_t>(colorSpace));
}

NativeValue* JsWindow::OnRegisterWindowManagerCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnRegisterWindowManagerCallback : Start...");
    if (info.argc < 2) { // 2: params num
        HILOG_ERROR("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOG_ERROR("Failed to convert parameter to callbackType");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WmErrorCode ret = g_registerManager->RegisterListener(windowToken_, cbType, CaseType::CASE_WINDOW,
        engine, info.argv[1]);
    if (ret != WmErrorCode::WM_OK) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    HILOG_INFO("Register end, type = %{public}s", cbType.c_str());
    return engine.CreateUndefined();
}

NativeValue* JsWindow::OnUnregisterWindowManagerCallback(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindow::OnUnregisterWindowManagerCallback : Start...");
    if (info.argc < 1) {
        HILOG_ERROR("Argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    std::string cbType;
    if (!ConvertFromJsValue(engine, info.argv[0], cbType)) {
        HILOG_ERROR("Failed to convert parameter to callbackType");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    if (cbType.compare("windowEvent") != 0) {
        HILOG_ERROR("JsWindow::OnUnregisterWindowManagerCallback : Envent %{public}s is invalid", cbType.c_str());
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    WmErrorCode ret = WmErrorCode::WM_OK;
    if (info.argc <= 1) {
        g_registerManager->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, nullptr);
    } else {
         NativeValue* value = info.argv[1];
        if (value != nullptr && value->TypeOf() == NATIVE_FUNCTION) {
            g_registerManager->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, value);
        } else {
            g_registerManager->UnregisterListener(windowToken_, cbType, CaseType::CASE_WINDOW, nullptr);
        }
    }
    if (ret != WmErrorCode::WM_OK) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(ret)));
        return engine.CreateUndefined();
    }
    HILOG_INFO("Unregister end, type = %{public}s", cbType.c_str());
    return engine.CreateUndefined();
}

std::shared_ptr<NativeReference> FindJsWindowObject(std::string windowName)
{
    HILOG_INFO("Try to find window %{public}s in g_jsWindowMap", windowName.c_str());
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    if (g_jsWindowMap.find(windowName) == g_jsWindowMap.end()) {
        HILOG_DEBUG("Can not find window %{public}s in g_jsWindowMap", windowName.c_str());
        return nullptr;
    }
    return g_jsWindowMap[windowName];
}

void RemoveJsWindowObject(std::string windowName)
{
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    auto iter = g_jsWindowMap.find(windowName);
    if (iter != g_jsWindowMap.end()) {
        g_jsWindowMap.erase(iter);
    }
}

std::string JsWindow::GetWindowName()
{
    std::weak_ptr<Window> weakToken(windowToken_);
    auto weakWindow = weakToken.lock();
    return (weakWindow == nullptr) ? "" : weakWindow->GetWindowName();
}

void JsWindow::Finalizer(NativeEngine* engine, void* data, void* hint)
{
#ifdef IOS_PLATFORM
    if (g_willTerminate) {
        return;
    }
#endif
    auto jsWin = std::unique_ptr<JsWindow>(static_cast<JsWindow*>(data));
    if (jsWin == nullptr) {
        HILOG_ERROR("jsWin is nullptr");
        return;
    }
    std::string windowName = jsWin->GetWindowName();
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    g_jsWindowMap.erase(windowName);
    HILOG_INFO("Remove window %{public}s from g_jsWindowMap", windowName.c_str());
}

NativeValue* JsWindow::GetUIContext(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("GetUIContext");
    JsWindow* me = CheckParamsAndGetThis<JsWindow>(engine, info);
    return (me != nullptr) ? me->OnGetUIContext(*engine, *info) : nullptr;
}

NativeValue* JsWindow::OnGetUIContext(NativeEngine& engine, NativeCallbackInfo& info)
{
    if (info.argc >= 1) {
        HILOG_ERROR("Argc is invalid: %{public}zu, expect zero params", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    std::weak_ptr<Window> weakToken(windowToken_);
    auto window = weakToken.lock();
    if (window == nullptr) {
        HILOG_ERROR("window is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }

    auto uicontent = window->GetUIContent();
    if (uicontent == nullptr) {
        HILOG_ERROR("uicontent is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }

    NativeValue* uiContext = uicontent->GetUIContext();
    if (uiContext == nullptr) {
        HILOG_ERROR("uiContext obtained from jsEngine is nullptr");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    } else {
        return uiContext;
    }
}

NativeValue* CreateJsWindowObject(NativeEngine& engine, std::shared_ptr<Rosen::Window>& window)
{
    HILOG_INFO("CreateJsWindowObject %p", window.get());
    std::string windowName = window->GetWindowName();
    // avoid repeatedly create js window when getWindow
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        LOGI("FindJsWindowObject %{public}s", windowName.c_str());
        return jsWindowObj->Get();
    }
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsWindow> jsWindow = std::make_unique<JsWindow>(window);
    object->SetNativePointer(jsWindow.release(), JsWindow::Finalizer, nullptr);

    const char* moduleName = "JsWindow";

    BindNativeFunction(engine, *object, "showWindow", moduleName, JsWindow::ShowWindow);
    BindNativeFunction(engine, *object, "destroyWindow", moduleName, JsWindow::DestroyWindow);
    BindNativeFunction(engine, *object, "moveWindowTo", moduleName, JsWindow::MoveWindowTo);
    BindNativeFunction(engine, *object, "resize", moduleName, JsWindow::Resize);
    BindNativeFunction(engine, *object, "getWindowProperties", moduleName, JsWindow::GetWindowPropertiesSync);
    BindNativeFunction(engine, *object, "setWindowSystemBarEnable", moduleName, JsWindow::SetWindowSystemBarEnable);
    BindNativeFunction(engine, *object, "setPreferredOrientation", moduleName, JsWindow::SetPreferredOrientation);
    BindNativeFunction(engine, *object, "loadContent", moduleName, JsWindow::LoadContent);
    BindNativeFunction(engine, *object, "getUIContext", moduleName, JsWindow::GetUIContext);
    BindNativeFunction(engine, *object, "setUIContent", moduleName, JsWindow::SetUIContent);
    BindNativeFunction(engine, *object, "isWindowShowing", moduleName, JsWindow::IsWindowShowingSync);
    BindNativeFunction(engine, *object, "setWindowBackgroundColor", moduleName, JsWindow::SetWindowBackgroundColorSync);
    BindNativeFunction(engine, *object, "setWindowBrightness", moduleName, JsWindow::SetWindowBrightness);
    BindNativeFunction(engine, *object, "setWindowKeepScreenOn", moduleName, JsWindow::SetWindowKeepScreenOn);
    BindNativeFunction(engine, *object, "on", moduleName, JsWindow::RegisterWindowManagerCallback);
    BindNativeFunction(engine, *object, "off", moduleName, JsWindow::UnregisterWindowManagerCallback);
    BindNativeFunction(engine, *object, "setWindowColorSpace", moduleName, JsWindow::SetWindowColorSpace);
    BindNativeFunction(engine, *object, "getWindowColorSpace", moduleName, JsWindow::GetWindowColorSpace);

    std::shared_ptr<NativeReference> jsWindowRef;
    jsWindowRef.reset(engine.CreateReference(objValue, 1));
    std::lock_guard<std::recursive_mutex> lock(g_jsWindowMutex);
    g_jsWindowMap[windowName] = jsWindowRef;
    return objValue;
}
} // namespace Rosen
} // namespace OHOS
