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
#include "hiappevent_userinfo.h"

#include <mutex>
#include <string>

#include "hiappevent_base.h"
#include "hiappevent_verify.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "UserInfo"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {

namespace {
constexpr int DB_FAILED = -1;

std::mutex g_mutex;
} // namespace

UserInfo& UserInfo::GetInstance()
{
    static UserInfo userInfo;
    return userInfo;
}

UserInfo::UserInfo() {}

int UserInfo::SetUserId(const std::string& name, const std::string& value)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userIds_[name] = value;
    return 0;
}

int UserInfo::GetUserId(const std::string& name, std::string& out)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userIds_.find(name) == userIds_.end()) {
        HILOG_INFO(LOG_CORE, "no userid.");
        return 0;
    }
    out = userIds_.at(name);
    return 0;
}

int UserInfo::RemoveUserId(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userIds_.erase(name);
    return 0;
}

int UserInfo::SetUserProperty(const std::string& name, const std::string& value)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userProperties_[name] = value;
    return 0;
}

int UserInfo::GetUserProperty(const std::string& name, std::string& out)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    if (userProperties_.find(name) == userProperties_.end()) {
        HILOG_INFO(LOG_CORE, "no user property.");
        return 0;
    }
    out = userProperties_.at(name);
    return 0;
}

int UserInfo::RemoveUserProperty(const std::string& name)
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userProperties_.erase(name);
    return 0;
}

void UserInfo::ClearData()
{
    std::lock_guard<std::mutex> lockGuard(g_mutex);
    userIds_.clear();
    userProperties_.clear();
}
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
