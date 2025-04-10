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

#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_JS_NAPI_INCLUDE_NAPI_UTIL_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_JS_NAPI_INCLUDE_NAPI_UTIL_H

#include <cstdint>
#include <string>
#include <unordered_set>

#include "json/json.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
namespace NapiUtil {
bool IsNull(const napi_env env, const napi_value value);
bool IsNumber(const napi_env env, const napi_value value);
bool IsString(const napi_env env, const napi_value value);
bool IsObject(const napi_env env, const napi_value value);
bool IsFunction(const napi_env env, const napi_value value);
bool IsArray(const napi_env env, const napi_value value);
bool IsArrayType(const napi_env env, const napi_value value, napi_valuetype type);
bool HasProperty(const napi_env env, const napi_value object, const std::string& name);

bool GetBoolean(const napi_env env, const napi_value value);
void GetBooleans(const napi_env env, const napi_value arr, std::vector<bool>& bools);
int32_t GetInt32(const napi_env env, const napi_value value);
void GetInt32s(const napi_env env, const napi_value arr, std::vector<int32_t>& ints);
double GetDouble(const napi_env env, const napi_value value);
void GetDoubles(const napi_env env, const napi_value arr, std::vector<double>& doubles);
std::string GetString(const napi_env env, const napi_value value);
void GetStrings(const napi_env env, const napi_value arr, std::vector<std::string>& strs);
void GetStringsToSet(const napi_env env, const napi_value arr, std::unordered_set<std::string>& strs);
napi_valuetype GetType(const napi_env env, const napi_value value);
napi_valuetype GetArrayType(const napi_env env, const napi_value value);
uint32_t GetArrayLength(const napi_env env, const napi_value arr);
napi_value GetElement(const napi_env env, const napi_value arr, uint32_t index);
napi_value GetProperty(const napi_env env, const napi_value object, const std::string& name);
void GetPropertyNames(const napi_env env, const napi_value object, std::vector<std::string>& names);
napi_value GetReferenceValue(const napi_env env, const napi_ref funcRef);
size_t GetCbInfo(const napi_env env, napi_callback_info info, napi_value argv[], size_t argc = 4); // 4: default size

napi_ref CreateReference(const napi_env env, const napi_value func);
napi_value CreateNull(const napi_env env);
napi_value CreateUndefined(const napi_env env);
napi_value CreateBoolean(const napi_env env, bool bValue);
napi_value CreateInt32(const napi_env env, int32_t num);
napi_value CreateInt64(const napi_env env, int64_t num);
napi_value CreateString(const napi_env env, const std::string& str);
napi_value CreateStrings(const napi_env env, const std::vector<std::string>& strs);
napi_value CreateObject(const napi_env env);
napi_value CreateObject(const napi_env env, const std::string& key, const napi_value value);
napi_value CreateArray(const napi_env env);
napi_value CreateDouble(const napi_env env, double dValue);

void SetElement(const napi_env env, const napi_value obj, uint32_t index, const napi_value value);
void SetNamedProperty(const napi_env env, const napi_value obj, const std::string& key, const napi_value value);
std::string ConvertToString(const napi_env env, const napi_value value);

void ThrowError(napi_env env, int code, const std::string& msg, bool isThrow = true);
napi_value CreateError(napi_env env, int code, const std::string& msg);
std::string CreateErrMsg(const std::string& name);
std::string CreateErrMsg(const std::string& name, const std::string& type);
std::string CreateErrMsg(const std::string& name, const napi_valuetype type);

napi_value CreateBaseValueByJson(const napi_env env, const Json::Value& jsonValue);
napi_value CreateValueByJson(napi_env env, const Json::Value& jsonValue);
napi_value CreateValueByJsonStr(napi_env env, const std::string& jsonStr);
napi_value CreateEventInfo(napi_env env, std::shared_ptr<AppEventPack> event);
napi_value CreateEventInfoArray(napi_env env, const std::vector<std::shared_ptr<AppEventPack>>& events);
napi_value CreateEventGroups(napi_env env, const std::vector<std::shared_ptr<AppEventPack>>& events);
} // namespace NapiUtil
} // namespace HiviewDFX
} // namespace OHOS
#endif // HIAPPEVENT_FRAMEWORKS_JS_NAPI_INCLUDE_NAPI_UTIL_H
