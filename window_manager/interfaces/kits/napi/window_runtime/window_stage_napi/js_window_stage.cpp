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

#include "js_window_stage.h"

#include <string>

#include "js_window_utils.h"
#include "js_window.h"
#include "js_window_register_manager.h"
#include "native_engine/native_engine.h"
#include "virtual_rs_window.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

std::unique_ptr<JsWindowRegisterManager> g_listenerManager = std::make_unique<JsWindowRegisterManager>();

JsWindowStage::JsWindowStage(const std::shared_ptr<Rosen::WindowStage>& windowStage) : windowStage_(windowStage) {}

JsWindowStage::~JsWindowStage() {}

napi_value JsWindowStage::LoadContent(napi_env env, napi_callback_info info)
{
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, false) : nullptr;
}

napi_value JsWindowStage::LoadContentByName(napi_env env, napi_callback_info info)
{
    WLOGD("[NAPI]LoadContentByName");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnLoadContent(env, info, true) : nullptr;
}

napi_value JsWindowStage::OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    napi_value storage = nullptr;
    napi_value callback = nullptr;
    std::string contextUrl;
    if (!GetContentArg(env, info, contextUrl, storage, callback)) {
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    napi_ref storageRef = nullptr;
    if ((errCode == WmErrorCode::WM_OK) && (storage != nullptr)) {
        napi_status status = napi_create_reference(env, storage, 1, &storageRef);
        errCode = (status == napi_ok) ? WmErrorCode::WM_OK : WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode != WmErrorCode::WM_OK) {
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    napi_value result = nullptr;
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowStage_, storageRef, contextUrl, isLoadedByName](napi_env env, NapiAsyncTask& task,
            int32_t status) {
            auto weakStage = weak.lock();
            if (weakStage == nullptr || weakStage->GetMainWindow() == nullptr) {
                WLOGE("JsWindowStage::OnLoadContent : no main window");
                task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            LoadContentTask(env, storageRef, contextUrl, weakStage->GetMainWindow(), task, isLoadedByName);
        };
    NapiAsyncTask::Schedule("JsWindowStage::OnLoadContent",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::GetMainWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::GetMainWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindow(env, info) : nullptr;
}

napi_value JsWindowStage::OnGetMainWindow(napi_env env, napi_callback_info info)
{
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    WLOGD("JsWindowStage::OnGetMainWindow : Start... / argc[%{public}zu]", argc);
    if (status != napi_ok || argc > 1) {
        WLOGE("JsWindowStage::OnGetMainWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    WLOGD("JsWindowStage::OnGetMainWindow : processing...");
    napi_value result = nullptr;
    NapiAsyncTask::CompleteCallback complete =
        [weak = windowStage_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakStage = weak.lock();
            if (weakStage == nullptr) {
                task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            auto window = weakStage->GetMainWindow();
            if (window != nullptr) {
                WLOGD("JsWindowStage::OnGetMainWindow : Get main window windowId=%{public}u, "
                            "windowName=%{public}s",
                    window->GetWindowId(), window->GetWindowName().c_str());
                task.Resolve(env, OHOS::Rosen::CreateJsWindowObject(env, window));
            } else {
                task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY,
                    "Get main window failed."));
            }
        };
    napi_value callback = nullptr;
    if (argc > 0 && IsFunction(env, argv[0])) {
        callback = argv[0];
    }
    NapiAsyncTask::Schedule("JsWindowStage::OnGetMainWindow",
        env, CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

napi_value JsWindowStage::GetMainWindowSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::GetMainWindowSync");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetMainWindowSync(env, info) : nullptr;
}

napi_value JsWindowStage::OnGetMainWindowSync(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::OnGetMainWindowSync : Start...");
    auto weakWindowStage = windowStage_.lock();
    auto win = weakWindowStage->GetMainWindow();
    return (win != nullptr) ? CreateJsWindowObject(env, win) : nullptr;
}

napi_value JsWindowStage::On(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::On");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnEvent(env, info) : nullptr;
}

napi_value JsWindowStage::OnEvent(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::OnEvent : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc != 2) {
        WLOGE("JsWindowStage::OnEvent : argc error![%{public}zu]", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    auto weakStage = windowStage_.lock();
    if (weakStage == nullptr) {
        WLOGE("JsWindowStage::OnEvent : Window stage is null");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString)) {
        WLOGE("JsWindowStage::OnEvent : Failed to convert parameter to string");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }
    napi_valuetype type;
    napi_typeof(env, argv[1], &type);
    if (type != napi_function) {
        WLOGE("JsWindowStage::OnEvent : Callback(info->argv[1]) is not callable");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    auto window = weakStage->GetMainWindow();
    if (window == nullptr) {
        WLOGE("JsWindowStage::OnEvent : Get window failed");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }
    g_listenerManager->RegisterListener(env, window, eventString, CaseType::CASE_STAGE, argv[1]);
    WLOGD("JsWindowStage::OnEvent : Window [%{public}u, %{public}s] register event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return CreateUndefined(env);
}

napi_value JsWindowStage::Off(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::Off");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OffEvent(env, info) : nullptr;
}

napi_value JsWindowStage::OffEvent(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::OffEvent : Start...");
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindowStage::OffEvent : argc error![%{public}zu]", argc);
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    auto weakStage = windowStage_.lock();
    if (weakStage == nullptr || weakStage->GetMainWindow() == nullptr) {
        WLOGE("[NAPI]Window scene is null");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
        return CreateUndefined(env);
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(env, argv[0], eventString) || eventString.compare("windowStageEvent") != 0) {
        WLOGE("JsWindowStage::OffEvent : Failed to convert parameter to string");
        napi_throw(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_INVALID_PARAM));
        return CreateUndefined(env);
    }

    auto window = weakStage->GetMainWindow();
    if (argc == 1 || argv[1] == nullptr) {
        g_listenerManager->UnregisterListener(env, window, eventString, CaseType::CASE_STAGE, nullptr);
    } else {
        napi_valuetype type;
        napi_typeof(env, argv[1], &type);
        if (type != napi_function) {
            g_listenerManager->UnregisterListener(env, window, eventString, CaseType::CASE_STAGE, nullptr);
        } else {
            g_listenerManager->UnregisterListener(env, window, eventString, CaseType::CASE_STAGE, argv[1]);
        }
    }

    WLOGD("JsWindowStage::OffEvent : Window [%{public}u, %{public}s] unregister event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());
    return CreateUndefined(env);
}

napi_value JsWindowStage::CreateSubWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::CreateSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnCreateSubWindow(env, info) : nullptr;
}

napi_value JsWindowStage::GetSubWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::GetSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(env, info);
    return (me != nullptr) ? me->OnGetSubWindow(env, info) : nullptr;
}

napi_value JsWindowStage::OnCreateSubWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::OnCreateSubWindow : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok || argc < 1) {
        WLOGE("JsWindowStage::OnCreateSubWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGE("JsWindowStage::OnCreateSubWindow : Failed to convert parameter to windowName");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = windowStage_, windowName](napi_env env, NapiAsyncTask& task, int32_t status) {
        auto weakWindowStage = weak.lock();
        if (weakWindowStage == nullptr) {
            WLOGE("JsWindowStage::OnCreateSubWindow : Window scene is null");
            task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
            return;
        }
        auto window = weakWindowStage->CreateSubWindow(windowName);
        if (window == nullptr) {
            WLOGE("JsWindowStage::OnCreateSubWindow : create sub window failed");
            task.Reject(env, CreateWindowsJsError(env,
                WmErrorCode::WM_ERROR_STATE_ABNORMALLY, "Get window failed"));
            return;
        }
        task.Resolve(env, CreateJsWindowObject(env, window));
        WLOGD("JsWindowStage::OnCreateSubWindow : Create sub window %{public}s end", windowName.c_str());
    };
    napi_value callback = nullptr;
    if (argc > 1 && IsFunction(env, argv[1])) {
        callback = argv[1];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnCreateSubWindow", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

static napi_value CreateJsSubWindowArrayObject(napi_env env, std::vector<std::shared_ptr<Window>> subWinVec)
{
    napi_value objValue = nullptr;
    napi_status status = napi_create_array_with_length(env, subWinVec.size(), &objValue);
    if (status != napi_ok) {
        return nullptr;
    }
    for (size_t i = 0; i < subWinVec.size(); i++) {
        napi_set_element(env, objValue, i, CreateJsWindowObject(env, subWinVec[i]));
    }
    return objValue;
}

napi_value JsWindowStage::OnGetSubWindow(napi_env env, napi_callback_info info)
{
    WLOGD("JsWindowStage::OnGetSubWindow : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    size_t argc = WINDOW_ARGC_MAX_COUNT;
    napi_value argv[WINDOW_ARGC_MAX_COUNT] = { nullptr };
    napi_status status = napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (status != napi_ok) {
        WLOGE("JsWindowStage::OnCreateSubWindow : argc error![%{public}zu]", argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        napi_throw(env, CreateWindowsJsError(env, errCode, "Invalid params."));
        return CreateJsUndefined(env);
    }

    NapiAsyncTask::CompleteCallback complete =
        [weak = windowStage_](napi_env env, NapiAsyncTask& task, int32_t status) {
            auto weakWindowStage = weak.lock();
            if (weakWindowStage == nullptr) {
                WLOGE("JsWindowStage::OnGetSubWindow : Window scene is nullptr");
                task.Reject(env, CreateWindowsJsError(env, WmErrorCode::WM_ERROR_STATE_ABNORMALLY));
                return;
            }
            std::vector<std::shared_ptr<Window>> subWindowVec = weakWindowStage->GetSubWindow();
            task.Resolve(env, CreateJsSubWindowArrayObject(env, subWindowVec));
            WLOGD("JsWindowStage::OnGetSubWindow : Get sub windows, size = %{public}zu", subWindowVec.size());
    };
    napi_value callback = nullptr;
    if (argc >= 1 && IsFunction(env, argv[0])) {
        callback = argv[0];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsWindowStage::OnGetSubWindow", env,
        CreateAsyncTaskWithLastParam(env, callback, nullptr, std::move(complete), &result));
    return result;
}

void JsWindowStage::Finalizer(napi_env env, void* data, void* hint)
{
    std::unique_ptr<JsWindowStage>(static_cast<JsWindowStage*>(data));
}

napi_value CreateJsWindowStage(napi_env env, std::shared_ptr<Rosen::WindowStage> WindowStage)
{
    WLOGD("CreateJsWindowStage");
    const napi_property_descriptor props[] = {
        DECLARE_NAPI_FUNCTION("loadContent", JsWindowStage::LoadContent),
        DECLARE_NAPI_FUNCTION("loadContentByName", JsWindowStage::LoadContentByName),
        DECLARE_NAPI_FUNCTION("getMainWindow", JsWindowStage::GetMainWindow),
        DECLARE_NAPI_FUNCTION("getMainWindowSync", JsWindowStage::GetMainWindowSync),
        DECLARE_NAPI_FUNCTION("createSubWindow", JsWindowStage::CreateSubWindow),
        DECLARE_NAPI_FUNCTION("getSubWindow", JsWindowStage::GetSubWindow),
        DECLARE_NAPI_FUNCTION("on", JsWindowStage::On),
        DECLARE_NAPI_FUNCTION("off", JsWindowStage::Off)
    };
    std::unique_ptr<JsWindowStage> jsWindowStage = std::make_unique<JsWindowStage>(WindowStage);
    napi_value object = CreateObject(env, "JsWindowStage", props, sizeof(props) / sizeof(props[0]));
    if (object != nullptr) {
        napi_wrap(env, object, jsWindowStage.release(), JsWindowStage::Finalizer, nullptr, nullptr);
    }
    return object;
}
} // namespace Rosen
} // namespace OHOS
