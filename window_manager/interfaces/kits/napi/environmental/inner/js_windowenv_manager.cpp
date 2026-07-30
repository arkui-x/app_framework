/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "js_windowenv_manager.h"

#include "js_window.h"
#include "virtual_rs_window.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_FOUR = 4;
} // namespace

napi_value NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

JsWindowEnvManager::JsWindowEnvManager() {}

JsWindowEnvManager::~JsWindowEnvManager() {}

void JsWindowEnvManager::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("JsWindowEnvManager Finalizer");
    std::unique_ptr<JsWindowEnvManager>(static_cast<JsWindowEnvManager*>(data));
}

napi_value JsWindowEnvManager::FindWindow(napi_env env, napi_callback_info info)
{
    JsWindowEnvManager* me = CheckParamsAndGetThis<JsWindowEnvManager>(env, info);
    return (me != nullptr) ? me->OnFindWindow(env, info) : nullptr;
}

napi_value JsWindowEnvManager::IsIosPlatform(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
#ifdef IOS_PLATFORM
    napi_get_boolean(env, true, &result);
#else
    napi_get_boolean(env, false, &result);
#endif
    return result;
}

napi_value JsWindowEnvManager::OnFindWindow(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
    if (argc < ARGC_ONE) {
        WLOGE("Invalid param size");
        return NapiGetUndefined(env);
    }
    std::string windowName;
    if (!ConvertFromJsValue(env, argv[0], windowName)) {
        WLOGE("Failed to convert windowName");
        return NapiGetUndefined(env);
    }
    std::shared_ptr<Window> window = Window::FindWindow(windowName);
    if (window == nullptr) {
        WLOGE("window null: %{public}s", windowName.c_str());
        return NapiGetUndefined(env);
    }
    return CreateJsWindowObject(env, window);
}

napi_value JsWindowEnvManagerInit(napi_env env, napi_value exportObj)
{
    WLOGD("JsWindowEnvManagerInit");

    if (env == nullptr || exportObj == nullptr) {
        WLOGE("JsWindowEnvManagerInit env or exportObj is nullptr");
        return nullptr;
    }

    std::unique_ptr<JsWindowEnvManager> jsWinEnvManager = std::make_unique<JsWindowEnvManager>();
    napi_wrap(env, exportObj, jsWinEnvManager.release(), JsWindowEnvManager::Finalizer, nullptr, nullptr);

    const char* moduleName = "JsWindowEnvManager";
    BindNativeFunction(env, exportObj, "findWindow", moduleName, JsWindowEnvManager::FindWindow);
    BindNativeFunction(env, exportObj, "isIosPlatform", moduleName, JsWindowEnvManager::IsIosPlatform);
    return exportObj;
}
} // namespace Rosen
} // namespace OHOS
