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

#include <string>
#include "js_runtime_utils.h"
#include "virtual_rs_window.h"
#include "js_window.h"
#include "js_window_register_manager.h"
#include "js_window_stage.h"
#include "hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

std::unique_ptr<JsWindowRegisterManager> g_listenerManager = std::make_unique<JsWindowRegisterManager>();

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    std::shared_ptr<Window> weakWindow, NativeEngine& engine, AsyncTask& task)
{
    NativeValue* nativeStorage =  (contentStorage == nullptr) ? nullptr : contentStorage->Get();
    int ret = weakWindow->SetUIContent(contextUrl, &engine, nativeStorage, false, nullptr);
    if (ret == 0) {
        task.Resolve(engine, engine.CreateUndefined());
    } else {
        task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window load content failed"));
    }
}

JsWindowStage::JsWindowStage(const std::shared_ptr<Rosen::WindowStage>& windowStage)
    : windowStage_(windowStage)
{
}

JsWindowStage::~JsWindowStage()
{
}

NativeValue* JsWindowStage::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::LoadContent");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnLoadContent : Start... / info.argc[%{public}zu]", info.argc);
    WmErrorCode errCode = WmErrorCode::WM_OK;
    if (info.argc < 1 || info.argc > 3) {
        HILOG_ERROR("JsWindowStage::OnLoadContent : info.argc error![%{public}zu]", info.argc);
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    std::string contextUrl;
    if (!ConvertFromJsValue(engine, info.argv[0], contextUrl)) {
        HILOG_ERROR("JsWindowStage::OnLoadContent : ConvertFromJsValue error!");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    
    NativeValue* storage = nullptr;
    NativeValue* callback = nullptr;
    NativeValue* value1 = info.argv[1];
    NativeValue* value2 = info.argv[2]; // 2: param index
    if (value1->TypeOf() == NATIVE_FUNCTION) {
        callback = value1;
    } else if (value1->TypeOf() == NATIVE_OBJECT) {
        storage = value1;
    }
    if (value2->TypeOf() == NATIVE_FUNCTION) {
        callback = value2;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        HILOG_ERROR("JsWindowStage::OnLoadContent : Window scene is null or get invalid param");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    std::shared_ptr<NativeReference> contentStorage = (storage == nullptr) ? nullptr :
        std::shared_ptr<NativeReference>(engine.CreateReference(storage, 1));
    AsyncTask::CompleteCallback complete =
        [weak = windowStage_, contentStorage, contextUrl](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto weakStage = weak.lock();
            if (weakStage == nullptr) {
                task.Reject(engine, CreateJsError(engine, -1));
                return;
            }
            auto win = weakStage->GetMainWindow();
            if (win == nullptr) {
                task.Reject(engine, CreateJsError(engine, -1));
                return;
            }
            LoadContentTask(contentStorage, contextUrl, win, engine, task);
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowStage::OnLoadContent",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowStage::GetMainWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::GetMainWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnGetMainWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OnGetMainWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnGetMainWindow : Start... / info.argc = %{public}d", info.argc);
    WMError errCode = WMError::WM_OK;
    NativeValue* callback = nullptr;
    if (info.argc > 1) {
        HILOG_INFO("JsWindowStage::OnGetMainWindow : info.argc > 1");
        errCode = WMError::WM_ERROR_INVALID_PARAM;
    } else {
        if (info.argc == 1 && info.argv[0] != nullptr && info.argv[0]->TypeOf() == NATIVE_FUNCTION) {
            callback = info.argv[0];
        }
    }
    if (errCode != WMError::WM_OK) {
        HILOG_INFO("JsWindowStage::OnGetMainWindow : errorCode != 0");
        engine.Throw(CreateJsError(engine, -1));
        return engine.CreateUndefined();
    }
    HILOG_INFO("JsWindowStage::OnGetMainWindow : processing...");
    AsyncTask::CompleteCallback complete =
        [weak = windowStage_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto weakStage = weak.lock();
            if (weakStage == nullptr) {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(-1)));
                HILOG_ERROR("JsWindowStage::OnGetMainWindow : windowStage_ is nullptr!");
                return;
            }
            auto window = weakStage->GetMainWindow();
            if (window != nullptr) {
                HILOG_ERROR("JsWindowStage::OnGetMainWindow : Get main window windowId=%{public}u, " \
                    "windowName=%{public}s", window->GetWindowId(), window->GetWindowName().c_str());
                task.Resolve(engine, OHOS::Rosen::CreateJsWindowObject(engine, window));
            } else {
                task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(-1), "Get main window failed."));
            }
        };

    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowStage::OnGetMainWindow",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

NativeValue* JsWindowStage::GetMainWindowSync(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::GetMainWindowSync");
   
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnGetMainWindowSync(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OnGetMainWindowSync(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnGetMainWindowSync : Start...");
    auto weakWindowStage = windowStage_.lock();
    auto win = weakWindowStage->GetMainWindow();
    return (win != nullptr) ? CreateJsWindowObject(engine, win) : nullptr;
}

NativeValue* JsWindowStage::On(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::On");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnEvent(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OnEvent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnEvent : Start...");
    auto weakStage = windowStage_.lock();
    if (weakStage == nullptr) {
        HILOG_ERROR("JsWindowStage::OnEvent : Window stage is null");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    if (info.argc != 2) {
        HILOG_ERROR("JsWindowStage::OnEvent : argc is invalid: %{public}zu", info.argc);
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(engine, info.argv[0], eventString)) {
        HILOG_ERROR("JsWindowStage::OnEvent : Failed to convert parameter to string");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    NativeValue* value = info.argv[1];
    if (!value->IsCallable()) {
        HILOG_ERROR("JsWindowStage::OnEvent : Callback(info->argv[1]) is not callable");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    auto window = weakStage->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("JsWindowStage::OnEvent : Get window failed");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    g_listenerManager->RegisterListener(window, eventString, CaseType::CASE_STAGE, engine, value);
    HILOG_INFO("JsWindowStage::OnEvent : Window [%{public}u, %{public}s] register event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return engine.CreateUndefined();
}

NativeValue* JsWindowStage::Off(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::Off");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OffEvent(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OffEvent(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OffEvent : Start...");
    auto weakStage = windowStage_.lock();
    if (weakStage == nullptr) {
        HILOG_ERROR("[NAPI]Window scene is null");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }

    // Parse info->argv[0] as string
    std::string eventString;
    if (!ConvertFromJsValue(engine, info.argv[0], eventString)) {
        HILOG_ERROR("JsWindowStage::OffEvent : Failed to convert parameter to string");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    if (eventString.compare("windowStageEvent") != 0) {
        HILOG_ERROR("JsWindowStage::OffEvent : Envent %{public}s is invalid", eventString.c_str());
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }

    auto window = weakStage->GetMainWindow();
    if (window == nullptr) {
        HILOG_ERROR("JsWindowStage::OffEvent : Get window failed");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
        return engine.CreateUndefined();
    }
    NativeValue* value = nullptr;
    if (info.argv[1] == nullptr || info.argv[1]->TypeOf() != NATIVE_FUNCTION) {
        g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, nullptr);
    } else {
        g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, info.argv[1]);
    }

    if (info.argc == 1) {
        g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, nullptr);
    } else {
        value = info.argv[1];
        if (value != nullptr && value->TypeOf() == NATIVE_FUNCTION) {
            g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, value);
        } else {
            g_listenerManager->UnregisterListener(window, eventString, CaseType::CASE_STAGE, nullptr);
        }
    }
    HILOG_INFO("JsWindowStage::OffEvent : Window [%{public}u, %{public}s] unregister event %{public}s",
        window->GetWindowId(), window->GetWindowName().c_str(), eventString.c_str());

    return engine.CreateUndefined();
}

NativeValue* JsWindowStage::CreateSubWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::CreateSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnCreateSubWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::GetSubWindow(NativeEngine* engine, NativeCallbackInfo* info)
{
    HILOG_INFO("JsWindowStage::GetSubWindow");
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnGetSubWindow(*engine, *info) : nullptr;
}

NativeValue* JsWindowStage::OnCreateSubWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnCreateSubWindow : Start...");
    WmErrorCode errCode = WmErrorCode::WM_OK;
    std::string windowName;
    if (!ConvertFromJsValue(engine, info.argv[0], windowName)) {
        HILOG_ERROR("JsWindowStage::OnCreateSubWindow : Failed to convert parameter to windowName");
        errCode = WmErrorCode::WM_ERROR_INVALID_PARAM;
    }
    if (errCode == WmErrorCode::WM_ERROR_INVALID_PARAM) {
        HILOG_ERROR("JsWindowStage::OnCreateSubWindow : get invalid param");
        engine.Throw(CreateJsError(engine, static_cast<int32_t>(WmErrorCode::WM_ERROR_INVALID_PARAM)));
        return engine.CreateUndefined();
    }
    AsyncTask::CompleteCallback complete =
        [weak = windowStage_, windowName](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto weakWindowStage = weak.lock();
            if (weakWindowStage == nullptr) {
                HILOG_ERROR("JsWindowStage::OnCreateSubWindow : Window scene is null");
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
                return;
            }
            auto window = weakWindowStage->CreateSubWindow(windowName);
            if (window == nullptr) {
                HILOG_ERROR("JsWindowStage::OnCreateSubWindow : Get window failed");
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY), "Get window failed"));
                return;
            }
            task.Resolve(engine, CreateJsWindowObject(engine, window));
            HILOG_INFO("JsWindowStage::OnCreateSubWindow : Create sub widdow %{public}s end", windowName.c_str());
        };
    NativeValue* callback = (info.argv[1] != nullptr && info.argv[1]->TypeOf() == NATIVE_FUNCTION) ?
        info.argv[1] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowStage::OnCreateSubWindow",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

static NativeValue* CreateJsSubWindowArrayObject(NativeEngine& engine,
    std::vector<std::shared_ptr<Window>> subWinVec)
{
    NativeValue* objValue = engine.CreateArray(subWinVec.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(objValue);
    if (array == nullptr) {
        HILOG_ERROR("CreateJsSubWindowArrayObject : Failed to convert subWinVec to jsArrayObject");
        return nullptr;
    }
    uint32_t index = 0;
    for (size_t i = 0; i < subWinVec.size(); i++) {
        array->SetElement(index++, CreateJsWindowObject(engine, subWinVec[i]));
    }
    return objValue;
}

NativeValue* JsWindowStage::OnGetSubWindow(NativeEngine& engine, NativeCallbackInfo& info)
{
    HILOG_INFO("JsWindowStage::OnGetSubWindow : Start...");
    AsyncTask::CompleteCallback complete =
        [weak = windowStage_](NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto weakWindowStage = weak.lock();
            if (weakWindowStage == nullptr) {
                HILOG_ERROR("JsWindowStage::OnGetSubWindow : Window scene is nullptr");
                task.Reject(engine, CreateJsError(engine,
                    static_cast<int32_t>(WmErrorCode::WM_ERROR_STATE_ABNORMALLY)));
                return;
            }
            std::vector<std::shared_ptr<Window>> subWindowVec = weakWindowStage->GetSubWindow();
            task.Resolve(engine, CreateJsSubWindowArrayObject(engine, subWindowVec));
            HILOG_INFO("JsWindowStage::OnGetSubWindow : Get sub windows, size = %{public}zu", subWindowVec.size());
        };
    NativeValue* callback = (info.argv[0] != nullptr && info.argv[0]->TypeOf() == NATIVE_FUNCTION) ?
        info.argv[0] : nullptr;
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowStage::OnGetSubWindow",
        engine, CreateAsyncTaskWithLastParam(engine, callback, nullptr, std::move(complete), &result));
    return result;
}

void JsWindowStage::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    std::unique_ptr<JsWindowStage>(static_cast<JsWindowStage*>(data));
}


NativeValue* CreateJsWindowStage(NativeEngine& engine, std::shared_ptr<Rosen::WindowStage> WindowStage)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsWindowStage> jsWindowStage = std::make_unique<JsWindowStage>(WindowStage);
    object->SetNativePointer(jsWindowStage.release(), JsWindowStage::Finalizer, nullptr);

    const char *moduleName = "JsWindowStage";
    BindNativeFunction(engine,
        *object, "loadContent", moduleName, JsWindowStage::LoadContent);
    BindNativeFunction(engine,
        *object, "getMainWindow", moduleName, JsWindowStage::GetMainWindow);
    BindNativeFunction(engine,
        *object, "getMainWindowSync", moduleName, JsWindowStage::GetMainWindowSync);
    BindNativeFunction(engine,
        *object, "createSubWindow", moduleName, JsWindowStage::CreateSubWindow);
    BindNativeFunction(engine,
        *object, "getSubWindow", moduleName, JsWindowStage::GetSubWindow);
    BindNativeFunction(engine, *object, "on", moduleName, JsWindowStage::On);
    BindNativeFunction(engine, *object, "off", moduleName, JsWindowStage::Off);

    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
