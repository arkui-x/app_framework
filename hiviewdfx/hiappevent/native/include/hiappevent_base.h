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

#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_BASE_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_BASE_H

#include <ctime>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
/**
 * HiAppEvent write app event error code
 */
namespace ErrorCode {
const int HIAPPEVENT_VERIFY_SUCCESSFUL = 0;
const int ERROR_INVALID_EVENT_NAME = -1;
const int ERROR_INVALID_PARAM_TYPE_JS = -2;
const int ERROR_INVALID_PARAM_NUM_JS = -3;
const int ERROR_INVALID_EVENT_DOMAIN = -4;
const int ERROR_INVALID_WATCHER = -5;
const int ERROR_WATCHER_NOT_ADDED = -6;
const int ERROR_INVALID_PROCESSOR = -7;
const int ERROR_PROCESSOR_NOT_ADDED = -8;
const int ERROR_INVALID_PARAM_VALUE = -9;
const int ERROR_INVALID_PARAM_NAME = 1;
const int ERROR_INVALID_PARAM_KEY_TYPE = 2;
const int ERROR_INVALID_PARAM_VALUE_TYPE = 3;
const int ERROR_INVALID_PARAM_VALUE_LENGTH = 4;
const int ERROR_INVALID_PARAM_NUM = 5;
const int ERROR_INVALID_LIST_PARAM_SIZE = 6;
const int ERROR_INVALID_LIST_PARAM_TYPE = 7;
const int ERROR_DUPLICATE_PARAM = 8;
const int ERROR_INVALID_CUSTOM_PARAM_NUM = 9;
const int ERROR_HIAPPEVENT_DISABLE = -99;
const int ERROR_UNKNOWN = -100;
const int ERROR_NOT_APP = -200;
} // namespace ErrorCode

/**
 * HiLog hiappevent domain code
 */
const unsigned int HIAPPEVENT_DOMAIN = 0xD002D07;

enum ObserverType { WATCHER = 0, PROCESSOR = 1 };

enum AppEventParamType {
    EMPTY = 0,
    BOOL = 1,
    CHAR = 2,
    SHORT = 3,
    INTEGER = 4,
    LONGLONG = 5,
    FLOAT = 6,
    DOUBLE = 7,
    STRING = 8,
    BVECTOR = 9,
    CVECTOR = 10,
    SHVECTOR = 11,
    IVECTOR = 12,
    LLVECTOR = 13,
    FVECTOR = 14,
    DVECTOR = 15,
    STRVECTOR = 16
};

struct AppEventParamValue {
    AppEventParamType type;
    union ValueUnion {
        bool b_;
        char c_;
        int16_t sh_;
        int i_;
        int64_t ll_;
        float f_;
        double d_;
        std::string str_;
        std::vector<bool> bs_;
        std::vector<char> cs_;
        std::vector<int16_t> shs_;
        std::vector<int> is_;
        std::vector<int64_t> lls_;
        std::vector<float> fs_;
        std::vector<double> ds_;
        std::vector<std::string> strs_;

        ValueUnion() {}

        ValueUnion(AppEventParamType type)
        {
            switch (type) {
                case AppEventParamType::STRING:
                    new (&str_) std::string;
                    break;
                case AppEventParamType::BVECTOR:
                    new (&bs_) std::vector<bool>;
                    break;
                case AppEventParamType::CVECTOR:
                    new (&cs_) std::vector<char>;
                    break;
                case AppEventParamType::SHVECTOR:
                    new (&shs_) std::vector<int16_t>;
                    break;
                case AppEventParamType::IVECTOR:
                    new (&is_) std::vector<int>;
                    break;
                case AppEventParamType::LLVECTOR:
                    new (&lls_) std::vector<int64_t>;
                    break;
                case AppEventParamType::FVECTOR:
                    new (&fs_) std::vector<float>;
                    break;
                case AppEventParamType::DVECTOR:
                    new (&ds_) std::vector<double>;
                    break;
                case AppEventParamType::STRVECTOR:
                    new (&strs_) std::vector<std::string>;
                    break;
                default:
                    break;
            }
        }

        ~ValueUnion() {}
    } valueUnion;

    explicit AppEventParamValue(AppEventParamType t);
    AppEventParamValue(const AppEventParamValue& value);
    ~AppEventParamValue();
};
using AppEventParamValue = struct AppEventParamValue;

struct AppEventParam {
    std::string name;
    AppEventParamType type;
    AppEventParamValue value;

    AppEventParam(std::string n, AppEventParamType t);
    AppEventParam(const AppEventParam& param);
    ~AppEventParam();
};
using AppEventParam = struct AppEventParam;

struct CustomEventParam {
    std::string key;
    std::string value;
    int type = 0;
};
using CustomEventParam = struct CustomEventParam;

class AppEventPack {
public:
    AppEventPack() = default;
    AppEventPack(const std::string& name, int type);
    AppEventPack(const std::string& domain, const std::string& name, int type = 0);
    ~AppEventPack() {}

public:
    void AddParam(const std::string& key);
    void AddParam(const std::string& key, bool b);
    void AddParam(const std::string& key, int8_t num);
    void AddParam(const std::string& key, char c);
    void AddParam(const std::string& key, int16_t s);
    void AddParam(const std::string& key, int i);
    void AddParam(const std::string& key, int64_t ll);
    void AddParam(const std::string& key, float f);
    void AddParam(const std::string& key, double d);
    void AddParam(const std::string& key, const char* s);
    void AddParam(const std::string& key, const std::string& s);
    void AddParam(const std::string& key, const std::vector<bool>& bs);
    void AddParam(const std::string& key, const std::vector<int8_t>& bs);
    void AddParam(const std::string& key, const std::vector<char>& cs);
    void AddParam(const std::string& key, const std::vector<int16_t>& shs);
    void AddParam(const std::string& key, const std::vector<int>& is);
    void AddParam(const std::string& key, const std::vector<int64_t>& lls);
    void AddParam(const std::string& key, const std::vector<float>& fs);
    void AddParam(const std::string& key, const std::vector<double>& ds);
    void AddParam(const std::string& key, const std::vector<const char*>& cps);
    void AddParam(const std::string& key, const std::vector<std::string>& strs);
    void AddCustomParams(const std::unordered_map<std::string, std::string>& customParams);

    int64_t GetSeq() const;
    std::string GetDomain() const;
    std::string GetName() const;
    int GetType() const;
    uint64_t GetTime() const;
    std::string GetTimeZone() const;
    int GetPid() const;
    int GetTid() const;
    std::string GetEventStr() const;
    std::string GetParamStr() const;
    std::string GetRunningId() const;
    void GetCustomParams(std::vector<CustomEventParam>& customParams) const;

    void SetSeq(int64_t seq);
    void SetDomain(const std::string& domain);
    void SetName(const std::string& name);
    void SetType(int type);
    void SetTime(uint64_t time);
    void SetTimeZone(const std::string& timeZone);
    void SetPid(int pid);
    void SetTid(uint64_t tid);
    void SetParamStr(const std::string& paramStr);
    void SetRunningId(const std::string& runningId);

    friend int VerifyAppEvent(std::shared_ptr<AppEventPack> appEventPack);
    friend int VerifyCustomEventParams(std::shared_ptr<AppEventPack> event);

private:
    void InitTime();
    void InitTimeZone();
    void InitProcessInfo();
    void InitRunningId();
    void AddBaseInfoToJsonString(std::stringstream& jsonStr) const;
    void AddParamsInfoToJsonString(std::stringstream& jsonStr) const;
    void AddParamsToJsonString(std::stringstream& jsonStr) const;

private:
    int64_t seq_ = 0;
    std::string domain_;
    std::string name_;
    int type_ = 0;
    uint64_t time_ = 0;
    std::string timeZone_;
    int pid_ = 0;
    uint64_t tid_ = 0;
    std::string runningId_;
    std::list<AppEventParam> baseParams_;
    std::string paramStr_;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_BASE_H
