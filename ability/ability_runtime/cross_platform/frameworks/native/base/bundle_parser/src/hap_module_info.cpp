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

#include "hap_module_info.h"

#include "bundle_constants.h"
#include "hilog.h"
#include "json_util.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AppExecFwk {
namespace {
const std::string HAP_MODULE_INFO_NAME = "name";
const std::string HAP_MODULE_INFO_PACKAGE = "package";
const std::string HAP_MODULE_INFO_DESCRIPTION = "description";
const std::string HAP_MODULE_INFO_DESCRIPTION_ID = "descriptionId";
const std::string HAP_MODULE_INFO_ICON_PATH = "iconPath";
const std::string HAP_MODULE_INFO_ICON_ID = "iconId";
const std::string HAP_MODULE_INFO_LABEL = "label";
const std::string HAP_MODULE_INFO_LABEL_ID = "labelId";
const std::string HAP_MODULE_INFO_BACKGROUND_IMG = "backgroundImg";
const std::string HAP_MODULE_INFO_MAIN_ABILITY = "mainAbility";
const std::string HAP_MODULE_INFO_SRC_PATH = "srcPath";
const std::string HAP_MODULE_INFO_HASH_VALUE = "hashValue";
const std::string HAP_MODULE_INFO_SUPPORTED_MODES = "supportedModes";
const std::string HAP_MODULE_INFO_REQ_CAPABILITIES = "reqCapabilities";
const std::string HAP_MODULE_INFO_DEVICE_TYPES = "deviceTypes";
const std::string HAP_MODULE_INFO_ABILITY_INFOS = "abilityInfos";
const std::string HAP_MODULE_INFO_COLOR_MODE = "colorMode";
const std::string HAP_MODULE_INFO_MAIN_ELEMENTNAME = "mainElementName";
const std::string HAP_MODULE_INFO_PAGES = "pages";
const std::string HAP_MODULE_INFO_PROCESS = "process";
const std::string HAP_MODULE_INFO_RESOURCE_PATH = "resourcePath";
const std::string HAP_MODULE_INFO_SRC_ENTRANCE = "srcEntrance";
const std::string HAP_MODULE_INFO_UI_SYNTAX = "uiSyntax";
const std::string HAP_MODULE_INFO_VIRTUAL_MACHINE = "virtualMachine";
const std::string HAP_MODULE_INFO_DELIVERY_WITH_INSTALL = "deliveryWithInstall";
const std::string HAP_MODULE_INFO_INSTALLATION_FREE = "installationFree";
const std::string HAP_MODULE_INFO_IS_MODULE_JSON = "isModuleJson";
const std::string HAP_MODULE_INFO_IS_STAGE_BASED_MODEL = "isStageBasedModel";
const std::string HAP_MODULE_INFO_IS_REMOVABLE = "isRemovable";
const std::string HAP_MODULE_INFO_MODULE_TYPE = "moduleType";
const std::string HAP_MODULE_INFO_EXTENSION_INFOS = "extensionInfos";
const std::string HAP_MODULE_INFO_META_DATA = "metadata";
const std::string HAP_MODULE_INFO_DEPENDENCIES = "dependencies";
const std::string HAP_MODULE_INFO_UPGRADE_FLAG = "upgradeFlag";
const std::string HAP_MODULE_INFO_HAP_PATH = "hapPath";
const std::string HAP_MODULE_INFO_COMPILE_MODE = "compileMode";
const std::string HAP_MODULE_INFO_HQF_INFO = "hqfInfo";
const std::string HAP_MODULE_INFO_IS_LIB_ISOLATED = "isLibIsolated";
const std::string HAP_MODULE_INFO_NATIVE_LIBRARY_PATH = "nativeLibraryPath";
const std::string HAP_MODULE_INFO_CPU_ABI = "cpuAbi";
const std::string HAP_MODULE_INFO_MODULE_SOURCE_DIR = "moduleSourceDir";
const std::string HAP_OVERLAY_MODULE_INFO = "overlayModuleInfos";
const std::string HAP_MODULE_INFO_ATOMIC_SERVICE_MODULE_TYPE = "atomicServiceModuleType";
const std::string HAP_MODULE_INFO_PRELOADS = "preloads";
const std::string PRELOAD_ITEM_MODULE_NAME = "moduleName";
const size_t MODULE_CAPACITY = 10240; // 10K
const char* HAP_MODULE_INFO_ROUTER_MAP = "routerMap";
const char* HAP_MODULE_INFO_ROUTER_ARRAY = "routerArray";
const char* ROUTER_ITEM_KEY_NAME = "name";
const char* ROUTER_ITEM_KEY_PAGE_SOURCE_FILE = "pageSourceFile";
const char* ROUTER_ITEM_KEY_BUILD_FUNCTION = "buildFunction";
const char* ROUTER_ITEM_KEY_DATA = "data";
const char* ROUTER_ITEM_KEY_CUSTOM_DATA = "customData";
const char* ROUTER_ITEM_KEY_OHMURL = "ohmurl";
const char* ROUTER_ITEM_KEY_BUNDLE_NAME = "bundleName";
const char* ROUTER_ITEM_KEY_MODULE_NAME = "moduleName";
} // namespace

void to_json(nlohmann::json& jsonObject, const HapModuleInfo& hapModuleInfo)
{
    jsonObject = nlohmann::json {
        { HAP_MODULE_INFO_NAME, hapModuleInfo.name },
        { HAP_MODULE_INFO_PACKAGE, hapModuleInfo.package },
        { Constants::MODULE_NAME, hapModuleInfo.moduleName },
        { HAP_MODULE_INFO_DESCRIPTION, hapModuleInfo.description },
        { HAP_MODULE_INFO_DESCRIPTION_ID, hapModuleInfo.descriptionId },
        { HAP_MODULE_INFO_ICON_PATH, hapModuleInfo.iconPath },
        { HAP_MODULE_INFO_ICON_ID, hapModuleInfo.iconId },
        { HAP_MODULE_INFO_LABEL, hapModuleInfo.label },
        { HAP_MODULE_INFO_LABEL_ID, hapModuleInfo.labelId },
        { HAP_MODULE_INFO_BACKGROUND_IMG, hapModuleInfo.backgroundImg },
        { HAP_MODULE_INFO_MAIN_ABILITY, hapModuleInfo.mainAbility },
        { HAP_MODULE_INFO_SRC_PATH, hapModuleInfo.srcPath },
        { HAP_MODULE_INFO_HASH_VALUE, hapModuleInfo.hashValue },
        { HAP_MODULE_INFO_HAP_PATH, hapModuleInfo.hapPath },
        { HAP_MODULE_INFO_SUPPORTED_MODES, hapModuleInfo.supportedModes },
        { HAP_MODULE_INFO_REQ_CAPABILITIES, hapModuleInfo.reqCapabilities },
        { HAP_MODULE_INFO_DEVICE_TYPES, hapModuleInfo.deviceTypes },
        { HAP_MODULE_INFO_ABILITY_INFOS, hapModuleInfo.abilityInfos },
        { HAP_MODULE_INFO_COLOR_MODE, hapModuleInfo.colorMode },
        { Constants::BUNDLE_NAME, hapModuleInfo.bundleName },
        { HAP_MODULE_INFO_MAIN_ELEMENTNAME, hapModuleInfo.mainElementName },
        { HAP_MODULE_INFO_PAGES, hapModuleInfo.pages },
        { HAP_MODULE_INFO_PROCESS, hapModuleInfo.process },
        { HAP_MODULE_INFO_RESOURCE_PATH, hapModuleInfo.resourcePath },
        { HAP_MODULE_INFO_SRC_ENTRANCE, hapModuleInfo.srcEntrance },
        { HAP_MODULE_INFO_UI_SYNTAX, hapModuleInfo.uiSyntax },
        { HAP_MODULE_INFO_VIRTUAL_MACHINE, hapModuleInfo.virtualMachine },
        { HAP_MODULE_INFO_DELIVERY_WITH_INSTALL, hapModuleInfo.deliveryWithInstall },
        { HAP_MODULE_INFO_INSTALLATION_FREE, hapModuleInfo.installationFree },
        { HAP_MODULE_INFO_IS_MODULE_JSON, hapModuleInfo.isModuleJson },
        { HAP_MODULE_INFO_IS_STAGE_BASED_MODEL, hapModuleInfo.isStageBasedModel },
        { HAP_MODULE_INFO_IS_REMOVABLE, hapModuleInfo.isRemovable },
        { HAP_MODULE_INFO_UPGRADE_FLAG, hapModuleInfo.upgradeFlag },
        { HAP_MODULE_INFO_MODULE_TYPE, hapModuleInfo.moduleType },
        { HAP_MODULE_INFO_META_DATA, hapModuleInfo.metadata },
        { HAP_MODULE_INFO_DEPENDENCIES, hapModuleInfo.dependencies },
        { HAP_MODULE_INFO_COMPILE_MODE, hapModuleInfo.compileMode },
        { HAP_MODULE_INFO_IS_LIB_ISOLATED, hapModuleInfo.isLibIsolated },
        { HAP_MODULE_INFO_NATIVE_LIBRARY_PATH, hapModuleInfo.nativeLibraryPath },
        { HAP_MODULE_INFO_CPU_ABI, hapModuleInfo.cpuAbi },
        { HAP_MODULE_INFO_MODULE_SOURCE_DIR, hapModuleInfo.moduleSourceDir },
        { HAP_MODULE_INFO_ROUTER_MAP, hapModuleInfo.routerMap },
        { HAP_MODULE_INFO_ROUTER_ARRAY, hapModuleInfo.routerArray },
    };
}

void from_json(const nlohmann::json& jsonObject, HapModuleInfo& hapModuleInfo)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_NAME, hapModuleInfo.name,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_PACKAGE, hapModuleInfo.package,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::MODULE_NAME, hapModuleInfo.moduleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_DESCRIPTION, hapModuleInfo.description,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_DESCRIPTION_ID, hapModuleInfo.descriptionId,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_ICON_PATH, hapModuleInfo.iconPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_ICON_ID, hapModuleInfo.iconId, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_LABEL, hapModuleInfo.label,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_LABEL_ID, hapModuleInfo.labelId, JsonType::NUMBER,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_BACKGROUND_IMG,
        hapModuleInfo.backgroundImg, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_MAIN_ABILITY, hapModuleInfo.mainAbility,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_SRC_PATH, hapModuleInfo.srcPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_HASH_VALUE, hapModuleInfo.hashValue,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_HAP_PATH, hapModuleInfo.hapPath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_SUPPORTED_MODES, hapModuleInfo.supportedModes,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_REQ_CAPABILITIES,
        hapModuleInfo.reqCapabilities, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_DEVICE_TYPES,
        hapModuleInfo.deviceTypes, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<std::vector<AbilityInfo>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_ABILITY_INFOS,
        hapModuleInfo.abilityInfos, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<ModuleColorMode>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_COLOR_MODE, hapModuleInfo.colorMode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, Constants::BUNDLE_NAME, hapModuleInfo.bundleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_MAIN_ELEMENTNAME,
        hapModuleInfo.mainElementName, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_PAGES, hapModuleInfo.pages,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_PROCESS, hapModuleInfo.process,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_RESOURCE_PATH, hapModuleInfo.resourcePath,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_SRC_ENTRANCE, hapModuleInfo.srcEntrance,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_UI_SYNTAX, hapModuleInfo.uiSyntax,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_VIRTUAL_MACHINE,
        hapModuleInfo.virtualMachine, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_DELIVERY_WITH_INSTALL,
        hapModuleInfo.deliveryWithInstall, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_INSTALLATION_FREE,
        hapModuleInfo.installationFree, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_IS_MODULE_JSON, hapModuleInfo.isModuleJson,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_IS_STAGE_BASED_MODEL,
        hapModuleInfo.isStageBasedModel, JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, bool>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_IS_REMOVABLE,
        hapModuleInfo.isRemovable, JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int32_t>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_UPGRADE_FLAG, hapModuleInfo.upgradeFlag,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<ModuleType>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_MODULE_TYPE, hapModuleInfo.moduleType,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<Metadata>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_META_DATA,
        hapModuleInfo.metadata, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    GetValueIfFindKey<std::vector<std::string>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_DEPENDENCIES,
        hapModuleInfo.dependencies, JsonType::ARRAY, false, parseResult, ArrayType::STRING);
    GetValueIfFindKey<CompileMode>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_COMPILE_MODE, hapModuleInfo.compileMode,
        JsonType::NUMBER, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<bool>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_IS_LIB_ISOLATED, hapModuleInfo.isLibIsolated,
        JsonType::BOOLEAN, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_NATIVE_LIBRARY_PATH,
        hapModuleInfo.nativeLibraryPath, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_CPU_ABI, hapModuleInfo.cpuAbi,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_MODULE_SOURCE_DIR,
        hapModuleInfo.moduleSourceDir, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_ROUTER_MAP, hapModuleInfo.routerMap,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::vector<RouterItem>>(jsonObject, jsonObjectEnd, HAP_MODULE_INFO_ROUTER_ARRAY,
        hapModuleInfo.routerArray, JsonType::ARRAY, false, parseResult, ArrayType::OBJECT);
    if (parseResult != ERR_OK) {
        HILOG_WARN("HapModuleInfo from_json error, error code : %{public}d", parseResult);
    }
}

void to_json(nlohmann::json& jsonObject, const RouterItem& routerItem)
{
    jsonObject = nlohmann::json {
        { ROUTER_ITEM_KEY_NAME, routerItem.name },
        { ROUTER_ITEM_KEY_PAGE_SOURCE_FILE, routerItem.pageSourceFile },
        { ROUTER_ITEM_KEY_BUILD_FUNCTION, routerItem.buildFunction },
        { ROUTER_ITEM_KEY_DATA, routerItem.data },
        { ROUTER_ITEM_KEY_CUSTOM_DATA, routerItem.customData },
        { ROUTER_ITEM_KEY_OHMURL, routerItem.ohmurl },
        { ROUTER_ITEM_KEY_BUNDLE_NAME, routerItem.bundleName },
        { ROUTER_ITEM_KEY_MODULE_NAME, routerItem.moduleName },
    };
}

void from_json(const nlohmann::json& jsonObject, RouterItem& routerItem)
{
    const auto& jsonObjectEnd = jsonObject.end();
    int32_t parseResult = ERR_OK;
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_NAME, routerItem.name, JsonType::STRING,
        false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_PAGE_SOURCE_FILE,
        routerItem.pageSourceFile, JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_BUILD_FUNCTION, routerItem.buildFunction,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_OHMURL, routerItem.ohmurl,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_BUNDLE_NAME, routerItem.bundleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_MODULE_NAME, routerItem.moduleName,
        JsonType::STRING, false, parseResult, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::map<std::string, std::string>>(jsonObject, jsonObjectEnd, ROUTER_ITEM_KEY_DATA,
        routerItem.data, JsonType::OBJECT, false, parseResult, ArrayType::NOT_ARRAY);
    if (parseResult != ERR_OK) {
        HILOG_ERROR("read RouterItem jsonObject error : %{public}d", parseResult);
    }
}
} // namespace AppExecFwk
} // namespace OHOS
