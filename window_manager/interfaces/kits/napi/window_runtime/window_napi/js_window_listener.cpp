/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "js_window_listener.h"

#include "hilog.h"
#include "js_window_utils.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;

JsWindowListener::~JsWindowListener()
{
    HILOG_INFO("[NAPI]~JsWindowListener");
    if (jsCallBack_) {
        napi_delete_reference(engine_, jsCallBack_);
    }
}

void JsWindowListener::CallJsMethod(napi_env env, const char* methodName, napi_value const* argv, size_t argc)
{
    HILOG_INFO("[NAPI]CallJsMethod methodName = %{public}s", methodName);
    if (env == nullptr || jsCallBack_ == nullptr) {
        HILOG_ERROR("[NAPI]engine_ nullptr or jsCallBack_ is nullptr");
        return;
    }
    napi_value method = nullptr;
    napi_status status = napi_get_reference_value(env, jsCallBack_, &method);
    if (method == nullptr) {
        HILOG_ERROR("[NAPI]Failed to get method callback from object");
        return;
    }
    napi_value userRet;
    status = napi_call_function(env, nullptr, method, argc, argv, &userRet);
    if (status != napi_ok) {
        HILOG_ERROR("CallJsMethod failed status=%{public}d", status);
    }
}

void JsWindowListener::LifeCycleCallBack(LifeCycleEventType eventType)
{
    HILOG_INFO("[NAPI]LifeCycleCallBack, envent type: %{public}u", eventType);
    NapiAsyncTask::CompleteCallback complete =
        [self = weakRef_, eventType, eng = engine_, caseType = caseType_]
            (napi_env env, NapiAsyncTask &task, int32_t status) {
            auto thisListener = self.promote();
            if (thisListener == nullptr || eng == nullptr) {
                HILOG_ERROR("[NAPI]this listener or engine is nullptr");
                return;
            }
            napi_value argv[] = {CreateJsValue(eng, static_cast<uint32_t>(eventType))};
            thisListener->CallJsMethod(eng, caseType.c_str(), argv, ArraySize(argv));
        };
    napi_value result;
    NapiAsyncTask::Schedule("JsWindowListener::LifeCycleCallBack",
        engine_, CreateAsyncTaskWithLastParam(engine_, nullptr, nullptr, std::move(complete), &result));
}

void JsWindowListener::AfterForeground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_HIDDEN) {
        LifeCycleCallBack(LifeCycleEventType::FOREGROUND);
        state_ = WindowState::STATE_SHOWN;
    } else {
        HILOG_DEBUG("[NAPI]window is already shown");
    }
}

void JsWindowListener::AfterBackground()
{
    if (state_ == WindowState::STATE_INITIAL || state_ == WindowState::STATE_SHOWN) {
        LifeCycleCallBack(LifeCycleEventType::BACKGROUND);
        state_ = WindowState::STATE_HIDDEN;
    } else {
        HILOG_DEBUG("[NAPI]window is already hide");
    }
}

void JsWindowListener::AfterFocused()
{
    LifeCycleCallBack(LifeCycleEventType::ACTIVE);
}

void JsWindowListener::AfterUnfocused()
{
    LifeCycleCallBack(LifeCycleEventType::INACTIVE);
}
} // namespace Rosen
} // namespace OHOS
