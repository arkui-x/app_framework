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

#ifndef OHOS_JS_WINDOW_LISTENER_H
#define OHOS_JS_WINDOW_LISTENER_H

#include <map>
#include <mutex>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "js_window_utils.h"
#include "refbase.h"
#include "window_interface.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";
const std::string WINDOW_STATUS_CHANGE_CB = "windowStatusChange";
const std::string AVOID_AREA_CHANGE_CB = "avoidAreaChange";

class JsWindowListener : public IWindowChangeListener,
                         public IWindowLifeCycle,
                         public IWindowStatusChangeListener,
                         public IAvoidAreaChangedListener {
public:
    JsWindowListener(const std::string& caseType, napi_env env, napi_ref callback)
        : caseType_(caseType), engine_(env), jsCallBack_(callback), weakRef_(wptr<JsWindowListener> (this)) {}
    ~JsWindowListener();
    void OnSizeChange(Rect rect) override;
    void OnAvoidAreaChanged(const Rosen::AvoidArea avoidArea, Rosen::AvoidAreaType type) override;
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
    void OnWindowStatusChange(WindowStatus status) override;
    void CallJsMethod(napi_env env, const char* methodName, napi_value const* argv = nullptr, size_t argc = 0);
private:
    uint32_t currentWidth_ = 0;
    uint32_t currentHeight_ = 0;
    WindowState state_ {WindowState::STATE_INITIAL};
    void LifeCycleCallBack(LifeCycleEventType eventType);
    std::string caseType_;
    napi_env engine_ = nullptr;
    napi_ref jsCallBack_ = nullptr;
    wptr<JsWindowListener> weakRef_  = nullptr;
};
}  // namespace Rosen
}  // namespace OHOS
#endif // OHOS_JS_WINDOW_LISTENER_H
