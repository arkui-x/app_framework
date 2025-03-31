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
#include "app_event_handler.h"

#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "AppEventHandler"

namespace OHOS {
namespace HiviewDFX {
AppEventHandler::AppEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner)
    : AppExecFwk::EventHandler(runner)
{
    HILOG_INFO(LOG_CORE, "AppEventHandler instance created");
}

AppEventHandler::~AppEventHandler()
{
    HILOG_INFO(LOG_CORE, "AppEventHandler instance destroyed");
}

void AppEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    if (event->GetInnerEventId() == AppEventType::WATCHER_TIMEOUT) {
        AppEventObserverMgr::GetInstance().HandleTimeout();
    } else {
        HILOG_WARN(LOG_CORE, "invalid event id=%{public}u", event->GetInnerEventId());
    }
}
} // namespace HiviewDFX
} // namespace OHOS
