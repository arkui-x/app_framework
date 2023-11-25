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
#include "js_window_utils.h"
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "refbase.h"
#include "window_interface.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
const std::string WINDOW_STAGE_EVENT_CB = "windowStageEvent";
const std::string WINDOW_EVENT_CB = "windowEvent";

class JsWindowListener : public IWindowLifeCycle {
public:
    JsWindowListener(const std::string& caseType, NativeEngine* engine, std::shared_ptr<NativeReference> callback)
        : caseType_(caseType), engine_(engine), jsCallBack_(callback), weakRef_(wptr<JsWindowListener> (this)) {}
    ~JsWindowListener();
    void AfterForeground() override;
    void AfterBackground() override;
    void AfterFocused() override;
    void AfterUnfocused() override;
    void CallJsMethod(const char* methodName, NativeValue* const* argv = nullptr, size_t argc = 0);
private:
    WindowState state_ {WindowState::STATE_INITIAL};
    void LifeCycleCallBack(LifeCycleEventType eventType);
    NativeEngine* engine_ = nullptr;
    std::shared_ptr<NativeReference> jsCallBack_ = nullptr;
    wptr<JsWindowListener> weakRef_  = nullptr;
    std::string caseType_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif // OHOS_JS_WINDOW_LISTENER_H
