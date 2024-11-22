/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <cstring>

#include "hilog.h"
#include "napi_common_error.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
bool IsTypeForNapiValue(napi_env env, napi_value param, napi_valuetype expectType)
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

bool UnwrapString(napi_env env, napi_value param, std::string& value)
{
    value = "";
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return false;
    }

    if (size == 0) {
        return true;
    }

    char* buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return false;
    }

    auto ret = memset_s(buf, (size + 1), 0, (size + 1));
    if (ret != EOK) {
        HILOG_ERROR("memset_s failed, ret: %{public}d", ret);
        delete[] buf;
        return false;
    }

    bool rev = napi_get_value_string_utf8(env, param, buf, size + 1, &size) == napi_ok;
    if (rev) {
        value = buf;
    }
    delete[] buf;
    buf = nullptr;
    return rev;
}

std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string& defaultValue)
{
    size_t size = 0;
    if (napi_get_value_string_utf8(env, param, nullptr, 0, &size) != napi_ok) {
        return defaultValue;
    }

    std::string value("");
    if (size == 0) {
        return defaultValue;
    }

    char* buf = new (std::nothrow) char[size + 1];
    if (buf == nullptr) {
        return value;
    }

    auto ret = memset_s(buf, size + 1, 0, size + 1);
    if (ret != EOK) {
        HILOG_ERROR("memset_s failed, ret: %{public}d", ret);
        delete[] buf;
        return value;
    }

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

/**
 * @brief Get the native string from the JSObject of the given property name.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param value Indicates the returned native value.
 *
 * @return Return true if successful, else return false.
 */
bool UnwrapStringByPropertyName(napi_env env, napi_value jsObject, const char* propertyName, std::string& value)
{
    napi_value jsValue = GetPropertyValueByPropertyName(env, jsObject, propertyName, napi_string);
    if (jsValue != nullptr) {
        return UnwrapString(env, jsValue, value);
    } else {
        return false;
    }
}

/**
 * @brief Get the JSValue of the specified name from the JS object.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 * @param expectType Indicates expected JS data type.
 *
 * @return Return the property value of the specified property name int jsObject on success, otherwise return nullptr.
 */
napi_value GetPropertyValueByPropertyName(
    napi_env env, napi_value jsObject, const char* propertyName, napi_valuetype expectType)
{
    napi_value value = nullptr;
    if (IsExistsByPropertyName(env, jsObject, propertyName) == false) {
        return nullptr;
    }

    if (napi_get_named_property(env, jsObject, propertyName, &value) != napi_ok) {
        return nullptr;
    }

    if (!IsTypeForNapiValue(env, value, expectType)) {
        return nullptr;
    }

    return value;
}

bool SetPropertyValueByPropertyName(napi_env env, napi_value jsObject, const char *propertyName, napi_value value)
{
    if (value != nullptr && propertyName != nullptr) {
        NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, propertyName, value), false);
        return true;
    }
    return false;
}

/**
 * @brief Indicates the specified attribute exists in the object passed by JS.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param jsObject Indicates object passed by JS.
 * @param propertyName Indicates the name of the property.
 *
 * @return Returns true if the attribute exists, else returns false.
 */
bool IsExistsByPropertyName(napi_env env, napi_value jsObject, const char* propertyName)
{
    bool result = false;
    if (napi_has_named_property(env, jsObject, propertyName, &result) == napi_ok) {
        return result;
    } else {
        return false;
    }
}
} // namespace AppExecFwk
} // namespace OHOS
