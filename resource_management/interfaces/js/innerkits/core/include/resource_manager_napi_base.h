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

#ifndef RESOURCE_MANAGER_NAPI_BASE_H
#define RESOURCE_MANAGER_NAPI_BASE_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"
// #include "hilog/log_cpp.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerNapiBase {
public:
    virtual ~ResourceManagerNapiBase() {};

    /**
     * Get the resource according to function name.
     *
     * @param env The environment that the API is invoked under.
     * @param info The additional information about the context in which the callback was invoked.
     * @param functionName The function name
     * @return The resource if resource exist, else errcode and errmessage.
     */
    virtual napi_value GetResource(napi_env env, napi_callback_info info, const std::string &functionName) = 0;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif