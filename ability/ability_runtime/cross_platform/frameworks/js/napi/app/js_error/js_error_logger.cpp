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

#include "js_error_logger.h"

#include "app_event_observer_mgr.h"
#include "json/json.h"
#include "time_util.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* APP_ERROR_EVENT_NAME = "APP_CRASH";
constexpr const char* APP_ERROR_EVENT_DOMAIN = "OS";
constexpr const char* APP_ERROR_CRASH_TYPE = "JsError";
void JsErrorLogger::SendExceptionToJsError(const JsErrorInfo& info, const AppExecFwk::ErrorObject& object,
    bool foreground)
{
    std::vector<std::shared_ptr<HiviewDFX::AppEventPack>> events;
    int64_t stamp = OHOS::HiviewDFX::TimeUtil::GetMilliseconds();

    auto event = std::make_shared<HiviewDFX::AppEventPack>(APP_ERROR_EVENT_NAME, 1);
    if (event == nullptr) {
        return;
    }
    event->SetDomain(APP_ERROR_EVENT_DOMAIN);
    event->SetTime(stamp);
    event->SetRunningId(info.appRunningId);
    Json::Value jsonValue;
    Json::Value jsonGroup;
    jsonGroup["name"] = object.name;
    jsonGroup["stack"] = object.stack;
    jsonGroup["message"] = object.message;
    jsonValue["time"] = Json::Int64(stamp);
    jsonValue["bundle_version"] = info.versionName;
    jsonValue["bundle_name"] = info.bundleName;
    jsonValue["pid"] = info.pid;
    jsonValue["uid"] = info.uid;
    jsonValue["crash_type"] = APP_ERROR_CRASH_TYPE;
    jsonValue["foreground"] = foreground;
    jsonValue["exception"] = jsonGroup;
    auto jsonStr = Json::FastWriter().write(jsonValue);
    event->SetParamStr(jsonStr);
    events.emplace_back(event);
    HiviewDFX::AppEventObserverMgr::GetInstance().HandleEvents(events);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
