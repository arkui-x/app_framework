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

#include "js_data_struct_converter.h"

#include "hilog.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;

NativeValue* CreateJsAbilityInfo(NativeEngine& engine, const AbilityInfo& abilityInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("bundleName", CreateJsValue(engine, abilityInfo.bundleName));
    object->SetProperty("moduleName", CreateJsValue(engine, abilityInfo.moduleName));
    object->SetProperty("name", CreateJsValue(engine, abilityInfo.name));
    object->SetProperty("label", CreateJsValue(engine, abilityInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, abilityInfo.labelId));
    object->SetProperty("description", CreateJsValue(engine, abilityInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, abilityInfo.descriptionId));
    object->SetProperty("icon", CreateJsValue(engine, abilityInfo.iconPath));
    object->SetProperty("iconId", CreateJsValue(engine, abilityInfo.iconId));
    object->SetProperty("type", CreateJsValue(engine, abilityInfo.type));
    object->SetProperty("applicationInfo", CreateJsApplicationInfo(engine, abilityInfo.applicationInfo));
    return objValue;
}

NativeValue* CreateJsApplicationInfo(NativeEngine& engine, const ApplicationInfo& applicationInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Native object is nullptr.");
        return objValue;
    }

    object->SetProperty("name", CreateJsValue(engine, applicationInfo.name));
    object->SetProperty("description", CreateJsValue(engine, applicationInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, applicationInfo.descriptionId));
    object->SetProperty("label", CreateJsValue(engine, applicationInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, std::to_string(applicationInfo.labelId)));
    object->SetProperty("icon", CreateJsValue(engine, applicationInfo.iconPath));
    object->SetProperty("iconId", CreateJsValue(engine, std::to_string(applicationInfo.iconId)));
    object->SetProperty("codePath", CreateJsValue(engine, applicationInfo.codePath));
    return objValue;
}

NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, const HapModuleInfo& hapModuleInfo)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        return objValue;
    }

    object->SetProperty("name", CreateJsValue(engine, hapModuleInfo.name));
    object->SetProperty("icon", CreateJsValue(engine, hapModuleInfo.iconPath));
    object->SetProperty("iconId", CreateJsValue(engine, hapModuleInfo.iconId));
    object->SetProperty("label", CreateJsValue(engine, hapModuleInfo.label));
    object->SetProperty("labelId", CreateJsValue(engine, hapModuleInfo.labelId));
    object->SetProperty("description", CreateJsValue(engine, hapModuleInfo.description));
    object->SetProperty("descriptionId", CreateJsValue(engine, hapModuleInfo.descriptionId));
    object->SetProperty("mainElementName",CreateJsValue(engine,hapModuleInfo.mainElementName));

    NativeValue *abilityInfoArrayValue = engine.CreateArray(hapModuleInfo.abilityInfos.size());
    NativeArray *abilityInfoArray = ConvertNativeValueTo<NativeArray>(abilityInfoArrayValue);
    if (abilityInfoArray != nullptr) {
        uint32_t index = 0;
        for (const auto& abilityInfo : hapModuleInfo.abilityInfos) {
            abilityInfoArray->SetElement(index++, CreateJsAbilityInfo(engine, abilityInfo));
        }
    }
    object->SetProperty("abilitiesInfo",abilityInfoArrayValue);
    
    return objValue;
}
} // namespace AbilityRuntime
} // namespace OHOS