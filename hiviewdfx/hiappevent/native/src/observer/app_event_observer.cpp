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
#include "app_event_observer.h"

#include <sstream>

#include "app_event.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hiappevent_common.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Observer"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
namespace {
constexpr int MAX_SIZE_ON_EVENTS = 100;

bool MeetNumberCondition(int currNum, int maxNum)
{
    return maxNum > 0 && currNum >= maxNum;
}

std::string GetStr(const std::unordered_set<std::string>& strSet)
{
    if (strSet.empty()) {
        return "[]";
    }
    std::stringstream strStream("[");
    for (const auto& str : strSet) {
        strStream << str << ",";
    }
    strStream.seekp(-1, std::ios_base::end); // -1 for delete ','
    strStream << "]";
    return strStream.str();
}

std::string GetStr(const std::vector<EventConfig>& eventConfigs)
{
    if (eventConfigs.empty()) {
        return "[]";
    }
    std::stringstream strStream("[");
    for (const auto& eventConfig : eventConfigs) {
        strStream << eventConfig.ToString() << ",";
    }
    strStream.seekp(-1, std::ios_base::end); // -1 for delete ','
    strStream << "]";
    return strStream.str();
}
} // namespace

AppEventFilter::AppEventFilter(const std::string& domain, const std::unordered_set<std::string>& names, uint32_t types)
    : domain(domain), names(names), types(types)
{}

AppEventFilter::AppEventFilter(const std::string& domain, uint32_t types) : domain(domain), types(types) {}

bool AppEventFilter::IsValidEvent(std::shared_ptr<AppEventPack> event) const
{
    return IsValidEvent(event->GetDomain(), event->GetName(), event->GetType());
}

bool AppEventFilter::IsValidEvent(const std::string& eventDomain, const std::string& eventName, int eventType) const
{
    if (domain.empty()) {
        return false;
    }
    if (!domain.empty() && domain != eventDomain) {
        return false;
    }
    if (!names.empty() && (names.find(eventName) == names.end())) {
        return false;
    }
    if (types != 0 && !(types & (1 << eventType))) { // 1: bit mask
        return false;
    }
    return true;
}

bool EventConfig::IsValidEvent(std::shared_ptr<AppEventPack> event) const
{
    if (domain.empty() && name.empty()) {
        return false;
    }
    if (!domain.empty() && (domain != event->GetDomain())) {
        return false;
    }
    if (!name.empty() && (name != event->GetName())) {
        return false;
    }
    return true;
}

bool EventConfig::IsRealTimeEvent(std::shared_ptr<AppEventPack> event) const
{
    return IsValidEvent(event) && isRealTime;
}

std::string EventConfig::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << domain << "," << name << "," << isRealTime << "}";
    return strStream.str();
}

std::string TriggerCondition::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << row << "," << size << "," << timeout << "," << onStartup << "," << onBackground << "}";
    return strStream.str();
}

std::string ReportConfig::ToString() const
{
    std::stringstream strStream;
    strStream << "{" << name << "," << debugMode << "," << routeInfo << "," << appId << "," << triggerCond.ToString()
              << "," << GetStr(userIdNames) << "," << GetStr(userPropertyNames) << "," << GetStr(eventConfigs) << "}";
    return strStream.str();
}

bool AppEventObserver::VerifyEvent(std::shared_ptr<AppEventPack> event)
{
    if (filters_.empty()) {
        return true;
    }
    auto it = std::find_if(
        filters_.begin(), filters_.end(), [event](const auto& filter) { return filter.IsValidEvent(event); });
    return it != filters_.end();
}

bool AppEventObserver::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    const auto& eventConfigs = reportConfig_.eventConfigs;
    if (eventConfigs.empty()) {
        return false;
    }
    auto it = std::find_if(eventConfigs.begin(), eventConfigs.end(), [event](const auto& config) {
        return config.IsRealTimeEvent(event);
    });
    return it != eventConfigs.end();
}

void AppEventObserver::ProcessEvent(std::shared_ptr<AppEventPack> event)
{
    HILOG_INFO(LOG_CORE, "observer=%{public}s start to process event", name_.c_str());
    ++currCond_.row;
    currCond_.size += static_cast<int>(event->GetEventStr().size());
    if (MeetProcessCondition()) {
        OnTrigger(currCond_);
        ResetCurrCondition();
    }
}

bool AppEventObserver::MeetProcessCondition()
{
    return MeetNumberCondition(currCond_.row, reportConfig_.triggerCond.row) ||
            MeetNumberCondition(currCond_.size, reportConfig_.triggerCond.size);
}

void AppEventObserver::ResetCurrCondition()
{
    currCond_.row = 0;
    currCond_.size = 0;
    currCond_.timeout = 0;
}

void AppEventObserver::OnTrigger(const TriggerCondition& triggerCond)
{
    std::vector<std::shared_ptr<AppEventPack>> events;
    QueryEvents(events);
    if (!events.empty()) {
        OnEvents(events);
    }
}

void AppEventObserver::QueryEvents(std::vector<std::shared_ptr<AppEventPack>>& events)
{
    if (AppEventStore::GetInstance().QueryEvents(events, seq_, MAX_SIZE_ON_EVENTS) != 0) {
        HILOG_WARN(LOG_CORE, "failed to take data from observer=%{public}s, seq=%{public}d", name_.c_str(), seq_);
        return;
    }
    HILOG_INFO(LOG_CORE, "end to take data from observer=%{public}s, seq=%{public}d , size=%{public}zu", name_.c_str(),
        seq_, events.size());
}

void AppEventObserver::ProcessTimeout()
{
    currCond_.timeout += TIMEOUT_STEP;
    if (!MeetTimeoutCondition()) {
        return;
    }
    OnTrigger(currCond_);
    ResetCurrCondition();
}

bool AppEventObserver::MeetTimeoutCondition()
{
    return MeetNumberCondition(currCond_.timeout, reportConfig_.triggerCond.timeout) && currCond_.row > 0;
}

bool AppEventObserver::HasTimeoutCondition()
{
    return reportConfig_.triggerCond.timeout > 0 && currCond_.row > 0;
}

void AppEventObserver::ProcessStartup()
{
    if (!MeetStartupCondition()) {
        return;
    }
    OnTrigger(currCond_);
    ResetCurrCondition();
}

bool AppEventObserver::MeetStartupCondition()
{
    return reportConfig_.triggerCond.onStartup && currCond_.row > 0;
}

std::string AppEventObserver::GetName()
{
    return name_;
}

int64_t AppEventObserver::GetSeq()
{
    return seq_;
}

void AppEventObserver::SetSeq(int64_t seq)
{
    seq_ = seq;
}

void AppEventObserver::SetCurrCondition(const TriggerCondition& triggerCond)
{
    currCond_ = triggerCond;
}

int64_t AppEventObserver::GenerateHashCode()
{
    if (reportConfig_.name.empty()) {
        // default hash code for watcher
        return 0;
    }
    int64_t hashCode = static_cast<int64_t>(std::hash<std::string> {}(reportConfig_.ToString()));
    // std::hash returns uint64_t type, should remove sign bit to make AddProcessor returns positive integer.
    return (reportConfig_.configId > 0) ? static_cast<int64_t>(reportConfig_.configId) : (hashCode & INT64_MAX);
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
