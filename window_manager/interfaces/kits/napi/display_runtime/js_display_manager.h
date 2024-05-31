/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Rosen {

napi_value JsDisplayManagerInit(napi_env env, napi_value exportObj);
class JsDisplayManager {
public:
    explicit JsDisplayManager(napi_env env) {
    }

    ~JsDisplayManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value GetDefaultDisplaySync(napi_env env, napi_callback_info info);

private:
    napi_value OnGetDefaultDisplaySync(napi_env env, napi_callback_info info);
};
}  // namespace Rosen
}  // namespace OHOS

#endif
