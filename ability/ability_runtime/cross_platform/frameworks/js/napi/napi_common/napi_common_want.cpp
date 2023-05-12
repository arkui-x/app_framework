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

#include "napi_common_want.h"

#include "hilog.h"
#include "js_runtime_utils.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AppExecFwk {
NativeValue* CreateJsWant(NativeEngine& engine, const Want& want)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = AbilityRuntime::ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return objValue;
    }

    object->SetProperty("bundleName", AbilityRuntime::CreateJsValue(engine, want.GetBundleName()));
    object->SetProperty("abilityName", AbilityRuntime::CreateJsValue(engine, want.GetAbilityName()));
    object->SetProperty("moduleName", AbilityRuntime::CreateJsValue(engine, want.GetModuleName()));
    object->SetProperty("parameters", CreateJsWantParams(engine, want));
    return objValue;
}

NativeValue* CreateJsWantParams(NativeEngine& engine, const Want& want)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = AbilityRuntime::ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return objValue;
    }

    std::map<std::string, int> types = want.GetTypes();
    for (auto iter = types.begin(); iter != types.end(); iter++) {
        if (iter->second == AAFwk::VALUE_TYPE_BOOLEAN) {
            auto natValue = want.GetBoolParam(iter->first, false);
            object->SetProperty(iter->first.c_str(), OHOS::AbilityRuntime::CreateJsValue(engine, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_INT) {
            auto natValue = want.GetIntParam(iter->first, 0);
            object->SetProperty(iter->first.c_str(), OHOS::AbilityRuntime::CreateJsValue(engine, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_DOUBLE) {
            auto natValue = want.GetDoubleParam(iter->first, 0);
            object->SetProperty(iter->first.c_str(), OHOS::AbilityRuntime::CreateJsValue(engine, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_STRING) {
            auto natValue = want.GetStringParam(iter->first);
            object->SetProperty(iter->first.c_str(), OHOS::AbilityRuntime::CreateJsValue(engine, natValue));
        }
    }
    return objValue;
}

bool UnwrapWant(napi_env env, napi_value param, Want& want)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return false;
    }

    napi_value jsValue = GetPropertyValueByPropertyName(env, param, "parameters", napi_object);
    if (jsValue != nullptr) {
        UnwrapWantParams(env, jsValue, want);
    }

    std::string natValue("");
    if (UnwrapStringByPropertyName(env, param, "bundleName", natValue)) {
        want.SetBundleName(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "abilityName", natValue)) {
        want.SetAbilityName(natValue);
    }

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "moduleName", natValue)) {
        want.SetModuleName(natValue);
    }
    return true;
}

bool UnwrapWantParams(napi_env env, napi_value param, Want& want)
{
    if (!IsTypeForNapiValue(env, param, napi_object)) {
        return false;
    }

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, param, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);

    napi_value jsProName = nullptr;
    napi_value jsProValue = nullptr;
    for (uint32_t index = 0; index < jsProCount; index++) {
        NAPI_CALL_BASE(env, napi_get_element(env, jsProNameList, index, &jsProName), false);

        std::string strProName = UnwrapStringFromJS(env, jsProName);
        HILOG_DEBUG("%{public}s called. Property name=%{public}s.", __func__, strProName.c_str());
        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = UnwrapStringFromJS(env, jsProValue);
                want.SetParam(strProName, natValue);
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                want.SetParam(strProName, natValue);
                break;
            }
            case napi_number: {
                InnerUnwrapWantParamsNumber(env, strProName, jsProValue, want);
                break;
            }
            default:
                break;
        }
    }

    return true;
}

void InnerUnwrapWantParamsNumber(napi_env env, const std::string& key, napi_value param, Want& want)
{
    int32_t natValue32 = 0;
    double natValueDouble = 0.0;
    bool isReadValue32 = false;
    bool isReadDouble = false;
    if (napi_get_value_int32(env, param, &natValue32) == napi_ok) {
        isReadValue32 = true;
    }

    if (napi_get_value_double(env, param, &natValueDouble) == napi_ok) {
        isReadDouble = true;
    }

    if (isReadValue32 && isReadDouble) {
        if (abs(natValueDouble - natValue32 * 1.0) > 0.0) {
            want.SetParam(key, natValueDouble);
        } else {
            want.SetParam(key, natValue32);
        }
    } else if (isReadValue32) {
        want.SetParam(key, natValue32);
    } else if (isReadDouble) {
        want.SetParam(key, natValueDouble);
    }
}
} // namespace AppExecFwk
} // namespace OHOS