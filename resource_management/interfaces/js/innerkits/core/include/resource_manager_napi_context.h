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

#ifndef RESOURCE_MANAGER_NAPI_CONTEXT_H
#define RESOURCE_MANAGER_NAPI_CONTEXT_H

#include "resource_manager_napi_base.h"
#include "resource_manager.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerNapiContext {
public:
    ResourceManagerNapiContext();

    ~ResourceManagerNapiContext();
    /**
     * Get the resource according to function name and function type.
     *
     * @param env The environment that the API is invoked under.
     * @param info The additional information about the context in which the callback was invoked.
     * @param functionName The function name
     * @param functionType The function type, async or sync
     * @return The resource if resource exist, else errcode and errmessage.
     */
    napi_value ContextGetResource(napi_env env, napi_callback_info info, const std::string &functionName,
        FunctionType functionType);
private:
    std::shared_ptr<ResourceManagerNapiBase> napiBase_;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif