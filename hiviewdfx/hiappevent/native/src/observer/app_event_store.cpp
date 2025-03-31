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
#include "app_event_store.h"

#include <cinttypes>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "app_event_cache_common.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Store"

using namespace OHOS::HiviewDFX::AppEventCacheCommon;
using namespace OHOS::HiviewDFX::HiAppEvent;
namespace OHOS {
namespace HiviewDFX {
namespace {
static constexpr size_t MAX_NUM_OF_CUSTOM_PARAMS = 64;
std::atomic<int> g_observerSeq(1);
std::atomic<int> g_eventObserverSeq(1);
std::atomic<int> g_eventSeq(1);
std::atomic<int> g_customEventParamSeq(1);
} // namespace

AppEventStore& AppEventStore::GetInstance()
{
    static AppEventStore instance;
    return instance;
}

int64_t AppEventStore::InsertEvent(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    g_eventSeq++;
    eventMap_[g_eventSeq] = event;
    return g_eventSeq;
}

int64_t AppEventStore::InsertObserver(const std::string& observer, int64_t hashCode)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    auto it = appEventObserverMap_.begin();
    while (it != appEventObserverMap_.end()) {
        auto& [seq, observerSharedPtr] = *it;
        auto& [existingObserver, existingHashCode] = *observerSharedPtr;
        if (existingObserver == observer && existingHashCode == hashCode) {
            return seq;
        } else {
            ++it;
        }
    }
    g_observerSeq++;
    appEventObserverMap_[g_observerSeq] = std::make_shared<Observer>(observer, hashCode);
    return g_observerSeq;
}

int64_t AppEventStore::InsertEventMapping(int64_t eventSeq, int64_t observerSeq)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    g_eventObserverSeq++;
    appEventObserverMapping_[g_eventObserverSeq] = std::make_pair(eventSeq, observerSeq);
    return g_eventObserverSeq;
}

std::unordered_set<std::string> AppEventStore::FindOldParamkeys(
    const std::string& runningId, const std::string& domain, const std::string& name)
{
    std::unordered_set<std::string> paramNames;
    for (const auto& pair : customParamMap_) {
        const auto& CustomEventParamExtended = pair.second;
        if (CustomEventParamExtended->running_id == runningId && CustomEventParamExtended->domain == domain &&
            CustomEventParamExtended->name == name) {
            paramNames.insert(CustomEventParamExtended.get()->param_key);
            break;
        }
    }
    return paramNames;
}

void AppEventStore::UpdateCustomEventParam(
    const std::string& runningId, const std::string& domain, const std::string& name, const CustomEventParam& param)
{
    for (auto& pair : customParamMap_) {
        auto& customEventParamExtended = pair.second;
        if (customEventParamExtended->running_id == runningId && customEventParamExtended->domain == domain &&
            customEventParamExtended->name == name && customEventParamExtended->param_key == param.key) {
            customEventParamExtended->param_value = param.value;
            customEventParamExtended->param_type = param.type;
        }
    }
}

int64_t AppEventStore::InsertCustomEventParams(const std::shared_ptr<AppEventPack> event)
{
    std::vector<CustomEventParam> newParams;
    event->GetCustomParams(newParams);
    if (newParams.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    std::unordered_set<std::string> oldParamkeys;
    std::string runningId = event->GetRunningId();
    std::string domain = event->GetDomain();
    std::string name = event->GetName();
    oldParamkeys = FindOldParamkeys(runningId, domain, name);
    size_t totalNum = oldParamkeys.size();
    for (const auto& param : newParams) {
        if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
            ++totalNum;
        }
    }
    if (totalNum > MAX_NUM_OF_CUSTOM_PARAMS) {
        return ErrorCode::ERROR_INVALID_CUSTOM_PARAM_NUM;
    }
    for (const auto& param : newParams) {
        if (oldParamkeys.find(param.key) == oldParamkeys.end()) {
            auto newEventParam =
                std::make_shared<CustomEventParamExtended>(runningId, domain, name, param);
            customParamMap_[g_customEventParamSeq++] = newEventParam;
        } else {
            UpdateCustomEventParam(runningId, domain, name, param);
        }
    }
    return DB_SUCC;
}

void AppEventStore::FindParams(std::unordered_map<std::string, std::string>& params, const std::string& runningId,
    const std::string& domain, const std::string& name)
{
    for (const auto& pair : customParamMap_) {
        const auto& customEventParam = pair.second;
        if (!customEventParam->name.empty() && customEventParam->running_id == runningId &&
            customEventParam->domain == domain && customEventParam->name == name) {
            params[customEventParam->param_key] = customEventParam->param_value;
        } else if (customEventParam->name.empty() && customEventParam->running_id == runningId &&
                    customEventParam->domain == domain) {
            params[customEventParam->param_key] = customEventParam->param_value;
        }
    }
}

int AppEventStore::QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events, int64_t observerSeq, uint32_t size)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    std::unordered_map<std::string, std::string> params;
    uint32_t addedEvents = 0;
    for (const auto& pair : appEventObserverMapping_) {
        if (addedEvents >= size && size != 0) {
            break;
        }
        if (pair.second.second == observerSeq) {
            auto event = eventMap_[pair.second.first];
            if (!event) {
                return DB_FAILED;
            }
            FindParams(params, event->GetRunningId(), event->GetDomain(), event->GetName());
            event->AddCustomParams(params);
            events.emplace_back(event);
            addedEvents++;
        }
    }
    return DB_SUCC;
}

int AppEventStore::QueryCustomParamsAdd2EventPack(std::shared_ptr<AppEventPack> event)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    std::unordered_map<std::string, std::string> params;
    FindParams(params, event->GetRunningId(), event->GetDomain(), event->GetName());
    event->AddCustomParams(params);
    return DB_SUCC;
}

int64_t AppEventStore::QueryObserverSeq(const std::string& observer, int64_t hashCode)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    for (const auto& pair : appEventObserverMap_) {
        const auto& observerPtr = pair.second;
        if (observerPtr->name == observer && observerPtr->hash == hashCode) {
            return pair.first;
        }
    }
    return DB_FAILED;
}

int AppEventStore::QueryObserverSeqs(const std::string& observer, int64_t& observerSeq, ObserverType type)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    for (const auto& observerPair : appEventObserverMap_) {
        const auto& observerPtr = observerPair.second;
        if (observerPtr->name == observer) {
            observerSeq = observerPair.first;
            return DB_SUCC;
        }
    }
    return DB_FAILED;
}

int AppEventStore::DeleteObserver(int64_t observerSeq, ObserverType type)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    appEventObserverMap_.erase(observerSeq);
    return DB_SUCC;
}

int AppEventStore::DeleteEventMapping(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    if (observerSeq == 0 && (!eventSeqs.empty())) {
        appEventObserverMapping_.clear();
        return DB_SUCC;
    }
    for (int64_t eventSeq : eventSeqs) {
        std::pair<int64_t, int64_t> key(observerSeq, eventSeq);
        for (auto it = appEventObserverMapping_.begin(); it != appEventObserverMapping_.end();) {
            if (it->second.first == eventSeq && it->second.second == observerSeq) {
                it = appEventObserverMapping_.erase(it);
            } else {
                ++it;
            }
        }
    }
    return DB_SUCC;
}

int AppEventStore::DeleteEvent(int64_t eventSeq)
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    eventMap_.clear();
    return DB_SUCC;
}

int AppEventStore::DeleteCustomEventParams()
{
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    customParamMap_.clear();
    return DB_SUCC;
}

int AppEventStore::DeleteEvent(const std::vector<int64_t>& eventSeqs)
{
    if (eventSeqs.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    for (const auto& seq : eventSeqs) {
        eventMap_.erase(seq);
    }
    return DB_SUCC;
}

int AppEventStore::DeleteUnusedParamsExceptCurId(const std::string& curRunningId)
{
    if (curRunningId.empty()) {
        return DB_SUCC;
    }
    std::lock_guard<std::mutex> lockGuard(storeMutex_);
    int deleteRows = 0;
    std::vector<int64_t> toBeDeleted;

    // delete custom_event_params if running_id not in events, and running_id isn't current runningId
    std::unordered_set<std::string> validRunningIds;
    for (const auto& event : eventMap_) {
        const auto& appEventPack = event.second;
        validRunningIds.insert(appEventPack->GetRunningId());
    }

    for (const auto& customParam : customParamMap_) {
        const auto& CustomEventParamExtended = customParam.second;
        const std::string& runningId = CustomEventParamExtended->running_id;
        if (runningId != curRunningId && validRunningIds.find(runningId) == validRunningIds.end()) {
            toBeDeleted.push_back(customParam.first);
        }
    }
    for (int64_t id : toBeDeleted) {
        customParamMap_.erase(id);
        ++deleteRows;
    }
    HILOG_INFO(LOG_CORE, "delete %{public}d params unused", deleteRows);
    return deleteRows;
}

bool AppEventStore::DeleteData(int64_t observerSeq, const std::vector<int64_t>& eventSeqs)
{
    if (DeleteEventMapping(observerSeq, eventSeqs) < 0) {
        return false;
    }
    if (DeleteEvent(eventSeqs) < 0) {
        HILOG_WARN(LOG_CORE, "failed to delete unused event");
    }
    std::string runningId = HiAppEventConfig::GetInstance().GetRunningId();
    if (!runningId.empty() && DeleteUnusedParamsExceptCurId(runningId) < 0) {
        HILOG_WARN(LOG_CORE, "failed to delete unused params");
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
