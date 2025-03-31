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
#include "app_event_memory_cleaner.h"

#include "app_event_store.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Cleaner"

namespace OHOS {
namespace HiviewDFX {
namespace {

void ClearAllData()
{
    if (AppEventStore::GetInstance().DeleteEvent() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear event table");
    }
    if (AppEventStore::GetInstance().DeleteCustomEventParams() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear custom event params table");
    }
    if (AppEventStore::GetInstance().DeleteEventMapping() < 0) {
        HILOG_WARN(LOG_CORE, "failed to clear event mapping table");
    }
}
} // namespace

uint64_t AppEventMemoryCleaner::GetFilesSize()
{
    return 0;
}

uint64_t AppEventMemoryCleaner::ClearSpace(uint64_t curSize, uint64_t maxSize)
{
    return 0;
}

void AppEventMemoryCleaner::ClearData()
{
    HILOG_INFO(LOG_CORE, "start to clear data");
    ClearAllData();
}
} // namespace HiviewDFX
} // namespace OHOS
