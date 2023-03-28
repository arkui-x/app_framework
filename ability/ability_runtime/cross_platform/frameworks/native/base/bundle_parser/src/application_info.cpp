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

#include "application_info.h"

#include <errno.h>
#include <fcntl.h>
#include <set>
#include <string.h>
#include <unistd.h>

#include "bundle_constants.h"
#include "json_serializer.h"
#include "json_util.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string APPLICATION_NAME = "name";
const std::string APPLICATION_VERSION_CODE = "versionCode";
const std::string APPLICATION_VERSION_NAME = "versionName";
const std::string APPLICATION_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
const std::string APPLICATION_API_COMPATIBLE_VERSION = "apiCompatibleVersion";
const std::string APPLICATION_API_TARGET_VERSION = "apiTargetVersion";
const std::string APPLICATION_ICON_PATH = "iconPath";
const std::string APPLICATION_ICON_ID = "iconId";
const std::string APPLICATION_LABEL = "label";
const std::string APPLICATION_LABEL_ID = "labelId";
const std::string APPLICATION_DESCRIPTION = "description";
const std::string APPLICATION_DESCRIPTION_ID = "descriptionId";
const std::string APPLICATION_KEEP_ALIVE = "keepAlive";
const std::string APPLICATION_REMOVABLE = "removable";
const std::string APPLICATION_SINGLETON = "singleton";
const std::string APPLICATION_USER_DATA_CLEARABLE = "userDataClearable";
const std::string APPLICATION_IS_SYSTEM_APP = "isSystemApp";
const std::string APPLICATION_IS_LAUNCHER_APP = "isLauncherApp";
const std::string APPLICATION_IS_FREEINSTALL_APP = "isFreeInstallApp";
const std::string APPLICATION_RUNNING_RESOURCES_APPLY = "runningResourcesApply";
const std::string APPLICATION_ASSOCIATED_WAKE_UP = "associatedWakeUp";
const std::string APPLICATION_HIDE_DESKTOP_ICON = "hideDesktopIcon";
const std::string APPLICATION_FORM_VISIBLE_NOTIFY = "formVisibleNotify";
const std::string APPLICATION_ALLOW_COMMON_EVENT = "allowCommonEvent";
const std::string APPLICATION_CODE_PATH = "codePath";
const std::string APPLICATION_DATA_DIR = "dataDir";
const std::string APPLICATION_DATA_BASE_DIR = "dataBaseDir";
const std::string APPLICATION_CACHE_DIR = "cacheDir";
const std::string APPLICATION_ENTRY_DIR = "entryDir";
const std::string APPLICATION_API_RELEASETYPE = "apiReleaseType";
const std::string APPLICATION_DEBUG = "debug";
const std::string APPLICATION_DEVICE_ID = "deviceId";
const std::string APPLICATION_DISTRIBUTED_NOTIFICATION_ENABLED = "distributedNotificationEnabled";
const std::string APPLICATION_ENTITY_TYPE = "entityType";
const std::string APPLICATION_PROCESS = "process";
const std::string APPLICATION_SUPPORTED_MODES = "supportedModes";
const std::string APPLICATION_VENDOR = "vendor";
const std::string APPLICATION_ACCESSIBLE = "accessible";
const std::string APPLICATION_PRIVILEGE_LEVEL = "appPrivilegeLevel";
const std::string APPLICATION_ACCESSTOKEN_ID = "accessTokenId";
const std::string APPLICATION_ACCESSTOKEN_ID_EX = "accessTokenIdEx";
const std::string APPLICATION_ENABLED = "enabled";
const std::string APPLICATION_UID = "uid";
const std::string APPLICATION_PERMISSIONS = "permissions";
const std::string APPLICATION_MODULE_SOURCE_DIRS = "moduleSourceDirs";
const std::string APPLICATION_MODULE_INFOS = "moduleInfos";
const std::string APPLICATION_META_DATA_CONFIG_JSON = "metaData";
const std::string APPLICATION_META_DATA_MODULE_JSON = "metadata";
const std::string APPLICATION_FINGERPRINT = "fingerprint";
const std::string APPLICATION_ICON = "icon";
const std::string APPLICATION_FLAGS = "flags";
const std::string APPLICATION_ENTRY_MODULE_NAME = "entryModuleName";
const std::string APPLICATION_NATIVE_LIBRARY_PATH = "nativeLibraryPath";
const std::string APPLICATION_CPU_ABI = "cpuAbi";
const std::string APPLICATION_ARK_NATIVE_FILE_PATH = "arkNativeFilePath";
const std::string APPLICATION_ARK_NATIVE_FILE_ABI = "arkNativeFileAbi";
const std::string APPLICATION_IS_COMPRESS_NATIVE_LIBS = "isCompressNativeLibs";
const std::string APPLICATION_SIGNATURE_KEY = "signatureKey";
const std::string APPLICATION_TARGETBUNDLELIST = "targetBundleList";
const std::string APPLICATION_APP_DISTRIBUTION_TYPE = "appDistributionType";
const std::string APPLICATION_APP_PROVISION_TYPE = "appProvisionType";
const std::string APPLICATION_ICON_RESOURCE = "iconResource";
const std::string APPLICATION_LABEL_RESOURCE = "labelResource";
const std::string APPLICATION_DESCRIPTION_RESOURCE = "descriptionResource";
const std::string APPLICATION_MULTI_PROJECTS = "multiProjects";
const std::string APPLICATION_CROWDTEST_DEADLINE = "crowdtestDeadline";
const std::string APPLICATION_APP_QUICK_FIX = "appQuickFix";
const std::string RESOURCE_ID = "id";
const size_t APPLICATION_CAPACITY = 10240; // 10K
const std::string APPLICATION_NEED_APP_DETAIL = "needAppDetail";
const std::string APPLICATION_APP_DETAIL_ABILITY_LIBRARY_PATH = "appDetailAbilityLibraryPath";
const std::string APPLICATION_APP_TARGET_BUNDLE_NAME = "targetBundleName";
const std::string APPLICATION_APP_TARGET_PRIORITY = "targetPriority";
const std::string APPLICATION_ASAN_ENABLED = "asanEnabled";
const std::string APPLICATION_ASAN_LOG_PATH = "asanLogPath";
const std::string APPLICATION_SPLIT = "split";
const std::string APPLICATION_APP_TYPE = "bundleType";
} // namespace

void to_json(nlohmann::json& jsonObject, const Resource& resource)
{
    jsonObject = nlohmann::json { { Constants::BUNDLE_NAME, resource.bundleName },
        { Constants::MODULE_NAME, resource.moduleName }, { RESOURCE_ID, resource.id } };
}

void from_json(const nlohmann::json& jsonObject, Resource& resource)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::BUNDLE_NAME, resource.bundleName,
        JsonType::STRING, true, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::MODULE_NAME, resource.moduleName,
        JsonType::STRING, true, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(
        jsonObject, jsonObjectEnd, RESOURCE_ID, resource.id, JsonType::NUMBER, true, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read Resource from database error, error code : %{public}d", parseResult);
    }
}

void to_json(nlohmann::json& jsonObject, const ApplicationInfo& applicationInfo)
{
    jsonObject = nlohmann::json {
        { APPLICATION_NAME, applicationInfo.name },
        { Constants::BUNDLE_NAME, applicationInfo.bundleName },
        { APPLICATION_VERSION_CODE, applicationInfo.versionCode },
        { APPLICATION_VERSION_NAME, applicationInfo.versionName },
        { APPLICATION_MIN_COMPATIBLE_VERSION_CODE, applicationInfo.minCompatibleVersionCode },
        { APPLICATION_API_COMPATIBLE_VERSION, applicationInfo.apiCompatibleVersion },
        { APPLICATION_API_TARGET_VERSION, applicationInfo.apiTargetVersion },
        { APPLICATION_ICON_PATH, applicationInfo.iconPath },
        { APPLICATION_ICON_ID, applicationInfo.iconId },
        { APPLICATION_LABEL, applicationInfo.label },
        { APPLICATION_LABEL_ID, applicationInfo.labelId },
        { APPLICATION_DESCRIPTION, applicationInfo.description },
        { APPLICATION_DESCRIPTION_ID, applicationInfo.descriptionId },
        { APPLICATION_KEEP_ALIVE, applicationInfo.keepAlive },
        { APPLICATION_REMOVABLE, applicationInfo.removable },
        { APPLICATION_SINGLETON, applicationInfo.singleton },
        { APPLICATION_USER_DATA_CLEARABLE, applicationInfo.userDataClearable },
        { APPLICATION_ACCESSIBLE, applicationInfo.accessible },
        { APPLICATION_IS_SYSTEM_APP, applicationInfo.isSystemApp },
        { APPLICATION_IS_LAUNCHER_APP, applicationInfo.isLauncherApp },
        { APPLICATION_IS_FREEINSTALL_APP, applicationInfo.isFreeInstallApp },
        { APPLICATION_RUNNING_RESOURCES_APPLY, applicationInfo.runningResourcesApply },
        { APPLICATION_ASSOCIATED_WAKE_UP, applicationInfo.associatedWakeUp },
        { APPLICATION_HIDE_DESKTOP_ICON, applicationInfo.hideDesktopIcon },
        { APPLICATION_FORM_VISIBLE_NOTIFY, applicationInfo.formVisibleNotify },
        { APPLICATION_ALLOW_COMMON_EVENT, applicationInfo.allowCommonEvent },
        { APPLICATION_CODE_PATH, applicationInfo.codePath },
        { APPLICATION_DATA_DIR, applicationInfo.dataDir },
        { APPLICATION_DATA_BASE_DIR, applicationInfo.dataBaseDir },
        { APPLICATION_CACHE_DIR, applicationInfo.cacheDir },
        { APPLICATION_ENTRY_DIR, applicationInfo.entryDir },
        { APPLICATION_API_RELEASETYPE, applicationInfo.apiReleaseType },
        { APPLICATION_DEBUG, applicationInfo.debug },
        { APPLICATION_DEVICE_ID, applicationInfo.deviceId },
        { APPLICATION_DISTRIBUTED_NOTIFICATION_ENABLED, applicationInfo.distributedNotificationEnabled },
        { APPLICATION_ENTITY_TYPE, applicationInfo.entityType },
        { APPLICATION_PROCESS, applicationInfo.process },
        { APPLICATION_SUPPORTED_MODES, applicationInfo.supportedModes },
        { APPLICATION_VENDOR, applicationInfo.vendor },
        { APPLICATION_PRIVILEGE_LEVEL, applicationInfo.appPrivilegeLevel },
        { APPLICATION_ACCESSTOKEN_ID, applicationInfo.accessTokenId },
        { APPLICATION_ACCESSTOKEN_ID_EX, applicationInfo.accessTokenIdEx },
        { APPLICATION_ENABLED, applicationInfo.enabled },
        { APPLICATION_UID, applicationInfo.uid },
        { APPLICATION_PERMISSIONS, applicationInfo.permissions },
        { APPLICATION_MODULE_SOURCE_DIRS, applicationInfo.moduleSourceDirs },
        { APPLICATION_MODULE_INFOS, applicationInfo.moduleInfos },
        { APPLICATION_META_DATA_CONFIG_JSON, applicationInfo.metaData },
        { APPLICATION_META_DATA_MODULE_JSON, applicationInfo.metadata },
        { APPLICATION_FINGERPRINT, applicationInfo.fingerprint },
        { APPLICATION_ICON, applicationInfo.icon },
        { APPLICATION_FLAGS, applicationInfo.flags },
        { APPLICATION_ENTRY_MODULE_NAME, applicationInfo.entryModuleName },
        { APPLICATION_NATIVE_LIBRARY_PATH, applicationInfo.nativeLibraryPath },
        { APPLICATION_CPU_ABI, applicationInfo.cpuAbi },
        { APPLICATION_ARK_NATIVE_FILE_PATH, applicationInfo.arkNativeFilePath },
        { APPLICATION_ARK_NATIVE_FILE_ABI, applicationInfo.arkNativeFileAbi },
        { APPLICATION_IS_COMPRESS_NATIVE_LIBS, applicationInfo.isCompressNativeLibs },
        { APPLICATION_SIGNATURE_KEY, applicationInfo.signatureKey },
        { APPLICATION_TARGETBUNDLELIST, applicationInfo.targetBundleList },
        { APPLICATION_APP_DISTRIBUTION_TYPE, applicationInfo.appDistributionType },
        { APPLICATION_APP_PROVISION_TYPE, applicationInfo.appProvisionType },
        { APPLICATION_ICON_RESOURCE, applicationInfo.iconResource },
        { APPLICATION_LABEL_RESOURCE, applicationInfo.labelResource },
        { APPLICATION_DESCRIPTION_RESOURCE, applicationInfo.descriptionResource },
        { APPLICATION_MULTI_PROJECTS, applicationInfo.multiProjects },
        { APPLICATION_CROWDTEST_DEADLINE, applicationInfo.crowdtestDeadline },
        { APPLICATION_NEED_APP_DETAIL, applicationInfo.needAppDetail },
        { APPLICATION_APP_DETAIL_ABILITY_LIBRARY_PATH, applicationInfo.appDetailAbilityLibraryPath },
    };
}

void from_json(const nlohmann::json& jsonObject, ApplicationInfo& applicationInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_NAME, applicationInfo.name, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::BUNDLE_NAME, applicationInfo.bundleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, APPLICATION_VERSION_CODE, applicationInfo.versionCode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_VERSION_NAME, applicationInfo.versionName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_MIN_COMPATIBLE_VERSION_CODE,
        applicationInfo.minCompatibleVersionCode, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_API_COMPATIBLE_VERSION,
        applicationInfo.apiCompatibleVersion, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_API_TARGET_VERSION,
        applicationInfo.apiTargetVersion, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ICON_PATH, applicationInfo.iconPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_ICON_ID, applicationInfo.iconId, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_LABEL, applicationInfo.label,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_LABEL_ID, applicationInfo.labelId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_DESCRIPTION, applicationInfo.description,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, APPLICATION_DESCRIPTION_ID, applicationInfo.descriptionId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_KEEP_ALIVE, applicationInfo.keepAlive,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_REMOVABLE, applicationInfo.removable,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_SINGLETON, applicationInfo.singleton,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_USER_DATA_CLEARABLE,
        applicationInfo.userDataClearable, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_ACCESSIBLE, applicationInfo.accessible,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_IS_SYSTEM_APP, applicationInfo.isSystemApp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_IS_LAUNCHER_APP, applicationInfo.isLauncherApp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_IS_FREEINSTALL_APP, applicationInfo.isFreeInstallApp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_RUNNING_RESOURCES_APPLY,
        applicationInfo.runningResourcesApply, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_ASSOCIATED_WAKE_UP, applicationInfo.associatedWakeUp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_HIDE_DESKTOP_ICON, applicationInfo.hideDesktopIcon,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_FORM_VISIBLE_NOTIFY,
        applicationInfo.formVisibleNotify, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, APPLICATION_ALLOW_COMMON_EVENT,
        applicationInfo.allowCommonEvent, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_CODE_PATH, applicationInfo.codePath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_DATA_DIR, applicationInfo.dataDir,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_DATA_BASE_DIR, applicationInfo.dataBaseDir,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_CACHE_DIR, applicationInfo.cacheDir,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ENTRY_DIR, applicationInfo.entryDir,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_API_RELEASETYPE,
        applicationInfo.apiReleaseType, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_DEBUG, applicationInfo.debug, JsonType::BOOLEAN,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_DEVICE_ID, applicationInfo.deviceId,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_DISTRIBUTED_NOTIFICATION_ENABLED,
        applicationInfo.distributedNotificationEnabled, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ENTITY_TYPE, applicationInfo.entityType,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_PROCESS, applicationInfo.process,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, APPLICATION_SUPPORTED_MODES, applicationInfo.supportedModes,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_VENDOR, applicationInfo.vendor,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_PRIVILEGE_LEVEL,
        applicationInfo.appPrivilegeLevel, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, APPLICATION_ACCESSTOKEN_ID, applicationInfo.accessTokenId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint64_t>(jsonObject, jsonObjectEnd, APPLICATION_ACCESSTOKEN_ID_EX,
        applicationInfo.accessTokenIdEx, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_ENABLED, applicationInfo.enabled, JsonType::BOOLEAN,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, APPLICATION_UID, applicationInfo.uid, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, APPLICATION_PERMISSIONS,
        applicationInfo.permissions, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, APPLICATION_MODULE_SOURCE_DIRS,
        applicationInfo.moduleSourceDirs, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<ModuleInfo>>(jsonObject, jsonObjectEnd, APPLICATION_MODULE_INFOS,
        applicationInfo.moduleInfos, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<std::map<std::string, std::vector<CustomizeData>>>(jsonObject, jsonObjectEnd,
        APPLICATION_META_DATA_CONFIG_JSON, applicationInfo.metaData, JsonType::OBJECT, false, parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, std::vector<Metadata>>>(jsonObject, jsonObjectEnd,
        APPLICATION_META_DATA_MODULE_JSON, applicationInfo.metadata, JsonType::OBJECT, false, parseResult,
        ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_FINGERPRINT, applicationInfo.fingerprint,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ICON, applicationInfo.icon, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, APPLICATION_FLAGS, applicationInfo.flags, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ENTRY_MODULE_NAME,
        applicationInfo.entryModuleName, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_NATIVE_LIBRARY_PATH,
        applicationInfo.nativeLibraryPath, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_CPU_ABI, applicationInfo.cpuAbi,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ARK_NATIVE_FILE_PATH,
        applicationInfo.arkNativeFilePath, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_ARK_NATIVE_FILE_ABI,
        applicationInfo.arkNativeFileAbi, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_IS_COMPRESS_NATIVE_LIBS,
        applicationInfo.isCompressNativeLibs, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_SIGNATURE_KEY, applicationInfo.signatureKey,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, APPLICATION_TARGETBUNDLELIST,
        applicationInfo.targetBundleList, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_APP_DISTRIBUTION_TYPE,
        applicationInfo.appDistributionType, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_APP_PROVISION_TYPE,
        applicationInfo.appProvisionType, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Resource>(jsonObject, jsonObjectEnd, APPLICATION_ICON_RESOURCE, applicationInfo.iconResource,
        JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Resource>(jsonObject, jsonObjectEnd, APPLICATION_LABEL_RESOURCE, applicationInfo.labelResource,
        JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<Resource>(jsonObject, jsonObjectEnd, APPLICATION_DESCRIPTION_RESOURCE,
        applicationInfo.descriptionResource, JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_MULTI_PROJECTS, applicationInfo.multiProjects,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject, jsonObjectEnd, APPLICATION_CROWDTEST_DEADLINE,
        applicationInfo.crowdtestDeadline, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, APPLICATION_NEED_APP_DETAIL, applicationInfo.needAppDetail,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, APPLICATION_APP_DETAIL_ABILITY_LIBRARY_PATH,
        applicationInfo.appDetailAbilityLibraryPath, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("from_json error, error code : %{public}d", parseResult);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
