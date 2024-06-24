/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "array_wrapper.h"
#include "bool_wrapper.h"
#include "byte_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "securec.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"
#include "zchar_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
const int PROPERTIES_SIZE = 2;

typedef struct __ComplexArrayData {
    std::vector<int> intList;
    std::vector<long> longList;
    std::vector<bool> boolList;
    std::vector<double> doubleList;
    std::vector<std::string> stringList;
    std::vector<napi_value> objectList;
} ComplexArrayData;

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams& wantParams);
bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams& wantParams);

napi_value StringToJSValue(napi_env env, const std::string &value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result));
    return result;
}

napi_value BoolToJSValue(napi_env env, bool value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_get_boolean(env, value, &result));
    return result;
}

napi_value Int32ToJSValue(napi_env env, int32_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, value, &result));
    return result;
}

napi_value Int64ToJSValue(napi_env env, int64_t value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int64(env, value, &result));
    return result;
}

napi_value DoubleToJSValue(napi_env env, double value)
{
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_double(env, value, &result));
    return result;
}

napi_value ArrayStringToJSValue(napi_env env, const std::vector<std::string> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_string_utf8(env, value[i].c_str(), NAPI_AUTO_LENGTH, &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

napi_value ArrayBoolToJSValue(napi_env env, const std::vector<bool> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_get_boolean(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

napi_value ArrayInt32ToJSValue(napi_env env, const std::vector<int> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_int32(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

napi_value ArrayInt64ToJSValue(napi_env env, const std::vector<int64_t> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_int64(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

bool IsArrayForNapiValue(napi_env env, napi_value param, uint32_t &arraySize)
{
    bool isArray = false;
    arraySize = 0;

    if (napi_is_array(env, param, &isArray) != napi_ok || isArray == false) {
        return false;
    }

    return (napi_get_array_length(env, param, &arraySize) == napi_ok);
}

bool IsSameTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
{
    napi_valuetype valueType = napi_undefined;

    if (param == nullptr) {
        return false;
    }

    if (napi_typeof(env, param, &valueType) != napi_ok) {
        return false;
    }

    return valueType == expectType;
}

bool InnerWrapWantParamsBool(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IBoolean *bo = AAFwk::IBoolean::Query(value);
    if (bo == nullptr) {
        return false;
    }

    bool natValue = AAFwk::Boolean::Unbox(bo);
    napi_value jsValue = BoolToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsShort(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IShort *ao = AAFwk::IShort::Query(value);
    if (ao == nullptr) {
        return false;
    }

    short natValue = AAFwk::Short::Unbox(ao);
    napi_value jsValue = Int32ToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt32(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IInteger *ao = AAFwk::IInteger::Query(value);
    if (ao == nullptr) {
        return false;
    }

    int natValue = AAFwk::Integer::Unbox(ao);
    napi_value jsValue = Int32ToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsInt64(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::ILong *ao = AAFwk::ILong::Query(value);
    if (ao == nullptr) {
        return false;
    }

    int64_t natValue = AAFwk::Long::Unbox(ao);
    napi_value jsValue = Int64ToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsFloat(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IFloat *ao = AAFwk::IFloat::Query(value);
    if (ao == nullptr) {
        return false;
    }

    float natValue = AAFwk::Float::Unbox(ao);
    napi_value jsValue = DoubleToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsDouble(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IDouble *ao = AAFwk::IDouble::Query(value);
    if (ao == nullptr) {
        return false;
    }

    double natValue = AAFwk::Double::Unbox(ao);
    napi_value jsValue = DoubleToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsChar(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IChar *ao = AAFwk::IChar::Query(value);
    if (ao == nullptr) {
        return false;
    }

    std::string natValue(static_cast<AAFwk::Char *>(ao)->ToString());
    napi_value jsValue = StringToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsString(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IString *ao = AAFwk::IString::Query(value);
    if (ao == nullptr) {
        return false;
    }

    std::string natValue = AAFwk::String::Unbox(ao);
    napi_value jsValue = StringToJSValue(env, natValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsByte(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IByte *bo = AAFwk::IByte::Query(value);
    if (bo == nullptr) {
        return false;
    }

    int intValue = (int)AAFwk::Byte::Unbox(bo);
    napi_value jsValue = Int32ToJSValue(env, intValue);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

bool InnerWrapWantParamsArrayString(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IString *iValue = AAFwk::IString::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::String::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayStringToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayBool(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<bool> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IBoolean *iValue = AAFwk::IBoolean::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Boolean::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayBoolToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayShort(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IShort *iValue = AAFwk::IShort::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Short::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayInt32ToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt32(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IInteger *iValue = AAFwk::IInteger::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Integer::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayInt32ToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayInt64(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int64_t> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::ILong *iValue = AAFwk::ILong::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Long::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayInt64ToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

napi_value ArrayDoubleToJSValue(napi_env env, const std::vector<double> &value)
{
    napi_value jsArray = nullptr;
    napi_value jsValue = nullptr;
    uint32_t index = 0;

    NAPI_CALL(env, napi_create_array(env, &jsArray));
    for (uint32_t i = 0; i < value.size(); i++) {
        jsValue = nullptr;
        if (napi_create_double(env, value[i], &jsValue) == napi_ok) {
            if (napi_set_element(env, jsArray, index, jsValue) == napi_ok) {
                index++;
            }
        }
    }
    return jsArray;
}

napi_value ArrayWantParamsToJSValue(napi_env env, const std::vector<AAFwk::WantParams> &value)
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

bool InnerWrapWantParamsArrayFloat(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IFloat *iValue = AAFwk::IFloat::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Float::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayDoubleToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayByte(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<int> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IByte *iValue = AAFwk::IByte::Query(iface);
            if (iValue != nullptr) {
                int intValue = (int)AAFwk::Byte::Unbox(iValue);
                natArray.push_back(intValue);
            }
        }
    }

    napi_value jsValue = ArrayInt32ToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayChar(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<std::string> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IChar *iValue = AAFwk::IChar::Query(iface);
            if (iValue != nullptr) {
                std::string str(static_cast<AAFwk::Char *>(iValue)->ToString());
                natArray.push_back(str);
            }
        }
    }

    napi_value jsValue = ArrayStringToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayDouble(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<double> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IDouble *iValue = AAFwk::IDouble::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::Double::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayDoubleToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArrayWantParams(napi_env env, napi_value jsObject,
    const std::string &key, sptr<AAFwk::IArray> &ao)
{
    long size = 0;
    if (ao->GetLength(size) != ERR_OK) {
        return false;
    }

    std::vector<AAFwk::WantParams> natArray;
    for (long i = 0; i < size; i++) {
        sptr<AAFwk::IInterface> iface = nullptr;
        if (ao->Get(i, iface) == ERR_OK) {
            AAFwk::IWantParams *iValue = AAFwk::IWantParams::Query(iface);
            if (iValue != nullptr) {
                natArray.push_back(AAFwk::WantParamWrapper::Unbox(iValue));
            }
        }
    }

    napi_value jsValue = ArrayWantParamsToJSValue(env, natArray);
    if (jsValue != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
        return true;
    }
    return false;
}

bool InnerWrapWantParamsArray(napi_env env, napi_value jsObject, const std::string &key, sptr<AAFwk::IArray> &ao)
{
    if (AAFwk::Array::IsStringArray(ao)) {
        return InnerWrapWantParamsArrayString(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsBooleanArray(ao)) {
        return InnerWrapWantParamsArrayBool(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsShortArray(ao)) {
        return InnerWrapWantParamsArrayShort(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsIntegerArray(ao)) {
        return InnerWrapWantParamsArrayInt32(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsLongArray(ao)) {
        return InnerWrapWantParamsArrayInt64(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsFloatArray(ao)) {
        return InnerWrapWantParamsArrayFloat(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsByteArray(ao)) {
        return InnerWrapWantParamsArrayByte(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsCharArray(ao)) {
        return InnerWrapWantParamsArrayChar(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsDoubleArray(ao)) {
        return InnerWrapWantParamsArrayDouble(env, jsObject, key, ao);
    } else if (AAFwk::Array::IsWantParamsArray(ao)) {
        return InnerWrapWantParamsArrayWantParams(env, jsObject, key, ao);
    } else {
        return false;
    }
}

bool InnerWrapWantParamsWantParams(
    napi_env env, napi_value jsObject, const std::string &key, const AAFwk::WantParams &wantParams)
{
    auto value = wantParams.GetParam(key);
    AAFwk::IWantParams *o = AAFwk::IWantParams::Query(value);
    if (o == nullptr) {
        return false;
    }

    AAFwk::WantParams wp = AAFwk::WantParamWrapper::Unbox(o);
    napi_value jsValue = WrapWantParams(env, wp);
    if (jsValue == nullptr) {
        return false;
    }

    NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
    return true;
}

void InnerUnwrapWantParamsNumber(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
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
            wantParams.SetParam(key, AAFwk::Double::Box(natValueDouble));
        } else {
            wantParams.SetParam(key, AAFwk::Integer::Box(natValue32));
        }
    } else if (isReadValue32) {
        wantParams.SetParam(key, AAFwk::Integer::Box(natValue32));
    } else if (isReadDouble) {
        wantParams.SetParam(key, AAFwk::Double::Box(natValueDouble));
    }
}

bool UnwrapArrayComplexFromJSNumber(napi_env env, ComplexArrayData &value, bool isDouble, napi_value jsValue)
{
    int32_t elementInt32 = 0;
    double elementDouble = 0.0;
    if (isDouble) {
        if (napi_get_value_double(env, jsValue, &elementDouble) == napi_ok) {
            value.doubleList.push_back(elementDouble);
        }
        return isDouble;
    }

    bool isReadValue32 = napi_get_value_int32(env, jsValue, &elementInt32) == napi_ok;
    bool isReadDouble = napi_get_value_double(env, jsValue, &elementDouble) == napi_ok;
    if (isReadValue32 && isReadDouble) {
        if (abs(elementDouble - elementInt32 * 1.0) > 0.0) {
            isDouble = true;
            if (value.intList.size() > 0) {
                for (size_t j = 0; j < value.intList.size(); j++) {
                    value.doubleList.push_back(value.intList[j]);
                }
                value.intList.clear();
            }
            value.doubleList.push_back(elementDouble);
        } else {
            value.intList.push_back(elementInt32);
        }
    } else if (isReadValue32) {
        value.intList.push_back(elementInt32);
    } else if (isReadDouble) {
        isDouble = true;
        if (value.intList.size() > 0) {
            for (size_t j = 0; j < value.intList.size(); j++) {
                value.doubleList.push_back(value.intList[j]);
            }
            value.intList.clear();
        }
        value.doubleList.push_back(elementDouble);
    }
    return isDouble;
}

bool GetStringFromJSValue(napi_env env, napi_value param, std::string &value)
{
    value = "";
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return false;
    }

    if (size == 0) {
        return true;
    }

    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return false;
    }
    (void)memset_s(buf, (size + 1), 0, (size + 1));

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    }
    delete[] buf;
    buf = nullptr;
    return rev;
}

std::string GetStringWithDefaultFromJSValue(napi_env env, napi_value param, const std::string &defaultValue)
{
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return defaultValue;
    }

    std::string value("");
    if (size == 0) {
        return defaultValue;
    }

    char *buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return value;
    }
    (void)memset_s(buf, size + 1, 0, size + 1);

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    } else {
        value = defaultValue;
    }

    delete[] buf;
    buf = nullptr;
    return value;
}

bool GetArrayComplexFromJSValue(napi_env env, napi_value param, ComplexArrayData &value)
{
    uint32_t arraySize = 0;
    if (!IsArrayForNapiValue(env, param, arraySize)) {
        return false;
    }

    napi_valuetype valueType = napi_undefined;
    napi_value jsValue = nullptr;
    bool isDouble = false;

    value.intList.clear();
    value.longList.clear();
    value.boolList.clear();
    value.doubleList.clear();
    value.stringList.clear();
    value.objectList.clear();

    for (uint32_t i = 0; i < arraySize; i++) {
        jsValue = nullptr;
        valueType = napi_undefined;
        NAPI_CALL_BASE(env, napi_get_element(env, param, i, &jsValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsValue, &valueType), false);
        switch (valueType) {
            case napi_string: {
                std::string elementValue("");
                if (GetStringFromJSValue(env, jsValue, elementValue)) {
                    value.stringList.push_back(elementValue);
                } else {
                    return false;
                }
                break;
            }
            case napi_boolean: {
                bool elementValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsValue, &elementValue), false);
                value.boolList.push_back(elementValue);
                break;
            }
            case napi_number: {
                isDouble = UnwrapArrayComplexFromJSNumber(env, value, isDouble, jsValue);
                break;
            }
            case napi_object: {
                value.objectList.push_back(jsValue);
                break;
            }
            default:
                break;
        }
    }
    return true;
}

bool InnerSetWantParamsArrayObject(napi_env env, const std::string &key,
    const std::vector<napi_value> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IWantParams);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            AAFwk::WantParams wp;
            UnwrapWantParams(env, value[i], wp);
            ao->Set(i, AAFwk::WantParamWrapper::Box(wp));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayString(
    const std::string &key, const std::vector<std::string> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IString);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::String::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayInt(const std::string &key, const std::vector<int> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IInteger);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Integer::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayLong(const std::string &key, const std::vector<long> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_ILong);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Long::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayBool(const std::string &key, const std::vector<bool> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IBoolean);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Boolean::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerSetWantParamsArrayDouble(
    const std::string &key, const std::vector<double> &value, AAFwk::WantParams &wantParams)
{
    size_t size = value.size();
    sptr<AAFwk::IArray> ao = new (std::nothrow) AAFwk::Array(size, AAFwk::g_IID_IDouble);
    if (ao != nullptr) {
        for (size_t i = 0; i < size; i++) {
            ao->Set(i, AAFwk::Double::Box(value[i]));
        }
        wantParams.SetParam(key, ao);
        return true;
    } else {
        return false;
    }
}

bool InnerUnwrapWantParamsArray(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    ComplexArrayData natArrayValue;
    if (!GetArrayComplexFromJSValue(env, param, natArrayValue)) {
        return false;
    }
    if (natArrayValue.objectList.size() > 0) {
        return InnerSetWantParamsArrayObject(env, key, natArrayValue.objectList, wantParams);
    }
    if (natArrayValue.stringList.size() > 0) {
        return InnerSetWantParamsArrayString(key, natArrayValue.stringList, wantParams);
    }
    if (natArrayValue.intList.size() > 0) {
        return InnerSetWantParamsArrayInt(key, natArrayValue.intList, wantParams);
    }
    if (natArrayValue.longList.size() > 0) {
        return InnerSetWantParamsArrayLong(key, natArrayValue.longList, wantParams);
    }
    if (natArrayValue.boolList.size() > 0) {
        return InnerSetWantParamsArrayBool(key, natArrayValue.boolList, wantParams);
    }
    if (natArrayValue.doubleList.size() > 0) {
        return InnerSetWantParamsArrayDouble(key, natArrayValue.doubleList, wantParams);
    }

    return false;
}

bool InnerUnwrapWantParams(napi_env env, const std::string &key, napi_value param, AAFwk::WantParams &wantParams)
{
    AAFwk::WantParams wp;
    if (UnwrapWantParams(env, param, wp)) {
        sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
        if (pWantParams != nullptr) {
            wantParams.SetParam(key, pWantParams);
            return true;
        }
    }
    return false;
}

bool IsSpecialObject(napi_env env, napi_value param, std::string &strProName, const std::string &type,
    napi_valuetype jsValueProType)
{
    napi_value jsWantParamProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsWantParamProValue), false);

    napi_valuetype jsValueType = napi_undefined;
    napi_value jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, AAFwk::TYPE_PROPERTY, &jsProValue), false);
    NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);
    if (jsValueType != napi_string) {
        return false;
    }
    std::string natValue = GetStringWithDefaultFromJSValue(env, jsProValue, std::string(""));
    if (natValue != type) {
        return false;
    }
    napi_value jsProNameList = nullptr;
    uint32_t jsProCount = 0;

    NAPI_CALL_BASE(env, napi_get_property_names(env, jsWantParamProValue, &jsProNameList), false);
    NAPI_CALL_BASE(env, napi_get_array_length(env, jsProNameList, &jsProCount), false);

    if (jsProCount != PROPERTIES_SIZE) {
        return false;
    }

    jsValueType = napi_undefined;
    jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, AAFwk::VALUE_PROPERTY, &jsProValue),
        false);
    NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);
    return (jsValueType == jsValueProType);
}

bool HandleFdObject(napi_env env, napi_value param, std::string &strProName, AAFwk::WantParams &wantParams)
{
    napi_value jsWantParamProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsWantParamProValue), false);
    napi_value jsProValue = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, jsWantParamProValue, AAFwk::VALUE_PROPERTY, &jsProValue),
        false);

    int32_t natValue32 = 0;
    napi_get_value_int32(env, jsProValue, &natValue32);
    AAFwk::WantParams wp;
    wp.SetParam(AAFwk::TYPE_PROPERTY, AAFwk::String::Box(AAFwk::FD));
    wp.SetParam(AAFwk::VALUE_PROPERTY, AAFwk::Integer::Box(natValue32));
    sptr<AAFwk::IWantParams> pWantParams = AAFwk::WantParamWrapper::Box(wp);
    wantParams.SetParam(strProName, pWantParams);
    return true;
}

void HandleNapiObject(napi_env env, napi_value param, napi_value jsProValue, std::string &strProName,
    AAFwk::WantParams &wantParams)
{
    if (IsSpecialObject(env, param, strProName, AAFwk::FD, napi_number)) {
        HandleFdObject(env, param, strProName, wantParams);
    } else if (IsSpecialObject(env, param, strProName, "RemoteObject", napi_object)) {
    } else {
        bool isArray = false;
        if (napi_is_array(env, jsProValue, &isArray) == napi_ok) {
            if (isArray) {
                InnerUnwrapWantParamsArray(env, strProName, jsProValue, wantParams);
            } else {
                InnerUnwrapWantParams(env, strProName, jsProValue, wantParams);
            }
        }
    }
}

napi_value WrapWantParams(napi_env env, const AAFwk::WantParams& wantParams)
{
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    const std::map<std::string, sptr<AAFwk::IInterface>> paramList = wantParams.GetParams();
    for (auto iter = paramList.begin(); iter != paramList.end(); iter++) {
        if (AAFwk::IString::Query(iter->second) != nullptr) {
            InnerWrapWantParamsString(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IBoolean::Query(iter->second) != nullptr) {
            InnerWrapWantParamsBool(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IShort::Query(iter->second) != nullptr) {
            InnerWrapWantParamsShort(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IInteger::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt32(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::ILong::Query(iter->second) != nullptr) {
            InnerWrapWantParamsInt64(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IFloat::Query(iter->second) != nullptr) {
            InnerWrapWantParamsFloat(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IDouble::Query(iter->second) != nullptr) {
            InnerWrapWantParamsDouble(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IChar::Query(iter->second) != nullptr) {
            InnerWrapWantParamsChar(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IByte::Query(iter->second) != nullptr) {
            InnerWrapWantParamsByte(env, jsObject, iter->first, wantParams);
        } else if (AAFwk::IArray::Query(iter->second) != nullptr) {
            AAFwk::IArray *ao = AAFwk::IArray::Query(iter->second);
            if (ao != nullptr) {
                sptr<AAFwk::IArray> array(ao);
                InnerWrapWantParamsArray(env, jsObject, iter->first, array);
            }
        } else if (AAFwk::IWantParams::Query(iter->second) != nullptr) {
            InnerWrapWantParamsWantParams(env, jsObject, iter->first, wantParams);
        }
    }
    return jsObject;
}

bool UnwrapWantParams(napi_env env, napi_value param, AAFwk::WantParams& wantParams)
{
    if (!IsSameTypeForNapiValue(env, param, napi_object)) {
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

        std::string strProName = GetStringWithDefaultFromJSValue(env, jsProName, std::string(""));

        NAPI_CALL_BASE(env, napi_get_named_property(env, param, strProName.c_str(), &jsProValue), false);
        NAPI_CALL_BASE(env, napi_typeof(env, jsProValue, &jsValueType), false);

        switch (jsValueType) {
            case napi_string: {
                std::string natValue = GetStringWithDefaultFromJSValue(env, jsProValue, std::string(""));
                wantParams.SetParam(strProName, AAFwk::String::Box(natValue));
                break;
            }
            case napi_boolean: {
                bool natValue = false;
                NAPI_CALL_BASE(env, napi_get_value_bool(env, jsProValue, &natValue), false);
                wantParams.SetParam(strProName, AAFwk::Boolean::Box(natValue));
                break;
            }
            case napi_number: {
                InnerUnwrapWantParamsNumber(env, strProName, jsProValue, wantParams);
                break;
            }
            case napi_object: {
                HandleNapiObject(env, param, jsProValue, strProName, wantParams);
                break;
            }
            default:
                break;
        }
    }
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS