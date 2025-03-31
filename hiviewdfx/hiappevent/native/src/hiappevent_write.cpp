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

#include "hiappevent_write.h"

#include <mutex>
#include <string>

#include "app_event_observer_mgr.h"
#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hiappevent_clean.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Write"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr int DB_FAILED = -1;
} // namespace

void WriteEvent(std::shared_ptr<AppEventPack> appEventPack)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HILOG_WARN(LOG_CORE, "the HiAppEvent function is disabled.");
        return;
    }
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "appEventPack is null.");
        return;
    }
    std::vector<std::shared_ptr<AppEventPack>> events;
    events.emplace_back(appEventPack);
    AppEventObserverMgr::GetInstance().HandleEvents(events);
}

int SetEventParam(std::shared_ptr<AppEventPack> appEventPack)
{
    if (appEventPack == nullptr) {
        HILOG_ERROR(LOG_CORE, "appEventPack is null.");
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    int res = AppEventStore::GetInstance().InsertCustomEventParams(appEventPack);
    if (res == DB_FAILED) {
        HILOG_ERROR(LOG_CORE, "failed to insert event param, domain=%{public}s.", appEventPack->GetDomain().c_str());
        return ErrorCode::HIAPPEVENT_VERIFY_SUCCESSFUL;
    }
    return res;
}
} // namespace HiviewDFX
} // namespace OHOS
