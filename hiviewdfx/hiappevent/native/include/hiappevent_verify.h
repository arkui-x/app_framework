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

#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_VERIFY_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_VERIFY_H

#include <memory>
#include <string>

#include "base_type.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
using HiAppEvent::EventConfig;
using HiAppEvent::ReportConfig;

int VerifyAppEvent(std::shared_ptr<AppEventPack> event);
int VerifyCustomEventParams(std::shared_ptr<AppEventPack> event);

bool IsValidDomain(const std::string& domain);
bool IsValidEventName(const std::string& eventName);
bool IsValidWatcherName(const std::string& watcherName);
bool IsValidEventType(int eventType);
bool IsValidProcessorName(const std::string& name);
bool IsValidRouteInfo(const std::string& name);
bool IsValidAppId(const std::string& name);
bool IsValidPeriodReport(int timeout);
bool IsValidBatchReport(int count);
bool IsValidUserIdName(const std::string& name);
bool IsValidUserIdValue(const std::string& value);
bool IsValidUserPropName(const std::string& name);
bool IsValidUserPropValue(const std::string& value);
bool IsValidEventConfig(const EventConfig& eventCfg);
bool IsValidConfigId(int configId);
bool IsValidCustomConfigsNum(size_t num);
bool IsValidCustomConfig(const std::string& name, const std::string& value);
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_VERIFY_H
