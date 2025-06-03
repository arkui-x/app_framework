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
#ifndef OHOS_JS_WINDOW_REGISTER_MANAGER_H
#define OHOS_JS_WINDOW_REGISTER_MANAGER_H
#include <map>
#include <mutex>

#include "js_window_listener.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "refbase.h"
#include "virtual_rs_window.h"

namespace OHOS {
namespace Rosen {
enum class CaseType {
    CASE_WINDOW_MANAGER = 0,
    CASE_WINDOW,
    CASE_STAGE
};
class JsWindowRegisterManager {
public:
    JsWindowRegisterManager();
    ~JsWindowRegisterManager();
    static JsWindowRegisterManager& GetInstance();
    WmErrorCode RegisterListener(napi_env env, std::shared_ptr<Window> window, std::string type, CaseType caseType, napi_value value);
    WmErrorCode UnregisterListener(napi_env env, std::shared_ptr<Window> window, std::string type, CaseType caseType, napi_value value);
private:
    bool IsEqualRegister(napi_env env, napi_value lhs, napi_ref rref);
    bool IsCallbackRegistered(napi_env env, std::string type, napi_value jsListenerObject);
    WmErrorCode ProcessLifeCycleEventRegister(sptr<JsWindowListener> listener, std::shared_ptr<OHOS::Rosen::Window> window, bool isRegister);
    WmErrorCode ProcessSizeChangeRegister(sptr<JsWindowListener> listener,
    std::shared_ptr<Window> window, bool isRegister);
    WmErrorCode ProcessAvoidAreaChangeRegister(sptr<JsWindowListener> listener,
        std::shared_ptr<OHOS::Rosen::Window> window, bool isRegister);
    WmErrorCode ProcessWindowStatusChangeRegister(sptr<JsWindowListener> listener,
        std::shared_ptr<OHOS::Rosen::Window> window, bool isRegister);
    using Func_t = WmErrorCode(JsWindowRegisterManager::*)(sptr<JsWindowListener>, std::shared_ptr<OHOS::Rosen::Window> window, bool);
    std::map<std::string, std::map<napi_ref, sptr<JsWindowListener>>> jsCbMap_;
    std::mutex mtx_;
    std::map<CaseType, std::map<std::string, Func_t>> listenerProcess_;
};
} // namespace Rosen
} // namespace OHOS

#endif