/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "want_params_wrapper.h"

#include <algorithm>

#include "array_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const int TYPEID_TRANSFORM_START = 6;
const int TYPEID_TRANSFORM_END = 10;
size_t FindMatchingBrackets(const std::string &str, size_t leftIndex)
{
    int count = 0;
    for (size_t i = leftIndex + 1; i < str.size(); ++i) {
        if (str[i] == '{') {
            count++;
        } else if (str[i] == '}') {
            if (count == 0) {
                return i;
            }
            count--;
        }
    }
    return -1;
}
}
constexpr int32_t WANT_PARAM_WRAPPER_TWO = 2;

IINTERFACE_IMPL_1(WantParamWrapper, Object, IWantParams);
const InterfaceID g_IID_IWantParams = {
    0xa75b9db6, 0x9813, 0x4371, 0x8848, {0xd, 0x2, 0x9, 0x6, 0x6, 0xc, 0xe, 0x6, 0xe, 0xc, 0x6, 0x8}
};
ErrCode WantParamWrapper::GetValue(WantParams &value)
{
    value = wantParams_;
    return ERR_OK;
}

bool WantParamWrapper::Equals(IObject &other)
{
    WantParamWrapper *otherObj = static_cast<WantParamWrapper *>(IWantParams::Query(&other));
    return otherObj != nullptr && otherObj->wantParams_ == wantParams_;
}

std::string WantParamWrapper::ToString()
{
    std::string result;
    if (wantParams_.Size() == 0) {
        return "[]";
    }
    result += "[";
    for (auto it : wantParams_.GetParams()) {
        int typeId = WantParams::GetDataType(it.second);
        result = result + "{\"" + JSON_WANTPARAMS_KEY + "\":\"" + it.first + "\",\"" + JSON_WANTPARAMS_TYPE + "\":";
        if (typeId > TYPEID_TRANSFORM_START && typeId < TYPEID_TRANSFORM_END) {
            result = result + std::to_string(typeId + 1);
        } else {
            result = result + std::to_string(typeId);
        }
        result = result + ",\"" + JSON_WANTPARAMS_VALUE + "\":";
        if (IWantParams::Query(it.second) != nullptr) {
            result = result + static_cast<WantParamWrapper*>(IWantParams::Query(it.second))->ToString();
        } else {
            if (typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_ARRAY) ||
                typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_SHORT) ||
                typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_BOOLEAN) ||
                typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_BYTE) ||
                typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_INT) ||
                typeId == static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_LONG) ||
                typeId == (static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_FLOAT) - 1) ||
                typeId == (static_cast<int>(OHOS::AAFwk::WantValueType::VALUE_TYPE_DOUBLE) - 1)) {
                result = result + WantParams::GetStringByType(it.second, typeId);
            } else {
                result = result + "\"" + WantParams::GetStringByType(it.second, typeId) + "\"";
            }
        }
        if (it == *wantParams_.GetParams().rbegin()) {
            result += "}";
        } else {
            result += "},";
        }
    }
    result += "]";
    return result;
}

sptr<IWantParams> WantParamWrapper::Box(const WantParams &value)
{
    sptr<IWantParams> object = new (std::nothrow)WantParamWrapper(value);
    return object;
}

sptr<IWantParams> WantParamWrapper::Box(WantParams &&value)
{
    sptr<IWantParams> object = new (std::nothrow) WantParamWrapper(std::move(value));
    return object;
}

WantParams WantParamWrapper::Unbox(IWantParams *object)
{
    WantParams value;
    if (object != nullptr) {
        object->GetValue(value);
    }
    return value;
}
bool WantParamWrapper::ValidateStr(const std::string &str)
{
    if (str == "" || str == "{}" || str == "{\"\"}") {
        return false;
    }
    if (count(str.begin(), str.end(), '\"') % WANT_PARAM_WRAPPER_TWO != 0) {
        return false;
    }
    if (count(str.begin(), str.end(), '{') != count(str.begin(), str.end(), '}')) {
        return false;
    }
    int count = 0;
    for (auto it : str) {
        if (it == '{') {
            count++;
        }
        if (it == '}') {
            count--;
        }
        if (count < 0) {
            return false;
        }
    }
    return true;
}

sptr<IWantParams> WantParamWrapper::Parse(const std::string &str)
{
    WantParams wantPaqrams;
    std::string key = "";
    if (ValidateStr(str)) {
        int typeId = 0;
        for (size_t strnum = 0; strnum < str.size(); strnum++) {
            if (str[strnum] == '{' && key != "" && typeId == WantParams::VALUE_TYPE_WANTPARAMS) {
                size_t num;
                int count = 0;
                for (num = strnum; num < str.size(); num++) {
                    if (str[num] == '{') {
                        count++;
                    } else if (str[num] == '}') {
                        count--;
                    }
                    if (count == 0) {
                        break;
                    }
                }
                wantPaqrams.SetParam(key, WantParamWrapper::Parse(str.substr(strnum, num - strnum + 1)));
                key = "";
                typeId = 0;
                strnum = num + 1;
            } else if (str[strnum] == '"') {
                if (key == "") {
                    strnum++;
                    key = str.substr(strnum, str.find('"', strnum) - strnum);
                    strnum = str.find('"', strnum);
                } else if (typeId == 0) {
                    strnum++;
                    typeId = atoi(str.substr(strnum, str.find('"', strnum) - strnum).c_str());
                    if (errno == ERANGE) {
                        return nullptr;
                    }
                    strnum = str.find('"', strnum);
                } else {
                    strnum++;
                    wantPaqrams.SetParam(key,
                        WantParams::GetInterfaceByType(typeId, str.substr(strnum, str.find('"', strnum) - strnum)));
                    strnum = str.find('"', strnum);
                    typeId = 0;
                    key = "";
                }
            }
        }
    }
    sptr<IWantParams> iwantParams = new (std::nothrow) WantParamWrapper(wantPaqrams);
    return iwantParams;
}

WantParams WantParamWrapper::ParseWantParams(const std::string &str)
{
    WantParams wantPaqrams;
    std::string key = "";
    int typeId = 0;
    if (!ValidateStr(str)) {
        return wantPaqrams;
    }
    for (size_t strnum = 0; strnum < str.size(); strnum++) {
        if (str[strnum] == '{' && key != "" && typeId == WantParams::VALUE_TYPE_WANTPARAMS) {
            size_t num;
            int count = 0;
            for (num = strnum; num < str.size(); num++) {
                if (str[num] == '{') {
                    count++;
                } else if (str[num] == '}') {
                    count--;
                }
                if (count == 0) {
                    break;
                }
            }
            wantPaqrams.SetParam(key, WantParamWrapper::Parse(str.substr(strnum, num - strnum)));
            key = "";
            typeId = 0;
            strnum = num + 1;
        } else if (str[strnum] == '"') {
            if (key == "") {
                strnum++;
                key = str.substr(strnum, str.find('"', strnum) - strnum);
                strnum = str.find('"', strnum);
            } else if (typeId == 0) {
                strnum++;
                typeId = atoi(str.substr(strnum, str.find('"', strnum) - strnum).c_str());
                if (errno == ERANGE) {
                    return wantPaqrams;
                }
                strnum = str.find('"', strnum);
            } else {
                strnum++;
                wantPaqrams.SetParam(key,
                    WantParams::GetInterfaceByType(typeId, str.substr(strnum, str.find('"', strnum) - strnum)));
                strnum = str.find('"', strnum);
                typeId = 0;
                key = "";
            }
        }
    }
    return wantPaqrams;
}

WantParams WantParamWrapper::ParseWantParamsWithBrackets(const std::string &str)
{
    WantParams wantPaqrams;
    std::string key = "";
    int typeId = 0;
    size_t type_index_before = 0;
    if (!ValidateStr(str)) {
        return wantPaqrams;
    }
    for (size_t strnum = 0; strnum < str.size(); strnum++) {
        if (str[strnum] == '{' && key != "" && typeId == WantParams::VALUE_TYPE_WANTPARAMS) {
            size_t num;
            int count = 0;
            for (num = strnum; num < str.size(); num++) {
                if (str[num] == '{') {
                    count++;
                } else if (str[num] == '}') {
                    count--;
                }
                if (count == 0) {
                    break;
                }
            }
            wantPaqrams.SetParam(key, WantParamWrapper::Parse(str.substr(strnum, num - strnum + 1)));
            key = "";
            typeId = 0;
            strnum = num + 1;
        } else if (str[strnum] == '"') {
            if (key == "") {
                strnum++;
                key = str.substr(strnum, str.find('"', strnum) - strnum);
                strnum = str.find('"', strnum);
            } else if (typeId == 0) {
                type_index_before = strnum;
                strnum++;
                typeId = atoi(str.substr(strnum, str.find('"', strnum) - strnum).c_str());
                if (errno == ERANGE) {
                    return wantPaqrams;
                }
                strnum = str.find('"', strnum);
            } else {
                strnum++;
                auto index = FindMatchingBrackets(str, type_index_before - 1);
                wantPaqrams.SetParam(key,
                    WantParams::GetInterfaceByType(typeId, str.substr(strnum, index - 1 - strnum)));
                strnum = index + 1;
                typeId = 0;
                key = "";
            }
        }
    }
    return wantPaqrams;
}

sptr<IWantParams> WantParamWrapper::Parse(const nlohmann::json& wantJson)
{
    WantParams wantParams;
    ParseWantParams(wantJson, wantParams);
    sptr<IWantParams> wantParamsPtr = new (std::nothrow) WantParamWrapper(wantParams);
    return wantParamsPtr;
}

void WantParamWrapper::ParseWantParams(const nlohmann::json& wantArray, OHOS::AAFwk::WantParams& wantParams)
{
    for (auto& element : wantArray) {
        auto typeId = element[JSON_WANTPARAMS_TYPE].get<int>();
        auto elementKey = element[JSON_WANTPARAMS_KEY];
        auto elemetnValue = element[JSON_WANTPARAMS_VALUE];
        auto localType = static_cast<AAFwk::WantValueType>(typeId);
        if (localType == AAFwk::WantValueType::VALUE_TYPE_BOOLEAN) {
            wantParams.SetParam(elementKey, WantParams::GetInterfaceByType(typeId, elemetnValue ? "true" : "false"));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_INT) {
            wantParams.SetParam(
                elementKey, WantParams::GetInterfaceByType(typeId, std::to_string(elemetnValue.get<int64_t>())));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_DOUBLE) {
            auto intType = elemetnValue.type();
            if (intType == nlohmann::json::value_t::number_float) {
                wantParams.SetParam(
                    elementKey, WantParams::GetInterfaceByType(typeId - 1, std::to_string(elemetnValue.get<double>())));
            } else if (intType == nlohmann::json::value_t::number_integer ||
                       intType == nlohmann::json::value_t::number_unsigned) {
                wantParams.SetParam(elementKey,
                    WantParams::GetInterfaceByType(typeId - 1, std::to_string(elemetnValue.get<int64_t>())));
            }
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_STRING) {
            wantParams.SetParam(
                elementKey, WantParams::GetInterfaceByType(typeId - 1, elemetnValue.get<std::string>()));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_ARRAY) {
            wantParams.SetParam(elementKey, Array::ParseCrossPlatformArray(elemetnValue));
        } else if (localType == AAFwk::WantValueType::VALUE_TYPE_WANTPARAMS) {
            WantParams localWantParams;
            WantParamWrapper::ParseWantParams(elemetnValue, localWantParams);
            sptr<IWantParams> localIwantParams = new (std::nothrow) WantParamWrapper(localWantParams);
            wantParams.SetParam(elementKey, localIwantParams);
        }
    }
}
}  // namespace AAFwk
}  // namespace OHOS