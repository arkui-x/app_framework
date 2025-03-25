/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "bundle_parser.h"

#include <nlohmann/json.hpp>

#include "bundle_info.h"
#include "hilog.h"
#include "inner_bundle_info.h"
#include "json_serializer.h"
#include "module_profile.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
static constexpr const char* ROUTER_MAP = "routerMap";
static constexpr const char* ROUTER_MAP_DATA = "data";
static constexpr const char* ROUTER_ITEM_KEY_CUSTOM_DATA = "customData";
static constexpr const uint16_t DATA_MAX_LENGTH = 4096;
} // namespace
int32_t BundleParser::Parse(const std::vector<uint8_t>& buf, InnerBundleInfo& bundleInfo)
{
    bundleInfo.SetIsNewVersion(true);
    ModuleProfile moduleProfile;
    return moduleProfile.TransformTo(buf, bundleInfo);
}

bool BundleParser::CheckRouterData(nlohmann::json data) const
{
    if (data.find(ROUTER_MAP_DATA) == data.end()) {
        HILOG_WARN("data is not existed");
        return false;
    }
    if (!data.at(ROUTER_MAP_DATA).is_object()) {
        HILOG_WARN("data is not a json object");
        return false;
    }
    for (nlohmann::json::iterator kt = data.at(ROUTER_MAP_DATA).begin(); kt != data.at(ROUTER_MAP_DATA).end(); ++kt) {
        // check every value is string
        if (!kt.value().is_string()) {
            HILOG_WARN("Error: Value in data object for key %{public}s must be a string", kt.key().c_str());
            return false;
        }
    }
    return true;
}

ErrCode BundleParser::ParseRouterArray(const std::string& jsonString, std::vector<RouterItem>& routerArray) const
{
    if (jsonString.empty()) {
        HILOG_ERROR("jsonString is empty");
        return ERR_APPEXECFWK_PARSE_NO_PROFILE;
    }
    HILOG_DEBUG("Parse RouterItem from %{private}s", jsonString.c_str());
    nlohmann::json jsonBuf = nlohmann::json::parse(jsonString, nullptr, false);
    if (jsonBuf.is_discarded()) {
        HILOG_ERROR("json file %{private}s discarded", jsonString.c_str());
        return ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
    }
    if (jsonBuf.find(ROUTER_MAP) == jsonBuf.end()) {
        HILOG_ERROR("routerMap no exist");
        return ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
    }
    nlohmann::json routerJson = jsonBuf.at(ROUTER_MAP);
    if (!routerJson.is_array()) {
        HILOG_ERROR("json under routerMap is not a json array");
        return ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
    }

    for (const auto& object : routerJson) {
        if (!object.is_object()) {
            return ERR_APPEXECFWK_PARSE_PROFILE_PROP_TYPE_ERROR;
        }
        RouterItem routerItem;
        if (object.count(ROUTER_MAP_DATA) > 0 && !CheckRouterData(object)) {
            HILOG_WARN("check data type failed");
            continue;
        }
        from_json(object, routerItem);
        if (object.find(ROUTER_ITEM_KEY_CUSTOM_DATA) != object.end()) {
            if (object[ROUTER_ITEM_KEY_CUSTOM_DATA].dump().size() <= DATA_MAX_LENGTH) {
                routerItem.customData = object[ROUTER_ITEM_KEY_CUSTOM_DATA].dump();
            } else {
                HILOG_ERROR("customData in routerMap profile is too long");
            }
        }
        routerArray.emplace_back(routerItem);
    }
    return ERR_OK;
}
} // namespace AppExecFwk
} // namespace OHOS