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
#include "js_window_manager.h"

#include <ability.h>

#include "hilog.h"
#include "js_window.h"
#include "js_window_utils.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

JsWindowManager::JsWindowManager() {}

JsWindowManager::~JsWindowManager() {}

static void GetNativeContext(NativeEngine& engine, NativeValue* nativeContext, void*& contextPtr, WMError& errCode)
{
    if (nativeContext != nullptr) {
        auto objContext = AbilityRuntime::ConvertNativeValueTo<NativeObject>(nativeContext);
        if (objContext == nullptr) {
            HILOG_ERROR("GetNativeContext : ConvertNativeValueTo Context Object failed");
            errCode = WMError::WM_ERROR_INVALID_PARAM;
            return;
        }
        contextPtr = objContext->GetNativePointer();
    }
}

static void CreateSubWindowTask(
    void* contextPtr, std::shared_ptr<WindowOption> windowOption, NativeEngine& engine, AsyncTask& task)
{
    if (windowOption == nullptr) {
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_SYSTEM_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "New window option failed"));
        HILOG_ERROR("CreateSubWindowTask : New window option failed");
        return;
    }
    windowOption->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    if (windowOption->GetParentId() == INVALID_WINDOW_ID) {
        HILOG_ERROR("CreateSubWindowTask : subwindow option parentid error");
        return;
    }
    auto context = static_cast<std::weak_ptr<OHOS::AbilityRuntime::Platform::AbilityContext>*>(contextPtr);
    std::shared_ptr<Window> window = Window::CreateSubWindow(context->lock(), windowOption);
    if (window != nullptr) {
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    } else {
        HILOG_ERROR("CreateSubWindowTask : Create window failed");
        int32_t err = static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY);
        task.Reject(engine, CreateJsError(engine, err, "Create window failed"));
    }
}

void JsWindowManager::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsWindowManager::Finalizer");
    std::unique_ptr<JsWindowManager>(static_cast<JsWindowManager*>(data));
}

NativeValue* JsWindowManager::CreateWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowManager::CreateWindow");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnCreateWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::FindWindowSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowManager::FindWindowSync");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnFindWindowSync(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::GetLastWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowManager::GetLastWindow");
    JsWindowManager* me = CheckParamsAndGetThis<JsWindowManager>(engine, info);
    return (me != nullptr) ? me->OnGetLastWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowManager::OnCreateWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("JsWindowManager::OnCreateWindow : Start...");
    if (info.argc < 1) {
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(info.argv[0]);
    if (nativeObj == nullptr) {
        HILOG_ERROR("JsWindowManager::OnCreateWindow : Failed to convert object to CreateWindow");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    WindowOption option;
    void* contextPtr = nullptr;
    if (!ParseConfigOption(engine, nativeObj, option, contextPtr)) {
        HILOG_ERROR("JsWindowManager::OnCreateWindow : Failed to parse config");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    if (option.GetWindowType() != WindowType::WINDOW_TYPE_APP_SUB_WINDOW) {
        HILOG_ERROR("JsWindowManager::OnCreateWindow : Invalid Window Type!");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_NOT_SYSTEM_APP)));
        return engine.CreateUndefined();
    }
    NativeValue* callback = nullptr;
    if (info.argc > 1) {
        callback = info.argv[1]->TypeOf() == NATIVE_FUNCTION ? info.argv[1] : nullptr; // 1: index of callback
    }
    AsyncTask::CompleteCallback complete = [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
        std::shared_ptr<WindowOption> windowOption = std::make_shared<WindowOption>(option);
        return CreateSubWindowTask(contextPtr, windowOption, engine, task);
    };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnCreateWindow", engine,
        CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

bool JsWindowManager::ParseConfigOption(
    NativeEngine& engine, NativeObject* jsObject, WindowOption& option, void*& contextPtr)
{
    std::string windowName;
    if (ParseJsValue(jsObject, engine, "name", windowName)) {
        option.SetWindowName(windowName);
    } else {
        HILOG_ERROR("JsWindowManager::ParseConfigOption : Failed to convert parameter to windowName");
        return false;
    }

    uint32_t winType;
    if (ParseJsValue(jsObject, engine, "windowType", winType)) {
        if (winType >= static_cast<uint32_t>(ApiWindowType::TYPE_BASE) &&
            winType < static_cast<uint32_t>(ApiWindowType::TYPE_END)) {
            option.SetWindowType(JS_TO_NATIVE_WINDOW_TYPE_MAP.at(static_cast<ApiWindowType>(winType)));
        } else {
            option.SetWindowType(static_cast<WindowType>(winType));
        }
    } else {
        HILOG_ERROR("JsWindowManager::ParseConfigOption : Failed to convert parameter to winType");
        return false;
    }

    NativeValue* value = jsObject->GetProperty("ctx");
    if (value->TypeOf() == NATIVE_UNDEFINED) {
        return true;
    }
    WMError errCode = WMError::WM_OK;
    GetNativeContext(engine, value, contextPtr, errCode);
    if (errCode != WMError::WM_OK) {
        HILOG_ERROR("JsWindowManager::ParseConfigOption : GetNativeContext Failed!");
        return false;
    }

    int64_t parentId = -1;
    if (ParseJsValue(jsObject, engine, "parentId", parentId)) {
        option.SetParentId(parentId);
    }

    return true;
}

NativeValue* JsWindowManager::OnFindWindowSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_DEBUG("JsWindowManager::OnFindWindowSync : Start...");
    std::string windowName;
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1) { // 1: params num
        HILOG_ERROR("JsWindowManager::OnFindWindowSync : Argc is invalid: %{public}zu", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    } else {
        if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
            HILOG_ERROR("JsWindowManager::OnFindWindowSync : Failed to convert parameter to windowName");
            errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
        }
    }
    if (errCode != WmErrorCode::WM_OK) {
        HILOG_ERROR("JsWindowManager::OnFindWindowSync : errCode = %{public}d", errCode);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    }

    HILOG_INFO(
        "JsWindowManager::OnFindWindowSync : Window name = %{public}s, err = %{public}d", windowName.c_str(), errCode);
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        HILOG_INFO(
            "JsWindowManager::OnFindWindowSync : Find window: %{public}s, use exist js window", windowName.c_str());
        return jsWindowObj->Get();
    } else {
        HILOG_INFO("JsWindowManager::OnFindWindowSync : could not find window from js window, call Window::FindWindow");
        std::shared_ptr<Window> window = Window::FindWindow(windowName);
        if (window == nullptr) {
            HILOG_ERROR("JsWindowManager::OnFindWindowSync : call Window::FindWindow return failed!");
            engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
            return engine.CreateUndefined();
        } else {
            HILOG_ERROR("JsWindowManager::OnFindWindowSync : call Window::FindWindow return success!");
            return CreateJsWindowObject(engine, window);
        }
    }
}

static void GetTopWindowTask(void* contextPtr, NativeEngine& engine, AsyncTask& task, bool newApi)
{
    HILOG_INFO("WindowManager:GetTopWindowTask : Start...");
    std::string windowName;
    std::shared_ptr<Rosen::Window> window = nullptr;
    auto context = static_cast<std::weak_ptr<AbilityRuntime::Platform::Context>*>(contextPtr);
    if (contextPtr == nullptr || context == nullptr) {
        HILOG_ERROR("WindowManager:GetTopWindowTask: Stage mode without context");
    }

    window = Window::GetTopWindow(context == nullptr ? nullptr : context->lock());
    HILOG_INFO("WindowManager:GetTopWindowTask: GetTopWindow[%{public}p]", window.get());
    if (window == nullptr) {
        task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(-1), "Get top window failed"));
        HILOG_ERROR("WindowManager:GetTopWindowTask : Get top window failed");
        return;
    }
    windowName = window->GetWindowName();
    HILOG_ERROR("WindowManager:GetTopWindowTask : GetWindowName[%{public}s]", windowName.c_str());
    std::shared_ptr<NativeReference> jsWindowObj = FindJsWindowObject(windowName);
    if (jsWindowObj != nullptr && jsWindowObj->Get() != nullptr) {
        HILOG_INFO("WindowManager:GetTopWindowTask : FindJsWIndowObject is not null");
        task.Resolve(engine, jsWindowObj->Get());
    } else {
        HILOG_INFO("WindowManager:GetTopWindowTask : FindJsWIndowObject is null");
        task.Resolve(engine, CreateJsWindowObject(engine, window));
    }
    HILOG_INFO("WindowManager:GetTopWindowTask : Get top window %{public}s success", windowName.c_str());
    return;
}

NativeValue* JsWindowManager::OnGetLastWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowManager::OnGetLastWindow : Start...");
    WMError errCode = WMError::WM_OK;
    NativeValue* nativeContext = nullptr;
    NativeValue* nativeCallback = nullptr;
    void* contextPtr = nullptr;
    HILOG_INFO("JsWindowManager::OnGetLastWindow : check parameter, info.argc = %{public}zu", info.argc);
    if (info.argc < 1 || info.argc > 2) {
        HILOG_ERROR("JsWindowManager::OnGetLastWindow : info.argc error[%{public}zu]", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(errCode)));
        return engine.CreateUndefined();
    } else {
        nativeContext = info.argv[0];
        nativeCallback = (info.argc == 1) ? nullptr : info.argv[1];
        GetNativeContext(engine, nativeContext, contextPtr, errCode);
    }
    if (errCode != WMError::WM_OK) {
        HILOG_INFO("JsWindowManager::OnGetLastWindow : errorCode != 0");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(-1)));
        return engine.CreateUndefined();
    }
    HILOG_INFO("JsWindowManager::OnGetLastWindow : processing...");
    AsyncTask::CompleteCallback complete = [=](NativeEngine& engine, AsyncTask& task, int32_t status) {
        return GetTopWindowTask(contextPtr, engine, task, true);
    };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowManager::OnGetTopWindow", engine,
        CreateAsyncTaskWithLastParam(engine, nativeCallback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowManagerInit(NativeEngine* engine, NativeValue* exportObj)
{
    HILOG_INFO("JsWindowManagerInit");

    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("JsWindowManagerInit : engine or exportObj is nullptr");
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("JsWindowManagerInit : object is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowManager> jsWinManager = std::make_unique<JsWindowManager>();
    object->SetNativePointer(jsWinManager.release(), JsWindowManager::Finalizer, nullptr);
    object->SetProperty("WindowType", WindowTypeInit(engine));
    object->SetProperty("WindowMode", WindowModeInit(engine));
    object->SetProperty("Orientation", OrientationInit(engine));
    object->SetProperty("WindowStageEventType", WindowStageEventTypeInit(engine));
    object->SetProperty("WmErrorCode", WindowErrorCodeInit(engine));
    object->SetProperty("WMError", WindowErrorInit(engine));
    const char* moduleName = "JsWindowManager";
    BindNativeFunction(*engine, *object, "createWindow", moduleName, JsWindowManager::CreateWindow);
    BindNativeFunction(*engine, *object, "findWindow", moduleName, JsWindowManager::FindWindowSync);
    BindNativeFunction(*engine, *object, "getLastWindow", moduleName, JsWindowManager::GetLastWindow);
    return engine->CreateUndefined();
}
} // namespace Rosen
} // namespace OHOS
