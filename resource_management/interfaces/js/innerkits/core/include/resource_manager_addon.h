/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef RESOURCE_MANAGER_ADDON_H
#define RESOURCE_MANAGER_ADDON_H

#include <cstddef>
#include <memory>
#include <string>
#if defined(__ARKUI_CROSS__)
#include "foundation/appframework/ability/ability_runtime/cross_platform/interfaces/kits/native/appkit/context.h"
#else
#include "foundation/ability/ability_runtime/interfaces/kits/native/appkit/ability_runtime/context/context.h"
#endif
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "resource_manager.h"

namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerAddon {
public:
#if defined(__ARKUI_CROSS__)
    static napi_value Create(
        napi_env env, const std::string& bundleName,
            const std::shared_ptr<ResourceManager>& resMgr, std::shared_ptr<AbilityRuntime::Platform::Context> context);
#else
    static napi_value Create(
        napi_env env, const std::string& bundleName,
            const std::shared_ptr<ResourceManager>& resMgr, std::shared_ptr<AbilityRuntime::Context> context);
#endif
    static bool Init(napi_env env);

    static void Destructor(napi_env env, void *nativeObject, void *finalize_hint);

#if defined(__ARKUI_CROSS__)
    ResourceManagerAddon(const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
        const std::shared_ptr<AbilityRuntime::Platform::Context>& context);
#else
    ResourceManagerAddon(const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
        const std::shared_ptr<AbilityRuntime::Context>& context);
#endif
    ~ResourceManagerAddon();

    inline std::shared_ptr<ResourceManager> GetResMgr()
    {
        return resMgr_;
    }
#if defined(__ARKUI_CROSS__)
    inline std::shared_ptr<AbilityRuntime::Platform::Context> GetContext()
#else
    inline std::shared_ptr<AbilityRuntime::Context> GetContext()
#endif
    {
        return context_;
    }

    std::string GetLocale(std::unique_ptr<ResConfig> &cfg);

    static int GetResId(napi_env env, size_t argc, napi_value *argv);

    static int32_t GetResourceObject(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value);
private:

    static napi_value ProcessOnlyIdParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetString(napi_env env, napi_callback_info info);

    static napi_value GetStringArray(napi_env env, napi_callback_info info);

    static napi_value GetMedia(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64(napi_env env, napi_callback_info info);

    static napi_value ProcessNoParam(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetConfiguration(napi_env env, napi_callback_info info);

    static napi_value GetDeviceCapability(napi_env env, napi_callback_info info);

    static napi_value GetPluralString(napi_env env, napi_callback_info info);

    static napi_value New(napi_env env, napi_callback_info info);

    static napi_value GetRawFile(napi_env env, napi_callback_info info);

    static std::string GetResNameOrPath(napi_env env, size_t argc, napi_value *argv);

    static napi_value GetRawFileDescriptor(napi_env env, napi_callback_info info);

    static napi_value CloseRawFileDescriptor(napi_env env, napi_callback_info info);

    static napi_value Release(napi_env env, napi_callback_info info);

    static std::string GetResName(napi_env env, size_t argc, napi_value *argv);

    static napi_value GetMediaByName(napi_env env, napi_callback_info info);

    static napi_value GetMediaBase64ByName(napi_env env, napi_callback_info info);

    static napi_value GetStringByName(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayByName(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringByName(napi_env env, napi_callback_info info);

    static napi_value ProcessIdNameParam(napi_env env, napi_callback_info info, const std::string& name,
        napi_async_execute_callback execute);

    static napi_value GetNumber(napi_env env, napi_callback_info info);

    static napi_value GetNumberByName(napi_env env, napi_callback_info info);

    static napi_value GetBoolean(napi_env env, napi_callback_info info);

    static napi_value GetBooleanByName(napi_env env, napi_callback_info info);

    static napi_value GetStringSync(napi_env env, napi_callback_info info);

    static napi_value GetStringByNameSync(napi_env env, napi_callback_info info);

    static napi_valuetype GetType(napi_env env, napi_value value);

    static bool GetResourceObjectName(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value, int32_t type);

    static bool GetResourceObjectId(napi_env env, std::shared_ptr<ResourceManager::Resource> &resourcePtr,
        napi_value &value);

    static napi_value ProcessIdParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value ProcessNameParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value ProcessResourceParamV9(napi_env env, napi_callback_info info, const std::string &name,
        napi_async_execute_callback execute);

    static napi_value GetStringValue(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayValue(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringValue(napi_env env, napi_callback_info info);

    static napi_value GetMediaContent(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentBase64(napi_env env, napi_callback_info info);

    static napi_value GetRawFileContent(napi_env env, napi_callback_info info);

    static napi_value GetRawFd(napi_env env, napi_callback_info info);

    static napi_value CloseRawFd(napi_env env, napi_callback_info info);
#if !defined(__ARKUI_CROSS__)
    static napi_value GetDrawableDescriptor(napi_env env, napi_callback_info info);

    static napi_value GetDrawableDescriptorByName(napi_env env, napi_callback_info info);
#endif

    static bool InitParamsFromParamArray(napi_env env, napi_value value,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams);

    static bool InitNapiParameters(napi_env env, napi_callback_info value,
        std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams);

    std::string bundleName_;
    std::shared_ptr<ResourceManager> resMgr_;
#if defined(__ARKUI_CROSS__)
    std::shared_ptr<AbilityRuntime::Platform::Context> context_;
#else
    std::shared_ptr<AbilityRuntime::Context> context_;
#endif
};

struct ResMgrAsyncContext {
    napi_async_work work_;

    std::string bundleName_;
    int32_t resId_;
    int32_t param_;

    std::string path_;
    std::string resName_;
    int iValue_;
    float fValue_;
    bool bValue_;

    typedef napi_value (*CreateNapiValue)(napi_env env, ResMgrAsyncContext &context);
    CreateNapiValue createValueFunc_;
    std::string value_;
    std::vector<std::string> arrayValue_;
    std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> jsParams_;

    std::unique_ptr<uint8_t[]> mediaData;
    size_t len_;

    napi_deferred deferred_;
    napi_ref callbackRef_;

    std::string errMsg_;
    int success_;
    int errCode_;
    uint32_t density_;

    std::shared_ptr<ResourceManagerAddon> addon_;
    std::shared_ptr<ResourceManager> resMgr_;
    std::shared_ptr<ResourceManager::Resource> resource_;

    ResMgrAsyncContext() : work_(nullptr), resId_(0), param_(0), iValue_(0), fValue_(0.0f), bValue_(false),
        createValueFunc_(nullptr), len_(0), deferred_(nullptr), callbackRef_(nullptr), success_(true), errCode_(0),
        density_(0) {}

    void SetErrorMsg(const std::string &msg, bool withResId = false, int32_t errCode = 0);

    static void Complete(napi_env env, napi_status status, void* data);

    static bool GetHapResourceManager(const ResMgrAsyncContext* asyncContext, std::shared_ptr<ResourceManager> &resMgr,
        int32_t &resId);

    static napi_value getResult(napi_env env, std::unique_ptr<ResMgrAsyncContext> &asyncContext,
        const std::string &name, napi_async_execute_callback &execute);
    
    static void NapiThrow(napi_env env, int32_t errCode);

    static int32_t ProcessIdResourceParam(napi_env env, napi_callback_info info,
        std::unique_ptr<ResMgrAsyncContext> &asyncContext);
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif