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

#ifndef OHOS_JS_WINDOW_H
#define OHOS_JS_WINDOW_H
#include <map>
#include <stdbool.h>
#include "js_window_register_manager.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "virtual_rs_window.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsWindowObject(napi_env env, std::shared_ptr<Rosen::Window>& Window);
napi_ref FindJsWindowObject(const std::string &windowName);
void RemoveJsWindowObject(napi_env env, const std::string &windowName);
class JsWindow {
public:
    explicit JsWindow(std::shared_ptr<Rosen::Window>& Window);
    ~JsWindow();
    static napi_value ShowWindow(napi_env env, napi_callback_info info);
    static napi_value DestroyWindow(napi_env env, napi_callback_info info);
    static napi_value MoveWindowTo(napi_env env, napi_callback_info info);
    static napi_value Resize(napi_env env, napi_callback_info info);
    static napi_value GetWindowPropertiesSync(napi_env env, napi_callback_info info);
    static napi_value SetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    static napi_value SetPreferredOrientation(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    static napi_value LoadContentByName(napi_env env, napi_callback_info info);
    static napi_value SetUIContent(napi_env env, napi_callback_info info);
    static napi_value IsWindowShowingSync(napi_env env, napi_callback_info info);
    static napi_value SetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    static napi_value SetWindowBrightness(napi_env env, napi_callback_info info);
    static napi_value SetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    static napi_value SetWindowPrivacyMode(napi_env env, napi_callback_info info);
    static napi_value RegisterWindowManagerCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterWindowManagerCallback(napi_env env, napi_callback_info info);
    static napi_value SetWindowColorSpace(napi_env env, napi_callback_info info);
    static napi_value GetWindowColorSpace(napi_env env, napi_callback_info info);
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetUIContext(napi_env env, napi_callback_info info);
    static napi_value SetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    static napi_value SetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    static napi_value GetWindowAvoidAreaSync(napi_env env, napi_callback_info info);
    static napi_value SetWindowFocusable(napi_env env, napi_callback_info info);
    static napi_value SetWindowSystemBarProperties(napi_env env, napi_callback_info info);

private:
    napi_value OnShowWindow(napi_env env, napi_callback_info info);
    napi_value OnDestroyWindow(napi_env env, napi_callback_info info);
    napi_value OnMoveWindowTo(napi_env env, napi_callback_info info);
    napi_value OnResize(napi_env env, napi_callback_info info);
    napi_value OnGetWindowPropertiesSync(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSystemBarEnable(napi_env env, napi_callback_info info);
    napi_value OnSetPreferredOrientation(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value OnSetUIContent(napi_env env, napi_callback_info info);
    napi_value OnIsWindowShowingSync(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBackgroundColorSync(napi_env env, napi_callback_info info);
    napi_value OnSetWindowBrightness(napi_env env, napi_callback_info info);
    napi_value OnSetWindowKeepScreenOn(napi_env env, napi_callback_info info);
    napi_value OnSetWindowPrivacyMode(napi_env env, napi_callback_info info);
    napi_value OnGetUIContext(napi_env env, napi_callback_info info);
    napi_value OnRegisterWindowManagerCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterWindowManagerCallback(napi_env env, napi_callback_info info);
    napi_value OnSetWindowColorSpace(napi_env env, napi_callback_info info);
    napi_value OnGetWindowColorSpace(napi_env env, napi_callback_info info);
    napi_value OnSetSpecificSystemBarEnabled(napi_env env, napi_callback_info info);
    napi_value OnSetWindowLayoutFullScreen(napi_env env, napi_callback_info info);
    napi_value OnGetWindowAvoidAreaSync(napi_env env, napi_callback_info info);
    napi_value OnSetWindowFocusable(napi_env env, napi_callback_info info);
    napi_value OnSetWindowSystemBarProperties(napi_env env, napi_callback_info info);

    std::string GetWindowName();
    std::shared_ptr<Rosen::Window> windowToken_;
    std::unique_ptr<JsWindowRegisterManager> registerManager_ = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_WINDOW_H
