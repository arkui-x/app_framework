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

#include "resource_manager_addon.h"
#include "resource_manager_napi_utils.h"
#include "hilog/log_cpp.h"

namespace OHOS {
namespace Global {
namespace Resource {
static std::shared_ptr<ResourceManager> sysResMgr = nullptr;
static std::mutex sysMgrMutex;

#if defined(__ARKUI_CROSS__)
napi_value ResourceManagerAddon::Create(
    napi_env env, const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    std::shared_ptr<AbilityRuntime::Platform::Context> context)
#else
napi_value ResourceManagerAddon::Create(
    napi_env env, const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    std::shared_ptr<AbilityRuntime::Context> context)
#endif
{
    std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(bundleName, resMgr, context);
    return WrapResourceManager(env, addon);
}

napi_value ResourceManagerAddon::CreateOverrideAddon(napi_env env, const std::shared_ptr<ResourceManager>& resMgr)
{
    std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(bundleName_, resMgr, context_);
    addon->isOverrideAddon_ = true;
    return WrapResourceManager(env, addon);
}

napi_property_descriptor ResourceManagerAddon::properties[] = {
    DECLARE_NAPI_FUNCTION("getString", GetString),
    DECLARE_NAPI_FUNCTION("getStringByName", GetStringByName),
    DECLARE_NAPI_FUNCTION("getStringArray", GetStringArray),
    DECLARE_NAPI_FUNCTION("getStringArrayByName", GetStringArrayByName),
    DECLARE_NAPI_FUNCTION("getMedia", GetMedia),
    DECLARE_NAPI_FUNCTION("getMediaByName", GetMediaByName),
    DECLARE_NAPI_FUNCTION("getMediaBase64", GetMediaBase64),
    DECLARE_NAPI_FUNCTION("getMediaBase64ByName", GetMediaBase64ByName),
    DECLARE_NAPI_FUNCTION("getConfiguration", GetConfiguration),
    DECLARE_NAPI_FUNCTION("getDeviceCapability", GetDeviceCapability),
    DECLARE_NAPI_FUNCTION("getPluralString", GetPluralString),
    DECLARE_NAPI_FUNCTION("getPluralStringByName", GetPluralStringByName),
    DECLARE_NAPI_FUNCTION("getRawFile", GetRawFile),
    DECLARE_NAPI_FUNCTION("getRawFileDescriptor", GetRawFileDescriptor),
    DECLARE_NAPI_FUNCTION("closeRawFileDescriptor", CloseRawFileDescriptor),
    DECLARE_NAPI_FUNCTION("getStringSync", GetStringSync),
    DECLARE_NAPI_FUNCTION("getStringByNameSync", GetStringByNameSync),
    DECLARE_NAPI_FUNCTION("getBoolean", GetBoolean),
    DECLARE_NAPI_FUNCTION("getNumber", GetNumber),
    DECLARE_NAPI_FUNCTION("getBooleanByName", GetBooleanByName),
    DECLARE_NAPI_FUNCTION("getNumberByName", GetNumberByName),
    DECLARE_NAPI_FUNCTION("release", Release),
    DECLARE_NAPI_FUNCTION("getStringValue", GetStringValue),
    DECLARE_NAPI_FUNCTION("getStringArrayValue", GetStringArrayValue),
    DECLARE_NAPI_FUNCTION("getPluralStringValue", GetPluralStringValue),
    DECLARE_NAPI_FUNCTION("getMediaContent", GetMediaContent),
    DECLARE_NAPI_FUNCTION("getMediaContentBase64", GetMediaContentBase64),
    DECLARE_NAPI_FUNCTION("getRawFileContent", GetRawFileContent),
    DECLARE_NAPI_FUNCTION("getRawFd", GetRawFd),
    DECLARE_NAPI_FUNCTION("closeRawFd", CloseRawFd),
    DECLARE_NAPI_FUNCTION("getDrawableDescriptor", GetDrawableDescriptor),
    DECLARE_NAPI_FUNCTION("getDrawableDescriptorByName", GetDrawableDescriptorByName),
    DECLARE_NAPI_FUNCTION("getRawFileList", GetRawFileList),
    DECLARE_NAPI_FUNCTION("getColor", GetColor),
    DECLARE_NAPI_FUNCTION("getColorByName", GetColorByName),
    DECLARE_NAPI_FUNCTION("getColorSync", GetColorSync),
    DECLARE_NAPI_FUNCTION("getColorByNameSync", GetColorByNameSync),
    DECLARE_NAPI_FUNCTION("addResource", AddResource),
    DECLARE_NAPI_FUNCTION("removeResource", RemoveResource),
    DECLARE_NAPI_FUNCTION("getMediaContentBase64Sync", GetMediaContentBase64Sync),
    DECLARE_NAPI_FUNCTION("getMediaContentSync", GetMediaContentSync),
    DECLARE_NAPI_FUNCTION("getPluralStringValueSync", GetPluralStringValueSync),
    DECLARE_NAPI_FUNCTION("getStringArrayValueSync", GetStringArrayValueSync),
    DECLARE_NAPI_FUNCTION("getRawFileContentSync", GetRawFileContentSync),
    DECLARE_NAPI_FUNCTION("getRawFdSync", GetRawFdSync),
    DECLARE_NAPI_FUNCTION("closeRawFdSync", CloseRawFdSync),
    DECLARE_NAPI_FUNCTION("getRawFileListSync", GetRawFileListSync),
    DECLARE_NAPI_FUNCTION("getPluralStringByNameSync", GetPluralStringByNameSync),
    DECLARE_NAPI_FUNCTION("getMediaBase64ByNameSync", GetMediaBase64ByNameSync),
    DECLARE_NAPI_FUNCTION("getMediaByNameSync", GetMediaByNameSync),
    DECLARE_NAPI_FUNCTION("getStringArrayByNameSync", GetStringArrayByNameSync),
    DECLARE_NAPI_FUNCTION("getConfigurationSync", GetConfigurationSync),
    DECLARE_NAPI_FUNCTION("getDeviceCapabilitySync", GetDeviceCapabilitySync),
    DECLARE_NAPI_FUNCTION("getLocales", GetLocales),
    DECLARE_NAPI_FUNCTION("getSymbol", GetSymbol),
    DECLARE_NAPI_FUNCTION("getSymbolByName", GetSymbolByName),
    DECLARE_NAPI_FUNCTION("isRawDir", IsRawDir),
    DECLARE_NAPI_FUNCTION("getOverrideResourceManager", GetOverrideResourceManager),
    DECLARE_NAPI_FUNCTION("getOverrideConfiguration", GetOverrideConfiguration),
    DECLARE_NAPI_FUNCTION("updateOverrideConfiguration", UpdateOverrideConfiguration)
};

napi_value ResourceManagerAddon::WrapResourceManager(napi_env env, std::shared_ptr<ResourceManagerAddon> &addon)
{
    napi_value constructor;
    napi_status status = napi_define_class(env, "ResourceManager", NAPI_AUTO_LENGTH, New, nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor), properties, &constructor);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to define class at Init, status = %{public}d", status);
        return nullptr;
    }

    napi_value result = nullptr;
    status = napi_new_instance(env, constructor, 0, nullptr, &result);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to new instance in Create, status = %{public}d", status);
        return nullptr;
    }

    auto addonPtr = std::make_unique<std::shared_ptr<ResourceManagerAddon>>(addon);
    status = napi_wrap(env, result, reinterpret_cast<void *>(addonPtr.get()), ResourceManagerAddon::Destructor,
        nullptr, nullptr);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to wrape in Create");
        return nullptr;
    }
    addonPtr.release();
    return result;
}

napi_value ResourceManagerAddon::GetSystemResMgr(napi_env env)
{
    if (sysResMgr == nullptr) {
        std::lock_guard<std::mutex> lock(sysMgrMutex);
        if (sysResMgr == nullptr) {
            std::shared_ptr<Global::Resource::ResourceManager>
                systemResManager(Global::Resource::GetSystemResourceManager());
            if (systemResManager == nullptr) {
                ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_SYSTEM_RES_MANAGER_GET_FAILED);
                return nullptr;
            }
            sysResMgr = systemResManager;
        }
    }
    std::shared_ptr<ResourceManagerAddon> addon = std::make_shared<ResourceManagerAddon>(sysResMgr, true);
    return WrapResourceManager(env, addon);
}
#if defined(__ARKUI_CROSS__)
ResourceManagerAddon::ResourceManagerAddon(
    const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    const std::shared_ptr<AbilityRuntime::Platform::Context>& context, bool isSystem)
    : bundleName_(bundleName), resMgr_(resMgr), context_(context), isSystem_(isSystem)
#else
ResourceManagerAddon::ResourceManagerAddon(
    const std::string& bundleName, const std::shared_ptr<ResourceManager>& resMgr,
    const std::shared_ptr<AbilityRuntime::Context>& context, bool isSystem)
    : bundleName_(bundleName), resMgr_(resMgr), context_(context), isSystem_(isSystem)
#endif
{
    napiContext_ = std::make_shared<ResourceManagerNapiContext>();
}

ResourceManagerAddon::ResourceManagerAddon(const std::shared_ptr<ResourceManager>& resMgr, bool isSystem)
    : resMgr_(resMgr), isSystem_(isSystem)
{
    napiContext_ = std::make_shared<ResourceManagerNapiContext>();
}

ResourceManagerAddon::~ResourceManagerAddon()
{
    HiLog::Info(LABEL, "~ResourceManagerAddon %{public}s", bundleName_.c_str());
}

void ResourceManagerAddon::Destructor(napi_env env, void *nativeObject, void *hint)
{
    std::unique_ptr<std::shared_ptr<ResourceManagerAddon>> addonPtr;
    addonPtr.reset(static_cast<std::shared_ptr<ResourceManagerAddon>*>(nativeObject));
}

napi_value ResourceManagerAddon::New(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 1);

    napi_value target = nullptr;
    napi_get_new_target(env, info, &target);
    if (target != nullptr) {
        return thisVar;
    }

    return nullptr;
}

napi_value ResourceManagerAddon::Release(napi_env env, napi_callback_info info)
{
    napi_value undefined;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        return nullptr;
    }
    return undefined;
}

napi_value ResourceManagerAddon::AddonGetResource(napi_env env, napi_callback_info info, const std::string& name,
    FunctionType type)
{
    auto addon = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (addon == nullptr) {
        return nullptr;
    }
    return addon->napiContext_->ContextGetResource(env, info, name, type);
}

/*====================================Here is the asynchronization function==================================*/
napi_value ResourceManagerAddon::GetString(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetString", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetStringArray(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringArray", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMedia(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMedia", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMediaBase64(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaBase64", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetConfiguration(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetConfiguration", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetDeviceCapability(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetDeviceCapability", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetPluralString(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetPluralString", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetRawFile(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFile", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFileDescriptor", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::CloseRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "CloseRawFileDescriptor", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetPluralStringByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetPluralStringByName", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMediaBase64ByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaBase64ByName", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMediaByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaByName", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetStringArrayByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringArrayByName", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetStringByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringByName", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetStringValue(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringValue", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetStringArrayValue(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringArrayValue", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMediaContent(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaContent", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetMediaContentBase64(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaContentBase64", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetPluralStringValue(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetPluralStringValue", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetRawFileContent(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFileContent", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetRawFd(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFd", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::CloseRawFd(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "CloseRawFd", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetRawFileList(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFileList", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetColor(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetColor", FunctionType::ASYNC);
}

napi_value ResourceManagerAddon::GetColorByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetColorByName", FunctionType::ASYNC);
}

/*====================================Here is the synchronization function===================================*/
napi_value ResourceManagerAddon::GetStringSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetStringByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetBoolean(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetBoolean", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetNumber(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetNumber", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetNumberByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetNumberByName", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetBooleanByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetBooleanByName", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetDrawableDescriptor(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetDrawableDescriptor", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetDrawableDescriptorByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetDrawableDescriptorByName", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetColorSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetColorSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetColorByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetColorByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::AddResource(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "AddResource", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::RemoveResource(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "RemoveResource", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetMediaContentBase64Sync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaContentBase64Sync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetMediaContentSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaContentSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetPluralStringValueSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetPluralStringValueSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetStringArrayValueSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringArrayValueSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetRawFileContentSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFileContentSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetRawFdSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFdSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::CloseRawFdSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "CloseRawFdSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetRawFileListSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetRawFileListSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetPluralStringByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetPluralStringByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetMediaBase64ByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaBase64ByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetMediaByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetMediaByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetStringArrayByNameSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetStringArrayByNameSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetConfigurationSync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetConfigurationSync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetDeviceCapabilitySync(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetDeviceCapabilitySync", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetLocales(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetLocales", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetSymbol(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetSymbol", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetSymbolByName(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetSymbolByName", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::IsRawDir(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "IsRawDir", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetOverrideResourceManager(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetOverrideResourceManager", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::GetOverrideConfiguration(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "GetOverrideConfiguration", FunctionType::SYNC);
}

napi_value ResourceManagerAddon::UpdateOverrideConfiguration(napi_env env, napi_callback_info info)
{
    return AddonGetResource(env, info, "UpdateOverrideConfiguration", FunctionType::SYNC);
}
} // namespace Resource
} // namespace Global
} // namespace OHOS