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

#include "want.h"

namespace OHOS {
namespace AAFwk {
const std::string Want::ABILITY_ID("ability_id");
static constexpr int VALUE_TYPE_BOOLEAN = 1;
static constexpr int VALUE_TYPE_BYTE = 2;
static constexpr int VALUE_TYPE_CHAR = 3;
static constexpr int VALUE_TYPE_SHORT = 4;
static constexpr int VALUE_TYPE_INT = 5;
static constexpr int VALUE_TYPE_LONG = 6;
static constexpr int VALUE_TYPE_LONGLONG = 7;
static constexpr int VALUE_TYPE_FLOAT = 8;
static constexpr int VALUE_TYPE_DOUBLE = 9;
static constexpr int VALUE_TYPE_STRING = 10;
static constexpr int VALUE_TYPE_BOOLEANARRAY = 11;
static constexpr int VALUE_TYPE_BYTEARRAY = 12;
static constexpr int VALUE_TYPE_CHARARRAY = 13;
static constexpr int VALUE_TYPE_SHORTARRAY = 14;
static constexpr int VALUE_TYPE_INTARRAY = 15;
static constexpr int VALUE_TYPE_LONGARRAY = 16;
static constexpr int VALUE_TYPE_FLOATARRAY = 17;
static constexpr int VALUE_TYPE_DOUBLEARRAY = 18;
static constexpr int VALUE_TYPE_STRINGARRAY = 19;

/**
 * @description:Default construcotr of Want class, which is used to initialzie flags and URI.
 * @param None
 * @return None
 */
Want::Want() {}

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
    CopyFromWant(want);
}

Want& Want::operator=(const Want& want)
{
    CopyFromWant(want);
    return *this;
}

/**
 * @description: clear the specific want object.
 * @param want Indicates the want to clear
 */
void Want::ClearWant(Want* want)
{
    want->params_.clear();
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
    return GetValue<bool>(key, defaultValue);
}

/**
 * @description:Obtains a bool-type array matching the given key.
 * @param key   Indicates the key of WantParams.
 * @return Returns the bool-type array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<bool> Want::GetBoolArrayParam(const std::string& key) const
{
    return GetArrayValue<bool>(key);
}

/**
 * @description: Sets a parameter value of the boolean type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the boolean value of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, bool value)
{
    SetValue<bool>(key, value);
    types_[key] = VALUE_TYPE_BOOLEAN;
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
    SetArrayValue<bool>(key, value);
    types_[key] = VALUE_TYPE_BOOLEANARRAY;
    return *this;
}

/**
 * @description: Obtains a byte-type value matching the given key.
 * @param key   Indicates the key of WantParams.
 * @param defaultValue  Indicates the default byte-type value.
 * @return Returns the byte-type value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
byte Want::GetByteParam(const std::string& key, const byte defaultValue) const
{
    return GetValue<byte>(key, defaultValue);
    ;
}

/**
 * @description: Obtains a byte-type array matching the given key.
 * @param key   Indicates the key of WantParams.
 * @return Returns the byte-type array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<byte> Want::GetByteArrayParam(const std::string& key) const
{
    return GetArrayValue<byte>(key);
}

/**
 * @description: Sets a parameter value of the byte type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, byte value)
{
    SetValue<byte>(key, value);
    types_[key] = VALUE_TYPE_BYTE;
    return *this;
}

/**
 * @description: Sets a parameter value of the byte array type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the byte array of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<byte>& value)
{
    SetArrayValue<byte>(key, value);
    types_[key] = VALUE_TYPE_BYTEARRAY;
    return *this;
}

/**
 * @description: Obtains a char value matching the given key.
 * @param key   Indicates the key of wnatParams.
 * @param value Indicates the default char value.
 * @return Returns the char value of the parameter matching the given key;
 * returns the default value if the key does not exist.
 */
zchar Want::GetCharParam(const std::string& key, zchar defaultValue) const
{
    return GetValue<zchar>(key, defaultValue);
}

/**
 * @description: Obtains a char array matching the given key.
 * @param key   Indicates the key of wantParams.
 * @return Returns the char array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<zchar> Want::GetCharArrayParam(const std::string& key) const
{
    return GetArrayValue<zchar>(key);
}

/**
 * @description: Sets a parameter value of the char type.
 * @param key   Indicates the key of wantParams.
 * @param value Indicates the char value of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, zchar value)
{
    SetValue<zchar>(key, value);
    types_[key] = VALUE_TYPE_CHAR;
    return *this;
}

/**
 * @description: Sets a parameter value of the char array type.
 * @param key   Indicates the key of wantParams.
 * @param value Indicates the char array of the parameter.
 * @return Returns this want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::vector<zchar>& value)
{
    SetArrayValue<zchar>(key, value);
    types_[key] = VALUE_TYPE_CHARARRAY;
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
    return GetValue<int>(key, defaultValue);
}

/**
 * @description: Obtains an int array matching the given key.
 * @param key   Indicates the key of wantParams.
 * @return Returns the int array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<int> Want::GetIntArrayParam(const std::string& key) const
{
    return GetArrayValue<int>(key);
}

/**
 * @description: Sets a parameter value of the int type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the int value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, int value)
{
    SetValue<int>(key, value);
    types_[key] = VALUE_TYPE_INT;
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
    SetArrayValue<int>(key, value);
    types_[key] = VALUE_TYPE_INTARRAY;
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
    return GetValue<double>(key, defaultValue);
}

/**
 * @description: Obtains a double array matching the given key.
 * @param key   Indicates the key of WantParams.
 * @return Returns the double array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<double> Want::GetDoubleArrayParam(const std::string& key) const
{
    return GetArrayValue<double>(key);
}

/**
 * @description: Sets a parameter value of the double type.
 * @param key   Indicates the key matching the parameter.
 * @param value Indicates the int value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, double value)
{
    SetValue<double>(key, value);
    types_[key] = VALUE_TYPE_DOUBLE;
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
    SetArrayValue<double>(key, value);
    types_[key] = VALUE_TYPE_DOUBLEARRAY;
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
    return GetValue<float>(key, defaultValue);
}

/**
 * @description: Obtains a float array matching the given key.
 * @param key Indicates the key of WantParams.
 * @return Obtains a float array matching the given key.
 */
std::vector<float> Want::GetFloatArrayParam(const std::string& key) const
{
    return GetArrayValue<float>(key);
}

/**
 * @description: Sets a parameter value of the float type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, float value)
{
    SetValue<float>(key, value);
    types_[key] = VALUE_TYPE_FLOAT;
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
    SetArrayValue<float>(key, value);
    types_[key] = VALUE_TYPE_FLOATARRAY;
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
    return GetValue<long>(key, defaultValue);
}

/**
 * @description: Obtains a long array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the long array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<long> Want::GetLongArrayParam(const std::string& key) const
{
    return GetArrayValue<long>(key);
}

/**
 * @description: Sets a parameter value of the long type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, long value)
{
    SetValue<long>(key, value);
    types_[key] = VALUE_TYPE_LONG;
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
    SetArrayValue<long>(key, value);
    types_[key] = VALUE_TYPE_LONGARRAY;
    return *this;
}

Want& Want::SetParam(const std::string& key, long long value)
{
    SetValue<long long>(key, value);
    types_[key] = VALUE_TYPE_LONGLONG;
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
    return GetValue<short>(key, defaultValue);
}

/**
 * @description: Obtains a short array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the short array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<short> Want::GetShortArrayParam(const std::string& key) const
{
    return GetArrayValue<short>(key);
}

/**
 * @description: Sets a parameter value of the short type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, short value)
{
    SetValue<short>(key, value);
    types_[key] = VALUE_TYPE_SHORT;
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
    SetArrayValue<short>(key, value);
    types_[key] = VALUE_TYPE_SHORTARRAY;
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
    return GetValue<std::string>(key, std::string());
}

/**
 * @description: Obtains a string array matching the given key.
 * @param key Indicates the key of wantParams.
 * @return Returns the string array of the parameter matching the given key;
 * returns null if the key does not exist.
 */
std::vector<std::string> Want::GetStringArrayParam(const std::string& key) const
{
    return GetArrayValue<std::string>(key);
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

/**
 * @description: Sets a parameter value of the string type.
 * @param key Indicates the key matching the parameter.
 * @param value Indicates the byte-type value of the parameter.
 * @return Returns this Want object containing the parameter value.
 */
Want& Want::SetParam(const std::string& key, const std::string& value)
{
    SetValue<std::string>(key, value);
    types_[key] = VALUE_TYPE_STRING;
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
    SetArrayValue<std::string>(key, value);
    types_[key] = VALUE_TYPE_STRINGARRAY;
    return *this;
}

/**
 * @description: Checks whether a Want contains the parameter matching a given key.
 * @param key Indicates the key.
 * @return Returns true if the Want contains the parameter; returns false otherwise.
 */
bool Want::HasParameter(const std::string& key) const
{
    return params_.find(key) == params_.end();
}

/**
 * @description: Removes the parameter matching the given key.
 * @param key Indicates the key matching the parameter to be removed.
 */
void Want::RemoveParam(const std::string& key)
{
    auto iter = params_.find(key);
    if (iter != params_.end()) {
        params_.erase(iter);
    }
}

template<class T>
void Want::SetValue(const std::string& key, T value)
{
    params_[key] = std::make_shared<T>(value);
}

template<class T>
void Want::SetArrayValue(const std::string& key, const std::vector<T>& value)
{
    auto ptr = std::make_shared<std::vector<T>>();
    for (auto it = value.begin(); it != value.end(); it++) {
        ptr->push_back(*it);
    }
    params_[key] = ptr;
}

template<class T>
T Want::GetValue(const std::string& key, T defaultValue) const
{
    T retValue = defaultValue;
    auto iter = params_.find(key);
    if (iter != params_.end()) {
        auto ptr = static_cast<T*>(iter->second.get());
        if (ptr != nullptr) {
            retValue = *ptr;
        }
    }
    return retValue;
}

template<class T>
std::vector<T> Want::GetArrayValue(const std::string& key) const
{
    std::vector<T> array;
    auto iter = params_.find(key);
    if (iter != params_.end()) {
        auto ptr = static_cast<std::vector<T>*>(iter->second.get());
        if (ptr != nullptr) {
            for (auto it = ptr->begin(); it != ptr->end(); it++) {
                array.push_back(*it);
            }
        }
    }
    return array;
}

void Want::CopyFromWant(const Want& want)
{
    for (auto iter = want.params_.begin(); iter != want.params_.end(); iter++) {
        auto key = iter->first;
        auto value = iter->second;
        auto it = want.types_.find(key);
        if (it != want.types_.end()) {
            if (it->second == VALUE_TYPE_BOOLEAN) {
                SetParam(key, *(static_cast<bool*>(value.get())));
            } else if (it->second == VALUE_TYPE_BOOLEANARRAY) {
                SetParam(key, *(static_cast<std::vector<bool>*>(value.get())));
            } else if (it->second == VALUE_TYPE_BYTE) {
                SetParam(key, *(static_cast<byte*>(value.get())));
            } else if (it->second == VALUE_TYPE_BYTEARRAY) {
                SetParam(key, *(static_cast<std::vector<byte>*>(value.get())));
            } else if (it->second == VALUE_TYPE_CHAR) {
                SetParam(key, *(static_cast<zchar*>(value.get())));
            } else if (it->second == VALUE_TYPE_CHARARRAY) {
                SetParam(key, *(static_cast<std::vector<zchar>*>(value.get())));
            } else if (it->second == VALUE_TYPE_INT) {
                SetParam(key, *(static_cast<int*>(value.get())));
            } else if (it->second == VALUE_TYPE_INTARRAY) {
                SetParam(key, *(static_cast<std::vector<int>*>(value.get())));
            } else if (it->second == VALUE_TYPE_FLOAT) {
                SetParam(key, *(static_cast<float*>(value.get())));
            } else if (it->second == VALUE_TYPE_FLOATARRAY) {
                SetParam(key, *(static_cast<std::vector<float>*>(value.get())));
            } else if (it->second == VALUE_TYPE_DOUBLE) {
                SetParam(key, *(static_cast<double*>(value.get())));
            } else if (it->second == VALUE_TYPE_DOUBLEARRAY) {
                SetParam(key, *(static_cast<std::vector<double>*>(value.get())));
            } else if (it->second == VALUE_TYPE_SHORT) {
                SetParam(key, *(static_cast<short*>(value.get())));
            } else if (it->second == VALUE_TYPE_SHORTARRAY) {
                SetParam(key, *(static_cast<std::vector<short>*>(value.get())));
            } else if (it->second == VALUE_TYPE_LONG) {
                SetParam(key, *(static_cast<long*>(value.get())));
            } else if (it->second == VALUE_TYPE_LONGLONG) {
                SetParam(key, *(static_cast<long long*>(value.get())));
            } else if (it->second == VALUE_TYPE_LONGARRAY) {
                SetParam(key, *(static_cast<std::vector<long>*>(value.get())));
            } else if (it->second == VALUE_TYPE_STRING) {
                SetParam(key, *(static_cast<std::string*>(value.get())));
            } else if (it->second == VALUE_TYPE_STRINGARRAY) {
                SetParam(key, *(static_cast<std::vector<std::string>*>(value.get())));
            }
        }
    }
}

} // namespace AAFwk
} // namespace OHOS
