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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_JSON_SERIALIZER_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_JSON_SERIALIZER_H

#include "bundle_info.h"
#include "bundle_user_info.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {

/*
 * form_json and to_json is global static overload method, which need callback by json library,
 * and can not rename this function, so don't named according UpperCamelCase style
 */
void to_json(nlohmann::json& jsonObject, const CustomizeData& customizeData);
void from_json(const nlohmann::json& jsonObject, CustomizeData& customizeData);
void to_json(nlohmann::json& jsonObject, const MetaData& metaData);
void from_json(const nlohmann::json& jsonObject, MetaData& metaData);
void to_json(nlohmann::json& jsonObject, const Metadata& metadata);
void from_json(const nlohmann::json& jsonObject, Metadata& metadata);
void to_json(nlohmann::json& jsonObject, const AbilityInfo& abilityInfo);
void from_json(const nlohmann::json& jsonObject, AbilityInfo& abilityInfo);
void to_json(nlohmann::json& jsonObject, const ApplicationInfo& applicationInfo);
void from_json(const nlohmann::json& jsonObject, ApplicationInfo& applicationInfo);
void to_json(nlohmann::json& jsonObject, const BundleInfo& bundleInfo);
void from_json(const nlohmann::json& jsonObject, BundleInfo& bundleInfo);
void to_json(nlohmann::json& jsonObject, const ModuleInfo& moduleInfo);
void from_json(const nlohmann::json& jsonObject, ModuleInfo& moduleInfo);
void to_json(nlohmann::json& jsonObject, const HapModuleInfo& hapModuleInfo);
void from_json(const nlohmann::json& jsonObject, HapModuleInfo& hapModuleInfo);
void to_json(nlohmann::json& jsonObject, const BundleUserInfo& bundleUserInfo);
void from_json(const nlohmann::json& jsonObject, BundleUserInfo& bundleUserInfo);
void to_json(nlohmann::json& jsonObject, const RequestPermissionUsedScene& usedScene);
void from_json(const nlohmann::json& jsonObject, RequestPermissionUsedScene& usedScene);
void to_json(nlohmann::json& jsonObject, const RequestPermission& requestPermission);
void from_json(const nlohmann::json& jsonObject, RequestPermission& requestPermission);
void to_json(nlohmann::json& jsonObject, const Resource& resource);
void from_json(const nlohmann::json& jsonObject, Resource& resource);
void to_json(nlohmann::json& jsonObject, const RouterItem& routerItem);
void from_json(const nlohmann::json& jsonObject, RouterItem& routerItem);
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_JSON_SERIALIZER_H
