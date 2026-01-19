/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "want_params_interface.h"

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
     * @description: Obtains a string value of Type.
     * @return Returns the string value of Type;
     */
    std::string GetType() const;

    /**
     * @description: Set the ability name.
     * @param abilityName The ability name to set.
     */
    void SetAbilityName(const std::string& abilityName);

    /**
     * @description: Set the type.
     * @param type The type to set.
     */
    void SetType(const std::string& type);

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

    /**
     * @description: Sets a wantParams object in a want.
     * @param wantParams  Indicates the wantParams description.
     * @return Returns this want object containing the wantParams.
     */
    Want& SetParams(const std::shared_ptr<WantParamsInterface> wantParams);

    /**
     * @description: Obtains the description of the WantParams object in a Want
     * @return Returns the WantParams description in the Want
     */
    const std::shared_ptr<WantParamsInterface>& GetParams() const
    {
        return wantParams_;
    }

    /**
     * @description: Sets the description of an action in a want.
     * @param action Indicates the action description to set.
     */
    void SetAction(const std::string& action);

    /**
     * @description: Obtains the description of an action in a want.
     * @return Returns the string value of the action.
     */
    std::string GetAction() const
    {
        return action_;
    }

    /**
     * @description: Sets the description of a URI in a Want.
     * @param uri Indicates the URI description.
     */
    void SetUri(const std::string& uri);

    /**
     * @description: Obtains the description of a URI in a Want.
     * @return Returns the string of the URI.
     */
    std::string GetUri() const
    {
        return uri_;
    }

    /**
     * @description: Sets the entities of this Want.
     * @param entities Indicates entities to set.
     */
    void SetEntities(const std::vector<std::string>& entities);

    /**
     * @description: Obtains the description of all entities in a Want
     * @return Returns a set of entities
     */
    const std::vector<std::string>& GetEntities() const
    {
        return entities_;
    }

    /**
     * @description: Adds the description of an entity to a Want. Duplicate entities will be ignored.
     * @param entity Indicates the entity description to add.
     */
    void AddEntity(const std::string& entity);

    /**
     * @description: Removes the description of an entity from a Want
     * @param entity Indicates the entity description to remove.
     */
    void RemoveEntity(const std::string& entity);

    /**
     * @description: Checks whether a Want contains the given entity
     * @param entity Indicates the entity to check
     * @return Returns true if the given entity is contained; returns false otherwise
     */
    bool HasEntity(const std::string& entity) const;

    /**
     * @description: Obtains the number of entities in a Want
     * @return Returns the entity quantity
     */
    size_t CountEntities() const
    {
        return entities_.size();
    }

    bool HasParameter(const std::string& key) const;
    void RemoveParam(const std::string& key);
    std::string ToJson() const;
    void ParseJson(const std::string& jsonParams);
    bool IsEmpty() const;

public:
    static const std::string ABILITY_ID;
    static const std::string INSTANCE_NAME;
    static const std::string ELEMENT_BUNDLE_NAME;
    static const std::string ACTION_VIEWDATA;
    static const std::string ENTITY_BROWSER;

private:
    void InnerCopyWant(const Want& want);
    std::shared_ptr<WantParamsInterface> wantParams_;
    std::string type_;
    std::string bundleName_;
    std::string moduleName_;
    std::string abilityName_;
    std::string action_;
    std::string uri_;
    std::vector<std::string> entities_;
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_BASE_WANT_H