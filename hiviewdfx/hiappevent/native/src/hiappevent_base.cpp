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

#include "hiappevent_base.h"

#include <ctime>
#include <iomanip>
#include <pthread.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "hiappevent_config.h"
#include "hilog/log.h"
#include "time_util.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "EventBase"

namespace OHOS {
namespace HiviewDFX {
namespace {
const std::string DEFAULT_DOMAIN = "default";
constexpr size_t MIN_PARAM_STR_LEN = 3; // 3: '{}\0'

std::string TrimRightZero(const std::string& str)
{
    auto endIndex = str.find_last_not_of("0");
    if (endIndex == std::string::npos) {
        return str;
    }

    return (str[endIndex] == '.') ? str.substr(0, endIndex) : str.substr(0, endIndex + 1);
}

void InitValueByBaseType(AppEventParamValue* value, const AppEventParamValue& other)
{
    if (value == nullptr) {
        return;
    }

    switch (other.type) {
        case AppEventParamType::BOOL:
            value->valueUnion.b_ = other.valueUnion.b_;
            break;
        case AppEventParamType::CHAR:
            value->valueUnion.c_ = other.valueUnion.c_;
            break;
        case AppEventParamType::SHORT:
            value->valueUnion.sh_ = other.valueUnion.sh_;
            break;
        case AppEventParamType::INTEGER:
            value->valueUnion.i_ = other.valueUnion.i_;
            break;
        case AppEventParamType::LONGLONG:
            value->valueUnion.ll_ = other.valueUnion.ll_;
            break;
        case AppEventParamType::FLOAT:
            value->valueUnion.f_ = other.valueUnion.f_;
            break;
        case AppEventParamType::DOUBLE:
            value->valueUnion.d_ = other.valueUnion.d_;
            break;
        default:
            break;
    }
}

void InitValueByReferType(AppEventParamValue* value, const AppEventParamValue& other)
{
    if (value == nullptr) {
        return;
    }

    switch (other.type) {
        case AppEventParamType::STRING:
            new (&value->valueUnion.str_) auto(other.valueUnion.str_);
            break;
        case AppEventParamType::BVECTOR:
            new (&value->valueUnion.bs_) auto(other.valueUnion.bs_);
            break;
        case AppEventParamType::CVECTOR:
            new (&value->valueUnion.cs_) auto(other.valueUnion.cs_);
            break;
        case AppEventParamType::SHVECTOR:
            new (&value->valueUnion.shs_) auto(other.valueUnion.shs_);
            break;
        case AppEventParamType::IVECTOR:
            new (&value->valueUnion.is_) auto(other.valueUnion.is_);
            break;
        case AppEventParamType::LLVECTOR:
            new (&value->valueUnion.lls_) auto(other.valueUnion.lls_);
            break;
        case AppEventParamType::FVECTOR:
            new (&value->valueUnion.fs_) auto(other.valueUnion.fs_);
            break;
        case AppEventParamType::DVECTOR:
            new (&value->valueUnion.ds_) auto(other.valueUnion.ds_);
            break;
        case AppEventParamType::STRVECTOR:
            new (&value->valueUnion.strs_) auto(other.valueUnion.strs_);
            break;
        default:
            break;
    }
}

template<typename T>
std::string GetValueStr(const T& value)
{
    return std::to_string(value);
}

std::string GetValueStr(bool value)
{
    return value ? "true" : "false";
}

std::string GetValueStr(char value)
{
    return "\"" + std::to_string(value) + "\"";
}

std::string GetValueStr(float value)
{
    return TrimRightZero(std::to_string(value));
}

std::string GetValueStr(double value)
{
    return TrimRightZero(std::to_string(value));
}

std::string GetValueStr(const std::string& value)
{
    return "\"" + value + "\"";
}

template<typename T>
std::string GetValuesStr(const std::vector<T>& values)
{
    std::string valuesStr;
    valuesStr.append("[");
    size_t valuesSize = values.size();
    for (size_t i = 0; i < valuesSize; ++i) {
        if constexpr (std::is_same_v<std::decay_t<T>,
                          bool>) { // vector<bool> is stored as bit type
            bool bValue = values[i];
            valuesStr.append(GetValueStr(bValue));
        } else {
            valuesStr.append(GetValueStr(values[i]));
        }
        if (i != (valuesSize - 1)) { // -1 for last value
            valuesStr.append(",");
        }
    }
    valuesStr.append("]");
    return valuesStr;
}

std::string GetEmptyParamValueStr(const AppEventParamValue& value)
{
    return "\"\"";
}

std::string GetBoolParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.b_);
}

std::string GetCharParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.c_);
}

std::string GetShortParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.sh_);
}

std::string GetIntParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.i_);
}

std::string GetLongParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.ll_);
}

std::string GetFloatParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.f_);
}

std::string GetDoubleParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.d_);
}

std::string GetStrParamValueStr(const AppEventParamValue& value)
{
    return GetValueStr(value.valueUnion.str_);
}

std::string GetBoolsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.bs_);
}

std::string GetCharsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.cs_);
}

std::string GetShortsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.shs_);
}

std::string GetIntsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.is_);
}

std::string GetLongsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.lls_);
}

std::string GetFloatsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.fs_);
}

std::string GetDoublesParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.ds_);
}

std::string GetStrsParamValueStr(const AppEventParamValue& value)
{
    return GetValuesStr(value.valueUnion.strs_);
}

using GetParamValueFunc = std::string (*)(const AppEventParamValue& value);
const std::unordered_map<AppEventParamType, GetParamValueFunc> GET_PARAM_VALUE_FUNCS = {
    { EMPTY, &GetEmptyParamValueStr },
    { BOOL, &GetBoolParamValueStr },
    { CHAR, &GetCharParamValueStr },
    { SHORT, &GetShortParamValueStr },
    { INTEGER, &GetIntParamValueStr },
    { LONGLONG, &GetLongParamValueStr },
    { FLOAT, &GetFloatParamValueStr },
    { DOUBLE, &GetDoubleParamValueStr },
    { STRING, &GetStrParamValueStr },
    { BVECTOR, &GetBoolsParamValueStr },
    { CVECTOR, &GetCharsParamValueStr },
    { SHVECTOR, &GetShortsParamValueStr },
    { IVECTOR, &GetIntsParamValueStr },
    { LLVECTOR, &GetLongsParamValueStr },
    { FVECTOR, &GetFloatsParamValueStr },
    { DVECTOR, &GetDoublesParamValueStr },
    { STRVECTOR, &GetStrsParamValueStr },
};

std::string GetParamValueStr(const AppEventParam& param)
{
    if (GET_PARAM_VALUE_FUNCS.find(param.value.type) == GET_PARAM_VALUE_FUNCS.end()) {
        HILOG_WARN(LOG_CORE, "Invalid param value");
        return "";
    }
    return GET_PARAM_VALUE_FUNCS.at(param.value.type)(param.value);
}
} // namespace

AppEventParamValue::AppEventParamValue(AppEventParamType t) : type(t), valueUnion(t) {}

AppEventParamValue::AppEventParamValue(const AppEventParamValue& other) : type(other.type)
{
    if (other.type < AppEventParamType::STRING) {
        InitValueByBaseType(this, other);
    } else {
        InitValueByReferType(this, other);
    }
}

AppEventParamValue::~AppEventParamValue()
{
    switch (type) {
        case AppEventParamType::STRING:
            valueUnion.str_.~basic_string();
            break;
        case AppEventParamType::BVECTOR:
            valueUnion.bs_.~vector();
            break;
        case AppEventParamType::CVECTOR:
            valueUnion.cs_.~vector();
            break;
        case AppEventParamType::SHVECTOR:
            valueUnion.shs_.~vector();
            break;
        case AppEventParamType::IVECTOR:
            valueUnion.is_.~vector();
            break;
        case AppEventParamType::LLVECTOR:
            valueUnion.lls_.~vector();
            break;
        case AppEventParamType::FVECTOR:
            valueUnion.fs_.~vector();
            break;
        case AppEventParamType::DVECTOR:
            valueUnion.ds_.~vector();
            break;
        case AppEventParamType::STRVECTOR:
            valueUnion.strs_.~vector();
            break;
        default:
            break;
    }
}

AppEventParam::AppEventParam(std::string n, AppEventParamType t) : name(n), type(t), value(t) {}

AppEventParam::AppEventParam(const AppEventParam& param) : name(param.name), type(param.type), value(param.value) {}

AppEventParam::~AppEventParam() {}

AppEventPack::AppEventPack(const std::string& name, int type) : AppEventPack(DEFAULT_DOMAIN, name, type) {}

AppEventPack::AppEventPack(const std::string& domain, const std::string& name, int type)
    : domain_(domain), name_(name), type_(type)
{
    InitTime();
    InitTimeZone();
    InitProcessInfo();
    InitRunningId();
}

void AppEventPack::InitTime()
{
    time_ = TimeUtil::GetMilliseconds();
}

void AppEventPack::InitTimeZone()
{
    timeZone_ = TimeUtil::GetTimeZone();
}

void AppEventPack::InitProcessInfo()
{
    pid_ = getpid();
#if defined(IOS_PLATFORM)
    pthread_threadid_np(NULL, &tid_);
#else
    tid_ = gettid();
#endif
}

void AppEventPack::InitRunningId()
{
    runningId_ = HiAppEventConfig::GetInstance().GetRunningId();
}

void AppEventPack::AddParam(const std::string& key)
{
    AppEventParam appEventParam(key, AppEventParamType::EMPTY);
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, bool b)
{
    AppEventParam appEventParam(key, AppEventParamType::BOOL);
    appEventParam.value.valueUnion.b_ = b;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, char c)
{
    AppEventParam appEventParam(key, AppEventParamType::CHAR);
    appEventParam.value.valueUnion.c_ = c;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, int8_t num)
{
    AppEventParam appEventParam(key, AppEventParamType::SHORT);
    appEventParam.value.valueUnion.sh_ = static_cast<int16_t>(num);
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, int16_t s)
{
    AppEventParam appEventParam(key, AppEventParamType::SHORT);
    appEventParam.value.valueUnion.sh_ = s;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, int i)
{
    AppEventParam appEventParam(key, AppEventParamType::INTEGER);
    appEventParam.value.valueUnion.i_ = i;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, int64_t ll)
{
    AppEventParam appEventParam(key, AppEventParamType::LONGLONG);
    appEventParam.value.valueUnion.ll_ = ll;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, float f)
{
    AppEventParam appEventParam(key, AppEventParamType::FLOAT);
    appEventParam.value.valueUnion.f_ = f;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, double d)
{
    AppEventParam appEventParam(key, AppEventParamType::DOUBLE);
    appEventParam.value.valueUnion.d_ = d;
    baseParams_.emplace_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const char* s)
{
    AppEventParam appEventParam(key, AppEventParamType::STRING);
    appEventParam.value.valueUnion.str_ = s;
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::string& s)
{
    AppEventParam appEventParam(key, AppEventParamType::STRING);
    appEventParam.value.valueUnion.str_ = s;
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<bool>& bs)
{
    AppEventParam appEventParam(key, AppEventParamType::BVECTOR);
    appEventParam.value.valueUnion.bs_.assign(bs.begin(), bs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<char>& cs)
{
    AppEventParam appEventParam(key, AppEventParamType::CVECTOR);
    appEventParam.value.valueUnion.cs_.assign(cs.begin(), cs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int8_t>& shs)
{
    AppEventParam appEventParam(key, AppEventParamType::SHVECTOR);
    appEventParam.value.valueUnion.shs_.assign(shs.begin(), shs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int16_t>& shs)
{
    AppEventParam appEventParam(key, AppEventParamType::SHVECTOR);
    appEventParam.value.valueUnion.shs_.assign(shs.begin(), shs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int>& is)
{
    AppEventParam appEventParam(key, AppEventParamType::IVECTOR);
    appEventParam.value.valueUnion.is_.assign(is.begin(), is.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<int64_t>& lls)
{
    AppEventParam appEventParam(key, AppEventParamType::LLVECTOR);
    appEventParam.value.valueUnion.lls_.assign(lls.begin(), lls.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<float>& fs)
{
    AppEventParam appEventParam(key, AppEventParamType::FVECTOR);
    appEventParam.value.valueUnion.fs_.assign(fs.begin(), fs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<double>& ds)
{
    AppEventParam appEventParam(key, AppEventParamType::DVECTOR);
    appEventParam.value.valueUnion.ds_.assign(ds.begin(), ds.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<const char*>& cps)
{
    AppEventParam appEventParam(key, AppEventParamType::STRVECTOR);
    std::vector<std::string> strs;
    if (cps.size() != 0) {
        for (auto cp : cps) {
            if (cp != nullptr) {
                strs.push_back(cp);
            }
        }
    }
    appEventParam.value.valueUnion.strs_.assign(strs.begin(), strs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddParam(const std::string& key, const std::vector<std::string>& strs)
{
    AppEventParam appEventParam(key, AppEventParamType::STRVECTOR);
    appEventParam.value.valueUnion.strs_.assign(strs.begin(), strs.end());
    baseParams_.push_back(appEventParam);
}

void AppEventPack::AddCustomParams(const std::unordered_map<std::string, std::string>& customParams)
{
    if (customParams.empty()) {
        return;
    }
    std::string paramStr = GetParamStr();
    if (paramStr.size() >= MIN_PARAM_STR_LEN) {
        std::stringstream jsonStr;
        if (paramStr.size() > MIN_PARAM_STR_LEN) {
            jsonStr << ",";
        }
        for (auto it = customParams.begin(); it != customParams.end(); ++it) {
            jsonStr << "\"" << it->first << "\":" << it->second << ",";
        }
        std::string customParamStr = jsonStr.str();
        customParamStr.erase(customParamStr.end() - 1);       // -1 for delete ','
        paramStr.insert(paramStr.size() - 2, customParamStr); // 2 for '}\0'
        paramStr_ = paramStr;
    }
}

std::string AppEventPack::GetEventStr() const
{
    std::stringstream jsonStr;
    jsonStr << "{";
    AddBaseInfoToJsonString(jsonStr);
    AddParamsInfoToJsonString(jsonStr);
    jsonStr << "}" << std::endl;
    return jsonStr.str();
}

std::string AppEventPack::GetParamStr() const
{
    if (!paramStr_.empty()) {
        return paramStr_;
    }

    std::stringstream jsonStr;
    jsonStr << "{";
    AddParamsToJsonString(jsonStr);
    jsonStr << "}" << std::endl;
    return jsonStr.str();
}

void AppEventPack::AddBaseInfoToJsonString(std::stringstream& jsonStr) const
{
    jsonStr << "\""
            << "domain_"
            << "\":"
            << "\"" << domain_ << "\",";
    jsonStr << "\""
            << "name_"
            << "\":"
            << "\"" << name_ << "\",";
    jsonStr << "\""
            << "type_"
            << "\":" << type_ << ",";
    jsonStr << "\""
            << "time_"
            << "\":" << std::to_string(time_) << ",";
    jsonStr << "\"tz_\":\"" << timeZone_ << "\",";
    jsonStr << "\""
            << "pid_"
            << "\":" << pid_ << ",";
    jsonStr << "\""
            << "tid_"
            << "\":" << tid_;
}

void AppEventPack::AddParamsInfoToJsonString(std::stringstream& jsonStr) const
{
    // for event from writing
    if (baseParams_.size() != 0) {
        jsonStr << ",";
        AddParamsToJsonString(jsonStr);
        return;
    }

    size_t paramStrLen = paramStr_.length();
    if (paramStrLen > MIN_PARAM_STR_LEN) {
        jsonStr << "," << paramStr_.substr(1, paramStrLen - MIN_PARAM_STR_LEN); // 1: '{' for next char
    }
}

void AppEventPack::AddParamsToJsonString(std::stringstream& jsonStr) const
{
    if (baseParams_.empty()) {
        return;
    }
    for (const auto& param : baseParams_) {
        jsonStr << "\"" << param.name << "\":" << GetParamValueStr(param) << ",";
    }
    jsonStr.seekp(-1, std::ios_base::end); // -1 for delete ','
}

void AppEventPack::GetCustomParams(std::vector<CustomEventParam>& customParams) const
{
    for (const auto& param : baseParams_) {
        CustomEventParam customParam = {
            .key = param.name,
            .value = GetParamValueStr(param),
            .type = param.value.type,
        };
        customParams.push_back(customParam);
    }
}

int64_t AppEventPack::GetSeq() const
{
    return seq_;
}

std::string AppEventPack::GetDomain() const
{
    return domain_;
}

std::string AppEventPack::GetName() const
{
    return name_;
}

int AppEventPack::GetType() const
{
    return type_;
}

uint64_t AppEventPack::GetTime() const
{
    return time_;
}

std::string AppEventPack::GetTimeZone() const
{
    return timeZone_;
}

int AppEventPack::GetPid() const
{
    return pid_;
}

int AppEventPack::GetTid() const
{
    return tid_;
}

std::string AppEventPack::GetRunningId() const
{
    return runningId_;
}

void AppEventPack::SetSeq(int64_t seq)
{
    seq_ = seq;
}

void AppEventPack::SetDomain(const std::string& domain)
{
    domain_ = domain;
}

void AppEventPack::SetName(const std::string& name)
{
    name_ = name;
}

void AppEventPack::SetType(int type)
{
    type_ = type;
}

void AppEventPack::SetTime(uint64_t time)
{
    time_ = time;
}

void AppEventPack::SetTimeZone(const std::string& timeZone)
{
    timeZone_ = timeZone;
}

void AppEventPack::SetPid(int pid)
{
    pid_ = pid;
}

void AppEventPack::SetTid(uint64_t tid)
{
    tid_ = tid;
}

void AppEventPack::SetParamStr(const std::string& paramStr)
{
    paramStr_ = paramStr;
}

void AppEventPack::SetRunningId(const std::string& runningId)
{
    runningId_ = runningId;
}
} // namespace HiviewDFX
} // namespace OHOS
