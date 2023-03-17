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

#ifndef OHOS_ABILITY_BASE_WANT_H
#define OHOS_ABILITY_BASE_WANT_H

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>
typedef unsigned char byte;
typedef char zchar;

namespace OHOS {
namespace AAFwk {
class Want final {
public:
    /**
     * @description:  Default construcotr of Want class, which is used to initialzie flags and URI.
     * @param None
     * @return None
     */
    Want();

    /**
     * @description: Default deconstructor of Want class
     * @param None
     * @return None
     */
    ~Want();

    /**
     * @description: Copy construcotr of Want class, which is used to initialzie flags, URI, etc.
     * @param want the source instance of Want.
     * @return None
     */
    Want(const Want& want);
    Want& operator=(const Want& want);

    /**
     * @description: clear the specific want object.
     * @param want Indicates the want to clear
     */
    static void ClearWant(Want* want);

    /**
     * @description: Obtains a bool-type value matching the given key.
     * @param key   Indicates the key of WantParams.
     * @param defaultValue  Indicates the default bool-type value.
     * @return Returns the bool-type value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    bool GetBoolParam(const std::string& key, bool defaultValue) const;

    /**
     * @description: Obtains a bool-type array matching the given key.
     * @param key   Indicates the key of WantParams.
     * @return Returns the bool-type array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<bool> GetBoolArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the boolean type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the boolean value of the parameter.
     * @return Returns this want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, bool value);

    /**
     * @description: Sets a parameter value of the boolean array type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the boolean array of the parameter.
     * @return Returns this want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<bool>& value);

    /**
     * @description: Obtains a byte-type value matching the given key.
     * @param key   Indicates the key of WantParams.
     * @param defaultValue  Indicates the default byte-type value.
     * @return Returns the byte-type value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    byte GetByteParam(const std::string& key, byte defaultValue) const;

    /**
     * @description: Obtains a byte-type array matching the given key.
     * @param key   Indicates the key of WantParams.
     * @return Returns the byte-type array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<byte> GetByteArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the byte type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, byte value);

    /**
     * @description: Sets a parameter value of the byte array type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the byte array of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<byte>& value);

    /**
     * @description: Obtains a char value matching the given key.
     * @param key   Indicates the key of wnatParams.
     * @param value Indicates the default char value.
     * @return Returns the char value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    zchar GetCharParam(const std::string& key, zchar defaultValue) const;

    /**
     * @description: Obtains a char array matching the given key.
     * @param key   Indicates the key of wantParams.
     * @return Returns the char array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<zchar> GetCharArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the char type.
     * @param key   Indicates the key of wantParams.
     * @param value Indicates the char value of the parameter.
     * @return Returns this want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, zchar value);

    /**
     * @description: Sets a parameter value of the char array type.
     * @param key   Indicates the key of wantParams.
     * @param value Indicates the char array of the parameter.
     * @return Returns this want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<zchar>& value);

    /**
     * @description: Obtains an int value matching the given key.
     * @param key   Indicates the key of wantParams.
     * @param value Indicates the default int value.
     * @return Returns the int value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    int GetIntParam(const std::string& key, int defaultValue) const;

    /**
     * @description: Obtains an int array matching the given key.
     * @param key   Indicates the key of wantParams.
     * @return Returns the int array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<int> GetIntArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the int type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the int value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, int value);

    /**
     * @description: Sets a parameter value of the int array type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the int array of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<int>& value);

    /**
     * @description: Obtains a double value matching the given key.
     * @param key   Indicates the key of wantParams.
     * @param defaultValue  Indicates the default double value.
     * @return Returns the double value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    double GetDoubleParam(const std::string& key, double defaultValue) const;

    /**
     * @description: Obtains a double array matching the given key.
     * @param key   Indicates the key of WantParams.
     * @return Returns the double array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<double> GetDoubleArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the double type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the int value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, double value);

    /**
     * @description: Sets a parameter value of the double array type.
     * @param key   Indicates the key matching the parameter.
     * @param value Indicates the double array of the parameter.
     * @return Returns this want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<double>& value);

    /**
     * @description: Obtains a float value matching the given key.
     * @param key   Indicates the key of wnatParams.
     * @param value Indicates the default float value.
     * @return Returns the float value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    float GetFloatParam(const std::string& key, float defaultValue) const;

    /**
     * @description: Obtains a float array matching the given key.
     * @param key Indicates the key of WantParams.
     * @return Obtains a float array matching the given key.
     */
    std::vector<float> GetFloatArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the float type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, float value);

    /**
     * @description: Sets a parameter value of the float array type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<float>& value);

    /**
     * @description: Obtains a long value matching the given key.
     * @param key Indicates the key of wantParams.
     * @param value Indicates the default long value.
     * @return Returns the long value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    long GetLongParam(const std::string& key, long defaultValue) const;

    /**
     * @description: Obtains a long array matching the given key.
     * @param key Indicates the key of wantParams.
     * @return Returns the long array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<long> GetLongArrayParam(const std::string& key) const;

    Want& SetParam(const std::string& key, long long value);

    /**
     * @description: Sets a parameter value of the long type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, long value);

    /**
     * @description: Sets a parameter value of the long array type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<long>& value);

    /**
     * @description: a short value matching the given key.
     * @param key Indicates the key of wantParams.
     * @param defaultValue Indicates the default short value.
     * @return Returns the short value of the parameter matching the given key;
     * returns the default value if the key does not exist.
     */
    short GetShortParam(const std::string& key, short defaultValue) const;

    /**
     * @description: Obtains a short array matching the given key.
     * @param key Indicates the key of wantParams.
     * @return Returns the short array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<short> GetShortArrayParam(const std::string& key) const;

    /**
     * @description: Sets a parameter value of the short type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, short value);

    /**
     * @description: Sets a parameter value of the short array type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<short>& value);

    /**
     * @description: Obtains a string value matching the given key.
     * @param key Indicates the key of wantParams.
     * @return Returns the string value of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::string GetStringParam(const std::string& key) const;

    /**
     * @description: Obtains a string array matching the given key.
     * @param key Indicates the key of wantParams.
     * @return Returns the string array of the parameter matching the given key;
     * returns null if the key does not exist.
     */
    std::vector<std::string> GetStringArrayParam(const std::string& key) const;

    /**
     * @description: Obtains a string value of bunde name.
     * @return Returns the string value of bunde name;
     */
    std::string GetBundleName() const;

    /**
     * @description: Set the bundle name.
     * @param bundleName The bundle name to set.
     */
    void SetBundleName(const std::string& bundleName);

    /**
     * @description: Obtains a string value of module name.
     * @return Returns the string value of module name;
     */
    std::string GetModuleName() const;

    /**
     * @description: Set the module name.
     * @param moduleName The module name to set.
     */
    void SetModuleName(const std::string& moduleName);

    /**
     * @description: Obtains a string value of ability name.
     * @return Returns the string value of ability name;
     */
    std::string GetAbilityName() const;

    /**
     * @description: Set the ability name.
     * @param abilityName The ability name to set.
     */
    void SetAbilityName(const std::string& abilityName);

    /**
     * @description: Sets a parameter value of the string type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::string& value);

    /**
     * @description: Sets a parameter value of the string array type.
     * @param key Indicates the key matching the parameter.
     * @param value Indicates the byte-type value of the parameter.
     * @return Returns this Want object containing the parameter value.
     */
    Want& SetParam(const std::string& key, const std::vector<std::string>& value);

    bool HasParameter(const std::string& key) const;
    void RemoveParam(const std::string& key);

public:
    static const std::string ABILITY_ID;

private:
    void CopyFromWant(const Want& want);
    template<class T>
    void SetValue(const std::string& key, T value);

    template<class T>
    void SetArrayValue(const std::string& key, const std::vector<T>& value);

    template<class T>
    T GetValue(const std::string& key, T defaultValue) const;

    template<class T>
    std::vector<T> GetArrayValue(const std::string& key) const;

private:
    std::map<std::string, std::shared_ptr<void>> params_;
    std::map<std::string, int> types_;
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_BASE_WANT_H