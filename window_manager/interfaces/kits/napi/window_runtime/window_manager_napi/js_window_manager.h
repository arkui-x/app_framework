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

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Rosen {
class WindowOption;
napi_value JsWindowManagerInit(napi_env env, napi_value exportObj);
class JsWindowManager {
public:
    JsWindowManager();
    ~JsWindowManager();
    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value CreateWindow(napi_env env, napi_callback_info info);
    static napi_value FindWindowSync(napi_env env, napi_callback_info info);
    static napi_value GetLastWindow(napi_env env, napi_callback_info info);
private:
    static napi_value OnCreateWindow(napi_env env, napi_callback_info info);
    static napi_value OnFindWindowSync(napi_env env, napi_callback_info info);
    static napi_value OnGetLastWindow(napi_env env, napi_callback_info info);
    static bool ParseConfigOption(napi_env env, napi_value jsObject, WindowOption& option, void*& contextPtr);
};
}  // namespace Rosen
}  // namespace OHOS
#endif // OHOS_JS_WINDOW_MANAGER_H
