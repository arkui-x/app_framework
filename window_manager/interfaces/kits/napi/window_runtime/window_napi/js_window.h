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

#ifndef OHOS_JS_WINDOW_H
#define OHOS_JS_WINDOW_H
#include <map>
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "virtual_rs_window.h"

namespace OHOS {
namespace Rosen {
NativeValue* CreateJsWindowObject(NativeEngine& engine, std::shared_ptr<Rosen::Window>& Window);
std::shared_ptr<NativeReference> FindJsWindowObject(std::string windowName);
void RemoveJsWindowObject(std::string windowName);
class JsWindow {
public:
    explicit JsWindow(std::shared_ptr<Rosen::Window>& Window);
    ~JsWindow();
    static NativeValue* ShowWindow(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* DestroyWindow(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* MoveWindowTo(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* Resize(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetWindowPropertiesSync(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowSystemBarEnable(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetPreferredOrientation(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* LoadContent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetUIContent(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* IsWindowShowingSync(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowBackgroundColorSync(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowBrightness(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* SetWindowKeepScreenOn(NativeEngine* engine, NativeCallbackInfo* info);
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* GetUIContext(NativeEngine* engine, NativeCallbackInfo* info);

private:
    NativeValue* OnShowWindow(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnDestroyWindow(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnMoveWindowTo(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnResize(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetWindowPropertiesSync(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetWindowSystemBarEnable(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetPreferredOrientation(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnLoadContent(NativeEngine& engine, NativeCallbackInfo&info);
    NativeValue* OnSetUIContent(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnIsWindowShowingSync(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetWindowBackgroundColorSync(NativeEngine& engine, NativeCallbackInfo&info);
    NativeValue* OnSetWindowBrightness(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnSetWindowKeepScreenOn(NativeEngine& engine, NativeCallbackInfo& info);
    NativeValue* OnGetUIContext(NativeEngine& engine, NativeCallbackInfo& info);

    std::string GetWindowName();

    std::shared_ptr<Rosen::Window> windowToken_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_WINDOW_H
