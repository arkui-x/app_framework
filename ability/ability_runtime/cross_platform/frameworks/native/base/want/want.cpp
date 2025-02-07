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

#include "want.h"

#include <memory>
#include <regex>

#include "array_wrapper.h"
#include "base_interfaces.h"
#include "base_obj.h"
#include "bool_wrapper.h"
#include "double_wrapper.h"
#include "float_wrapper.h"
#include "hilog.h"
#include "int_wrapper.h"
#include "long_wrapper.h"
#include "securec.h"
#include "short_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "want_params_wrapper.h"

namespace OHOS {
namespace AAFwk {
const std::string Want::ABILITY_ID("ability_id");
const std::string Want::INSTANCE_NAME("instance_name");
const std::string Want::ELEMENT_BUNDLE_NAME("elementBundleName");
namespace {
using Json = nlohmann::json;
const std::regex NUMBER_REGEX("^[-+]?([0-9]+)([.]([0-9]+))?$");
const std::regex BOOL_REGEX("^(true|false)$");
const std::regex INT_REGEX("^[-+]?([0-9]+)$");

void SetBoolIntDouble(AAFwk::WantParams& wantParams, const std::string& key, const std::string& value,
    const OHOS::AAFwk::WantValueType type)
{
    std::regex pattern(R"(^\s+|\s+$)");
    std::string valueStr = std::regex_replace(value, pattern, "");
    switch (type) {
        case OHOS::AAFwk::WantValueType::VALUE_TYPE_INT:
            if (std::regex_match(valueStr, INT_REGEX)) {
                wantParams.SetParam(key, WantParams::GetInterfaceByType(static_cast<int>(type), valueStr));
            } else {
                HILOG_ERROR("Want parse failed. int value is incorrect. value = %{public}s", valueStr.c_str());
            }
            break;
        case OHOS::AAFwk::WantValueType::VALUE_TYPE_BOOLEAN:
            if (std::regex_match(valueStr, BOOL_REGEX)) {
                wantParams.SetParam(key, WantParams::GetInterfaceByType(static_cast<int>(type), valueStr));
            } else {
                HILOG_ERROR("Want parse failed. bool value is incorrect. value = %{public}s", valueStr.c_str());
            }
            break;
        case OHOS::AAFwk::WantValueType::VALUE_TYPE_DOUBLE:
            if (std::regex_match(valueStr, NUMBER_REGEX)) {
                wantParams.SetParam(key, WantParams::GetInterfaceByType(static_cast<int>(type) - 1, valueStr));
            } else {
                HILOG_ERROR("Want parse failed. double value is incorrect. value = %{public}s", valueStr.c_str());
            }
            break;
        default:
            break;
    }
}

void SetDouble(Json& elemetnValue, std::shared_ptr<AAFwk::WantParams> wantParams, std::string elementKey,
    AAFwk::WantValueType localType)
{
    auto intType = elemetnValue.type();
    if (intType == Json::value_t::number_float) {
        wantParams->SetParam(
            elementKey, WantParams::GetInterfaceByType(static_cast<int>(WantValueType::VALUE_TYPE_DOUBLE) - 1,
                            std::to_string(elemetnValue.get<double>())));
    } else if (intType == Json::value_t::number_integer || intType == Json::value_t::number_unsigned) {
        wantParams->SetParam(
            elementKey, WantParams::GetInterfaceByType(static_cast<int>(WantValueType::VALUE_TYPE_DOUBLE) - 1,
                            std::to_string(elemetnValue.get<int64_t>())));
    } else if (intType == Json::value_t::string) {
        SetBoolIntDouble(*wantParams, elementKey, elemetnValue, localType);
    }
}

bool CheckElementIsVaild(const Json& element)
{
    if (element.find(JSON_WANTPARAMS_KEY) == element.end() || element.find(JSON_WANTPARAMS_TYPE) == element.end() ||
        element.find(JSON_WANTPARAMS_VALUE) == element.end()) {
        HILOG_ERROR("Want parse failed. not a valid element. value = %{public}s", element.dump().c_str());
        return false;
    }
    return true;
}

bool CheckParamsIsVaild(const std::string& params, Json& jsonObject)
{
    if (params.empty()) {
        return false;
    }
    jsonObject = Json::parse(params.c_str(), nullptr, false);
    if (jsonObject.is_discarded() || !jsonObject.contains(JSON_WANTPARAMS_PARAM)) {
        HILOG_ERROR("jsonObject is discarded. value = %{public}s", params.c_str());
        return false;
    }
    if (jsonObject[JSON_WANTPARAMS_PARAM].is_array() == false) {
        HILOG_ERROR("jsonObject is not array. value = %{public}s", params.c_str());
        return false;
    }
    return true;
}

template<typename T1, typename T2, typename T3>
void GetArrayParams(IArray* ao, std::vector<T3>& array)
{
    auto func = [&](IInterface* object) {
        if (object != nullptr) {
            T1* value = T1::Query(object);
            if (value != nullptr) {
                array.push_back(T2::Unbox(value));
            }
        }
    };
    Array::ForEach(ao, func);
}
}; // namespace

/**
 * @description:Default construcotr of Want class, which is used to initialzie flags and URI.
 * @param None
 * @return None
 */
Want::Want()
{
    wantParams_ = std::make_shared<AAFwk::WantParams>();
}

/**
 * @description: Default deconstructor of Want class
 * @param None
 * @return None
 */
Want::~Want() {}

/**
 * @description: Copy construcotr of Want class, which is used to initialzie flags, URI, etc.
 * @param want the source instance of Want.
 * @return None
 */
Want::Want(const Want& want)
{
    InnerCopyWant(want);
}

Want& Want::operator=(const Want& want)
{
    InnerCopyWant(want);
    return *this;
}

/**
 * @description: clear the specific want object.
 * @param want Indicates the want to clear
 */
void Want::ClearWant(Want* want)
{
    want->wantParams_ = std::make_shared<WantParams>();
    want->bundleName_ = "";
    want->moduleName_ = "";
    want->abilityName_ = "";
    want->type_ = "";
}

/**
 * @description: Obtains a bool-type value matching the given key.
 * @param key   Indicates the key of WantParams.
 * @param defaultValue  Indicates the default bool-type value.
 * @return Returns the bool-type value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
bool Want::GetBoolParam(const std::string& key, bool defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IBoolean* bo = IBoolean::Query(value);
    if (bo != nullptr) {
        return Boolean::Unbox(bo);
    }
    return defaultValue;
}

/**
 * @description:Obtains a bool-type array matching the given key.
 * @param key   Indicates the key of WantParams.
 * @return Returns the bool-type array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<bool> Want::GetBoolArrayParam(const std::string& key) const
{
    std::vector<bool> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsBooleanArray(ao)) {
        GetArrayParams<IBoolean, Boolean, bool>(ao, array);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the boolean type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the boolean value of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, bool value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Boolean::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the boolean array type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the boolean array of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<bool>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IBoolean);
    if (ao != nullptr) {
        for (std::size_t i = 0; i < size; i++) {
            ao->Set(i, Boolean::Box(value[i]));
        }
        std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    }
    return *this;
}

/**
 * @description: Obtains an int value matching the given key.
 * @param key   Indicates the key of wantParams.
 * @param value Indicates the default int value.
 * @return Returns the int value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
int Want::GetIntParam(const std::string& key, const int defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IInteger* ao = IInteger::Query(value);
    if (ao != nullptr) {
        return Integer::Unbox(ao);
    }
    return defaultValue;
}

/**
 * @description: Obtains an int array matching the given key.
 * @param key   Indicates the key of wantParams.
 * @return Returns the int array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<int> Want::GetIntArrayParam(const std::string& key) const
{
    std::vector<int> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsIntegerArray(ao)) {
        GetArrayParams<IInteger, Integer, int>(ao, array);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the int type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the int value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, int value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Integer::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the int array type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the int array of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<int>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IInteger);
    if (ao == nullptr) {
        return *this;
    }
    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, Integer::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

/**
 * @description: Obtains a double value matching the given key.
 * @param key   Indicates the key of wantParams.
 * @param defaultValue  Indicates the default double value.
 * @return Returns the double value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
double Want::GetDoubleParam(const std::string& key, double defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IDouble* ao = IDouble::Query(value);
    if (ao != nullptr) {
        return Double::Unbox(ao);
    }
    return defaultValue;
}

/**
 * @description: Obtains a double array matching the given key.
 * @param key   Indicates the key of WantParams.
 * @return Returns the double array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<double> Want::GetDoubleArrayParam(const std::string& key) const
{
    std::vector<double> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsDoubleArray(ao)) {
        GetArrayParams<IDouble, Double, double>(ao, array);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the double type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the int value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, double value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Double::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the double array type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the double array of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<double>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IDouble);
    if (ao == nullptr) {
        return *this;
    }
    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, Double::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

/**
 * @description: Obtains a float value matching the given key.
 * @param key   Indicates the key of wnatParams.
 * @param value Indicates the default float value.
 * @return Returns the float value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
float Want::GetFloatParam(const std::string& key, float defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IFloat* ao = IFloat::Query(value);
    if (ao != nullptr) {
        return Float::Unbox(ao);
    }
    return defaultValue;
}

/**
 * @description: Obtains a float array matching the given key.
 * @param key Indicates the key of WantParams.
 * @return Obtains a float array matching the given key.
 */
std::vector<float> Want::GetFloatArrayParam(const std::string& key) const
{
    std::vector<float> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsFloatArray(ao)) {
        GetArrayParams<IFloat, Float, float>(ao, array);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the float type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, float value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Float::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the float array type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<float>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IFloat);
    if (ao == nullptr) {
        return *this;
    }

    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, Float::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

/**
 * @description: Obtains a long value matching the given key.
 * @param key Indicates the key of wantParams.
 * @param value Indicates the default long value.
 * @return Returns the long value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
long Want::GetLongParam(const std::string& key, long defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    if (ILong::Query(value) != nullptr) {
        return Long::Unbox(ILong::Query(value));
    } else if (IString::Query(value) != nullptr) {
        std::string str = String::Unbox(IString::Query(value));
        if (std::regex_match(str, NUMBER_REGEX)) {
            return std::atoll(str.c_str());
        }
    }
    return defaultValue;
}

/**
 * @description: Obtains a long array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the long array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<long> Want::GetLongArrayParam(const std::string& key) const
{
    std::vector<long> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsLongArray(ao)) {
        GetArrayParams<ILong, Long, long>(ao, array);
    } else if (ao != nullptr && Array::IsStringArray(ao)) {
        auto func = [&](IInterface* object) {
            IString* o = IString::Query(object);
            if (o != nullptr) {
                std::string str = String::Unbox(o);
                if (std::regex_match(str, NUMBER_REGEX)) {
                    array.push_back(std::atoll(str.c_str()));
                }
            }
        };
        Array::ForEach(ao, func);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the long type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, long value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Long::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the long array type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<long>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_ILong);
    if (ao == nullptr) {
        return *this;
    }
    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, Long::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

Want& Want::SetParam(const std::string& key, long long value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Long::Box(value));
    return *this;
}

/**
 * @description: a short value matching the given key.
 * @param key Indicates the key of wantParams.
 * @param defaultValue Indicates the default short value.
 * @return Returns the short value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
short Want::GetShortParam(const std::string& key, short defaultValue) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IShort* ao = IShort::Query(value);
    if (ao != nullptr) {
        return Short::Unbox(ao);
    }
    return defaultValue;
}

/**
 * @description: Obtains a short array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the short array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<short> Want::GetShortArrayParam(const std::string& key) const
{
    std::vector<short> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsShortArray(ao)) {
        GetArrayParams<IShort, Short, short>(ao, array);
    }
    return array;
}

/**
 * @description: Sets a parameter value of the short type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, short value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, Short::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the short array type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<short>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IShort);
    if (ao == nullptr) {
        return *this;
    }
    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, Short::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

/**
 * @description: Obtains a string value matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the string value of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::string Want::GetStringParam(const std::string& key) const
{
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IString* ao = IString::Query(value);
    if (ao != nullptr) {
        return String::Unbox(ao);
    }
    return std::string();
}

/**
 * @description: Obtains a string array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the string array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<std::string> Want::GetStringArrayParam(const std::string& key) const
{
    std::vector<std::string> array;
    auto value = std::static_pointer_cast<WantParams>(wantParams_)->GetParam(key);
    IArray* ao = IArray::Query(value);
    if (ao != nullptr && Array::IsStringArray(ao)) {
        GetArrayParams<IString, String, std::string>(ao, array);
    }
    return array;
}

std::string Want::GetBundleName() const
{
    return bundleName_;
}

void Want::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

std::string Want::GetModuleName() const
{
    return moduleName_;
}

void Want::SetModuleName(const std::string& moduleName)
{
    moduleName_ = moduleName;
}

std::string Want::GetAbilityName() const
{
    return abilityName_;
}

void Want::SetAbilityName(const std::string& abilityName)
{
    abilityName_ = abilityName;
}

std::string Want::GetType() const
{
    return type_;
}

void Want::SetType(const std::string& type)
{
    type_ = type;
}

/**
 * @description: Sets a parameter value of the string type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::string& value)
{
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, String::Box(value));
    return *this;
}

/**
 * @description: Sets a parameter value of the string array type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<std::string>& value)
{
    std::size_t size = value.size();
    sptr<IArray> ao = new (std::nothrow) Array(size, g_IID_IString);
    if (ao == nullptr) {
        return *this;
    }
    for (std::size_t i = 0; i < size; i++) {
        ao->Set(i, String::Box(value[i]));
    }
    std::static_pointer_cast<WantParams>(wantParams_)->SetParam(key, ao);
    return *this;
}

/**
 * @description: Checks whether a Want contains the parameter matching a given key.
 * @param key Indicates the key.
 * @return Returns true if the Want contains the parameter; returns false otherwise.
 */
bool Want::HasParameter(const std::string& key) const
{
    return std::static_pointer_cast<WantParams>(wantParams_)->HasParam(key);
}

/**
 * @description: Removes the parameter matching the given key.
 * @param key Indicates the key matching the parameter to be removed.
 */
void Want::RemoveParam(const std::string& key)
{
    std::static_pointer_cast<WantParams>(wantParams_)->Remove(key);
}

/**
 * @description: Sets a wantParams object in a want.
 * @param wantParams  Indicates the wantParams description.
 * @return Returns this want object containing the wantParams.
 */
Want& Want::SetParams(const std::shared_ptr<WantParamsInterface> wantParams)
{
    wantParams_ = wantParams;
    return *this;
}

std::string Want::ToJson() const
{
    auto wantParams = std::static_pointer_cast<WantParams>(wantParams_);
    WantParamWrapper wrapper(*wantParams);
    std::string wantJson = "{\"" + JSON_WANTPARAMS_PARAM + "\":" + wrapper.ToString() + "}";
    return wantJson;
}

void Want::ParseJson(const std::string& jsonParams)
{
    Json jsonObject;
    if (CheckParamsIsVaild(jsonParams, jsonObject) == false) {
        return;
    }
    auto wantParams = std::static_pointer_cast<AAFwk::WantParams>(wantParams_);
    for (auto& element : jsonObject[JSON_WANTPARAMS_PARAM]) {
        if (CheckElementIsVaild(element) == false) {
            continue;
        }
        auto typeId = element[JSON_WANTPARAMS_TYPE].get<int>();
        auto elementKey = element[JSON_WANTPARAMS_KEY];
        auto elemetnValue = element[JSON_WANTPARAMS_VALUE];
        auto localType = static_cast<AAFwk::WantValueType>(typeId);
        if (localType == AAFwk::WantValueType::VALUE_TYPE_BOOLEAN) {
            if (elemetnValue.type() == Json::value_t::boolean) {
                wantParams->SetParam(
                    elementKey, WantParams::GetInterfaceByType(typeId, elemetnValue ? "true" : "false"));
            } else {
                SetBoolIntDouble(*wantParams, elementKey, elemetnValue, localType);
            }
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_INT) {
            auto intType = elemetnValue.type();
            if (intType == Json::value_t::number_integer || intType == Json::value_t::number_unsigned) {
                wantParams->SetParam(
                    elementKey, WantParams::GetInterfaceByType(typeId, std::to_string(elemetnValue.get<int64_t>())));
            } else {
                SetBoolIntDouble(*wantParams, elementKey, elemetnValue, localType);
            }
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_DOUBLE) {
            SetDouble(elemetnValue, wantParams, elementKey, localType);
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_STRING) {
            wantParams->SetParam(
                elementKey, WantParams::GetInterfaceByType(typeId - 1, elemetnValue.get<std::string>()));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_ARRAY) {
            wantParams->SetParam(elementKey, Array::ParseCrossPlatformArray(elemetnValue));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_WANTPARAMS) {
            WantParams localWantParams;
            WantParamWrapper::ParseWantParams(elemetnValue, localWantParams);
            sptr<IWantParams> localIwantParams = new (std::nothrow) WantParamWrapper(localWantParams);
            wantParams->SetParam(elementKey, localIwantParams);
        }
    }
}

bool Want::IsEmpty() const
{
    if (!bundleName_.empty() || !abilityName_.empty() || !moduleName_.empty() || !type_.empty()) {
        return false;
    }

    if (wantParams_ != nullptr && !std::static_pointer_cast<WantParams>(wantParams_)->IsEmpty()) {
        return false;
    }
    return true;
}

void Want::InnerCopyWant(const Want& want)
{
    bundleName_ = want.bundleName_;
    moduleName_ = want.moduleName_;
    abilityName_ = want.abilityName_;
    wantParams_ = want.wantParams_;
    type_ = want.type_;
}
} // namespace AAFwk
} // namespace OHOS
