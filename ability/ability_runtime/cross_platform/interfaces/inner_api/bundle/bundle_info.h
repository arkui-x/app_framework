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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_INFO_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_INFO_H

#include <string>
#include <vector>

#include "ability_info.h"
#include "application_info.h"
#include "hap_module_info.h"

namespace OHOS {
namespace AppExecFwk {
enum BundleFlag {
    // get bundle info except abilityInfos
    GET_BUNDLE_DEFAULT = 0x00000000,
    // get bundle info include abilityInfos
    GET_BUNDLE_WITH_ABILITIES = 0x00000001,
    // get bundle info include request permissions
    GET_BUNDLE_WITH_REQUESTED_PERMISSION = 0x00000010,
    // get bundle info include extension info
    GET_BUNDLE_WITH_EXTENSION_INFO = 0x00000020,
    // get bundle info include hash value
    GET_BUNDLE_WITH_HASH_VALUE = 0x00000030,
    // get bundle info inlcude router map, only for dump usage
    GET_BUNDLE_WITH_ROUTER_MAP = 0x00000080,
};

enum class GetBundleInfoFlag {
    GET_BUNDLE_INFO_DEFAULT = 0x00000000,
    GET_BUNDLE_INFO_WITH_APPLICATION = 0x00000001,
    GET_BUNDLE_INFO_WITH_HAP_MODULE = 0x00000002,
    GET_BUNDLE_INFO_WITH_ABILITY = 0x00000004,
    GET_BUNDLE_INFO_WITH_EXTENSION_ABILITY = 0x00000008,
    GET_BUNDLE_INFO_WITH_REQUESTED_PERMISSION = 0x00000010,
    GET_BUNDLE_INFO_WITH_METADATA = 0x00000020,
    GET_BUNDLE_INFO_WITH_DISABLE = 0x00000040,
    GET_BUNDLE_INFO_WITH_SIGNATURE_INFO = 0x00000080,
    GET_BUNDLE_INFO_WITH_ROUTER_MAP = 0x00000200,
};

struct RequestPermissionUsedScene {
    std::vector<std::string> abilities;
    std::string when;
};

struct RequestPermission {
    std::string name;
    std::string reason;
    int32_t reasonId = 0;
    RequestPermissionUsedScene usedScene;
};

struct SignatureInfo {
    std::string appId;
    std::string fingerprint;
};

// configuration information about a bundle
struct BundleInfo {
    std::string name;

    uint32_t versionCode = 0;
    std::string versionName;
    uint32_t minCompatibleVersionCode = 0;

    uint32_t compatibleVersion = 0;
    uint32_t targetVersion = 0;

    bool isKeepAlive = false;
    bool singleton = false;
    bool isPreInstallApp = false;

    std::string vendor;
    std::string releaseType;
    bool isNativeApp = false;

    std::string mainEntry;              // modulePackage
    std::string entryModuleName;        // moduleName
    bool entryInstallationFree = false; // application : false; atomic service : true
    bool asanEnabled = false;

    std::string appId;

    // user related fields, assign when calling the get interface
    int uid = -1;
    int gid = -1;
    int64_t installTime = 0;
    int64_t updateTime = 0;
    int32_t appIndex = 0; // index for sandbox app

    ApplicationInfo applicationInfo;
    std::vector<AbilityInfo> abilityInfos;
    std::vector<HapModuleInfo> hapModuleInfos;
    std::vector<std::string> hapModuleNames;   // the "module.package" in each config.json
    std::vector<std::string> moduleNames;      // the "module.name" in each config.json
    std::vector<std::string> modulePublicDirs; // the public paths of all modules of the application.
    std::vector<std::string> moduleDirs;       // the paths of all modules of the application.
    std::vector<std::string> moduleResPaths;   // the paths of all resources paths.

    std::vector<std::string> reqPermissions;
    std::vector<std::string> defPermissions;
    std::vector<int32_t> reqPermissionStates;
    std::vector<RequestPermission> reqPermissionDetails;

    // unused
    std::string cpuAbi;
    std::string seInfo;
    std::string label;
    std::string description;
    std::string jointUserId;
    int32_t minSdkVersion = -1;
    int32_t maxSdkVersion = -1;
    bool isDifferentName = false;

    SignatureInfo signatureInfo;

    std::vector<RouterItem> routerArray;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_INFO_H
