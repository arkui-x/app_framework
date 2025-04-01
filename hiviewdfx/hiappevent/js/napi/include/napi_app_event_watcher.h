/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H

#include "app_event_watcher.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
struct OnTriggerContext {
    ~OnTriggerContext();
    napi_env env = nullptr;
    napi_ref onTrigger = nullptr;
    napi_ref holder = nullptr;
    int row = 0;
    int size = 0;
};

struct OnReceiveContext {
    ~OnReceiveContext();
    napi_env env = nullptr;
    napi_ref onReceive = nullptr;
    std::string domain;
    std::vector<std::shared_ptr<AppEventPack>> events;
    int64_t observerSeq = 0;
};

struct WatcherContext {
    ~WatcherContext();
    OnTriggerContext* triggerContext = nullptr;
    OnReceiveContext* receiveContext = nullptr;
};

class NapiAppEventWatcher : public AppEventWatcher {
public:
    NapiAppEventWatcher(const std::string& name, const std::vector<AppEventFilter>& filters, TriggerCondition cond);
    ~NapiAppEventWatcher();
    void InitTrigger(const napi_env env, const napi_value trigger);
    void InitHolder(const napi_env env, const napi_value holder);
    void InitReceiver(const napi_env env, const napi_value receiver);
    void OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events) override;
    bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) override;

protected:
    void OnTrigger(const TriggerCondition& triggerCond) override;

private:
    WatcherContext* context_ = nullptr;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_JS_NAPI_INCLUDE_NAPI_APP_EVENT_WATCHER_H
