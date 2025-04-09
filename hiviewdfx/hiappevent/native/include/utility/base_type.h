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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_UTILITY_BASE_TYPE_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_UTILITY_BASE_TYPE_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
namespace HiAppEvent {
struct UserId {
    std::string name;
    std::string value;
};

struct UserProperty {
    std::string name;
    std::string value;
};

struct TriggerCondition {
    /* Triggered by the number of events */
    int row = 0;

    /* Triggered by the size of events. Note: Only watcher.onTrigger is supported
     */
    int size = 0;

    /* Triggered periodically */
    int timeout = 0;

    /* Triggered by application startup */
    bool onStartup = false;

    /* Triggered by switching to the background */
    bool onBackground = false;

    std::string ToString() const;
};

struct EventConfig {
    /* Specifies the event domain that can be reported */
    std::string domain;

    /* Specifies the event name that can be reported */
    std::string name;

    /* Specifies whether the event is a real-time report event */
    bool isRealTime = false;

    bool IsValidEvent(std::shared_ptr<AppEventPack> event) const;

    bool IsRealTimeEvent(std::shared_ptr<AppEventPack> event) const;

    std::string ToString() const;
};

struct ReportConfig {
    /* The report name */
    std::string name;

    /* Enable the developer to debug */
    bool debugMode = false;

    /* The server location information */
    std::string routeInfo;

    /* The application ID */
    std::string appId;

    /* The condition for triggering reporting */
    TriggerCondition triggerCond;

    /* The user id names that can be reported */
    std::unordered_set<std::string> userIdNames;

    /* The user property names that can be reported */
    std::unordered_set<std::string> userPropertyNames;

    /* Event configurations to report */
    std::vector<EventConfig> eventConfigs;

    /* The report configId */
    int configId = 0;

    /* Custom config data */
    std::unordered_map<std::string, std::string> customConfigs;

    std::string ToString() const;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_UTILITY_BASE_TYPE_H
