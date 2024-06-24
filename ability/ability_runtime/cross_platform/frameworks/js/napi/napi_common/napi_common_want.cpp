/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
napi_value CreateJsWant(napi_env env, const Want &want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_set_named_property(env, object, "bundleName",
        AbilityRuntime::CreateJsValue(env, want.GetBundleName()));
    napi_set_named_property(env, object, "abilityName",
        AbilityRuntime::CreateJsValue(env, want.GetAbilityName()));
    napi_set_named_property(env, object, "moduleName",
        AbilityRuntime::CreateJsValue(env, want.GetModuleName()));
    napi_set_named_property(env, object, "parameters",
        CreateJsWantParams(env, want));
    return object;
}

napi_value CreateJsWantParams(napi_env env, const Want& want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    std::map<std::string, int> types = want.GetTypes();
    for (auto iter = types.begin(); iter != types.end(); iter++) {
        if (iter->second == AAFwk::VALUE_TYPE_BOOLEAN) {
            auto natValue = want.GetBoolParam(iter->first, false);
            napi_set_named_property(env, object, iter->first.c_str(),
                OHOS::AbilityRuntime::CreateJsValue(env, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_INT) {
            auto natValue = want.GetIntParam(iter->first, 0);
            napi_set_named_property(env, object, iter->first.c_str(),
                OHOS::AbilityRuntime::CreateJsValue(env, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_DOUBLE) {
            auto natValue = want.GetDoubleParam(iter->first, 0);
            napi_set_named_property(env, object, iter->first.c_str(),
                OHOS::AbilityRuntime::CreateJsValue(env, natValue));
        } else if (iter->second == AAFwk::VALUE_TYPE_STRING) {
            auto natValue = want.GetStringParam(iter->first);
            napi_set_named_property(env, object, iter->first.c_str(),
                OHOS::AbilityRuntime::CreateJsValue(env, natValue));
        }
    }
    return object;
}

bool InnerWrapWantParamsStringByWant(
    napi_env env, napi_value jsObject, const std::string &key, const Want &want)
{
    auto natValue = want.GetStringParam(key);
    napi_value jsValue = WantStringToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsBoolByWant(
    napi_env env, napi_value jsObject, const std::string &key, const Want &want)
{
    auto natValue = want.GetBoolParam(key, false);
    napi_value jsValue = WantBoolToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsDoubleByWant(
    napi_env env, napi_value jsObject, const std::string &key, const Want &want)
{
    auto natValue = want.GetDoubleParam(key, 0);
    napi_value jsValue = WantDoubleToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt32ByWant(
    napi_env env, napi_value jsObject, const std::string &key, const Want &want)
{
    auto natValue = want.GetIntParam(key, 0);
    napi_value jsValue = WantInt32ToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

napi_value WrapWantParamsByWant(napi_env env, const Want &want)
{
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    const std::map<std::string, std::shared_ptr<void>> paramList = want.GetParams();
    std::map<std::string, int> paramTypes = want.GetTypes();

    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        auto key = iter->first;
        auto it = paramTypes.find(key);
        if (it != paramTypes.end()) {
            if (it->second == AAFwk::VALUE_TYPE_BOOLEAN) {
                InnerWrapWantParamsBoolByWant(env, jsObject, iter->first, want);
            } else if (it->second == AAFwk::VALUE_TYPE_INT) {
                InnerWrapWantParamsInt32ByWant(env, jsObject, iter->first, want);
            } else if (it->second == AAFwk::VALUE_TYPE_DOUBLE) {
                InnerWrapWantParamsDoubleByWant(env, jsObject, iter->first, want);
            }  else if (it->second == AAFwk::VALUE_TYPE_STRING) {
                InnerWrapWantParamsStringByWant(env, jsObject, iter->first, want);
            }
        }
    }

    return jsObject;
}

napi_value WrapWant(napi_env env, const Want &want)
{
    napi_value jsObject = nullptr;
    napi_value jsValue = nullptr;

    NAPI_CALL(env, napi_create_object(env, &jsObject));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, want.GetBundleName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "bundleName", jsValue));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, want.GetAbilityName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "abilityName", jsValue));

    jsValue = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, want.GetModuleName().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "moduleName", jsValue));

    jsValue = nullptr;
    jsValue = WrapWantParamsByWant(env, want);
    SetPropertyValueByPropertyName(env, jsObject, "parameters", jsValue);

    return jsObject;
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

napi_value WantStringToJSValue(napi_env env, const std::string &value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result));
    return result;
}

napi_value WantBoolToJSValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value WantInt32ToJSValue(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

napi_value WantDoubleToJSValue(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}
} // namespace AppExecFwk
} // namespace OHOS