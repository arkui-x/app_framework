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

#include "json_want.h"

#include "hilog.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string JSON_KEY_KEY = "key";
const std::string JSON_KEY_VALUE = "value";
const std::string JSON_KEY_TYPE = "type";
const std::string JSON_KEY_PARAM = "params";
constexpr uint8_t MAX_JSON_ELEMENT_LENGTH = 255;
constexpr uint16_t MAX_JSON_ARRAY_LENGTH = 512;
} // namespace

enum class JsonType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING,
};

enum class ArrayType {
    NUMBER,
    OBJECT,
    STRING,
    NOT_ARRAY,
};

template<typename T, typename dataType>
void CheckArrayType(const nlohmann::json& jsonObject, const std::string& key, dataType& data, ArrayType arrayType)
{
    auto arrays = jsonObject.at(key);
    if (arrays.empty()) {
        return;
    }
    if (arrays.size() > MAX_JSON_ARRAY_LENGTH) {
        return;
    }
    switch (arrayType) {
        case ArrayType::OBJECT:
            for (const auto& array : arrays) {
                if (!array.is_object()) {
                    HILOG_ERROR("array %{public}s is not object type", key.c_str());
                    break;
                }
            }
            data = jsonObject.at(key).get<T>();
            break;
        case ArrayType::NOT_ARRAY:
            HILOG_ERROR("array %{public}s is not string type", key.c_str());
            break;
        default:
            HILOG_ERROR("array %{public}s type error", key.c_str());
            break;
    }
}

template<typename T, typename dataType>
void GetValueIfFindKey(const nlohmann::json& jsonObject, const nlohmann::detail::iter_impl<const nlohmann::json>& end,
    const std::string& key, dataType& data, JsonType jsonType, bool isNecessary, ArrayType arrayType)
{
    if (jsonObject.find(key) != end) {
        switch (jsonType) {
            case JsonType::BOOLEAN:
                if (!jsonObject.at(key).is_boolean()) {
                    HILOG_ERROR("type is error %{public}s is not boolean", key.c_str());
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::NUMBER:
                if (!jsonObject.at(key).is_number()) {
                    HILOG_ERROR("type is error %{public}s is not number", key.c_str());
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::OBJECT:
                if (!jsonObject.at(key).is_object()) {
                    HILOG_ERROR("type is error %{public}s is not object", key.c_str());
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::ARRAY:
                if (!jsonObject.at(key).is_array()) {
                    HILOG_ERROR("type is error %{public}s is not array", key.c_str());
                    break;
                }
                CheckArrayType<T>(jsonObject, key, data, arrayType);
                break;
            case JsonType::STRING:
                if (!jsonObject.at(key).is_string()) {
                    HILOG_ERROR("type is error %{public}s is not string", key.c_str());
                    break;
                }
                data = jsonObject.at(key).get<T>();
                if (jsonObject.at(key).get<std::string>().length() > MAX_JSON_ELEMENT_LENGTH) {
                }
                break;
            case JsonType::NULLABLE:
                HILOG_ERROR("type is error %{public}s is nullable", key.c_str());
                break;
            default:
                HILOG_ERROR("type is error %{public}s is not jsonType", key.c_str());
        }
        return;
    }
    if (isNecessary) {
        HILOG_ERROR("profile prop %{public}s is mission", key.c_str());
    }
}

template<typename T>
const std::string GetJsonStrFromInfo(T& t)
{
    nlohmann::json json = t;
    return json.dump();
}

template<typename T>
bool ParseInfoFromJsonStr(const char* data, T& t)
{
    if (data == nullptr) {
        HILOG_ERROR("data is nullptr");
        return false;
    }

    nlohmann::json jsonObject = nlohmann::json::parse(data, nullptr, false);
    if (jsonObject.is_discarded()) {
        HILOG_ERROR("failed due to data is discarded");
        return false;
    }

    t = jsonObject.get<T>();
    return true;
}

void to_json(nlohmann::json& jsonObject, const JsonWant& jsonWant)
{
    jsonObject = nlohmann::json { { JSON_KEY_PARAM, jsonWant.params } };
}

void from_json(const nlohmann::json& jsonObject, JsonWant& jsonWant)
{
    const auto& jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::vector<WantParamsJson>>(
        jsonObject, jsonObjectEnd, JSON_KEY_PARAM, jsonWant.params, JsonType::ARRAY, false, ArrayType::OBJECT);
}

void to_json(nlohmann::json& jsonObject, const WantParamsJson& wantParams)
{
    jsonObject = nlohmann::json { { JSON_KEY_KEY, wantParams.key }, { JSON_KEY_VALUE, wantParams.value },
        { JSON_KEY_TYPE, wantParams.type } };
}

void from_json(const nlohmann::json& jsonObject, WantParamsJson& wantParams)
{
    const auto& jsonObjectEnd = jsonObject.end();
    GetValueIfFindKey<std::string>(
        jsonObject, jsonObjectEnd, JSON_KEY_KEY, wantParams.key, JsonType::STRING, false, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<std::string>(
        jsonObject, jsonObjectEnd, JSON_KEY_VALUE, wantParams.value, JsonType::STRING, false, ArrayType::NOT_ARRAY);
    GetValueIfFindKey<int>(
        jsonObject, jsonObjectEnd, JSON_KEY_TYPE, wantParams.type, JsonType::NUMBER, false, ArrayType::NOT_ARRAY);
}
} // namespace AAFwk
} // namespace OHOS