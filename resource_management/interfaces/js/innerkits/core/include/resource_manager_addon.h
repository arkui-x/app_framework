/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#if defined(__ARKUI_CROSS__)
#include "foundation/appframework/ability/ability_runtime/cross_platform/interfaces/kits/native/appkit/context.h"
#else
#include "foundation/ability/ability_runtime/interfaces/kits/native/appkit/ability_runtime/context/context.h"
#endif
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "resource_manager.h"
#include "resource_manager_napi_context.h"
namespace OHOS {
namespace Global {
namespace Resource {
class ResourceManagerAddon {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)
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
        const std::shared_ptr<AbilityRuntime::Platform::Context>& context, bool isSystem = false);
#else
    ResourceManagerAddon(const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
        const std::shared_ptr<AbilityRuntime::Context>& context, bool isSystem = false);
#endif

    ResourceManagerAddon(const std::shared_ptr<ResourceManager>& resMgr, bool isSystem = false);

    static napi_value GetSystemResMgr(napi_env env);

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

    inline bool IsSystem()
    {
        return isSystem_;
    }

    napi_value CreateOverrideAddon(napi_env env, const std::shared_ptr<ResourceManager>& resMgr);

    bool isOverrideAddon()
    {
        return isOverrideAddon_;
    }

private:
    static napi_value AddonGetResource(napi_env env, napi_callback_info info, const std::string& name,
        FunctionType type);

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

    static napi_value GetNumber(napi_env env, napi_callback_info info);

    static napi_value GetNumberByName(napi_env env, napi_callback_info info);

    static napi_value GetBoolean(napi_env env, napi_callback_info info);

    static napi_value GetBooleanByName(napi_env env, napi_callback_info info);

    static napi_value GetStringSync(napi_env env, napi_callback_info info);

    static napi_value GetStringByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetStringValue(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayValue(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringValue(napi_env env, napi_callback_info info);

    static napi_value GetMediaContent(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentBase64(napi_env env, napi_callback_info info);

    static napi_value GetRawFileContent(napi_env env, napi_callback_info info);

    static napi_value GetRawFd(napi_env env, napi_callback_info info);

    static napi_value CloseRawFd(napi_env env, napi_callback_info info);

    static napi_value GetDrawableDescriptor(napi_env env, napi_callback_info info);

    static napi_value GetDrawableDescriptorByName(napi_env env, napi_callback_info info);

    static napi_value GetRawFileList(napi_env env, napi_callback_info info);

    static napi_value GetColorByNameSync(napi_env env, napi_callback_info info);

    static napi_value GetColorSync(napi_env env, napi_callback_info info);

    static napi_value GetColor(napi_env env, napi_callback_info info);

    static napi_value GetColorByName(napi_env env, napi_callback_info info);

    static napi_value WrapResourceManager(napi_env env, std::shared_ptr<ResourceManagerAddon> &addon);

    static napi_value AddResource(napi_env env, napi_callback_info info);

    static napi_value RemoveResource(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentBase64Sync(napi_env env, napi_callback_info info);

    static napi_value GetMediaContentSync(napi_env env, napi_callback_info info);

    static napi_value GetPluralStringValueSync(napi_env env, napi_callback_info info);

    static napi_value GetStringArrayValueSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFileContentSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFdSync(napi_env env, napi_callback_info info);

    static napi_value CloseRawFdSync(napi_env env, napi_callback_info info);

    static napi_value GetRawFileListSync(napi_env env, napi_callback_info info);

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

    std::string bundleName_;
    std::shared_ptr<ResourceManager> resMgr_;
#if defined(__ARKUI_CROSS__)
    std::shared_ptr<AbilityRuntime::Platform::Context> context_;
#else
    std::shared_ptr<AbilityRuntime::Context> context_;
#endif
    bool isSystem_;
    std::shared_ptr<ResourceManagerNapiContext> napiContext_;
    static napi_property_descriptor properties[];
    bool isOverrideAddon_ = false;
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif