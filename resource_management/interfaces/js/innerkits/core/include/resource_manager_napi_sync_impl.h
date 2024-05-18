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

#ifndef RESOURCE_MANAGER_NAPI_SYNC_IMPL_H
#define RESOURCE_MANAGER_NAPI_SYNC_IMPL_H

#include "resource_manager_napi_base.h"
#include "resource_manager_data_context.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerNapiSyncImpl : public ResourceManagerNapiBase {
public:
    ResourceManagerNapiSyncImpl();

    ~ResourceManagerNapiSyncImpl();
    /**
     * Get the resource according to the function name.
     *
     * @param env The environment that the API is invoked under.
     * @param info The additional information about the context in which the callback was invoked.
     * @param functionName The function name
     * @return The resource if resource exist, else errcode and errmessage.
     */
    virtual napi_value GetResource(napi_env env, napi_callback_info info, const std::string &functionName) override;

private:

    static std::unordered_map<std::string, std::function<napi_value(napi_env&, napi_callback_info&)>> syncFuncMatch;

    static napi_value GetStringSync(napi_env env, napi_callback_info info);

    static napi_value GetStringByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetBoolean(napi_env env, napi_callback_info info);

    static napi_value GetBooleanByName(napi_env env, napi_callback_info info);

    static napi_value GetNumber(napi_env env, napi_callback_info info);

    static napi_value GetNumberByName(napi_env env, napi_callback_info info);

    static napi_value GetDrawableDescriptor(napi_env env, napi_callback_info info);

    static napi_value GetDrawableDescriptorByName(napi_env env, napi_callback_info info);

    static napi_value GetColorSync(napi_env env, napi_callback_info info);

    static napi_value GetColorByNameSync(napi_env env, napi_callback_info info);

    static napi_value AddResource(napi_env env, napi_callback_info info);

    static napi_value RemoveResource(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentBase64Sync(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentSync(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringValueSync(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayValueSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFileListSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFileContentSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFdSync(napi_env env, napi_callback_info info);

    static napi_value CloseRawFdSync(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64ByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetMediaByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetConfigurationSync(napi_env env, napi_callback_info info);

    static napi_value GetDeviceCapabilitySync(napi_env env, napi_callback_info info);

    static napi_value GetLocales(napi_env env, napi_callback_info info);

    static napi_value GetSymbol(napi_env env, napi_callback_info info);

    static napi_value GetSymbolByName(napi_env env, napi_callback_info info);

    static napi_value IsRawDir(napi_env env, napi_callback_info info);

    static napi_value GetOverrideResourceManager(napi_env env, napi_callback_info info);

    static napi_value GetOverrideConfiguration(napi_env env, napi_callback_info info);

    static napi_value UpdateOverrideConfiguration(napi_env env, napi_callback_info info);

    static int32_t InitIdResourceAddon(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t InitNameAddon(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t InitPathAddon(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static bool InitNapiParameters(napi_env env, napi_callback_info info,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams);

    static bool InitParamsFromParamArray(napi_env env, napi_value value,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams);

    static int32_t ProcessStrResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessStrResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessNumResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessNumResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessBoolResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessColorResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessColorResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcesstMediaContentBase64Resource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessMediaContentResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessPluralStringValueResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessStringArrayValueResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessPluralStrResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessMediaBase64ResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessMediaResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessStringArrayResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessSymbolResource(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static int32_t ProcessSymbolResourceByName(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);

    static RState getAddonAndConfig(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrDataContext> &dataContext);
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif