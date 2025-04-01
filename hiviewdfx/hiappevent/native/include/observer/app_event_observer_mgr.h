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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_OBSERVER_MGR_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_OBSERVER_MGR_H

#include <memory>
#include <mutex>
#include <unordered_map>

#include "app_event_observer.h"
#include "hiappevent_base.h"
#include "nocopyable.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventHandler;
class OsEventListener;
namespace HiAppEvent {
class AppStateCallback;
}
using HiAppEvent::AppEventObserver;
using HiAppEvent::AppStateCallback;
using HiAppEvent::ReportConfig;

class AppEventObserverMgr : public NoCopyable {
public:
    static AppEventObserverMgr& GetInstance();

    void CreateEventHandler();
    void DestroyEventHandler();
    int64_t RegisterObserver(std::shared_ptr<AppEventObserver> observer);
    int UnregisterObserver(int64_t observerSeq, ObserverType type);
    int UnregisterObserver(const std::string& observerName, ObserverType type);
    void HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events);
    void HandleTimeout();
    void HandleClearUp();
private:
    AppEventObserverMgr();
    ~AppEventObserverMgr();
    void SendEventToHandler();

private:
    std::unordered_map<int64_t, std::shared_ptr<AppEventObserver>> watchers_;
    std::shared_ptr<AppEventHandler> handler_;
    std::mutex observerMutex_;
    bool hasHandleTimeout_ = false;
    std::mutex handlerMutex_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_OBSERVER_MGR_H
