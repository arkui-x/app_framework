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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPLICATION_INFO_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPLICATION_INFO_H

#include <map>
#include <string>
#include <vector>

#include "module_info.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string AVAILABLELEVEL_NORMAL = "normal";
const std::string DEFAULT_ENTITY_TYPE = "unspecified";
} // namespace

enum ApplicationFlag {
    GET_BASIC_APPLICATION_INFO = 0x00000000,
    GET_APPLICATION_INFO_WITH_PERMISSION = 0x00000008,
    GET_APPLICATION_INFO_WITH_METADATA = 0x00000040,
    GET_APPLICATION_INFO_WITH_DISABLE = 0x00000200,
    GET_APPLICATION_INFO_WITH_CERTIFICATE_FINGERPRINT = 0x00000400,
    GET_ALL_APPLICATION_INFO = 0xFFFF0000,
};

enum class GetApplicationFlag {
    GET_APPLICATION_INFO_DEFAULT = 0x00000000,
    GET_APPLICATION_INFO_WITH_PERMISSION = 0x00000001,
    GET_APPLICATION_INFO_WITH_METADATA = 0x00000002,
    GET_APPLICATION_INFO_WITH_DISABLE = 0x00000004,
};

enum class BundleType {
    APP = 0,
    ATOMIC_SERVICE = 1,
};

struct Metadata {
    std::string name;
    std::string value;
    std::string resource;
};

struct CustomizeData {
    std::string name;
    std::string value;
    std::string extra;
};

struct MetaData {
    std::vector<CustomizeData> customizeData;
};

struct Resource {
    /** the hap bundle name */
    std::string bundleName;

    /** the hap module name */
    std::string moduleName;

    /** the resource id in hap */
    int32_t id = 0;
};

// configuration information about an application
struct ApplicationInfo {
    std::string name; // application name is same to bundleName
    std::string bundleName;

    uint32_t versionCode = 0;
    std::string versionName;
    int32_t minCompatibleVersionCode = 0;

    uint32_t apiCompatibleVersion = 0;
    int32_t apiTargetVersion = 0;
    int64_t crowdtestDeadline = -1;

    std::string iconPath;
    int32_t iconId = 0;
    Resource iconResource;

    std::string label;
    int32_t labelId = 0;
    Resource labelResource;

    std::string description;
    int32_t descriptionId = 0;
    Resource descriptionResource;

    bool keepAlive = false;
    bool removable = true;
    bool singleton = false;
    bool userDataClearable = true;
    bool accessible = false;
    bool runningResourcesApply = false;
    bool associatedWakeUp = false;
    bool hideDesktopIcon = false;
    bool formVisibleNotify = false;
    std::vector<std::string> allowCommonEvent;

    bool isSystemApp = false;
    bool isLauncherApp = false;
    bool isFreeInstallApp = false;

    std::string codePath;
    std::string dataDir;
    std::string dataBaseDir;
    std::string cacheDir;
    std::string entryDir;

    std::string apiReleaseType;
    bool debug = false;
    std::string deviceId;
    bool distributedNotificationEnabled = true;
    std::string entityType;
    std::string process;
    int32_t supportedModes = 0; // returns 0 if the application does not support the driving mode
    std::string vendor;

    // apl
    std::string appPrivilegeLevel = AVAILABLELEVEL_NORMAL;
    // provision
    std::string appDistributionType = "none";
    std::string appProvisionType = "release";

    // user related fields, assign when calling the get interface
    uint32_t accessTokenId = 0;
    uint64_t accessTokenIdEx = 0;
    bool enabled = false;
    int32_t uid = -1;
    int32_t pid = -1;

    // native so
    std::string nativeLibraryPath;
    std::string cpuAbi;
    std::string arkNativeFilePath;
    std::string arkNativeFileAbi;

    // assign when calling the get interface
    std::vector<std::string> permissions;
    std::vector<std::string> moduleSourceDirs;
    std::vector<ModuleInfo> moduleInfos;
    std::map<std::string, std::vector<CustomizeData>> metaData;
    std::map<std::string, std::vector<Metadata>> metadata;
    // Installation-free
    std::vector<std::string> targetBundleList;

    std::string fingerprint;

    // unused
    std::string icon;
    int32_t flags = 0;
    std::string entryModuleName;
    bool isCompressNativeLibs = true;
    std::string signatureKey;

    // switch
    bool multiProjects = false;

    // app detail ability
    bool needAppDetail = false;
    std::string appDetailAbilityLibraryPath;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_APPLICATION_INFO_H
