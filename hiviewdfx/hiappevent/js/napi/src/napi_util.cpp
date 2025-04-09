/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "napi_util.h"

#include <unordered_map>

#include "hiappevent_base.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiUtil"

namespace OHOS {
namespace HiviewDFX {
namespace NapiUtil {
namespace {
const std::string DOMAIN_PROPERTY = "domain";
const std::string NAME_PROPERTY = "name";
const std::string EVENT_TYPE_PROPERTY = "eventType";
const std::string PARAM_PROPERTY = "params";
const std::string EVENT_INFOS_PROPERTY = "appEventInfos";
} // namespace

bool IsNull(const napi_env env, const napi_value value)
{
    return GetType(env, value) == napi_null;
}

bool IsNumber(const napi_env env, const napi_value value)
{
    return GetType(env, value) == napi_number;
}

bool IsString(const napi_env env, const napi_value value)
{
    return GetType(env, value) == napi_string;
}

bool IsObject(const napi_env env, const napi_value value)
{
    return GetType(env, value) == napi_object;
}

bool IsFunction(const napi_env env, const napi_value value)
{
    return GetType(env, value) == napi_function;
}

bool IsArray(const napi_env env, const napi_value value)
{
    bool result = false;
    if (napi_is_array(env, value, &result) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to check array type");
        return false;
    }
    return result;
}

bool IsArrayType(const napi_env env, const napi_value value, napi_valuetype type)
{
    if (!IsArray(env, value)) {
        return false;
    }
    if (GetArrayLength(env, value) == 0) {
        return true;
    }
    return GetArrayType(env, value) == type;
}

napi_valuetype GetType(const napi_env env, const napi_value value)
{
    napi_valuetype type;
    if (napi_typeof(env, value, &type) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get value type");
        return napi_undefined;
    }
    return type;
}

napi_valuetype GetArrayType(const napi_env env, const napi_value arr)
{
    uint32_t result = 0;
    if (napi_get_array_length(env, arr, &result) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return napi_undefined;
    }

    napi_valuetype type = napi_null; // note: empty array returns null type
    for (size_t i = 0; i < result; ++i) {
        napi_value element = nullptr;
        if (napi_get_element(env, arr, i, &element) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            return napi_undefined;
        }
        if (i == 0) {
            type = GetType(env, element);
            continue;
        }
        if (type != GetType(env, element)) {
            HILOG_ERROR(LOG_CORE, "array has different element types");
            return napi_undefined;
        }
    }
    return type;
}

uint32_t GetArrayLength(const napi_env env, const napi_value arr)
{
    uint32_t result = 0;
    if (napi_get_array_length(env, arr, &result) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the length of array");
        return 0;
    }
    return result;
}

napi_value GetElement(const napi_env env, const napi_value arr, uint32_t index)
{
    napi_value element = nullptr;
    if (napi_get_element(env, arr, index, &element) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get the element of array.");
        return nullptr;
    }
    return element;
}

bool GetBoolean(const napi_env env, const napi_value value)
{
    bool bValue = false;
    if (napi_get_value_bool(env, value, &bValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get bool value");
        return false;
    }
    return bValue;
}

void GetBooleans(const napi_env env, const napi_value arr, std::vector<bool>& bools)
{
    uint32_t len = GetArrayLength(env, arr);
    for (size_t i = 0; i < len; ++i) {
        napi_value element = GetElement(env, arr, i);
        if (element == nullptr) {
            continue;
        }
        bools.push_back(GetBoolean(env, element));
    }
}

int32_t GetInt32(const napi_env env, const napi_value value)
{
    int32_t iValue = 0;
    if (napi_get_value_int32(env, value, &iValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get int32 value");
        return 0;
    }
    return iValue;
}

void GetInt32s(const napi_env env, const napi_value arr, std::vector<int32_t>& ints)
{
    uint32_t len = GetArrayLength(env, arr);
    for (size_t i = 0; i < len; ++i) {
        napi_value element = GetElement(env, arr, i);
        if (element == nullptr) {
            continue;
        }
        ints.push_back(GetInt32(env, element));
    }
}

double GetDouble(const napi_env env, const napi_value value)
{
    double dValue = 0;
    if (napi_get_value_double(env, value, &dValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get double value");
        return 0;
    }
    return dValue;
}

void GetDoubles(const napi_env env, const napi_value arr, std::vector<double>& doubles)
{
    uint32_t len = GetArrayLength(env, arr);
    for (size_t i = 0; i < len; ++i) {
        napi_value element = GetElement(env, arr, i);
        if (element == nullptr) {
            continue;
        }
        doubles.push_back(GetDouble(env, element));
    }
}

std::string GetString(const napi_env env, const napi_value value)
{
    size_t bufsize = 0;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &bufsize) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get string length");
        return "";
    }
    std::vector<char> charVec(bufsize + 1); // 1 for '\0'
    if (napi_get_value_string_utf8(env, value, charVec.data(), bufsize + 1, &bufsize) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get string value");
        return "";
    }
    return charVec.data();
}

void GetStrings(const napi_env env, const napi_value arr, std::vector<std::string>& strs)
{
    uint32_t len = GetArrayLength(env, arr);
    for (size_t i = 0; i < len; ++i) {
        napi_value element = GetElement(env, arr, i);
        if (element == nullptr) {
            continue;
        }
        strs.push_back(GetString(env, element));
    }
}

void GetStringsToSet(const napi_env env, const napi_value arr, std::unordered_set<std::string>& strs)
{
    uint32_t len = GetArrayLength(env, arr);
    for (size_t i = 0; i < len; ++i) {
        napi_value element = GetElement(env, arr, i);
        if (element == nullptr) {
            continue;
        }
        strs.insert(GetString(env, element));
    }
}

bool HasProperty(const napi_env env, const napi_value object, const std::string& name)
{
    bool result = false;
    if (napi_has_named_property(env, object, name.c_str(), &result) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to check whether the object has the named property");
        return false;
    }
    return result;
}

napi_value GetProperty(const napi_env env, const napi_value object, const std::string& name)
{
    if (!HasProperty(env, object, name)) {
        return nullptr;
    }
    napi_value value = nullptr;
    if (napi_get_named_property(env, object, name.c_str(), &value) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get property from object");
        return nullptr;
    }
    return value;
}

void GetPropertyNames(const napi_env env, const napi_value object, std::vector<std::string>& names)
{
    napi_value propertyNames = nullptr;
    if (napi_get_property_names(env, object, &propertyNames) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get property names.");
        return;
    }
    uint32_t len = 0;
    if (napi_get_array_length(env, propertyNames, &len) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get array length");
        return;
    }
    for (uint32_t i = 0; i < len; ++i) {
        napi_value element = nullptr;
        if (napi_get_element(env, propertyNames, i, &element) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to get the element of array");
            continue;
        }
        names.push_back(GetString(env, element));
    }
}

napi_value GetReferenceValue(const napi_env env, const napi_ref funcRef)
{
    napi_value refValue = nullptr;
    if (napi_get_reference_value(env, funcRef, &refValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get reference value");
        return nullptr;
    }
    return refValue;
}

size_t GetCbInfo(const napi_env env, napi_callback_info info, napi_value argv[], size_t argc)
{
    size_t paramNum = argc;
    if (napi_get_cb_info(env, info, &paramNum, argv, nullptr, nullptr) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to get callback info");
        return 0;
    }
    return paramNum;
}

napi_ref CreateReference(const napi_env env, const napi_value func)
{
    napi_ref ref = nullptr;
    if (napi_create_reference(env, func, 1, &ref) != napi_ok) { // 1 means initial reference count
        HILOG_ERROR(LOG_CORE, "failed to create reference");
        return nullptr;
    }
    return ref;
}

napi_value CreateNull(const napi_env env)
{
    napi_value nullValue = nullptr;
    if (napi_get_null(env, &nullValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create null");
        return nullptr;
    }
    return nullValue;
}

napi_value CreateUndefined(const napi_env env)
{
    napi_value undefinedValue = nullptr;
    if (napi_get_undefined(env, &undefinedValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create undefined");
        return nullptr;
    }
    return undefinedValue;
}

napi_value CreateBoolean(const napi_env env, bool bValue)
{
    napi_value boolValue = nullptr;
    if (napi_get_boolean(env, bValue, &boolValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create boolean");
        return nullptr;
    }
    return boolValue;
}

napi_value CreateInt32(const napi_env env, int32_t num)
{
    napi_value intValue = nullptr;
    if (napi_create_int32(env, num, &intValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create int32");
        return nullptr;
    }
    return intValue;
}

napi_value CreateInt64(const napi_env env, int64_t num)
{
    napi_value intValue = nullptr;
    if (napi_create_int64(env, num, &intValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create int64");
        return nullptr;
    }
    return intValue;
}

napi_value CreateString(const napi_env env, const std::string& str)
{
    napi_value strValue = nullptr;
    if (napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &strValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create string");
        return nullptr;
    }
    return strValue;
}

napi_value CreateStrings(const napi_env env, const std::vector<std::string>& strs)
{
    napi_value arr = CreateArray(env);
    for (size_t i = 0; i < strs.size(); ++i) {
        SetElement(env, arr, i, CreateString(env, strs[i]));
    }
    return arr;
}

napi_value CreateObject(const napi_env env)
{
    napi_value obj = nullptr;
    if (napi_create_object(env, &obj) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create object");
        return nullptr;
    }
    return obj;
}

napi_value CreateObject(const napi_env env, const std::string& key, const napi_value value)
{
    napi_value obj = nullptr;
    if (napi_create_object(env, &obj) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create object");
        return nullptr;
    }
    if (napi_set_named_property(env, obj, key.c_str(), value) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to set property");
        return nullptr;
    }
    return obj;
}

napi_value CreateArray(const napi_env env)
{
    napi_value arr = nullptr;
    if (napi_create_array(env, &arr) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create array");
        return nullptr;
    }
    return arr;
}

napi_value CreateDouble(const napi_env env, double dValue)
{
    napi_value doubleValue = nullptr;
    if (napi_create_double(env, dValue, &doubleValue) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create double");
        return nullptr;
    }
    return doubleValue;
}

void SetElement(const napi_env env, const napi_value obj, uint32_t index, const napi_value value)
{
    if (napi_set_element(env, obj, index, value) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to set element");
    }
}

void SetNamedProperty(const napi_env env, const napi_value obj, const std::string& key, const napi_value value)
{
    if (napi_set_named_property(env, obj, key.c_str(), value) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to set property");
    }
}

std::string ConvertToString(const napi_env env, const napi_value value)
{
    napi_valuetype type = GetType(env, value);
    if (type == napi_undefined) {
        return "";
    }

    std::string result = "";
    switch (type) {
        case napi_boolean:
            result = GetBoolean(env, value) ? "true" : "false";
            break;
        case napi_number:
            result = std::to_string(GetDouble(env, value));
            break;
        case napi_string:
            result = GetString(env, value);
            break;
        default:
            break;
    }
    return result;
}

void ThrowError(napi_env env, int code, const std::string& msg, bool isThrow)
{
    // no error needs to be thrown before api 9
    if (!isThrow) {
        return;
    }

    if (napi_throw_error(env, std::to_string(code).c_str(), msg.c_str()) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to throw error, code=%{public}d, msg=%{public}s", code, msg.c_str());
    }
}

napi_value CreateError(napi_env env, int code, const std::string& msg)
{
    napi_value err = nullptr;
    if (napi_create_error(env, CreateString(env, std::to_string(code)), CreateString(env, msg), &err) != napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to create error");
        return nullptr;
    }
    return err;
}

std::string CreateErrMsg(const std::string& name)
{
    return "Parameter error. The " + name + " parameter is mandatory.";
}

std::string CreateErrMsg(const std::string& name, const std::string& type)
{
    return "Parameter error. The type of " + name + " must be " + type + ".";
}
std::string CreateErrMsg(const std::string& name, const napi_valuetype type)
{
    std::string typeStr = "";
    switch (type) {
        case napi_boolean:
            typeStr = "boolean";
            break;
        case napi_number:
            typeStr = "number";
            break;
        case napi_string:
            typeStr = "string";
            break;
        case napi_object:
            typeStr = "object";
            break;
        default:
            break;
    }
    return CreateErrMsg(name, typeStr);
}

napi_value CreateBaseValueByJson(const napi_env env, const Json::Value& jsonValue)
{
    if (jsonValue.isBool()) {
        return CreateBoolean(env, jsonValue.asBool());
    }
    if (jsonValue.isInt()) {
        return CreateInt32(env, jsonValue.asInt());
    }
    if (jsonValue.isInt64() && jsonValue.type() != Json::ValueType::uintValue) {
        return CreateInt64(env, jsonValue.asInt64());
    }
    if (jsonValue.isDouble()) {
        return CreateDouble(env, jsonValue.asDouble());
    }
    if (jsonValue.isString()) {
        return CreateString(env, jsonValue.asString());
    }
    return nullptr;
}

napi_value CreateValueByJson(napi_env env, const Json::Value& jsonValue)
{
    if (jsonValue.isArray()) {
        napi_value array = CreateArray(env);
        for (size_t i = 0; i < jsonValue.size(); ++i) {
            SetElement(env, array, i, CreateValueByJson(env, jsonValue[static_cast<int>(i)]));
        }
        return array;
    }
    if (jsonValue.isObject()) {
        napi_value obj = CreateObject(env);
        auto eventNameList = jsonValue.getMemberNames();
        for (auto it = eventNameList.cbegin(); it != eventNameList.cend(); ++it) {
            auto propertyName = *it;
            SetNamedProperty(env, obj, propertyName, CreateValueByJson(env, jsonValue[propertyName]));
        }
        return obj;
    }
    return CreateBaseValueByJson(env, jsonValue);
}

napi_value CreateValueByJsonStr(napi_env env, const std::string& jsonStr)
{
    Json::Value jsonValue;
    Json::Reader reader(Json::Features::strictMode());
    if (!reader.parse(jsonStr, jsonValue)) {
        HILOG_ERROR(LOG_CORE, "parse event detail info failed, please check the style of json");
        return nullptr;
    }
    return CreateValueByJson(env, jsonValue);
}

napi_value CreateEventInfo(napi_env env, std::shared_ptr<AppEventPack> event)
{
    napi_value obj = CreateObject(env);
    SetNamedProperty(env, obj, DOMAIN_PROPERTY, CreateString(env, event->GetDomain()));
    SetNamedProperty(env, obj, NAME_PROPERTY, CreateString(env, event->GetName()));
    SetNamedProperty(env, obj, EVENT_TYPE_PROPERTY, CreateInt32(env, event->GetType()));
    SetNamedProperty(env, obj, PARAM_PROPERTY, CreateValueByJsonStr(env, event->GetParamStr()));
    return obj;
}

napi_value CreateEventInfoArray(napi_env env, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    napi_value arr = CreateArray(env);
    for (size_t i = 0; i < events.size(); ++i) {
        SetElement(env, arr, i, CreateEventInfo(env, events[i]));
    }
    return arr;
}

napi_value CreateEventGroups(napi_env env, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::unordered_map<std::string, std::vector<std::shared_ptr<AppEventPack>>> eventMap;
    for (auto event : events) {
        eventMap[event->GetName()].emplace_back(event);
    }

    napi_value eventGroups = CreateArray(env);
    size_t index = 0;
    for (auto it = eventMap.begin(); it != eventMap.end(); ++it) {
        napi_value eventInfos = CreateArray(env);
        for (size_t i = 0; i < it->second.size(); ++i) {
            SetElement(env, eventInfos, i, CreateEventInfo(env, it->second[i]));
        }
        napi_value obj = CreateObject(env);
        SetNamedProperty(env, obj, NAME_PROPERTY, CreateString(env, it->first));
        SetNamedProperty(env, obj, EVENT_INFOS_PROPERTY, eventInfos);
        SetElement(env, eventGroups, index, obj);
        ++index;
    }
    return eventGroups;
}
} // namespace NapiUtil
} // namespace HiviewDFX
} // namespace OHOS
