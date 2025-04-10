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

#include "hiappevent_clean.h"

#include <memory>
#include <vector>

#include "app_event_memory_cleaner.h"
#include "app_event_observer_mgr.h"
#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hiappevent_userinfo.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Clean"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEventClean {
namespace {
constexpr int EVENT_COUNT_OF_CHECK_SPACE = 1000;

static int g_eventCount = -1;
} // namespace

void ClearData()
{
    // reset the status of observers
    AppEventObserverMgr::GetInstance().HandleClearUp();

    // clear user ids and properties
    HiAppEvent::UserInfo::GetInstance().ClearData();

    auto cleaner = std::make_shared<AppEventMemoryCleaner>();
    cleaner->ClearData();
}
} // namespace HiAppEventClean
} // namespace HiviewDFX
} // namespace OHOS
