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

#include "ability_info.h"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "bundle_constants.h"
#include "hilog.h"
#include "json_util.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string JSON_KEY_PACKAGE = "package";
const std::string JSON_KEY_NAME = "name";
const std::string JSON_KEY_APPLICATION_NAME = "applicationName";
const std::string JSON_KEY_LABEL = "label";
const std::string JSON_KEY_DESCRIPTION = "description";
const std::string JSON_KEY_ICON_PATH = "iconPath";
const std::string JSON_KEY_THEME = "theme";
const std::string JSON_KEY_VISIBLE = "visible";
const std::string JSON_KEY_KIND = "kind";
const std::string JSON_KEY_TYPE = "type";
const std::string JSON_KEY_EXTENSION_ABILITY_TYPE = "extensionAbilityType";
const std::string JSON_KEY_ORIENTATION = "orientation";
const std::string JSON_KEY_LAUNCH_MODE = "launchMode";
const std::string JSON_KEY_CODE_PATH = "codePath";
const std::string JSON_KEY_RESOURCE_PATH = "resourcePath";
const std::string JSON_KEY_PERMISSIONS = "permissions";
const std::string JSON_KEY_PROCESS = "process";
const std::string JSON_KEY_DEVICE_TYPES = "deviceTypes";
const std::string JSON_KEY_DEVICE_CAPABILITIES = "deviceCapabilities";
const std::string JSON_KEY_URI = "uri";
const std::string JSON_KEY_IS_LAUNCHER_ABILITY = "isLauncherAbility";
const std::string JSON_KEY_REMOVE_MISSION_AFTER_TERMINATE = "removeMissionAfterTerminate";
const std::string JSON_KEY_IS_NATIVE_ABILITY = "isNativeAbility";
const std::string JSON_KEY_ENABLED = "enabled";
const std::string JSON_KEY_SUPPORT_PIP_MODE = "supportPipMode";
const std::string JSON_KEY_TARGET_ABILITY = "targetAbility";
const std::string JSON_KEY_READ_PERMISSION = "readPermission";
const std::string JSON_KEY_WRITE_PERMISSION = "writePermission";
const std::string JSON_KEY_CONFIG_CHANGES = "configChanges";
const std::string JSON_KEY_FORM = "form";
const std::string JSON_KEY_FORM_ENTITY = "formEntity";
const std::string JSON_KEY_MIN_FORM_HEIGHT = "minFormHeight";
const std::string JSON_KEY_DEFAULT_FORM_HEIGHT = "defaultFormHeight";
const std::string JSON_KEY_MIN_FORM_WIDTH = "minFormWidth";
const std::string JSON_KEY_DEFAULT_FORM_WIDTH = "defaultFormWidth";
const std::string JSON_KEY_BACKGROUND_MODES = "backgroundModes";
const std::string JSON_KEY_CUSTOMIZE_DATA = "customizeData";
const std::string JSON_KEY_META_DATA = "metaData";
const std::string JSON_KEY_META_VALUE = "value";
const std::string JSON_KEY_META_EXTRA = "extra";
const std::string JSON_KEY_LABEL_ID = "labelId";
const std::string JSON_KEY_DESCRIPTION_ID = "descriptionId";
const std::string JSON_KEY_ICON_ID = "iconId";
const std::string JSON_KEY_FORM_ENABLED = "formEnabled";
const std::string JSON_KEY_SRC_PATH = "srcPath";
const std::string JSON_KEY_SRC_LANGUAGE = "srcLanguage";
const std::string JSON_KEY_START_WINDOW_ICON = "startWindowIcon";
const std::string JSON_KEY_START_WINDOW_ICON_ID = "startWindowIconId";
const std::string JSON_KEY_START_WINDOW_BACKGROUND = "startWindowBackground";
const std::string JSON_KEY_START_WINDOW_BACKGROUND_ID = "startWindowBackgroundId";
const std::string JSON_KEY_COMPILE_MODE = "compileMode";
const std::string META_DATA = "metadata";
const std::string META_DATA_NAME = "name";
const std::string META_DATA_VALUE = "value";
const std::string META_DATA_RESOURCE = "resource";
const std::string SRC_ENTRANCE = "srcEntrance";
const std::string IS_MODULE_JSON = "isModuleJson";
const std::string IS_STAGE_BASED_MODEL = "isStageBasedModel";
const std::string CONTINUABLE = "continuable";
const std::string PRIORITY = "priority";
const std::string JOSN_KEY_SUPPORT_WINDOW_MODE = "supportWindowMode";
const std::string JOSN_KEY_MAX_WINDOW_RATIO = "maxWindowRatio";
const std::string JOSN_KEY_MIN_WINDOW_RATIO = "minWindowRatio";
const std::string JOSN_KEY_MAX_WINDOW_WIDTH = "maxWindowWidth";
const std::string JOSN_KEY_MIN_WINDOW_WIDTH = "minWindowWidth";
const std::string JOSN_KEY_MAX_WINDOW_HEIGHT = "maxWindowHeight";
const std::string JOSN_KEY_MIN_WINDOW_HEIGHT = "minWindowHeight";
const std::string JOSN_KEY_UID = "uid";
const std::string JOSN_KEY_EXCLUDE_FROM_MISSIONS = "excludeFromMissions";
const std::string JSON_KEY_RECOVERABLE = "recoverable";
const size_t ABILITY_CAPACITY = 10240; // 10K
} // namespace

void to_json(nlohmann::json& jsonObject, const CustomizeData& customizeData)
{
    jsonObject = nlohmann::json { { JSON_KEY_NAME, customizeData.name }, { JSON_KEY_META_VALUE, customizeData.value },
        { JSON_KEY_META_EXTRA, customizeData.extra } };
}

void to_json(nlohmann::json& jsonObject, const MetaData& metaData)
{
    jsonObject = nlohmann::json { { JSON_KEY_CUSTOMIZE_DATA, metaData.customizeData } };
}

void to_json(nlohmann::json& jsonObject, const Metadata& metadata)
{
    jsonObject = nlohmann::json { { META_DATA_NAME, metadata.name }, { META_DATA_VALUE, metadata.value },
        { META_DATA_RESOURCE, metadata.resource } };
}

void to_json(nlohmann::json& jsonObject, const AbilityInfo& abilityInfo)
{
    HILOG_INFO("AbilityInfo to_json begin");
    jsonObject = nlohmann::json { { JSON_KEY_NAME, abilityInfo.name }, { JSON_KEY_LABEL, abilityInfo.label },
        { JSON_KEY_DESCRIPTION, abilityInfo.description }, { JSON_KEY_ICON_PATH, abilityInfo.iconPath },
        { JSON_KEY_LABEL_ID, abilityInfo.labelId }, { JSON_KEY_DESCRIPTION_ID, abilityInfo.descriptionId },
        { JSON_KEY_ICON_ID, abilityInfo.iconId }, { JSON_KEY_THEME, abilityInfo.theme },
        { JSON_KEY_VISIBLE, abilityInfo.visible }, { JSON_KEY_KIND, abilityInfo.kind },
        { JSON_KEY_TYPE, abilityInfo.type },
        // {JSON_KEY_EXTENSION_ABILITY_TYPE, abilityInfo.extensionAbilityType},
        { JSON_KEY_ORIENTATION, abilityInfo.orientation }, { JSON_KEY_LAUNCH_MODE, abilityInfo.launchMode },
        { JSON_KEY_SRC_PATH, abilityInfo.srcPath }, { JSON_KEY_SRC_LANGUAGE, abilityInfo.srcLanguage },
        { JSON_KEY_PERMISSIONS, abilityInfo.permissions }, { JSON_KEY_PROCESS, abilityInfo.process },
        { JSON_KEY_DEVICE_TYPES, abilityInfo.deviceTypes },
        { JSON_KEY_DEVICE_CAPABILITIES, abilityInfo.deviceCapabilities }, { JSON_KEY_URI, abilityInfo.uri },
        { JSON_KEY_TARGET_ABILITY, abilityInfo.targetAbility },
        { JSON_KEY_IS_LAUNCHER_ABILITY, abilityInfo.isLauncherAbility },
        { JSON_KEY_IS_NATIVE_ABILITY, abilityInfo.isNativeAbility }, { JSON_KEY_ENABLED, abilityInfo.enabled },
        { JSON_KEY_SUPPORT_PIP_MODE, abilityInfo.supportPipMode }, { JSON_KEY_FORM_ENABLED, abilityInfo.formEnabled },
        { JSON_KEY_READ_PERMISSION, abilityInfo.readPermission },
        { JSON_KEY_WRITE_PERMISSION, abilityInfo.writePermission },
        { JSON_KEY_CONFIG_CHANGES, abilityInfo.configChanges }, { JSON_KEY_FORM_ENTITY, abilityInfo.formEntity },
        { JSON_KEY_MIN_FORM_HEIGHT, abilityInfo.minFormHeight },
        { JSON_KEY_DEFAULT_FORM_HEIGHT, abilityInfo.defaultFormHeight },
        { JSON_KEY_MIN_FORM_WIDTH, abilityInfo.minFormWidth },
        { JSON_KEY_DEFAULT_FORM_WIDTH, abilityInfo.defaultFormWidth }, { JSON_KEY_META_DATA, abilityInfo.metaData },
        { JSON_KEY_BACKGROUND_MODES, abilityInfo.backgroundModes }, { JSON_KEY_PACKAGE, abilityInfo.package },
        { Constants::BUNDLE_NAME, abilityInfo.bundleName }, { Constants::MODULE_NAME, abilityInfo.moduleName },
        { JSON_KEY_APPLICATION_NAME, abilityInfo.applicationName }, { JSON_KEY_CODE_PATH, abilityInfo.codePath },
        { JSON_KEY_RESOURCE_PATH, abilityInfo.resourcePath }, { Constants::HAP_PATH, abilityInfo.hapPath },
        { SRC_ENTRANCE, abilityInfo.srcEntrance }, { META_DATA, abilityInfo.metadata },
        { IS_MODULE_JSON, abilityInfo.isModuleJson }, { IS_STAGE_BASED_MODEL, abilityInfo.isStageBasedModel },
        { CONTINUABLE, abilityInfo.continuable }, { PRIORITY, abilityInfo.priority },
        { JSON_KEY_START_WINDOW_ICON, abilityInfo.startWindowIcon },
        { JSON_KEY_START_WINDOW_ICON_ID, abilityInfo.startWindowIconId },
        { JSON_KEY_START_WINDOW_BACKGROUND, abilityInfo.startWindowBackground },
        { JSON_KEY_START_WINDOW_BACKGROUND_ID, abilityInfo.startWindowBackgroundId },
        { JSON_KEY_REMOVE_MISSION_AFTER_TERMINATE, abilityInfo.removeMissionAfterTerminate },
        { JSON_KEY_COMPILE_MODE, abilityInfo.compileMode }, { JOSN_KEY_SUPPORT_WINDOW_MODE, abilityInfo.windowModes },
        { JOSN_KEY_MAX_WINDOW_WIDTH, abilityInfo.maxWindowWidth },
        { JOSN_KEY_MIN_WINDOW_WIDTH, abilityInfo.minWindowWidth },
        { JOSN_KEY_MAX_WINDOW_HEIGHT, abilityInfo.maxWindowHeight },
        { JOSN_KEY_MIN_WINDOW_HEIGHT, abilityInfo.minWindowHeight }, { JOSN_KEY_UID, abilityInfo.uid },
        { JOSN_KEY_EXCLUDE_FROM_MISSIONS, abilityInfo.excludeFromMissions },
        { JSON_KEY_RECOVERABLE, abilityInfo.recoverable } };
    if (abilityInfo.maxWindowRatio == 0) {
        // maxWindowRatio in json string will be 0 instead of 0.0
        jsonObject[JOSN_KEY_MAX_WINDOW_RATIO] = 0;
    } else {
        jsonObject[JOSN_KEY_MAX_WINDOW_RATIO] = abilityInfo.maxWindowRatio;
    }

    if (abilityInfo.minWindowRatio == 0) {
        jsonObject[JOSN_KEY_MIN_WINDOW_RATIO] = 0;
    } else {
        jsonObject[JOSN_KEY_MIN_WINDOW_RATIO] = abilityInfo.minWindowRatio;
    }
}

void from_json(const nlohmann::json& jsonObject, CustomizeData& customizeData)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_NAME, customizeData.name, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_META_VALUE, customizeData.value,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_META_EXTRA, customizeData.extra,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
}

void from_json(const nlohmann::json& jsonObject, MetaData& metaData)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::vector<CustomizeData>>(jsonObject, jsonObjectEnd, JSON_KEY_CUSTOMIZE_DATA,
        metaData.customizeData, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
}

void from_json(const nlohmann::json& jsonObject, Metadata& metadata)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, META_DATA_NAME, metadata.name, JsonType::STRING, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, META_DATA_VALUE, metadata.value, JsonType::STRING, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, META_DATA_RESOURCE, metadata.resource, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read Ability Metadata from database error, error code : %{public}d", parseResult);
    }
}

void from_json(const nlohmann::json& jsonObject, AbilityInfo& abilityInfo)
{
    HILOG_INFO("AbilityInfo from_json begin");
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_NAME, abilityInfo.name, JsonType::STRING, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_LABEL, abilityInfo.label, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_DESCRIPTION, abilityInfo.description,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_ICON_PATH, abilityInfo.iconPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_LABEL_ID, abilityInfo.labelId, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_DESCRIPTION_ID, abilityInfo.descriptionId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_ICON_ID, abilityInfo.iconId, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_THEME, abilityInfo.theme, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_VISIBLE, abilityInfo.visible, JsonType::BOOLEAN, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_KIND, abilityInfo.kind, JsonType::STRING, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<AbilityType>(jsonObject, jsonObjectEnd, JSON_KEY_TYPE, abilityInfo.type, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<DisplayOrientation>(jsonObject, jsonObjectEnd, JSON_KEY_ORIENTATION, abilityInfo.orientation,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<LaunchMode>(jsonObject, jsonObjectEnd, JSON_KEY_LAUNCH_MODE, abilityInfo.launchMode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_SRC_PATH, abilityInfo.srcPath, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_SRC_LANGUAGE, abilityInfo.srcLanguage,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, JSON_KEY_PERMISSIONS,
        abilityInfo.permissions, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_PROCESS, abilityInfo.process, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, JSON_KEY_DEVICE_TYPES,
        abilityInfo.deviceTypes, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, JSON_KEY_DEVICE_CAPABILITIES,
        abilityInfo.deviceCapabilities, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_URI, abilityInfo.uri, JsonType::STRING, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_TARGET_ABILITY, abilityInfo.targetAbility,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_IS_LAUNCHER_ABILITY, abilityInfo.isLauncherAbility,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_IS_NATIVE_ABILITY, abilityInfo.isNativeAbility,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_ENABLED, abilityInfo.enabled, JsonType::BOOLEAN, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_SUPPORT_PIP_MODE, abilityInfo.supportPipMode,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_FORM_ENABLED, abilityInfo.formEnabled,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_READ_PERMISSION, abilityInfo.readPermission,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_WRITE_PERMISSION, abilityInfo.writePermission,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, JSON_KEY_CONFIG_CHANGES,
        abilityInfo.configChanges, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JSON_KEY_FORM_ENTITY, abilityInfo.formEntity,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_MIN_FORM_HEIGHT, abilityInfo.minFormHeight,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_DEFAULT_FORM_HEIGHT, abilityInfo.defaultFormHeight,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_MIN_FORM_WIDTH, abilityInfo.minFormWidth,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_DEFAULT_FORM_WIDTH, abilityInfo.defaultFormWidth,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<MetaData>(jsonObject, jsonObjectEnd, JSON_KEY_META_DATA, abilityInfo.metaData, JsonType::OBJECT,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JSON_KEY_BACKGROUND_MODES, abilityInfo.backgroundModes,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_PACKAGE, abilityInfo.package, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::BUNDLE_NAME, abilityInfo.bundleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::MODULE_NAME, abilityInfo.moduleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_APPLICATION_NAME, abilityInfo.applicationName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_CODE_PATH, abilityInfo.codePath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_RESOURCE_PATH, abilityInfo.resourcePath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::HAP_PATH, abilityInfo.hapPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, SRC_ENTRANCE, abilityInfo.srcEntrance, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject, jsonObjectEnd, META_DATA, abilityInfo.metadata,
        JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, IS_MODULE_JSON, abilityInfo.isModuleJson, JsonType::BOOLEAN,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, IS_STAGE_BASED_MODEL, abilityInfo.isStageBasedModel,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, CONTINUABLE, abilityInfo.continuable, JsonType::BOOLEAN, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, PRIORITY, abilityInfo.priority, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_START_WINDOW_ICON, abilityInfo.startWindowIcon,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_START_WINDOW_ICON_ID, abilityInfo.startWindowIconId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, JSON_KEY_START_WINDOW_BACKGROUND,
        abilityInfo.startWindowBackground, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JSON_KEY_START_WINDOW_BACKGROUND_ID,
        abilityInfo.startWindowBackgroundId, JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_REMOVE_MISSION_AFTER_TERMINATE,
        abilityInfo.removeMissionAfterTerminate, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<CompileMode>(jsonObject, jsonObjectEnd, JSON_KEY_COMPILE_MODE, abilityInfo.compileMode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<SupportWindowMode>>(jsonObject, jsonObjectEnd, JOSN_KEY_SUPPORT_WINDOW_MODE,
        abilityInfo.windowModes, JsonType::ARRAY, false, parseResult, ArrayType::NUMBER);
    GetValueIfFindKey<double>(jsonObject, jsonObjectEnd, JOSN_KEY_MAX_WINDOW_RATIO, abilityInfo.maxWindowRatio,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<double>(jsonObject, jsonObjectEnd, JOSN_KEY_MIN_WINDOW_RATIO, abilityInfo.minWindowRatio,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JOSN_KEY_MAX_WINDOW_WIDTH, abilityInfo.maxWindowWidth,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JOSN_KEY_MIN_WINDOW_WIDTH, abilityInfo.minWindowWidth,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JOSN_KEY_MAX_WINDOW_HEIGHT, abilityInfo.maxWindowHeight,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<uint32_t>(jsonObject, jsonObjectEnd, JOSN_KEY_MIN_WINDOW_HEIGHT, abilityInfo.minWindowHeight,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, JOSN_KEY_UID, abilityInfo.uid, JsonType::NUMBER, false,
        parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JOSN_KEY_EXCLUDE_FROM_MISSIONS, abilityInfo.excludeFromMissions,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, JSON_KEY_RECOVERABLE, abilityInfo.recoverable, JsonType::BOOLEAN,
        false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("AbilityInfo from_json error, error code : %{public}d", parseResult);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
