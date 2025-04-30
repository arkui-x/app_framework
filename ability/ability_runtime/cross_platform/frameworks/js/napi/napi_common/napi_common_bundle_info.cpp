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

#include "napi_common_bundle_info.h"

#include "hilog.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AppExecFwk {
const std::string CONTEXT_DATA_STORAGE_BUNDLE("/data/storage/el1/bundle/");
const std::string CODE_PATH_PREFIX = "/data/storage/el1/bundle/";
const std::string PATH_PREFIX = "/data/app/el1/bundle/public";
std::unordered_map<int32_t, std::string> g_errorMap = {
    { static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS), "Incorrect parameters." },
};

napi_value NapiCommonBundleInfo::ThrowJsError(napi_env env, int32_t errCode)
{
    napi_value error = AbilityRuntime::CreateJsError(env, errCode, g_errorMap[errCode]);
    napi_throw(env, error);
    return AbilityRuntime::CreateJsUndefined(env);
}

bool NapiCommonBundleInfo::ParseInt(napi_env env, napi_value args, int32_t& param)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, args, &valuetype);
    if (valuetype != napi_number) {
        HILOG_DEBUG("Wrong argument type. int32 expected");
        return false;
    }
    int32_t value = 0;
    if (napi_get_value_int32(env, args, &value) != napi_ok) {
        HILOG_DEBUG("napi_get_value_int32 failed");
        return false;
    }
    param = value;
    return true;
}

void NapiCommonBundleInfo::ConvertWindowSize(napi_env env, const AbilityInfo& abilityInfo, napi_value value)
{
    napi_value maxWindowRatio;
    NAPI_CALL_RETURN_VOID(env, napi_create_double(env, abilityInfo.maxWindowRatio, &maxWindowRatio));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowRatio", maxWindowRatio));

    napi_value minWindowRatio;
    NAPI_CALL_RETURN_VOID(env, napi_create_double(env, abilityInfo.minWindowRatio, &minWindowRatio));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowRatio", minWindowRatio));

    napi_value maxWindowWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.maxWindowWidth, &maxWindowWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowWidth", maxWindowWidth));

    napi_value minWindowWidth;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.minWindowWidth, &minWindowWidth));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowWidth", minWindowWidth));

    napi_value maxWindowHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.maxWindowHeight, &maxWindowHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "maxWindowHeight", maxWindowHeight));

    napi_value minWindowHeight;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.minWindowHeight, &minWindowHeight));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "minWindowHeight", minWindowHeight));
}

void NapiCommonBundleInfo::ConvertMetadata(napi_env env, const Metadata& metadata, napi_value value)
{
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.name.c_str(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "name", name));

    napi_value valueStr;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.value.c_str(), NAPI_AUTO_LENGTH, &valueStr));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "value", valueStr));

    napi_value resource;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, metadata.resource.c_str(), NAPI_AUTO_LENGTH, &resource));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "resource", resource));
}

void NapiCommonBundleInfo::ConvertApplicationInfo(napi_env env, napi_value objAppInfo, const ApplicationInfo& appInfo)
{
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.name.c_str(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "name", name));

    napi_value description;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.description.c_str(), NAPI_AUTO_LENGTH, &description));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "description", description));

    napi_value descriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, appInfo.descriptionId, &descriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "descriptionId", descriptionId));

    napi_value enabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, true, &enabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "enabled", enabled));

    napi_value label;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.label.c_str(), NAPI_AUTO_LENGTH, &label));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "label", label));

    napi_value labelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, appInfo.labelId, &labelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "labelId", labelId));

    napi_value iconPath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &iconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "icon", iconPath));

    napi_value iconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, appInfo.iconId, &iconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "iconId", iconId));

    napi_value process;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.process.c_str(), NAPI_AUTO_LENGTH, &process));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "process", process));

    napi_value permissions;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &permissions));
    for (size_t idx = 0; idx < appInfo.permissions.size(); idx++) {
        napi_value permission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, appInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &permission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, permissions, idx, permission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "permissions", permissions));

    napi_value codePath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, appInfo.codePath.c_str(), NAPI_AUTO_LENGTH, &codePath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "codePath", codePath));

    napi_value metaData;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &metaData));
    for (const auto& item : appInfo.metadata) {
        napi_value metaDataArray;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &metaDataArray));
        for (size_t j = 0; j < item.second.size(); j++) {
            napi_value metaDataItem;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &metaDataItem));
            ConvertMetadata(env, item.second[j], metaDataItem);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, metaDataArray, j, metaDataItem));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, metaData, item.first.c_str(), metaDataArray));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "metadata", metaData));

    napi_value metaDataArrayInfo;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &metaDataArrayInfo));
    ConvertModuleMetaInfos(env, appInfo.metadata, metaDataArrayInfo);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "metadataArray", metaDataArrayInfo));

    napi_value removable;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &removable));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "removable", removable));

    napi_value accessTokenId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, appInfo.accessTokenId, &accessTokenId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "accessTokenId", accessTokenId));

    napi_value uid;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, appInfo.uid, &uid));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "uid", uid));

    napi_value iconResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &iconResource));
    ConvertResource(env, appInfo.iconResource, iconResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "iconResource", iconResource));

    napi_value labelResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &labelResource));
    ConvertResource(env, appInfo.labelResource, labelResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "labelResource", labelResource));

    napi_value descriptionResource;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &descriptionResource));
    ConvertResource(env, appInfo.descriptionResource, descriptionResource);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "descriptionResource", descriptionResource));

    napi_value appDistributionType;
    NAPI_CALL_RETURN_VOID(env,
        napi_create_string_utf8(env, appInfo.appDistributionType.c_str(), NAPI_AUTO_LENGTH, &appDistributionType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "appDistributionType", appDistributionType));

    napi_value appProvisionType;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.appProvisionType.c_str(), NAPI_AUTO_LENGTH, &appProvisionType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "appProvisionType", appProvisionType));

    napi_value isSystemApp;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &isSystemApp));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "systemApp", isSystemApp));

    napi_value debug;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, appInfo.debug, &debug));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "debug", debug));

    napi_value dataUnclearable;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &dataUnclearable));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "dataUnclearable", dataUnclearable));

    std::string externalNativeLibraryPath = "";
    if (!appInfo.nativeLibraryPath.empty()) {
        externalNativeLibraryPath = CONTEXT_DATA_STORAGE_BUNDLE + appInfo.nativeLibraryPath;
    }
    napi_value nativeLibraryPath;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, "", NAPI_AUTO_LENGTH, &nativeLibraryPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "nativeLibraryPath", nativeLibraryPath));

    napi_value releaseType;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, appInfo.apiReleaseType.c_str(), NAPI_AUTO_LENGTH, &releaseType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAppInfo, "releaseType", releaseType));
}

void NapiCommonBundleInfo::ConvertAbilityInfo(napi_env env, const AbilityInfo& abilityInfo, napi_value objAbilityInfo)
{
    napi_value bundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &bundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "bundleName", bundleName));

    napi_value moduleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.moduleName.c_str(), NAPI_AUTO_LENGTH, &moduleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "moduleName", moduleName));

    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "name", name));

    napi_value label;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &label));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "label", label));

    napi_value labelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.labelId, &labelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "labelId", labelId));

    napi_value description;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &description));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "description", description));

    napi_value descriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.descriptionId, &descriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "descriptionId", descriptionId));

    napi_value iconPath;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, abilityInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &iconPath));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "icon", iconPath));

    napi_value iconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, abilityInfo.iconId, &iconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "iconId", iconId));

    napi_value process;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, abilityInfo.process.c_str(), NAPI_AUTO_LENGTH, &process));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "process", process));

    napi_value exported;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &exported));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "exported", exported));

    napi_value orientation;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.orientation), &orientation));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "orientation", orientation));

    napi_value launchType;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.launchMode), &launchType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "launchType", launchType));

    napi_value permissions;
    size_t size = abilityInfo.permissions.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &permissions));
    for (size_t idx = 0; idx < size; ++idx) {
        napi_value permission;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_string_utf8(env, abilityInfo.permissions[idx].c_str(), NAPI_AUTO_LENGTH, &permission));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, permissions, idx, permission));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "permissions", permissions));

    napi_value deviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &deviceTypes));
    napi_value deviceType;
    NAPI_CALL_RETURN_VOID(env,
                          napi_create_string_utf8(env, "default", NAPI_AUTO_LENGTH, &deviceType));
    NAPI_CALL_RETURN_VOID(env, napi_set_element(env, deviceTypes, 0, deviceType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "deviceTypes", deviceTypes));

    napi_value applicationInfo;
    if (!abilityInfo.applicationInfo.name.empty()) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &applicationInfo));
        ConvertApplicationInfo(env, applicationInfo, abilityInfo.applicationInfo);
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &applicationInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "applicationInfo", applicationInfo));

    napi_value metadata;
    size = abilityInfo.metadata.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &metadata));
    for (size_t index = 0; index < size; ++index) {
        napi_value metaData;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &metaData));
        ConvertMetadata(env, abilityInfo.metadata[index], metaData);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, metadata, index, metaData));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "metadata", metadata));

    napi_value enabled;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, true, &enabled));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "enabled", enabled));

    napi_value supportWindowModes;
    size = abilityInfo.windowModes.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &supportWindowModes));
    for (size_t index = 0; index < size; ++index) {
        napi_value innerMode;
        NAPI_CALL_RETURN_VOID(
            env, napi_create_int32(env, static_cast<int32_t>(abilityInfo.windowModes[index]), &innerMode));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, supportWindowModes, index, innerMode));
    }
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objAbilityInfo, "supportWindowModes", supportWindowModes));

    napi_value windowSize;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &windowSize));
    ConvertWindowSize(env, abilityInfo, windowSize);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objAbilityInfo, "windowSize", windowSize));
}

void NapiCommonBundleInfo::ConvertResource(napi_env env, const Resource& resource, napi_value objResource)
{
    napi_value bundleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, resource.bundleName.c_str(), NAPI_AUTO_LENGTH, &bundleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, "bundleName", bundleName));

    napi_value moduleName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, resource.moduleName.c_str(), NAPI_AUTO_LENGTH, &moduleName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, "moduleName", moduleName));

    napi_value id;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, resource.id, &id));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objResource, "id", id));
}

void NapiCommonBundleInfo::ConvertModuleMetaInfos(
    napi_env env, const std::map<std::string, std::vector<Metadata>>& metadata, napi_value objInfos)
{
    size_t index = 0;
    for (const auto& item : metadata) {
        napi_value objInfo = nullptr;
        napi_create_object(env, &objInfo);

        napi_value moduleName;
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, item.first.c_str(), NAPI_AUTO_LENGTH, &moduleName));
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objInfo, "moduleName", moduleName));

        napi_value metadataInfos;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &metadataInfos));
        for (size_t idx = 0; idx < item.second.size(); idx++) {
            napi_value moduleMetadata;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &moduleMetadata));
            ConvertMetadata(env, item.second[idx], moduleMetadata);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, metadataInfos, idx, moduleMetadata));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objInfo, "metadata", metadataInfos));

        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, objInfos, index++, objInfo));
    }
}

void NapiCommonBundleInfo::ConvertHapModuleInfo(
    napi_env env, const HapModuleInfo& hapModuleInfo, napi_value objHapModuleInfo, int32_t flags)
{
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.name.c_str(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "name", name));

    napi_value icon;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.iconPath.c_str(), NAPI_AUTO_LENGTH, &icon));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "icon", icon));

    napi_value iconId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, hapModuleInfo.iconId, &iconId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "iconId", iconId));

    napi_value label;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, hapModuleInfo.label.c_str(), NAPI_AUTO_LENGTH, &label));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "label", label));

    napi_value labelId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, hapModuleInfo.labelId, &labelId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "labelId", labelId));

    napi_value description;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.description.c_str(), NAPI_AUTO_LENGTH, &description));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "description", description));

    napi_value descriptionId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, hapModuleInfo.descriptionId, &descriptionId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "descriptionId", descriptionId));

    napi_value mainElementName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.mainElementName.c_str(), NAPI_AUTO_LENGTH, &mainElementName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "mainElementName", mainElementName));

    int32_t hapFlag = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
    int32_t abilityFlag = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY);
    const bool hasHapModule = (static_cast<uint32_t>(flags) & hapFlag) == hapFlag;
    const bool hasAbility = (static_cast<uint32_t>(flags) & abilityFlag) == abilityFlag;
    if (hasHapModule && hasAbility) {
        napi_value abilityInfos;
        NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &abilityInfos));
        for (size_t idx = 0; idx < hapModuleInfo.abilityInfos.size(); idx++) {
            napi_value objAbilityInfo;
            NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objAbilityInfo));
            ConvertAbilityInfo(env, hapModuleInfo.abilityInfos[idx], objAbilityInfo);
            NAPI_CALL_RETURN_VOID(env, napi_set_element(env, abilityInfos, idx, objAbilityInfo));
        }
        NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "abilitiesInfo", abilityInfos));
    }

    napi_value metadata;
    size_t size = hapModuleInfo.metadata.size();
    NAPI_CALL_RETURN_VOID(env, napi_create_array_with_length(env, size, &metadata));
    for (size_t index = 0; index < size; ++index) {
        napi_value innerMeta;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &innerMeta));
        ConvertMetadata(env, hapModuleInfo.metadata[index], innerMeta);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, metadata, index, innerMeta));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "metadata", metadata));

    napi_value deviceTypes;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &deviceTypes));
    napi_value deviceType;
    NAPI_CALL_RETURN_VOID(env,
                          napi_create_string_utf8(env, "default", NAPI_AUTO_LENGTH, &deviceType));
    NAPI_CALL_RETURN_VOID(env, napi_set_element(env, deviceTypes, 0, deviceType));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "deviceTypes", deviceTypes));

    napi_value installationFree;
    NAPI_CALL_RETURN_VOID(env, napi_get_boolean(env, false, &installationFree));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "installationFree", installationFree));

    napi_value hashValue;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, hapModuleInfo.hashValue.c_str(), NAPI_AUTO_LENGTH, &hashValue));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "hashValue", hashValue));

    napi_value type;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, static_cast<int32_t>(hapModuleInfo.moduleType), &type));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objHapModuleInfo, "type", type));
}

void NapiCommonBundleInfo::ConvertRequestPermissionUsedScene(
    napi_env env, const RequestPermissionUsedScene& requestPermissionUsedScene, napi_value result)
{
    napi_value abilities;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &abilities));
    for (size_t index = 0; index < requestPermissionUsedScene.abilities.size(); index++) {
        napi_value objAbility;
        NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, requestPermissionUsedScene.abilities[index].c_str(),
                                       NAPI_AUTO_LENGTH, &objAbility));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, abilities, index, objAbility));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "abilities", abilities));

    napi_value when;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, requestPermissionUsedScene.when.c_str(), NAPI_AUTO_LENGTH, &when));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "when", when));
}

void NapiCommonBundleInfo::ConvertSignatureInfo(napi_env env, const SignatureInfo& signatureInfo, napi_value value)
{
    napi_value appId;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, signatureInfo.appId.c_str(), NAPI_AUTO_LENGTH, &appId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "appId", appId));

    napi_value fingerprint;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, signatureInfo.fingerprint.c_str(), NAPI_AUTO_LENGTH, &fingerprint));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, value, "fingerprint", fingerprint));
}

void NapiCommonBundleInfo::ConvertRequestPermission(
    napi_env env, const RequestPermission& requestPermission, napi_value result)
{
    napi_value permissionName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, requestPermission.name.c_str(), NAPI_AUTO_LENGTH, &permissionName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "name", permissionName));

    napi_value reason;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, requestPermission.reason.c_str(), NAPI_AUTO_LENGTH, &reason));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "reason", reason));

    napi_value reasonId;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, requestPermission.reasonId, &reasonId));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "reasonId", reasonId));

    napi_value usedScene;
    NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &usedScene));
    ConvertRequestPermissionUsedScene(env, requestPermission.usedScene, usedScene);
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, result, "usedScene", usedScene));
}

void NapiCommonBundleInfo::ConvertBundleInfo(
    napi_env env, const BundleInfo& bundleInfo, napi_value objBundleInfo, int32_t flags)
{
    napi_value name;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.name.c_str(), NAPI_AUTO_LENGTH, &name));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "name", name));

    napi_value vendor;
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, bundleInfo.vendor.c_str(), NAPI_AUTO_LENGTH, &vendor));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "vendor", vendor));

    napi_value versionCode;
    NAPI_CALL_RETURN_VOID(env, napi_create_uint32(env, bundleInfo.versionCode, &versionCode));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "versionCode", versionCode));

    napi_value versionName;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_string_utf8(env, bundleInfo.versionName.c_str(), NAPI_AUTO_LENGTH, &versionName));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "versionName", versionName));

    napi_value minCompatibleVersionCode;
    NAPI_CALL_RETURN_VOID(
        env, napi_create_int32(env, bundleInfo.minCompatibleVersionCode, &minCompatibleVersionCode));
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "minCompatibleVersionCode", minCompatibleVersionCode));

    napi_value targetVersion;
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, bundleInfo.targetVersion, &targetVersion));
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "targetVersion", targetVersion));

    napi_value appInfo;
    if ((static_cast<uint32_t>(flags) & static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION)) ==
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION)) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &appInfo));
        ConvertApplicationInfo(env, appInfo, bundleInfo.applicationInfo);
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &appInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "appInfo", appInfo));

    napi_value hapModuleInfos;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &hapModuleInfos));
    for (size_t idx = 0; idx < bundleInfo.hapModuleInfos.size(); idx++) {
        napi_value objHapModuleInfo;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objHapModuleInfo));
        ConvertHapModuleInfo(env, bundleInfo.hapModuleInfos[idx], objHapModuleInfo, flags);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, hapModuleInfos, idx, objHapModuleInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "hapModulesInfo", hapModuleInfos));

    napi_value reqPermissionDetails;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &reqPermissionDetails));
    for (size_t idx = 0; idx < bundleInfo.reqPermissionDetails.size(); idx++) {
        napi_value objReqPermission;
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &objReqPermission));
        ConvertRequestPermission(env, bundleInfo.reqPermissionDetails[idx], objReqPermission);
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, reqPermissionDetails, idx, objReqPermission));
    }
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "reqPermissionDetails", reqPermissionDetails));

    napi_value reqPermissionStates;
    NAPI_CALL_RETURN_VOID(env, napi_create_array(env, &reqPermissionStates));
    for (size_t idx = 0; idx < bundleInfo.reqPermissionStates.size(); idx++) {
        napi_value reqPermissionState;
        NAPI_CALL_RETURN_VOID(env,
            napi_create_int32(env, static_cast<int32_t>(bundleInfo.reqPermissionStates[idx]), &reqPermissionState));
        NAPI_CALL_RETURN_VOID(env, napi_set_element(env, reqPermissionStates, idx, reqPermissionState));
    }
    NAPI_CALL_RETURN_VOID(
        env, napi_set_named_property(env, objBundleInfo, "permissionGrantStates", reqPermissionStates));

    napi_value signatureInfo;
    if ((static_cast<uint32_t>(flags) &
            static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO)) ==
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_SIGNATURE_INFO)) {
        NAPI_CALL_RETURN_VOID(env, napi_create_object(env, &signatureInfo));
        ConvertSignatureInfo(env, bundleInfo.signatureInfo, signatureInfo);
    } else {
        NAPI_CALL_RETURN_VOID(env, napi_get_null(env, &signatureInfo));
    }
    NAPI_CALL_RETURN_VOID(env, napi_set_named_property(env, objBundleInfo, "signatureInfo", signatureInfo));
}
} // namespace AppExecFwk
} // namespace OHOS