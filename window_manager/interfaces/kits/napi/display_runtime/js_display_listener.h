/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_DISPLAY_LISTENER_H
#define OHOS_JS_DISPLAY_LISTENER_H

#include <mutex>
#include "dm_common.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "refbase.h"
#include "display_manager.h"
#include "native_engine/native_reference.h"

namespace OHOS {
namespace Rosen {
class JsDisplayListener : public DisplayManager::DisplayListener,
                          public DisplayManager::FoldStatusListener,
                          public DisplayManager::DisplayModeListener {
public:
    explicit JsDisplayListener(napi_env env);
    ~JsDisplayListener() override;
    void AddCallback(const std::string& type, napi_value jsListenerObject);
    void RemoveAllCallback();
    void RemoveCallback(napi_env env, const std::string& type, napi_value jsListenerObject);
    void OnCreate(DisplayId id) override;
    void OnDestroy(DisplayId id) override;
    void OnChange(DisplayId id) override;
    void DispatchDisplayEvent(const std::string& eventType, DisplayId id);
    void OnFoldStatusChanged(FoldStatus foldStatus) override;
    void OnDisplayModeChanged(FoldDisplayMode displayMode) override;

private:
    void CallJsMethod(const std::string& methodName, napi_value const * argv = nullptr, size_t argc = 0);
    napi_env env_ = nullptr;
    std::mutex mtx_;
    wptr<JsDisplayListener> weakRef_  = nullptr;
    std::map<std::string, std::vector<std::unique_ptr<NativeReference>>> jsCallBack_;
    napi_value CreateDisplayIdArray(napi_env env, const std::vector<DisplayId>& data);
    static void CleanEnv(void* obj);
};
const std::string EVENT_ADD = "add";
const std::string EVENT_REMOVE = "remove";
const std::string EVENT_CHANGE = "change";
const std::string EVENT_FOLD_STATUS_CHANGED = "foldStatusChange";
const std::string EVENT_DISPLAY_MODE_CHANGED = "foldDisplayModeChange";
}  // namespace Rosen
}  // namespace OHOS
#endif /* OHOS_JS_DISPLAY_LISTENER_H */