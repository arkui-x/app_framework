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
#include "app_event_observer_mgr.h"

#include "app_event_handler.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "ObserverMgr"

namespace OHOS {
namespace HiviewDFX {
using HiAppEvent::AppEventFilter;
using HiAppEvent::TriggerCondition;
namespace {
constexpr int TIMEOUT_INTERVAL = HiAppEvent::TIMEOUT_STEP * 1000; // 30s
constexpr int MAX_SIZE_OF_INIT = 100;

void StoreEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    for (auto& event : events) {
        int64_t eventSeq = AppEventStore::GetInstance().InsertEvent(event);
        if (eventSeq <= 0) {
            HILOG_WARN(LOG_CORE, "failed to store event");
            continue;
        }
        event->SetSeq(eventSeq);
        AppEventStore::GetInstance().QueryCustomParamsAdd2EventPack(event);
    }
}

void StoreEventMapping(
    const std::vector<std::shared_ptr<AppEventPack>>& events, std::shared_ptr<AppEventObserver> observer)
{
    for (const auto& event : events) {
        if (observer->VerifyEvent(event)) {
            int64_t observerSeq = observer->GetSeq();
            if (observerSeq == 0) {
                HILOG_INFO(LOG_CORE, "observer=%{public}s seq not set", observer->GetName().c_str());
                continue;
            }
            if (AppEventStore::GetInstance().InsertEventMapping(event->GetSeq(), observerSeq) < 0) {
                HILOG_ERROR(LOG_CORE, "failed to add mapping record, seq=%{public}d", observerSeq);
            }
        }
    }
}

void SendEventsToObserver(
    const std::vector<std::shared_ptr<AppEventPack>>& events, std::shared_ptr<AppEventObserver> observer)
{
    std::vector<std::shared_ptr<AppEventPack>> realTimeEvents;
    for (const auto& event : events) {
        if (!observer->VerifyEvent(event)) {
            continue;
        }
        if (observer->IsRealTimeEvent(event)) {
            realTimeEvents.emplace_back(event);
        } else {
            observer->ProcessEvent(event);
        }
    }

    if (!realTimeEvents.empty()) {
        observer->OnEvents(realTimeEvents);
    }
}

int64_t InitObserver(std::shared_ptr<AppEventObserver> observer, const std::string& name, int64_t hashCode)
{
    int64_t observerSeq = AppEventStore::GetInstance().QueryObserverSeq(name, hashCode);
    if (observerSeq < 0) {
        HILOG_INFO(LOG_CORE, "the observer does not exist in database, name=%{public}s, hash=%{public}d", name.c_str(),
            hashCode);
        return -1;
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    if (AppEventStore::GetInstance().QueryEvents(events, observerSeq, MAX_SIZE_OF_INIT) < 0) {
        HILOG_ERROR(LOG_CORE, "failed to take events, seq=%{public}d", observerSeq);
        return -1;
    }
    observer->SetSeq(observerSeq);
    if (!events.empty()) {
        if (hashCode == 0) {
            // send old events to watcher where init
            SendEventsToObserver(events, observer);
        } else {
            TriggerCondition triggerCond;
            for (auto event : events) {
                triggerCond.row++;
                triggerCond.size += static_cast<int>(event->GetEventStr().size());
            }
            observer->SetCurrCondition(triggerCond);
            observer->ProcessStartup();
        }
    }
    return observerSeq;
}

int64_t InitWatcher(std::shared_ptr<AppEventObserver> observer)
{
    std::string observerName = observer->GetName();
    int64_t observerHashCode = observer->GenerateHashCode();
    int64_t observerSeq = InitObserver(observer, observerName, observerHashCode);
    if (observerSeq < 0) {
        observerSeq = AppEventStore::GetInstance().InsertObserver(observerName, observerHashCode);
        if (observerSeq < 0) {
            HILOG_ERROR(LOG_CORE, "failed to insert watcher=%{public}s", observerName.c_str());
            return -1;
        }
    }
    observer->SetSeq(observerSeq);
    return observerSeq;
}

bool HandleEventsByMap(std::vector<std::shared_ptr<AppEventPack>>& events,
    std::unordered_map<int64_t, std::shared_ptr<AppEventObserver>>& observers)
{
    for (auto it = observers.cbegin(); it != observers.cend(); ++it) {
        StoreEventMapping(events, it->second);
    }
    bool needSend = false;
    for (auto it = observers.cbegin(); it != observers.cend(); ++it) {
        // send events to observer, and then delete events not in event mapping
        SendEventsToObserver(events, it->second);
        needSend |= it->second->HasTimeoutCondition();
    }
    return needSend;
}
} // namespace

AppEventObserverMgr& AppEventObserverMgr::GetInstance()
{
    static AppEventObserverMgr instance;
    return instance;
}

AppEventObserverMgr::AppEventObserverMgr()
{
    CreateEventHandler();
}

void AppEventObserverMgr::CreateEventHandler()
{
    auto runner = OHOS::AppExecFwk::EventRunner::Create("OS_AppEvent_Hd", OHOS::AppExecFwk::ThreadMode::NEW_THREAD);
    if (runner == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to create event runner");
        return;
    }
    handler_ = std::make_shared<AppEventHandler>(runner);
}

AppEventObserverMgr::~AppEventObserverMgr()
{
    DestroyEventHandler();
}

void AppEventObserverMgr::DestroyEventHandler()
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (handler_ != nullptr) {
        HILOG_INFO(LOG_CORE, "start to RemoveAllEvents");
        // stop and wait task
        handler_->RemoveAllEvents();
    }
    handler_ = nullptr;
}

int64_t AppEventObserverMgr::RegisterObserver(std::shared_ptr<AppEventObserver> observer)
{
    int64_t observerSeq = InitWatcher(observer);
    if (observerSeq <= 0) {
        return observerSeq;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    watchers_[observerSeq] = observer;
    return observerSeq;
}

int AppEventObserverMgr::UnregisterObserver(int64_t observerSeq, ObserverType type)
{
    bool isWatcher = (type == ObserverType::WATCHER);
    std::lock_guard<std::mutex> lock(observerMutex_);
    if ((isWatcher && watchers_.find(observerSeq) == watchers_.cend())) {
        HILOG_WARN(LOG_CORE, "observer seq=%{public}d is not exist", observerSeq);
        return 0;
    }
    if (int ret = AppEventStore::GetInstance().DeleteObserver(observerSeq, type); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}d", observerSeq);
        return ret;
    }
    if (isWatcher) {
        watchers_.erase(observerSeq);
    } else {
        // The processing logic of processors.
    }
    HILOG_INFO(LOG_CORE, "unregister observer seq=%{public}d successfully", observerSeq);
    return 0;
}

int AppEventObserverMgr::UnregisterObserver(const std::string& observerName, ObserverType type)
{
    int64_t deleteSeq = 0;
    if (int ret = AppEventStore::GetInstance().QueryObserverSeqs(observerName, deleteSeq, type); ret < 0) {
        HILOG_ERROR(LOG_CORE, "failed to query observer=%{public}s seqs", observerName.c_str());
        return ret;
    }
    int ret = 0;
    if (int tempRet = UnregisterObserver(deleteSeq, type); tempRet < 0) {
        HILOG_ERROR(LOG_CORE, "failed to unregister observer seq=%{public}d", deleteSeq);
        ret = tempRet;
    }
    return ret;
}

void AppEventObserverMgr::HandleEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (watchers_.empty() || events.empty()) {
        return;
    }
    StoreEvents(events);
    bool needSend = HandleEventsByMap(events, watchers_);
    if (needSend && !hasHandleTimeout_) {
        SendEventToHandler();
        hasHandleTimeout_ = true;
    }
}

void AppEventObserverMgr::HandleTimeout()
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    bool needSend = false;
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        it->second->ProcessTimeout();
        needSend |= it->second->HasTimeoutCondition();
    }
    if (needSend) {
        SendEventToHandler();
    } else {
        hasHandleTimeout_ = false;
    }
}

void AppEventObserverMgr::SendEventToHandler()
{
    std::lock_guard<std::mutex> lock(handlerMutex_);
    if (handler_ == nullptr) {
        HILOG_ERROR(LOG_CORE, "failed to SendEventToHandler: handler is null");
        return;
    }
    handler_->SendEvent(AppEventType::WATCHER_TIMEOUT, 0, TIMEOUT_INTERVAL);
}

void AppEventObserverMgr::HandleClearUp()
{
    HILOG_INFO(LOG_CORE, "start to handle clear up");
    std::lock_guard<std::mutex> lock(observerMutex_);
    for (auto it = watchers_.cbegin(); it != watchers_.cend(); ++it) {
        it->second->ResetCurrCondition();
    }
}
} // namespace HiviewDFX
} // namespace OHOS
