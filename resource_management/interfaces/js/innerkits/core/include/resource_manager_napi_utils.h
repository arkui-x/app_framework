/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef RESOURCE_MANAGER_NAPI_UTILS_H
#define RESOURCE_MANAGER_NAPI_UTILS_H

#include "resource_manager_data_context.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerNapiUtils {
public:
    static bool IsNapiString(napi_env env, napi_callback_info info);

    static bool IsNapiNumber(napi_env env, napi_callback_info info);

    static bool IsNapiObject(napi_env env, napi_callback_info info);

    static napi_value CreateJsUint8Array(napi_env env, ResMgrDataContext &context);

    static napi_value CreateJsRawFd(napi_env env, ResMgrDataContext &context);

    static napi_value CloseJsRawFd(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsArray(napi_env env, ResMgrDataContext &context);

    static napi_value CreateJsString(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsDeviceCap(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsConfig(napi_env env, ResMgrDataContext& context);

    static napi_value CreateOverrideJsConfig(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsNumber(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsBool(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsColor(napi_env env, ResMgrDataContext& context);

    static napi_value CreateJsSymbol(napi_env env, ResMgrDataContext& context);

    static RState GetDataType(napi_env env, napi_value value, uint32_t& density);

    static napi_valuetype GetType(napi_env env, napi_value value);

    static std::string GetResNameOrPath(napi_env env, size_t argc, napi_value *argv);

    static int GetResId(napi_env env, size_t argc, napi_value *argv);

    static void NapiThrow(napi_env env, int32_t errCode);

    static int32_t GetResourceObject(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value);

    static bool GetHapResourceManager(const ResMgrDataContext* dataContext,
        std::shared_ptr<ResourceManager> &resMgr, int32_t &resId);

    static RState GetIncludeSystem(napi_env env, napi_value value, bool &includeSystem);

    static RState GetConfigObject(napi_env env, napi_value object, std::unique_ptr<ResMgrDataContext> &dataContext);

private:

    static const std::unordered_map<int32_t, std::string> ErrorCodeToMsg;
    
    static std::string FindErrMsg(int32_t errCode);

    static bool GetResourceObjectId(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value);

    static bool GetResourceObjectName(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value, int32_t type);

    static std::string GetLocale(std::unique_ptr<ResConfig> &cfg);

    static napi_value CreateConfig(napi_env env, ResMgrDataContext& context, std::unique_ptr<ResConfig> &cfg);

    static bool SetIntProperty(napi_env env, ResMgrDataContext& context,
        napi_value &object, const std::string &property, const int &value);

    static bool GetEnumParamOfConfig(napi_env env, std::shared_ptr<ResConfig> configPtr, napi_value &object);

    static bool GetLocaleOfConfig(napi_env env, std::shared_ptr<ResConfig> configPtr, napi_value &object);
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif