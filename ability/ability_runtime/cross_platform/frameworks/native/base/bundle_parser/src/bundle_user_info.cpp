/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "bundle_user_info.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "json_util.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_USER_INFO_USER_ID = "userId";
const std::string BUNDLE_USER_INFO_ENABLE = "enabled";
const std::string BUNDLE_USER_INFO_DISABLE_ABILITIES = "disabledAbilities";
} // namespace
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(exp)            \
    ({                                     \
    decltype(exp) _rc;                     \
    do {                                   \
        _rc = (exp);                       \
    } while ((_rc == -1) && (errno == EINTR)); \
    _rc;                                   \
    })
#endif

void BundleUserInfo::Dump(const std::string& prefix, int fd)
{
    HILOG_INFO("called dump BundleUserInfo");
    if (fd < 0) {
        HILOG_ERROR("dump BundleUserInfo fd error");
        return;
    }
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        HILOG_ERROR("dump BundleUserInfo fcntl error %{public}s", strerror(errno));
        return;
    }
    uint uflags = static_cast<uint>(flags);
    uflags &= O_ACCMODE;
    if ((uflags == O_WRONLY) || (uflags == O_RDWR)) {
        nlohmann::json jsonObject = *this;
        std::string result;
        result.append(prefix);
        result.append(jsonObject.dump(Constants::DUMP_INDENT));
        int ret = TEMP_FAILURE_RETRY(write(fd, result.c_str(), result.size()));
        if (ret < 0) {
            HILOG_ERROR("dump BundleUserInfo write error %{public}s", strerror(errno));
        }
    }
}

bool BundleUserInfo::IsInitialState() const
{
    return enabled && disabledAbilities.empty();
}

void BundleUserInfo::Reset()
{
    enabled = true;
    disabledAbilities.clear();
}

void to_json(nlohmann::json& jsonObject, const BundleUserInfo& bundleUserInfo)
{
    jsonObject = nlohmann::json {
        { BUNDLE_USER_INFO_USER_ID, bundleUserInfo.userId },
        { BUNDLE_USER_INFO_ENABLE, bundleUserInfo.enabled },
        { BUNDLE_USER_INFO_DISABLE_ABILITIES, bundleUserInfo.disabledAbilities },
    };
}

void from_json(const nlohmann::json& jsonObject, BundleUserInfo& bundleUserInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, BUNDLE_USER_INFO_USER_ID, bundleUserInfo.userId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_USER_INFO_ENABLE, bundleUserInfo.enabled,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_USER_INFO_DISABLE_ABILITIES,
        bundleUserInfo.disabledAbilities, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
}
} // namespace AppExecFwk
} // namespace OHOS