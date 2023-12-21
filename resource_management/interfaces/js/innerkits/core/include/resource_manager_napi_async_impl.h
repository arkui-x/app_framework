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

#ifndef RESOURCE_MANAGER_NAPI_ASYNC_IMPL_H
#define RESOURCE_MANAGER_NAPI_ASYNC_IMPL_H

#include "resource_manager_napi_base.h"
#include "resource_manager_data_context.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerNapiAsyncImpl : public ResourceManagerNapiBase {
public:
    ResourceManagerNapiAsyncImpl();

    ~ResourceManagerNapiAsyncImpl();
    /**
     * Get the resource according to function name.
     *
     * @param env The environment that the API is invoked under.
     * @param info The additional information about the context in which the callback was invoked.
     * @param functionName The function name
     * @return The resource if resource exist, else errcode and errmessage.
     */
    virtual napi_value GetResource(napi_env env, napi_callback_info info, const std::string &functionName) override;

    /**
     * Complete asynchronous function, pass the data to application.
     *
     * @param env The environment that the API is invoked under.
     * @param status The result status of napi function.
     * @param data The callback data passed into function
     */
    static void Complete(napi_env env, napi_status status, void* data);

private:

    static std::unordered_map<std::string, std::function<napi_value(napi_env&, napi_callback_info&)>> asyncFuncMatch;

    static napi_value GetString(napi_env env, napi_callback_info info);

    static napi_value GetStringArray(napi_env env, napi_callback_info info);

    static napi_value GetMedia(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64(napi_env env, napi_callback_info info);

    static napi_value GetConfiguration(napi_env env, napi_callback_info info);

    static napi_value GetDeviceCapability(napi_env env, napi_callback_info info);

    static napi_value GetPluralString(napi_env env, napi_callback_info info);

    static napi_value GetRawFile(napi_env env, napi_callback_info info);

    static napi_value GetRawFileDescriptor(napi_env env, napi_callback_info info);

    static napi_value CloseRawFileDescriptor(napi_env env, napi_callback_info info);

    static napi_value GetStringValue(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayValue(napi_env env, napi_callback_info info);

    static napi_value GetMediaContent(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentBase64(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringValue(napi_env env, napi_callback_info info);

    static napi_value GetRawFileContent(napi_env env, napi_callback_info info);

    static napi_value GetRawFd(napi_env env, napi_callback_info info);

    static napi_value CloseRawFd(napi_env env, napi_callback_info info);

    static napi_value GetStringByName(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayByName(napi_env env, napi_callback_info info);

    static napi_value GetMediaByName(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringByName(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64ByName(napi_env env, napi_callback_info info);

    static napi_value GetRawFileList(napi_env env, napi_callback_info info);

    static napi_value GetColor(napi_env env, napi_callback_info info);

    static napi_value GetColorByName(napi_env env, napi_callback_info info);

    static napi_value ProcessIdParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value ProcessResourceParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);
    
    static napi_value ProcessNameParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value ProcessOnlyIdParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value ProcessIdNameParam(napi_env env, napi_callback_info info, const std::string& name,
        napi_async_execute_callback execute);
    
    static napi_value ProcessNoParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetResult(napi_env env, std::unique_ptr<ResMgrDataContext> &dataContext,
        const std::string &name, napi_async_execute_callback &execute);
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif