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

#ifndef OHOS_JS_DISPLAY_H
#define OHOS_JS_DISPLAY_H

#include "display.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace Rosen {
napi_value CreateJsDisplayObject(napi_env env, sptr<Display>& display);
class JsDisplay final {
public:
    explicit JsDisplay(const sptr<Display>& display);
    ~JsDisplay();
    static void Finalizer(napi_env env, void* data, void* hint);
private:
    sptr<Display> display_ = nullptr;
};
enum class DisplayStateMode : uint32_t {
    STATE_UNKNOWN = 0,
    STATE_OFF,
    STATE_ON,
    STATE_DOZE,
    STATE_DOZE_SUSPEND,
    STATE_VR,
    STATE_ON_SUSPEND
};
}  // namespace Rosen
}  // namespace OHOS
#endif