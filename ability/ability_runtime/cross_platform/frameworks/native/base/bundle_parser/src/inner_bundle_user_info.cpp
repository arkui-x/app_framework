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

#include "inner_bundle_user_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string INNER_BUNDLE_USER_INFO_UID = "uid";
const std::string INNER_BUNDLE_USER_INFO_GIDS = "gids";
const std::string INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID = "accessTokenId";
const std::string INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID_EX = "accessTokenIdEx";
const std::string INNER_BUNDLE_USER_INFO_BUNDLE_NAME = "bundleName";
const std::string INNER_BUNDLE_USER_INFO_INSTALL_TIME = "installTime";
const std::string INNER_BUNDLE_USER_INFO_UPDATE_TIME = "updateTime";
const std::string INNER_BUNDLE_USER_INFO_BUNDLE_USER_INFO = "bundleUserInfo";
} // namespace

void to_json(nlohmann::json& jsonObject, const InnerBundleUserInfo& innerBundleUserInfo)
{
    jsonObject = nlohmann::json {
        { INNER_BUNDLE_USER_INFO_UID, innerBundleUserInfo.uid },
        { INNER_BUNDLE_USER_INFO_GIDS, innerBundleUserInfo.gids },
        { INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID, innerBundleUserInfo.accessTokenId },
        { INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID_EX, innerBundleUserInfo.accessTokenIdEx },
        { INNER_BUNDLE_USER_INFO_BUNDLE_NAME, innerBundleUserInfo.bundleName },
        { INNER_BUNDLE_USER_INFO_INSTALL_TIME, innerBundleUserInfo.installTime },
        { INNER_BUNDLE_USER_INFO_UPDATE_TIME, innerBundleUserInfo.updateTime },
        { INNER_BUNDLE_USER_INFO_BUNDLE_USER_INFO, innerBundleUserInfo.bundleUserInfo },
    };
}

void from_json(const nlohmann::json& jsonObject, InnerBundleUserInfo& innerBundleUserInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_UID, innerBundleUserInfo.uid,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<int32_t>>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_GIDS,
        innerBundleUserInfo.gids, JsonType::ARRAY, false, parseResult, ArrayType::NUMBER);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID,
        innerBundleUserInfo.accessTokenId, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint64_t>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_ACCESS_TOKEN_ID_EX,
        innerBundleUserInfo.accessTokenIdEx, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_BUNDLE_NAME,
        innerBundleUserInfo.bundleName, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_INSTALL_TIME,
        innerBundleUserInfo.installTime, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_UPDATE_TIME,
        innerBundleUserInfo.updateTime, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<BundleUserInfo>(jsonObject, jsonObjectEnd, INNER_BUNDLE_USER_INFO_BUNDLE_USER_INFO,
        innerBundleUserInfo.bundleUserInfo, JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
}
} // namespace AppExecFwk
} // namespace OHOS