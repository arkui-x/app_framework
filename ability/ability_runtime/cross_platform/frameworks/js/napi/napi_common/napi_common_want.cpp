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
#include "napi_common_util.h"

#include <memory>

#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "hilog.h"
#include "int_wrapper.h"
#include "js_runtime_utils.h"
#include "long_wrapper.h"
#include "napi_common_want.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"
#include "zchar_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
napi_value CreateJsWant(napi_env env, const Want& want)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    napi_set_named_property(env, object, "bundleName", AbilityRuntime::CreateJsValue(env, want.GetBundleName()));
    napi_set_named_property(env, object, "abilityName", AbilityRuntime::CreateJsValue(env, want.GetAbilityName()));
    napi_set_named_property(env, object, "moduleName", AbilityRuntime::CreateJsValue(env, want.GetModuleName()));
    napi_set_named_property(env, object, "type", AbilityRuntime::CreateJsValue(env, want.GetType()));
    auto wantParams = std::static_pointer_cast<AAFwk::WantParams>(want.GetParams());
    napi_set_named_property(env, object, "parameters", CreateJsWantParams(env, *wantParams));
    return object;
}

napi_value CreateJsWantParams(napi_env env, const AAFwk::WantParams& wantParams)
{
    napi_value object = nullptr;
    napi_create_object(env, &object);

    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IString, AAFwk::String, std::string>(env, object, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IBoolean, AAFwk::Boolean, bool>(env, object, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IShort, AAFwk::Short, short>(env, object, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IInteger, AAFwk::Integer, int>(env, object, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::ILong, AAFwk::Long, int64_t>(env, object, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IFloat, AAFwk::Float, float>(env, object, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IDouble, AAFwk::Double, double>(env, object, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IChar, AAFwk::Char, char>(env, object, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapJsWantParams<AAFwk::IByte, AAFwk::Byte, int>(env, object, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray* ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                WrapJsWantParamsArray(env, object, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapJsWantParamsWantParams(env, object, iter->first, wantParams);
        }
    }
    return object;
}

bool InnerWrapJsWantParamsWantParams(
    napi_env env, napi_value object, const std::string& key, const AAFwk::WantParams& wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams* o = AAFwk::IWantParams::Query(value);
    if (o != nullptr) {
        AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
        napi_value propertyValue = CreateJsWantParams(env, wp);
        napi_set_named_property(env, object, key.c_str(), propertyValue);
        return true;
    }
    return false;
}

napi_value WrapArrayWantParamsToJS(napi_env env, const std::vector<AAFwk::WantParams>& value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = WrapWantParams(env, value[i]);
        if (jsValue != nullptr) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool InnerWrapWantParamsArrayWantParamsArkUIX(
    napi_env env, napi_value jsObject, const std::string& key, sptr<AAFwk::IArray>& ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<AAFwk::WantParams> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IWantParams* iValue = AAFwk::IWantParams::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::WantParamWrapper::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = WrapArrayWantParamsToJS(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool WrapJsWantParamsArray(napi_env env, napi_value object, const std::string& key, sptr<AAFwk::IArray>& ao)
{
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IString, AAFwk::String, std::string>(env, object, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IBoolean, AAFwk::Boolean, bool>(env, object, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IShort, AAFwk::Short, short>(env, object, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IInteger, AAFwk::Integer, int>(env, object, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::ILong, AAFwk::Long, int64_t>(env, object, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IFloat, AAFwk::Float, float>(env, object, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IByte, AAFwk::Byte, int>(env, object, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IChar, AAFwk::Char, char>(env, object, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArrayArkUIX<AAFwk::IDouble, AAFwk::Double, double>(env, object, key, ao);
    } else if (AAFwk::Array::IsWantParamsArray(ao)) {
        return InnerWrapWantParamsArrayWantParamsArkUIX(env, object, key, ao);
    } else {
        return false;
    }
}

napi_value WrapWant(napi_env env, const Want& want)
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
    NAPI_CALL(env, napi_create_string_utf8(env, want.GetType().c_str(), NAPI_AUTO_LENGTH, &jsValue));
    NAPI_CALL(env, napi_set_named_property(env, jsObject, "type", jsValue));

    jsValue = nullptr;
    auto wantParams = std::static_pointer_cast<AAFwk::WantParams>(want.GetParams());
    jsValue = WrapWantParams(env, *wantParams);
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
        AAFwk::WantParams wantParams;
        if (UnwrapWantParams(env, jsValue, wantParams)) {
            auto params = std::make_shared<AAFwk::WantParams>(wantParams);
            want.SetParams(params);
        }
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

    natValue = "";
    if (UnwrapStringByPropertyName(env, param, "type", natValue)) {
       want.SetType(natValue);
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

napi_value WantStringToJSValue(napi_env env, const std::string& value)
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