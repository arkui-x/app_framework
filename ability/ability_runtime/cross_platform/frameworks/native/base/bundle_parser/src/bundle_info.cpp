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

#include "bundle_info.h"

#include "json_util.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string BUNDLE_INFO_NAME = "name";
const std::string BUNDLE_INFO_LABEL = "label";
const std::string BUNDLE_INFO_DESCRIPTION = "description";
const std::string BUNDLE_INFO_VENDOR = "vendor";
const std::string BUNDLE_INFO_IS_KEEP_ALIVE = "isKeepAlive";
const std::string BUNDLE_INFO_SINGLETON = "singleton";
const std::string BUNDLE_INFO_IS_NATIVE_APP = "isNativeApp";
const std::string BUNDLE_INFO_IS_PREINSTALL_APP = "isPreInstallApp";
const std::string BUNDLE_INFO_IS_DIFFERENT_NAME = "isDifferentName";
const std::string BUNDLE_INFO_HAP_MODULE_INFOS = "hapModuleInfos";
const std::string BUNDLE_INFO_JOINT_USERID = "jointUserId";
const std::string BUNDLE_INFO_VERSION_CODE = "versionCode";
const std::string BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
const std::string BUNDLE_INFO_VERSION_NAME = "versionName";
const std::string BUNDLE_INFO_MIN_SDK_VERSION = "minSdkVersion";
const std::string BUNDLE_INFO_MAX_SDK_VERSION = "maxSdkVersion";
const std::string BUNDLE_INFO_MAIN_ENTRY = "mainEntry";
const std::string BUNDLE_INFO_CPU_ABI = "cpuAbi";
const std::string BUNDLE_INFO_APPID = "appId";
const std::string BUNDLE_INFO_COMPATIBLE_VERSION = "compatibleVersion";
const std::string BUNDLE_INFO_TARGET_VERSION = "targetVersion";
const std::string BUNDLE_INFO_RELEASE_TYPE = "releaseType";
const std::string BUNDLE_INFO_UID = "uid";
const std::string BUNDLE_INFO_GID = "gid";
const std::string BUNDLE_INFO_SEINFO = "seInfo";
const std::string BUNDLE_INFO_INSTALL_TIME = "installTime";
const std::string BUNDLE_INFO_UPDATE_TIME = "updateTime";
const std::string BUNDLE_INFO_ENTRY_MODULE_NAME = "entryModuleName";
const std::string BUNDLE_INFO_ENTRY_INSTALLATION_FREE = "entryInstallationFree";
const std::string BUNDLE_INFO_REQ_PERMISSIONS = "reqPermissions";
const std::string BUNDLE_INFO_REQ_PERMISSION_STATES = "reqPermissionStates";
const std::string BUNDLE_INFO_REQ_PERMISSION_DETAILS = "reqPermissionDetails";
const std::string BUNDLE_INFO_DEF_PERMISSIONS = "defPermissions";
const std::string BUNDLE_INFO_HAP_MODULE_NAMES = "hapModuleNames";
const std::string BUNDLE_INFO_MODULE_NAMES = "moduleNames";
const std::string BUNDLE_INFO_MODULE_PUBLIC_DIRS = "modulePublicDirs";
const std::string BUNDLE_INFO_MODULE_DIRS = "moduleDirs";
const std::string BUNDLE_INFO_MODULE_RES_PATHS = "moduleResPaths";
const std::string REQUESTPERMISSION_NAME = "name";
const std::string REQUESTPERMISSION_REASON = "reason";
const std::string REQUESTPERMISSION_REASON_ID = "reasonId";
const std::string REQUESTPERMISSION_USEDSCENE = "usedScene";
const std::string REQUESTPERMISSION_ABILITIES = "abilities";
const std::string REQUESTPERMISSION_ABILITY = "ability";
const std::string REQUESTPERMISSION_WHEN = "when";
const std::string SIGNATUREINFO_APPID = "appId";
const std::string SIGNATUREINFO_FINGERPRINT = "fingerprint";
const std::string BUNDLE_INFO_APP_INDEX = "appIndex";
const std::string BUNDLE_INFO_SIGNATURE_INFO = "signatureInfo";
const std::string OVERLAY_TYPE = "overlayType";
const std::string BUNDLE_INFO_ASAN_ENABLED = "asanEnabled";
const std::string OVERLAY_BUNDLE_INFO = "overlayBundleInfos";
const size_t BUNDLE_CAPACITY = 10240; // 10K
const char* BUNDLE_INFO_ROUTER_ARRAY = "routerArray";
} // namespace

void to_json(nlohmann::json& jsonObject, const RequestPermissionUsedScene& usedScene)
{
    jsonObject = nlohmann::json { { REQUESTPERMISSION_ABILITIES, usedScene.abilities },
        { REQUESTPERMISSION_WHEN, usedScene.when } };
}

void to_json(nlohmann::json& jsonObject, const RequestPermission& requestPermission)
{
    jsonObject = nlohmann::json { { REQUESTPERMISSION_NAME, requestPermission.name },
        { REQUESTPERMISSION_REASON, requestPermission.reason },
        { REQUESTPERMISSION_REASON_ID, requestPermission.reasonId },
        { REQUESTPERMISSION_USEDSCENE, requestPermission.usedScene } };
}

void to_json(nlohmann::json& jsonObject, const SignatureInfo& signatureInfo)
{
    jsonObject = nlohmann::json { { SIGNATUREINFO_APPID, signatureInfo.appId },
        { SIGNATUREINFO_FINGERPRINT, signatureInfo.fingerprint } };
}

void from_json(const nlohmann::json& jsonObject, RequestPermissionUsedScene& usedScene)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_ABILITIES,
        usedScene.abilities, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_ABILITY,
        usedScene.abilities, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_WHEN, usedScene.when, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read RequestPermissionUsedScene from database error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json& jsonObject, RequestPermission& requestPermission)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_NAME, requestPermission.name,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_REASON, requestPermission.reason,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_REASON_ID, requestPermission.reasonId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<RequestPermissionUsedScene>(jsonObject, jsonObjectEnd, REQUESTPERMISSION_USEDSCENE,
        requestPermission.usedScene, JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read RequestPermission from database error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json& jsonObject, SignatureInfo& signatureInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, SIGNATUREINFO_APPID, signatureInfo.appId,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, SIGNATUREINFO_FINGERPRINT, signatureInfo.fingerprint,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read SignatureInfo from database error, error code : %{public}d", parseResult);
    }
}

void to_json(nlohmann::json& jsonObject, const BundleInfo& bundleInfo)
{
    jsonObject = nlohmann::json {
        { BUNDLE_INFO_NAME, bundleInfo.name },
        { BUNDLE_INFO_LABEL, bundleInfo.label },
        { BUNDLE_INFO_DESCRIPTION, bundleInfo.description },
        { BUNDLE_INFO_VENDOR, bundleInfo.vendor },
        { BUNDLE_INFO_IS_KEEP_ALIVE, bundleInfo.isKeepAlive },
        { BUNDLE_INFO_IS_NATIVE_APP, bundleInfo.isNativeApp },
        { BUNDLE_INFO_IS_PREINSTALL_APP, bundleInfo.isPreInstallApp },
        { BUNDLE_INFO_IS_DIFFERENT_NAME, bundleInfo.isDifferentName },
        { BUNDLE_INFO_HAP_MODULE_INFOS, bundleInfo.hapModuleInfos },
        { BUNDLE_INFO_JOINT_USERID, bundleInfo.jointUserId },
        { BUNDLE_INFO_VERSION_CODE, bundleInfo.versionCode },
        { BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE, bundleInfo.minCompatibleVersionCode },
        { BUNDLE_INFO_VERSION_NAME, bundleInfo.versionName },
        { BUNDLE_INFO_MIN_SDK_VERSION, bundleInfo.minSdkVersion },
        { BUNDLE_INFO_MAX_SDK_VERSION, bundleInfo.maxSdkVersion },
        { BUNDLE_INFO_MAIN_ENTRY, bundleInfo.mainEntry },
        { BUNDLE_INFO_CPU_ABI, bundleInfo.cpuAbi },
        { BUNDLE_INFO_APPID, bundleInfo.appId },
        { BUNDLE_INFO_COMPATIBLE_VERSION, bundleInfo.compatibleVersion },
        { BUNDLE_INFO_TARGET_VERSION, bundleInfo.targetVersion },
        { BUNDLE_INFO_RELEASE_TYPE, bundleInfo.releaseType },
        { BUNDLE_INFO_UID, bundleInfo.uid },
        { BUNDLE_INFO_GID, bundleInfo.gid },
        { BUNDLE_INFO_SEINFO, bundleInfo.seInfo },
        { BUNDLE_INFO_INSTALL_TIME, bundleInfo.installTime },
        { BUNDLE_INFO_UPDATE_TIME, bundleInfo.updateTime },
        { BUNDLE_INFO_ENTRY_MODULE_NAME, bundleInfo.entryModuleName },
        { BUNDLE_INFO_ENTRY_INSTALLATION_FREE, bundleInfo.entryInstallationFree },
        { BUNDLE_INFO_REQ_PERMISSIONS, bundleInfo.reqPermissions },
        { BUNDLE_INFO_REQ_PERMISSION_STATES, bundleInfo.reqPermissionStates },
        { BUNDLE_INFO_REQ_PERMISSION_DETAILS, bundleInfo.reqPermissionDetails },
        { BUNDLE_INFO_DEF_PERMISSIONS, bundleInfo.defPermissions },
        { BUNDLE_INFO_HAP_MODULE_NAMES, bundleInfo.hapModuleNames },
        { BUNDLE_INFO_MODULE_NAMES, bundleInfo.moduleNames },
        { BUNDLE_INFO_MODULE_PUBLIC_DIRS, bundleInfo.modulePublicDirs },
        { BUNDLE_INFO_MODULE_DIRS, bundleInfo.moduleDirs },
        { BUNDLE_INFO_MODULE_RES_PATHS, bundleInfo.moduleResPaths },
        { BUNDLE_INFO_SINGLETON, bundleInfo.singleton },
        { BUNDLE_INFO_APP_INDEX, bundleInfo.appIndex },
        { BUNDLE_INFO_SIGNATURE_INFO, bundleInfo.signatureInfo },
        { BUNDLE_INFO_ASAN_ENABLED, bundleInfo.asanEnabled },
        { BUNDLE_INFO_ROUTER_ARRAY, bundleInfo.routerArray },
    };
}

void from_json(const nlohmann::json& jsonObject, BundleInfo& bundleInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_NAME, bundleInfo.name, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_LABEL, bundleInfo.label, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_DESCRIPTION, bundleInfo.description,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_VENDOR, bundleInfo.vendor, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_IS_KEEP_ALIVE, bundleInfo.isKeepAlive,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_IS_NATIVE_APP, bundleInfo.isNativeApp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_IS_PREINSTALL_APP, bundleInfo.isPreInstallApp,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_IS_DIFFERENT_NAME, bundleInfo.isDifferentName,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<HapModuleInfo>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_HAP_MODULE_INFOS,
        bundleInfo.hapModuleInfos, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_VERSION_CODE, bundleInfo.versionCode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MIN_COMPATIBLE_VERSION_CODE,
        bundleInfo.minCompatibleVersionCode, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_VERSION_NAME, bundleInfo.versionName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_JOINT_USERID, bundleInfo.jointUserId,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MIN_SDK_VERSION, bundleInfo.minSdkVersion,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MAX_SDK_VERSION, bundleInfo.maxSdkVersion,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MAIN_ENTRY, bundleInfo.mainEntry,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_CPU_ABI, bundleInfo.cpuAbi, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_APPID, bundleInfo.appId, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, BUNDLE_INFO_COMPATIBLE_VERSION, bundleInfo.compatibleVersion,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_TARGET_VERSION, bundleInfo.targetVersion,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_RELEASE_TYPE, bundleInfo.releaseType,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, BUNDLE_INFO_UID, bundleInfo.uid, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, BUNDLE_INFO_GID, bundleInfo.gid, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_SEINFO, bundleInfo.seInfo, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_INSTALL_TIME, bundleInfo.installTime,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int64_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_UPDATE_TIME, bundleInfo.updateTime,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, BUNDLE_INFO_ENTRY_MODULE_NAME, bundleInfo.entryModuleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_ENTRY_INSTALLATION_FREE,
        bundleInfo.entryInstallationFree, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_REQ_PERMISSIONS,
        bundleInfo.reqPermissions, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<int32_t>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_REQ_PERMISSION_STATES,
        bundleInfo.reqPermissionStates, JsonType::ARRAY, false, parseResult, ArrayType::NUMBER);
    GetValueIfFindKey<std::vector<RequestPermission>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_REQ_PERMISSION_DETAILS,
        bundleInfo.reqPermissionDetails, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_DEF_PERMISSIONS,
        bundleInfo.defPermissions, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_HAP_MODULE_NAMES,
        bundleInfo.hapModuleNames, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MODULE_NAMES,
        bundleInfo.moduleNames, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MODULE_PUBLIC_DIRS,
        bundleInfo.modulePublicDirs, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MODULE_DIRS,
        bundleInfo.moduleDirs, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_MODULE_RES_PATHS,
        bundleInfo.moduleResPaths, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_SINGLETON, bundleInfo.singleton, JsonType::BOOLEAN,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, BUNDLE_INFO_APP_INDEX, bundleInfo.appIndex, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<SignatureInfo>(jsonObject, jsonObjectEnd, BUNDLE_INFO_SIGNATURE_INFO, bundleInfo.signatureInfo,
        JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, BUNDLE_INFO_ASAN_ENABLED, bundleInfo.asanEnabled,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<RouterItem>>(jsonObject, jsonObjectEnd, BUNDLE_INFO_ROUTER_ARRAY,
        bundleInfo.routerArray, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("BundleInfo from_json error, error code : %{public}d", parseResult);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
