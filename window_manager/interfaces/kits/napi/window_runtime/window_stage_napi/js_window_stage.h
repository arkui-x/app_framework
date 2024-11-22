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

#ifndef OHOS_JS_WINDOW_STAGE_H
#define OHOS_JS_WINDOW_STAGE_H
#include <map>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "window_stage.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsWindowStage(napi_env env, std::shared_ptr<Rosen::WindowStage> WindowStage);
class JsWindowStage {
public:
    explicit JsWindowStage(const std::shared_ptr<Rosen::WindowStage>& WindowStage);
    ~JsWindowStage();
    static napi_value GetMainWindow(napi_env env, napi_callback_info info);
    static napi_value GetMainWindowSync(napi_env env, napi_callback_info info);
    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static napi_value LoadContent(napi_env env, napi_callback_info info);
    static napi_value LoadContentByName(napi_env env, napi_callback_info info);
    static napi_value CreateSubWindow(napi_env env, napi_callback_info info);
    static napi_value GetSubWindow(napi_env env, napi_callback_info info);
    static void Finalizer(napi_env env, void* data, void* hint);

private:
    napi_value OnGetMainWindow(napi_env env, napi_callback_info info);
    napi_value OnGetMainWindowSync(napi_env env, napi_callback_info info);
    napi_value OnEvent(napi_env env, napi_callback_info info);
    napi_value OffEvent(napi_env env, napi_callback_info info);
    napi_value OnLoadContent(napi_env env, napi_callback_info info, bool isLoadedByName);
    napi_value OnCreateSubWindow(napi_env env, napi_callback_info info);
    napi_value OnGetSubWindow(napi_env env, napi_callback_info info);
    std::weak_ptr<Rosen::WindowStage> windowStage_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_WINDOW_STAGE_H