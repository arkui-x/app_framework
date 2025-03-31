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

#include "hiappevent_verify.h"

#include <cctype>
#include <iterator>
#include <unistd.h>
#include <unordered_set>

#include "hiappevent_base.h"
#include "hiappevent_config.h"
#include "hilog/log.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "Verify"

using namespace OHOS::HiviewDFX::ErrorCode;

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t MAX_LEN_OF_WATCHER = 32;
constexpr size_t MAX_LEN_OF_DOMAIN = 32;
static constexpr int MAX_LENGTH_OF_EVENT_NAME = 48;
static constexpr int MAX_LENGTH_OF_PARAM_NAME = 32;
static constexpr unsigned int MAX_NUM_OF_PARAMS = 32;
static constexpr size_t MAX_LENGTH_OF_STR_PARAM = 8 * 1024;
static constexpr size_t MAX_LENGTH_OF_SPECIAL_STR_PARAM = 1024 * 1024;
static constexpr int MAX_SIZE_OF_LIST_PARAM = 100;
static constexpr int MAX_LENGTH_OF_USER_INFO_NAME = 256;
static constexpr int MAX_LENGTH_OF_USER_ID_VALUE = 256;
static constexpr int MAX_LENGTH_OF_USER_PROPERTY_VALUE = 1024;
static constexpr int MAX_LENGTH_OF_PROCESSOR_NAME = 256;
static constexpr int MAX_LEN_OF_BATCH_REPORT = 1000;
static constexpr size_t MAX_NUM_OF_CUSTOM_CONFIGS = 32;
static constexpr size_t MAX_LENGTH_OF_CUSTOM_CONFIG_NAME = 32;
static constexpr size_t MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE = 1024;
static constexpr size_t MAX_NUM_OF_CUSTOM_PARAMS = 64;
static constexpr size_t MAX_LENGTH_OF_CUSTOM_PARAM = 1024;
constexpr int MIN_APP_UID = 20000;

bool IsValidName(const std::string& name, size_t maxSize, bool allowDollarSign = true)
{
    if (name.empty() || name.length() > maxSize) {
        return false;
    }
    // start char is [$a-zA-Z] or [a-zA-Z]
    if (!isalpha(name[0]) && (!allowDollarSign || name[0] != '$')) {
        return false;
    }
    // end char is [a-zA-Z0-9]
    if (name.length() > 1 && (!isalnum(name.back()))) {
        return false;
    }
    // middle char is [a-zA-Z0-9_]
    for (size_t i = 1; i < name.length() - 1; ++i) {
        if (!isalnum(name[i]) && name[i] != '_') {
            return false;
        }
    }
    return true;
}

bool CheckParamName(const std::string& paramName)
{
    return IsValidName(paramName, MAX_LENGTH_OF_PARAM_NAME);
}

void EscapeStringValue(std::string& value)
{
    std::string escapeValue;
    for (auto it = value.begin(); it != value.end(); it++) {
        switch (*it) {
            case '\\':
                escapeValue.append("\\\\");
                break;
            case '\"':
                escapeValue.append("\\\"");
                break;
            case '\b':
                escapeValue.append("\\b");
                break;
            case '\f':
                escapeValue.append("\\f");
                break;
            case '\n':
                escapeValue.append("\\n");
                break;
            case '\r':
                escapeValue.append("\\r");
                break;
            case '\t':
                escapeValue.append("\\t");
                break;
            default:
                escapeValue.push_back(*it);
                break;
        }
    }
    value = escapeValue;
}

bool CheckStrParamLength(std::string& strParamValue, size_t maxLen = MAX_LENGTH_OF_STR_PARAM)
{
    if (strParamValue.empty()) {
        return true;
    }

    if (strParamValue.length() > maxLen) {
        return false;
    }

    EscapeStringValue(strParamValue);
    return true;
}

bool CheckListValueSize(AppEventParamType type, AppEventParamValue::ValueUnion& vu)
{
    if (type == AppEventParamType::BVECTOR && vu.bs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.bs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::CVECTOR && vu.cs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.cs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::SHVECTOR && vu.shs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.shs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::IVECTOR && vu.is_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.is_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::LLVECTOR && vu.lls_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.lls_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::FVECTOR && vu.fs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.fs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::DVECTOR && vu.ds_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.ds_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else if (type == AppEventParamType::STRVECTOR && vu.strs_.size() > MAX_SIZE_OF_LIST_PARAM) {
        vu.strs_.resize(MAX_SIZE_OF_LIST_PARAM);
    } else {
        return true;
    }

    return false;
}

bool CheckStringLengthOfList(std::vector<std::string>& strs, size_t maxTotalLen = 0)
{
    if (strs.empty()) {
        return true;
    }
    size_t totalLen = 0;
    for (auto it = strs.begin(); it != strs.end(); it++) {
        if (!CheckStrParamLength(*it)) {
            return false;
        }
        totalLen += (*it).length();
    }
    if (maxTotalLen > 0 && totalLen > maxTotalLen) {
        return false;
    }
    return true;
}

bool CheckParamsNum(std::list<AppEventParam>& baseParams)
{
    if (baseParams.size() == 0) {
        return true;
    }

    auto listSize = baseParams.size();
    if (listSize > MAX_NUM_OF_PARAMS) {
        auto delStartPtr = baseParams.begin();
        std::advance(delStartPtr, MAX_NUM_OF_PARAMS);
        baseParams.erase(delStartPtr, baseParams.end());
        return false;
    }

    return true;
}

bool VerifyAppEventParam(AppEventParam& param, std::unordered_set<std::string>& paramNames, int& verifyRes)
{
    std::string name = param.name;
    if (paramNames.find(name) != paramNames.end()) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because param is duplicate.", name.c_str());
        verifyRes = ERROR_DUPLICATE_PARAM;
        return false;
    }

    if (!CheckParamName(name)) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because the paramName is invalid.", name.c_str());
        verifyRes = ERROR_INVALID_PARAM_NAME;
        return false;
    }

    const std::unordered_set<std::string> tempTrueNames = { "crash", "anr" };
    size_t maxLen =
        tempTrueNames.find(name) == tempTrueNames.end() ? MAX_LENGTH_OF_STR_PARAM : MAX_LENGTH_OF_SPECIAL_STR_PARAM;
    if (param.type == AppEventParamType::STRING && !CheckStrParamLength(param.value.valueUnion.str_, maxLen)) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because the string length exceeds %{public}zu.",
            name.c_str(), maxLen);
        verifyRes = ERROR_INVALID_PARAM_VALUE_LENGTH;
        return false;
    }

    if (param.type == AppEventParamType::STRVECTOR && !CheckStringLengthOfList(param.value.valueUnion.strs_)) {
        HILOG_WARN(
            LOG_CORE, "param=%{public}s is discarded because the string length of list exceeds 8192.", name.c_str());
        verifyRes = ERROR_INVALID_PARAM_VALUE_LENGTH;
        return false;
    }

    if (param.type > AppEventParamType::STRING && !CheckListValueSize(param.type, param.value.valueUnion)) {
        HILOG_WARN(LOG_CORE, "list param=%{public}s is truncated because the list size exceeds 100.", name.c_str());
        verifyRes = ERROR_INVALID_LIST_PARAM_SIZE;
        return true;
    }
    return true;
}

int VerifyCustomAppEventParam(AppEventParam& param, std::unordered_set<std::string>& paramNames)
{
    std::string name = param.name;
    if (paramNames.find(name) != paramNames.end()) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because param is duplicate.", name.c_str());
        return ERROR_DUPLICATE_PARAM;
    }

    if (!CheckParamName(name)) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because the paramName is invalid.", name.c_str());
        return ERROR_INVALID_PARAM_NAME;
    }

    if (param.type == AppEventParamType::STRING &&
        !CheckStrParamLength(param.value.valueUnion.str_, MAX_LENGTH_OF_CUSTOM_PARAM)) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because the string length exceeds %{public}zu.",
            name.c_str(), MAX_LENGTH_OF_CUSTOM_PARAM);
        return ERROR_INVALID_PARAM_VALUE_LENGTH;
    }

    if (param.type == AppEventParamType::STRVECTOR &&
        !CheckStringLengthOfList(param.value.valueUnion.strs_, MAX_LENGTH_OF_CUSTOM_PARAM)) {
        HILOG_WARN(LOG_CORE, "param=%{public}s is discarded because the string length of list exceeds %{public}zu.",
            name.c_str(), MAX_LENGTH_OF_CUSTOM_PARAM);
        return ERROR_INVALID_PARAM_VALUE_LENGTH;
    }

    return HIAPPEVENT_VERIFY_SUCCESSFUL;
}

using VerifyReportConfigFunc = int (*)(ReportConfig& config);

} // namespace

bool IsValidDomain(const std::string& eventDomain)
{
    return IsValidName(eventDomain, MAX_LEN_OF_DOMAIN, false);
}

bool IsValidEventName(const std::string& eventName)
{
    const std::string eventPrefix = "hiappevent.";
    return eventName.find(eventPrefix) == 0
               ? IsValidName(eventName.substr(eventPrefix.length()), MAX_LENGTH_OF_EVENT_NAME - eventPrefix.length())
               : IsValidName(eventName, MAX_LENGTH_OF_EVENT_NAME);
}

bool IsValidWatcherName(const std::string& watcherName)
{
    return IsValidName(watcherName, MAX_LEN_OF_WATCHER, false);
}

bool IsValidEventType(int eventType)
{
    return eventType >= 1 && eventType <= 4; // 1-4: value range of event type
}

int VerifyAppEvent(std::shared_ptr<AppEventPack> event)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HILOG_ERROR(LOG_CORE, "the HiAppEvent function is disabled.");
        return ERROR_HIAPPEVENT_DISABLE;
    }
    if (!IsValidDomain(event->GetDomain())) {
        HILOG_ERROR(LOG_CORE, "eventDomain=%{public}s is invalid.", event->GetDomain().c_str());
        return ERROR_INVALID_EVENT_DOMAIN;
    }
    if (!IsValidEventName(event->GetName())) {
        HILOG_ERROR(LOG_CORE, "eventName=%{public}s is invalid.", event->GetName().c_str());
        return ERROR_INVALID_EVENT_NAME;
    }

    int verifyRes = HIAPPEVENT_VERIFY_SUCCESSFUL;
    std::list<AppEventParam>& baseParams = event->baseParams_;
    std::unordered_set<std::string> paramNames;
    for (auto it = baseParams.begin(); it != baseParams.end();) {
        if (!VerifyAppEventParam(*it, paramNames, verifyRes)) {
            baseParams.erase(it++);
            continue;
        }
        paramNames.emplace(it->name);
        it++;
    }

    if (!CheckParamsNum(baseParams)) {
        HILOG_WARN(LOG_CORE, "params that exceed 32 are discarded because the number of params cannot exceed 32.");
        verifyRes = ERROR_INVALID_PARAM_NUM;
    }

    return verifyRes;
}

int VerifyCustomEventParams(std::shared_ptr<AppEventPack> event)
{
    if (HiAppEventConfig::GetInstance().GetDisable()) {
        HILOG_ERROR(LOG_CORE, "the HiAppEvent function is disabled.");
        return ERROR_HIAPPEVENT_DISABLE;
    }
    if (!IsValidDomain(event->GetDomain())) {
        HILOG_ERROR(LOG_CORE, "eventDomain=%{public}s is invalid.", event->GetDomain().c_str());
        return ERROR_INVALID_EVENT_DOMAIN;
    }
    if (!event->GetName().empty() && !IsValidEventName(event->GetName())) {
        HILOG_ERROR(LOG_CORE, "eventName=%{public}s is invalid.", event->GetName().c_str());
        return ERROR_INVALID_EVENT_NAME;
    }

    std::list<AppEventParam>& baseParams = event->baseParams_;
    if (baseParams.size() > MAX_NUM_OF_CUSTOM_PARAMS) {
        HILOG_WARN(LOG_CORE, "params that exceed 64 are discarded because the number of params cannot exceed 64.");
        return ERROR_INVALID_CUSTOM_PARAM_NUM;
    }
    std::unordered_set<std::string> paramNames;
    for (auto it = baseParams.begin(); it != baseParams.end(); ++it) {
        if (int ret = VerifyCustomAppEventParam(*it, paramNames); ret != HIAPPEVENT_VERIFY_SUCCESSFUL) {
            return ret;
        }
    }
    return HIAPPEVENT_VERIFY_SUCCESSFUL;
}

bool IsValidPropName(const std::string& name, size_t maxSize)
{
    if (name.empty() || name.length() > maxSize) {
        return false;
    }
    // start char is [a-zA-Z_$]
    if (!isalpha(name[0]) && name[0] != '_' && name[0] != '$') {
        return false;
    }
    // other char is [a-zA-Z0-9_$]
    for (size_t i = 1; i < name.length(); ++i) {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '$') {
            return false;
        }
    }
    return true;
}

bool IsValidPropValue(const std::string& val, size_t maxSize)
{
    return !val.empty() && val.length() <= maxSize;
}

bool IsValidProcessorName(const std::string& name)
{
    return IsValidPropName(name, MAX_LENGTH_OF_PROCESSOR_NAME);
}

bool IsValidRouteInfo(const std::string& name)
{
    return name.length() <= MAX_LENGTH_OF_STR_PARAM;
}

bool IsValidAppId(const std::string& name)
{
    return name.length() <= MAX_LENGTH_OF_STR_PARAM;
}

bool IsValidPeriodReport(int timeout)
{
    return timeout >= 0;
}

bool IsValidBatchReport(int count)
{
    return count >= 0 && count <= MAX_LEN_OF_BATCH_REPORT;
}

bool IsValidUserIdName(const std::string& name)
{
    return IsValidPropName(name, MAX_LENGTH_OF_USER_INFO_NAME);
}

bool IsValidUserIdValue(const std::string& value)
{
    return IsValidPropValue(value, MAX_LENGTH_OF_USER_ID_VALUE);
}

bool IsValidUserPropName(const std::string& name)
{
    return IsValidPropName(name, MAX_LENGTH_OF_USER_INFO_NAME);
}

bool IsValidUserPropValue(const std::string& value)
{
    return IsValidPropValue(value, MAX_LENGTH_OF_USER_PROPERTY_VALUE);
}

bool IsValidEventConfig(const EventConfig& eventCfg)
{
    if (eventCfg.domain.empty() && eventCfg.name.empty()) {
        return false;
    }
    if (!eventCfg.domain.empty() && !IsValidDomain(eventCfg.domain)) {
        return false;
    }
    if (!eventCfg.name.empty() && !IsValidEventName(eventCfg.name)) {
        return false;
    }
    return true;
}

bool IsValidConfigId(int configId)
{
    return configId >= 0;
}

bool IsValidCustomConfigsNum(size_t num)
{
    return num <= MAX_NUM_OF_CUSTOM_CONFIGS;
}

bool IsValidCustomConfig(const std::string& name, const std::string& value)
{
    if (!IsValidName(name, MAX_LENGTH_OF_CUSTOM_CONFIG_NAME) || value.length() > MAX_LENGTH_OF_CUSTOM_CONFIG_VALUE) {
        return false;
    }
    return true;
}
} // namespace HiviewDFX
} // namespace OHOS
