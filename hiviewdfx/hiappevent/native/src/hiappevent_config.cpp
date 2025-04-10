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
#include "hiappevent_config.h"

#include <algorithm>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>

#include "application_context.h"
#include "context.h"
#include "hiappevent_base.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Config"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DISABLE = "disable";

// mutex g_mutex;
std::mutex g_mutex;

std::string TransUpperToUnderscoreAndLower(const std::string& str)
{
    if (str.empty()) {
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < str.size(); i++) {
        char tmp = str[i];
        if (tmp < 'A' || tmp > 'Z') {
            ss << tmp;
            continue;
        }
        if (i != 0) { // prevent string from starting with an underscore
            ss << "_";
        }
        tmp += 32; // 32 means upper case to lower case
        ss << tmp;
    }

    return ss.str();
}
} // namespace

HiAppEventConfig& HiAppEventConfig::GetInstance()
{
    static HiAppEventConfig instance;
    return instance;
}

bool HiAppEventConfig::SetConfigurationItem(std::string name, std::string value)
{
    // trans uppercase to underscore and lowercase
    name = TransUpperToUnderscoreAndLower(name);
    if (name == "") {
        HILOG_ERROR(LOG_CORE, "item name can not be empty.");
        return false;
    }

    std::transform(name.begin(), name.end(), name.begin(), ::tolower);
    if (value == "") {
        HILOG_ERROR(LOG_CORE, "item value can not be empty.");
        return false;
    }

    std::transform(value.begin(), value.end(), value.begin(), ::tolower);
    if (name == DISABLE) {
        return SetDisableItem(value);
    } else {
        HILOG_ERROR(LOG_CORE, "unrecognized configuration item name.");
        return false;
    }
}

bool HiAppEventConfig::SetDisableItem(const std::string& value)
{
    if (value == "true") {
        SetDisable(true);
    } else if (value == "false") {
        SetDisable(false);
    } else {
        HILOG_ERROR(LOG_CORE, "invalid bool value=%{public}s of the application dotting switch.", value.c_str());
        return false;
    }
    return true;
}

void HiAppEventConfig::SetDisable(bool disable)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    this->disable = disable;
}

bool HiAppEventConfig::GetDisable()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    return this->disable;
}

std::string HiAppEventConfig::GetRunningId()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (!this->runningId.empty()) {
        return this->runningId;
    }
    std::shared_ptr<OHOS::AbilityRuntime::Platform::ApplicationContext> context =
        OHOS::AbilityRuntime::Platform::ApplicationContext::GetApplicationContext();
    if (context == nullptr) {
        HILOG_ERROR(LOG_CORE, "Context is null.");
        return "";
    }
    this->runningId = context->GetAppRunningUniqueId();
    if (this->runningId.empty()) {
        HILOG_ERROR(LOG_CORE, "The running id from context is empty.");
    }
    return this->runningId;
}
} // namespace HiviewDFX
} // namespace OHOS
