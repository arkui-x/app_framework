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

#include "js_data_struct_converter.h"

#include "hilog.h"
#include "js_runtime_utils.h"
#include "resource_manager_addon.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo)
{
    HILOG_INFO("CreateJsAbilityInfo call.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    napi_set_named_property(env, object, "bundleName", CreateJsValue(env, abilityInfo.bundleName));
    napi_set_named_property(env, object, "moduleName", CreateJsValue(env, abilityInfo.moduleName));
    napi_set_named_property(env, object, "name", CreateJsValue(env, abilityInfo.name));
    napi_set_named_property(env, object, "label", CreateJsValue(env, abilityInfo.label));
    napi_set_named_property(env, object, "labelId", CreateJsValue(env, abilityInfo.labelId));
    napi_set_named_property(env, object, "description", CreateJsValue(env, abilityInfo.description));
    napi_set_named_property(env, object, "descriptionId", CreateJsValue(env, abilityInfo.descriptionId));
    napi_set_named_property(env, object, "icon", CreateJsValue(env, abilityInfo.iconPath));
    napi_set_named_property(env, object, "iconId", CreateJsValue(env, abilityInfo.iconId));
    napi_set_named_property(env, object, "type", CreateJsValue(env, abilityInfo.type));
    napi_set_named_property(env, object, "launchType", CreateJsValue(env, abilityInfo.launchMode));
    napi_set_named_property(
        env, object, "applicationInfo", CreateJsApplicationInfo(env, abilityInfo.applicationInfo));
        
    napi_value metadataArray = nullptr;
    napi_create_array_with_length(env, abilityInfo.metadata.size(), &metadataArray);

    uint32_t index = 0;
    for (const auto& metadata : abilityInfo.metadata) {
        napi_value objVal = nullptr;
        napi_create_object(env, &objVal);
        if (objVal == nullptr) {
            HILOG_ERROR("Native object value is nullptr.");
            continue;
        }

        napi_set_named_property(env, objVal, "name", CreateJsValue(env, metadata.name));
        napi_set_named_property(env, objVal, "value", CreateJsValue(env, metadata.value));
        napi_set_named_property(env, objVal, "resource", CreateJsValue(env, metadata.resource));

        napi_set_element(env, metadataArray, index++, objVal);
    }

    napi_set_named_property(env, object, "metadata", metadataArray);
    return object;
}

napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    napi_set_named_property(env, object, "name", CreateJsValue(env, applicationInfo.name));
    napi_set_named_property(env, object, "description", CreateJsValue(env, applicationInfo.description));
    napi_set_named_property(env, object, "descriptionId", CreateJsValue(env, applicationInfo.descriptionId));
    napi_set_named_property(env, object, "label", CreateJsValue(env, applicationInfo.label));
    napi_set_named_property(env, object, "labelId", CreateJsValue(env, applicationInfo.labelId));
    napi_set_named_property(env, object, "icon", CreateJsValue(env, applicationInfo.iconPath));
    napi_set_named_property(env, object, "iconId", CreateJsValue(env, applicationInfo.iconId));
    napi_set_named_property(env, object, "codePath", CreateJsValue(env, applicationInfo.codePath));
    return object;
}

napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    HILOG_INFO("CreateJsHapModuleInfo call.");
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Create object failed.");
        return nullptr;
    }

    napi_set_named_property(env, object, "name", CreateJsValue(env, hapModuleInfo.name));
    napi_set_named_property(env, object, "icon", CreateJsValue(env, hapModuleInfo.iconPath));
    napi_set_named_property(env, object, "iconId", CreateJsValue(env, hapModuleInfo.iconId));
    napi_set_named_property(env, object, "label", CreateJsValue(env, hapModuleInfo.label));
    napi_set_named_property(env, object, "labelId", CreateJsValue(env, hapModuleInfo.labelId));
    napi_set_named_property(env, object, "description", CreateJsValue(env, hapModuleInfo.description));
    napi_set_named_property(env, object, "descriptionId", CreateJsValue(env, hapModuleInfo.descriptionId));
    napi_set_named_property(env, object, "mainElementName", CreateJsValue(env, hapModuleInfo.mainElementName));
    
    napi_value abilityInfoArray = nullptr;
    napi_create_array_with_length(env, hapModuleInfo.abilityInfos.size(), &abilityInfoArray);
    
    uint32_t index = 0;
    for (const auto& abilityInfo : hapModuleInfo.abilityInfos) {
        napi_value abilityObject = CreateJsAbilityInfo(env, abilityInfo);
        napi_set_element(env, abilityInfoArray, index++, abilityObject);
    }

    napi_set_named_property(env, object, "abilitiesInfo", abilityInfoArray);
    
    napi_value metadataArray = nullptr;
    napi_create_array_with_length(env, hapModuleInfo.metadata.size(), &metadataArray);

    index = 0;
    for (const auto& metadata : hapModuleInfo.metadata) {
        napi_value objVal = nullptr;
        napi_create_object(env, &objVal);
        if (objVal == nullptr) {
            HILOG_ERROR("Native object is nullptr.");
        } else {
            napi_set_named_property(env, objVal, "name", CreateJsValue(env, metadata.name));
            napi_set_named_property(env, objVal, "value", CreateJsValue(env, metadata.value));
            napi_set_named_property(env, objVal, "resource", CreateJsValue(env, metadata.resource));
        }
        napi_set_element(env, metadataArray, index++, objVal);
    }
    napi_set_named_property(env, object, "metadata", metadataArray);
    return object;
}

napi_value CreateJsConfiguration(napi_env env, const Platform::Configuration& configuration)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }

    napi_set_named_property(env, object, "language", CreateJsValue(env,
        configuration.GetItem(Platform::ConfigurationInner::APPLICATION_LANGUAGE)));
    napi_set_named_property(env, object, "colorMode", CreateJsValue(env,
        configuration.ConvertColorMode(configuration.GetItem(Platform::ConfigurationInner::SYSTEM_COLORMODE))));
    napi_set_named_property(env, object, "direction", CreateJsValue(env,
        configuration.ConvertDirection(configuration.GetItem(Platform::ConfigurationInner::APPLICATION_DIRECTION))));
    napi_set_named_property(env, object, "screenDensity", CreateJsValue(env,
        configuration.ConvertDensity(configuration.GetItem(Platform::ConfigurationInner::APPLICATION_DENSITYDPI))));
    std::string fontSizeScale = configuration.GetItem(Platform::ConfigurationInner::SYSTEM_FONT_SIZE_SCALE);
    napi_set_named_property(env, object, "fontSizeScale",
        CreateJsValue(env, fontSizeScale == "" ? 1.0 : std::stod(fontSizeScale)));
    return object;
}

napi_value CreateJsResourceManager(napi_env env,
    const std::shared_ptr<Global::Resource::ResourceManager>& resMgr, const std::shared_ptr<Platform::Context>& context)
{
    napi_value result = Global::Resource::ResourceManagerAddon::Create(env, "", resMgr, context);
    return result;
}

napi_value CreateJsProcessRunningInfoArray(
    napi_env env, const std::vector<Platform::RunningProcessInfo>& infos)
{
    napi_value array = nullptr;
    napi_create_array_with_length(env, infos.size(), &array);

    if (array != nullptr) {
        uint32_t index = 0;
        for (const auto& runningInfo : infos) {
            napi_set_element(env, array, index++, CreateJsProcessRunningInfo(env, runningInfo));
        }
    }
    return array;
}

napi_value CreateJsProcessRunningInfo(napi_env env, const Platform::RunningProcessInfo& info)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }
    if (object != nullptr) {
        napi_set_named_property(env, object, "processName", CreateJsValue(env, info.processName));
        napi_set_named_property(env, object, "pid", CreateJsValue(env, info.pid));
        napi_set_named_property(env, object, "bundleNames", CreateNativeArray(env, info.bundleNames));
    }
    return object;
}

napi_value CreateJsLaunchParam(napi_env env, const Platform::LaunchParam& launchParam)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return nullptr;
    }

    napi_set_named_property(env, object, "launchReason", CreateJsValue(env, launchParam.launchReason));
    napi_set_named_property(env, object, "lastExitReason", CreateJsValue(env, launchParam.lastExitReason));

    return object;
}
} // namespace AbilityRuntime
} // namespace OHOS