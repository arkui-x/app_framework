/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_DISPLAY_MANAGER_H
#define OHOS_JS_DISPLAY_MANAGER_H

#include "display_manager.h"
#include "js_display_listener.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_reference.h"

namespace OHOS {
namespace Rosen {

napi_value JsDisplayManagerInit(napi_env env, napi_value exportObj);
class JsDisplayManager {
public:
    explicit JsDisplayManager(napi_env env) {}

    ~JsDisplayManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetDefaultDisplaySync(napi_env env, napi_callback_info info);
    static napi_value RegisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    static napi_value UnregisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    static napi_value IsFoldable(napi_env env, napi_callback_info info);
    static napi_value GetFoldStatus(napi_env env, napi_callback_info info);
    static napi_value GetFoldDisplayMode(napi_env env, napi_callback_info info);

private:
    napi_value OnGetDefaultDisplaySync(napi_env env, napi_callback_info info);
    napi_value OnRegisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    napi_value OnUnregisterDisplayManagerCallback(napi_env env, napi_callback_info info);
    napi_value OnIsFoldable(napi_env env, napi_callback_info info);
    napi_value OnGetFoldStatus(napi_env env, napi_callback_info info);
    napi_value OnGetFoldDisplayMode(napi_env env, napi_callback_info info);
    bool IsCallbackRegistered(napi_env env, const std::string& type, napi_value jsListenerObject);
    DMError RegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value);
    DMError UnRegisterDisplayListenerWithType(napi_env env, const std::string& type, napi_value value);
    DMError UnregisterAllDisplayListenerWithType(const std::string& type);
    bool NapiIsCallable(napi_env env, napi_value value);
    std::map<std::string, std::map<std::unique_ptr<NativeReference>, sptr<JsDisplayListener>>> jsCbMap_;
    std::mutex mtx_;
};
} // namespace Rosen
} // namespace OHOS

#endif
