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

#ifndef OHOS_JS_WINDOW_MANAGER_H
#define OHOS_JS_WINDOW_MANAGER_H

#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"

namespace OHOS {
namespace Rosen {
class WindowOption;    
NativeValue* JsWindowManagerInit(NativeEngine* engine, NativeValue* exportObj);
class JsWindowManager {
public:
    JsWindowManager();
    ~JsWindowManager();
    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* CreateWindow(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* FindWindowSync(NativeEngine* engine, NativeCallbackInfo* info);
    static NativeValue* GetLastWindow(NativeEngine* engine, NativeCallbackInfo* info);
private:
    static NativeValue* OnCreateWindow(NativeEngine& engine, NativeCallbackInfo& info);
    static NativeValue* OnFindWindowSync(NativeEngine& engine, NativeCallbackInfo& info);
    static NativeValue* OnGetLastWindow(NativeEngine& engine, NativeCallbackInfo& info);
    static bool ParseConfigOption(
        NativeEngine& engine, NativeObject* jsObject, WindowOption& option, void*& contextPtr);
};
}  // namespace Rosen
}  // namespace OHOS
#endif // OHOS_JS_WINDOW_MANAGER_H
