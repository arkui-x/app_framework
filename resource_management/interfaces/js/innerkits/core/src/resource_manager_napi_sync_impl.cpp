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

#include "resource_manager_napi_sync_impl.h"

#include <unordered_map>

#include "drawable_descriptor.h"
#include "js_drawable_descriptor.h"
#include "resource_manager_addon.h"
#include "resource_manager_napi_utils.h"
namespace OHOS {
namespace Global {
namespace Resource {
using namespace std::placeholders;
constexpr int ARRAY_SUBCRIPTOR_ZERO = 0;
constexpr int ARRAY_SUBCRIPTOR_ONE = 1;
constexpr int ARRAY_SUBCRIPTOR_TWO = 2;
constexpr int PARAMS_NUM_TWO = 2;
constexpr int PARAMS_NUM_THREE = 3;
ResourceManagerNapiSyncImpl::ResourceManagerNapiSyncImpl()
{}

ResourceManagerNapiSyncImpl::~ResourceManagerNapiSyncImpl()
{}

std::unordered_map<std::string, std::function<napi_value(napi_env&, napi_callback_info&)>>
    ResourceManagerNapiSyncImpl::syncFuncMatch {
    {"GetStringSync", std::bind(&ResourceManagerNapiSyncImpl::GetStringSync, _1, _2)},
    {"GetStringByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetStringByNameSync, _1, _2)},
    {"GetBoolean", std::bind(&ResourceManagerNapiSyncImpl::GetBoolean, _1, _2)},
    {"GetBooleanByName", std::bind(&ResourceManagerNapiSyncImpl::GetBooleanByName, _1, _2)},
    {"GetNumber", std::bind(&ResourceManagerNapiSyncImpl::GetNumber, _1, _2)},
    {"GetNumberByName", std::bind(&ResourceManagerNapiSyncImpl::GetNumberByName, _1, _2)},
    {"GetDrawableDescriptor", std::bind(&ResourceManagerNapiSyncImpl::GetDrawableDescriptor, _1, _2)},
    {"GetDrawableDescriptorByName", std::bind(&ResourceManagerNapiSyncImpl::GetDrawableDescriptorByName, _1, _2)},
    {"GetColorSync", std::bind(&ResourceManagerNapiSyncImpl::GetColorSync, _1, _2)},
    {"GetColorByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetColorByNameSync, _1, _2)},
    {"AddResource", std::bind(&ResourceManagerNapiSyncImpl::AddResource, _1, _2)},
    {"RemoveResource", std::bind(&ResourceManagerNapiSyncImpl::RemoveResource, _1, _2)},
    {"GetMediaContentBase64Sync", std::bind(&ResourceManagerNapiSyncImpl::GetMediaContentBase64Sync, _1, _2)},
    {"GetMediaContentSync", std::bind(&ResourceManagerNapiSyncImpl::GetMediaContentSync, _1, _2)},
    {"GetPluralStringValueSync", std::bind(&ResourceManagerNapiSyncImpl::GetPluralStringValueSync, _1, _2)},
    {"GetStringArrayValueSync", std::bind(&ResourceManagerNapiSyncImpl::GetStringArrayValueSync, _1, _2)},
    {"GetRawFileContentSync", std::bind(&ResourceManagerNapiSyncImpl::GetRawFileContentSync, _1, _2)},
    {"GetRawFdSync", std::bind(&ResourceManagerNapiSyncImpl::GetRawFdSync, _1, _2)},
    {"CloseRawFdSync", std::bind(&ResourceManagerNapiSyncImpl::CloseRawFdSync, _1, _2)},
    {"GetRawFileListSync", std::bind(&ResourceManagerNapiSyncImpl::GetRawFileListSync, _1, _2)},
    {"GetPluralStringByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetPluralStringByNameSync, _1, _2)},
    {"GetMediaBase64ByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetMediaBase64ByNameSync, _1, _2)},
    {"GetMediaByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetMediaByNameSync, _1, _2)},
    {"GetStringArrayByNameSync", std::bind(&ResourceManagerNapiSyncImpl::GetStringArrayByNameSync, _1, _2)},
    {"GetConfigurationSync", std::bind(&ResourceManagerNapiSyncImpl::GetConfigurationSync, _1, _2)},
    {"GetDeviceCapabilitySync", std::bind(&ResourceManagerNapiSyncImpl::GetDeviceCapabilitySync, _1, _2)},
    {"GetLocales", std::bind(&ResourceManagerNapiSyncImpl::GetLocales, _1, _2)},
    {"GetSymbol", std::bind(&ResourceManagerNapiSyncImpl::GetSymbol, _1, _2)},
    {"GetSymbolByName", std::bind(&ResourceManagerNapiSyncImpl::GetSymbolByName, _1, _2)},
    {"IsRawDir", std::bind(&ResourceManagerNapiSyncImpl::IsRawDir, _1, _2)},
    {"GetOverrideResourceManager", std::bind(&ResourceManagerNapiSyncImpl::GetOverrideResourceManager, _1, _2)},
    {"GetOverrideConfiguration", std::bind(&ResourceManagerNapiSyncImpl::GetOverrideConfiguration, _1, _2)},
    {"UpdateOverrideConfiguration", std::bind(&ResourceManagerNapiSyncImpl::UpdateOverrideConfiguration, _1, _2)}
};

napi_value ResourceManagerNapiSyncImpl::GetResource(napi_env env, napi_callback_info info,
    const std::string &functionName)
{
    auto functionIndex = syncFuncMatch.find(functionName);
    if (functionIndex == syncFuncMatch.end()) {
        HiLog::Info(LABEL, "Invalid functionName, %{public}s", functionName.c_str());
        return nullptr;
    }
    return functionIndex->second(env, info);
}

int32_t ResourceManagerNapiSyncImpl::InitPathAddon(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "Failed to get addon in InitPathAddon");
        return NOT_FOUND;
    }
    if (ResourceManagerNapiUtils::IsNapiString(env, info)) {
        dataContext->path_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
    } else {
        return ERROR_CODE_INVALID_INPUT_PARAMETER;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetRawFileListSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = InitPathAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to init para in GetRawFileListSync by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->GetRawFileList(dataContext->path_.c_str(),
        dataContext->arrayValue_);
    if (state != RState::SUCCESS || dataContext->arrayValue_.empty()) {
        HiLog::Error(LABEL, "Failed to get rawfile list by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_RES_PATH_INVALID);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsArray(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetRawFileContentSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = InitPathAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to init para in GetRawFileContentSync by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->GetRawFileFromHap(dataContext->path_,
        dataContext->len_, dataContext->mediaData);
    if (state != SUCCESS) {
        HiLog::Error(LABEL, "Failed to get rawfile by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsUint8Array(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetRawFdSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = InitPathAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to init para in GetRawFdSync by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->GetRawFileDescriptorFromHap(dataContext->path_,
        dataContext->descriptor_);
    if (state != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to get rawfd by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    return ResourceManagerNapiUtils::CreateJsRawFd(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::CloseRawFdSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = InitPathAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to init para in CloseRawFdSync by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->CloseRawFileDescriptor(dataContext->path_);
    if (state != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to close rawfd by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    napi_value undefined;
    if (napi_get_undefined(env, &undefined) != napi_ok) {
        return nullptr;
    }
    return undefined;
}

bool ResourceManagerNapiSyncImpl::InitParamsFromParamArray(napi_env env, napi_value value,
    std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams)
{
    napi_valuetype valuetype = ResourceManagerNapiUtils::GetType(env, value);
    if (valuetype == napi_number) {
        double param;
        if (napi_get_value_double(env, value, &param) != napi_ok) {
            HiLog::Error(LABEL, "Failed to get parameter value in InitParamsFromParamArray");
            return false;
        }
        jsParams.push_back(std::make_tuple(ResourceManager::NapiValueType::NAPI_NUMBER, std::to_string(param)));
        return true;
    }
    if (valuetype == napi_string) {
        size_t len = 0;
        if (napi_get_value_string_utf8(env, value, nullptr, 0, &len) != napi_ok) {
            HiLog::Error(LABEL, "Failed to get parameter length in InitParamsFromParamArray");
            return false;
        }
        std::vector<char> buf(len + 1);
        if (napi_get_value_string_utf8(env, value, buf.data(), len + 1, &len) != napi_ok) {
            HiLog::Error(LABEL, "Failed to get parameter value in InitParamsFromParamArray");
            return false;
        }
        jsParams.push_back(std::make_tuple(ResourceManager::NapiValueType::NAPI_STRING, buf.data()));
        return true;
    }
    return false;
}

bool ResourceManagerNapiSyncImpl::InitNapiParameters(napi_env env, napi_callback_info info,
    std::vector<std::tuple<ResourceManager::NapiValueType, std::string>> &jsParams)
{
    size_t size = 1;
    napi_get_cb_info(env, info, &size, nullptr, nullptr, nullptr);
    // one parameter: resId or resource or Name
    if (size == 1) {
        return true;
    }
    napi_value paramArray[size];
    napi_get_cb_info(env, info, &size, paramArray, nullptr, nullptr);

    for (size_t i = 1; i < size; ++i) {
        if (!InitParamsFromParamArray(env, paramArray[i], jsParams)) {
            return false;
        }
    }
    return true;
}

int32_t ResourceManagerNapiSyncImpl::InitIdResourceAddon(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "Failed to get addon in InitIdResourceAddon");
        return NOT_FOUND;
    }
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        dataContext->resId_ = ResourceManagerNapiUtils::GetResId(env, argc, argv);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        auto resourcePtr = std::make_shared<ResourceManager::Resource>();
        int32_t retCode = ResourceManagerNapiUtils::GetResourceObject(env, resourcePtr, argv[ARRAY_SUBCRIPTOR_ZERO]);
        dataContext->resource_ = resourcePtr;
        return retCode;
    } else {
        return ERROR_CODE_INVALID_INPUT_PARAMETER;
    }
    return SUCCESS;
}

int32_t ResourceManagerNapiSyncImpl::ProcessStrResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetStringSync");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }

    if (!InitNapiParameters(env, info, dataContext->jsParams_)) {
        HiLog::Error(LABEL, "GetStringSync formatting error");
        return ERROR_CODE_RES_ID_FORMAT_ERROR;
    }

    RState state = resMgr->GetStringFormatById(resId, dataContext->value_, dataContext->jsParams_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetStringSync failed state", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetStringSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetStringSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessStrResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string in GetStringSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessSymbolResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetSymbol");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }

    RState state = resMgr->GetSymbolById(resId, dataContext->symbolValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetSymbol failed state", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
    }
    return state;
}

napi_value ResourceManagerNapiSyncImpl::GetSymbol(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetSymbol", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessSymbolResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process symbol in GetSymbol", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsSymbol(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessColorResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to get resMgr in ProcessColorResource");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }

    RState state = resMgr->GetColorById(resId, dataContext->colorValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetColor failed state", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetColorSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetColorSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessColorResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string in GetColorSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsColor(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessNumResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to ResourceManagerNapiUtils::GetHapResourceManager in GetNumber");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }

    RState state = resMgr->GetIntegerById(resId, dataContext->iValue_);
    if (state != RState::SUCCESS) {
        state = resMgr->GetFloatById(resId, dataContext->fValue_);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetNumber(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetNumber", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessNumResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string in GetNumber", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsNumber(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessBoolResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret2 = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret2) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetBoolean");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    RState state = resMgr->GetBooleanById(resId, dataContext->bValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to GetBoolean state", true);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetBoolean(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetBoolean", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessBoolResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process bool resource in GetBoolean", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsBool(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcesstMediaContentBase64Resource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret2 = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret2) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetMediaContentBase64Sync");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    RState state = resMgr->GetMediaBase64DataById(resId, dataContext->value_, dataContext->density_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to GetMediaContentBase64Sync state", true);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetMediaContentBase64Sync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetMediaContentBase64Sync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    state = ProcesstMediaContentBase64Resource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process media base64 resource in GetMediaContentBase64Sync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessMediaContentResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret2 = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret2) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetMediaContentSync");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    RState state = resMgr->GetMediaDataById(resId, dataContext->len_, dataContext->mediaData,
        dataContext->density_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to GetMediaContentSync state", true);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetMediaContentSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetMediaContentSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    state = ProcessMediaContentResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process media resource in GetMediaContentSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsUint8Array(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessPluralStringValueResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret2 = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret2) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetPluralStringValueSync");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    RState state = resMgr->GetPluralStringByIdFormat(dataContext->value_,
        resId, dataContext->param_, dataContext->param_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to GetPluralStringValueSync state", true);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetPluralStringValueSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetPluralStringValueSync",true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    if (ResourceManagerNapiUtils::GetType(env, argv[ARRAY_SUBCRIPTOR_ONE]) != napi_number) {
        HiLog::Error(LABEL, "Parameter type is not napi_number in GetPluralStringValueSync");
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    napi_get_value_int32(env, argv[ARRAY_SUBCRIPTOR_ONE], &dataContext->param_);

    state = ProcessPluralStringValueResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process plural string resource in GetPluralStringValueSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessStringArrayValueResource(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    bool ret2 = ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId);
    if (!ret2) {
        HiLog::Error(LABEL, "Failed to get resMgr in GetStringArrayValueSync");
        return ERROR_CODE_RES_NOT_FOUND_BY_ID;
    }
    RState state = resMgr->GetStringArrayById(resId, dataContext->arrayValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to GetStringArrayValueSync state", true);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetStringArrayValueSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetStringArrayValueSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessStringArrayValueResource(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string array resource in GetStringArrayValueSync", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsArray(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetDrawableDescriptor(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t ret = ResourceManagerNapiSyncImpl::InitIdResourceAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetDrawableDescriptor", true);
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }
    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }

    // data type optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_TWO], dataContext->iconType_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }

    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;
    if (!ResourceManagerNapiUtils::GetHapResourceManager(dataContext.get(), resMgr, resId)) {
        dataContext->SetErrorMsg("Failed to get GetHapResourceManager in GetDrawableDescriptor", true);
        return nullptr;
    }
    RState state = SUCCESS;
    Ace::Napi::DrawableDescriptor::DrawableType drawableType;
    if (dataContext->iconType_ == 1) {
        std::string themeMask = resMgr->GetThemeMask();
        std::pair<std::unique_ptr<uint8_t[]>, size_t> foregroundInfo;
        std::pair<std::unique_ptr<uint8_t[]>, size_t> backgroundInfo;
        state = resMgr->GetThemeIcons(resId, foregroundInfo, backgroundInfo, dataContext->density_);
        if (state == SUCCESS) {
            auto drawableDescriptor = Ace::Napi::DrawableDescriptorFactory::Create(foregroundInfo, backgroundInfo,
                themeMask, drawableType, resMgr);
            return Ace::Napi::JsDrawableDescriptor::ToNapi(env, drawableDescriptor.release(), drawableType);
        }
    }
    auto drawableDescriptor = Ace::Napi::DrawableDescriptorFactory::Create(resId, resMgr,
        state, drawableType, dataContext->density_);
    if (state != SUCCESS) {
        dataContext->SetErrorMsg("Failed to Create drawableDescriptor", true);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    return Ace::Napi::JsDrawableDescriptor::ToNapi(env, drawableDescriptor.release(), drawableType);
}

int32_t ResourceManagerNapiSyncImpl::InitNameAddon(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "Failed to get addon in InitNameAddon");
        return NOT_FOUND;
    }
    if (ResourceManagerNapiUtils::IsNapiString(env, info)) {
        dataContext->resName_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
    } else {
        return ERROR_CODE_INVALID_INPUT_PARAMETER;
    }
    return SUCCESS;
}

int32_t ResourceManagerNapiSyncImpl::ProcessStrResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    if (!InitNapiParameters(env, info, dataContext->jsParams_)) {
        HiLog::Error(LABEL, "GetStringByNameSync formatting error");
        return ERROR_CODE_RES_NAME_FORMAT_ERROR;
    }

    RState state = dataContext->addon_->GetResMgr()->GetStringFormatByName(dataContext->resName_.c_str(),
        dataContext->value_, dataContext->jsParams_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetStringByNameSync failed state", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetStringByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetStringByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessStrResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string in GetStringByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    
    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessSymbolResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetSymbolByName(dataContext->resName_.c_str(),
        dataContext->symbolValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetSymbolByName failed state", false);
    }
    return state;
}

napi_value ResourceManagerNapiSyncImpl::GetSymbolByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetSymbolByName", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessSymbolResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process symbol in GetSymbolByName", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsSymbol(env, *dataContext);
}


int32_t ResourceManagerNapiSyncImpl::ProcessColorResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetColorByName(dataContext->resName_.c_str(),
        dataContext->colorValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetColorByNameSync failed state", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetColorByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetColorByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessColorResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process color in GetColorByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    
    return ResourceManagerNapiUtils::CreateJsColor(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessNumResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    auto resMgr = dataContext->addon_->GetResMgr();
    RState state = resMgr->GetIntegerByName(dataContext->resName_.c_str(),
        dataContext->iValue_);
    if (state != RState::SUCCESS) {
        state = resMgr->GetFloatByName(dataContext->resName_.c_str(), dataContext->fValue_);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetNumberByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetNumberByName", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessNumResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process number in GetNumberByName", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsNumber(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetBooleanByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetBooleanByName", false);
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->GetBooleanByName(dataContext->resName_.c_str(),
        dataContext->bValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetBooleanByName failed state", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsBool(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetDrawableDescriptorByName(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }

    auto dataContext = std::make_unique<ResMgrDataContext>();
    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    // icon type optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_TWO], dataContext->iconType_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "Failed to get addon_ in GetDrawableDescriptorByName");
        return nullptr;
    }
    dataContext->resName_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
    auto resMgr = dataContext->addon_->GetResMgr();
    RState state = SUCCESS;
    Ace::Napi::DrawableDescriptor::DrawableType drawableType;
    if (dataContext->iconType_ == 1) {
        std::tuple<const char *, uint32_t, uint32_t> drawableInfo(dataContext->resName_.c_str(),
        dataContext->iconType_, dataContext->density_);
        auto drawableDescriptor = Ace::Napi::DrawableDescriptorFactory::Create(drawableInfo, resMgr,
            state, drawableType);
        if (state != SUCCESS) {
            dataContext->SetErrorMsg("Failed to Create drawableDescriptor ", false);
            ResourceManagerNapiUtils::NapiThrow(env, state);
            return nullptr;
        }
        return Ace::Napi::JsDrawableDescriptor::ToNapi(env, drawableDescriptor.release(), drawableType);
    }
    auto drawableDescriptor = Ace::Napi::DrawableDescriptorFactory::Create(dataContext->resName_.c_str(),
        resMgr, state, drawableType, dataContext->density_);
    if (state != SUCCESS) {
        dataContext->SetErrorMsg("Failed to Create drawableDescriptor ", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    return Ace::Napi::JsDrawableDescriptor::ToNapi(env, drawableDescriptor.release(), drawableType);
}

std::shared_ptr<ResourceManager> GetNativeResoruceManager(napi_env env, napi_callback_info info)
{
    auto addon = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (addon == nullptr) {
        HiLog::Error(LABEL, "Failed to get addon_ in GetNativeResoruceManager");
        return nullptr;
    }
    return addon->GetResMgr();
}

napi_value ResourceManagerNapiSyncImpl::AddResource(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->path_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
    auto resMgr = GetNativeResoruceManager(env, info);
    if (!resMgr->AddAppOverlay(dataContext->path_)) {
        HiLog::Error(LABEL, "Failed to add overlay path = %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_OVERLAY_RES_PATH_INVALID);
        return nullptr;
    }
    return nullptr;
}

napi_value ResourceManagerNapiSyncImpl::RemoveResource(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->path_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
    auto resMgr = GetNativeResoruceManager(env, info);
    if (!resMgr->RemoveAppOverlay(dataContext->path_)) {
        HiLog::Error(LABEL, "Failed to add overlay path = %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_OVERLAY_RES_PATH_INVALID);
        return nullptr;
    }
    return nullptr;
}

int32_t ResourceManagerNapiSyncImpl::ProcessPluralStrResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetPluralStringByNameFormat(dataContext->value_,
        dataContext->resName_.c_str(), dataContext->param_, dataContext->param_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetPluralStringByNameSync failed state", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetPluralStringByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetPluralStringByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    if (ResourceManagerNapiUtils::GetType(env, argv[ARRAY_SUBCRIPTOR_ONE]) != napi_number) {
        HiLog::Error(LABEL, "Parameter type is not napi_number in GetPluralStringByNameSync");
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    napi_get_value_int32(env, argv[ARRAY_SUBCRIPTOR_ONE], &dataContext->param_);

    state = ProcessPluralStrResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process plural string in GetPluralStringByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessMediaBase64ResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetMediaBase64DataByName(dataContext->resName_.c_str(),
        dataContext->value_, dataContext->density_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to get media data in GetMediaBase64ByNameSync", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetMediaBase64ByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetMediaBase64ByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }

    state = ProcessMediaBase64ResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process media base64 resource in GetMediaBase64ByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsString(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessMediaResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetMediaDataByName(dataContext->resName_.c_str(),
        dataContext->len_, dataContext->mediaData, dataContext->density_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetMediaByNameSync failed state", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetMediaByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetMediaByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    // density optional parameters
    if (ResourceManagerNapiUtils::GetDataType(env, argv[ARRAY_SUBCRIPTOR_ONE], dataContext->density_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }

    state = ProcessMediaResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process media resource in GetMediaByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    return ResourceManagerNapiUtils::CreateJsUint8Array(env, *dataContext);
}

int32_t ResourceManagerNapiSyncImpl::ProcessStringArrayResourceByName(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    RState state = dataContext->addon_->GetResMgr()->GetStringArrayByName(dataContext->resName_.c_str(),
        dataContext->arrayValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetStringArrayByNameSync failed state", false);
        return state;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetStringArrayByNameSync(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = ResourceManagerNapiSyncImpl::InitNameAddon(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to init para in GetStringArrayByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    state = ProcessStringArrayResourceByName(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to process string array resource in GetStringArrayByNameSync", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    return ResourceManagerNapiUtils::CreateJsArray(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetConfigurationSync(napi_env env, napi_callback_info info)
{
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    return ResourceManagerNapiUtils::CreateJsConfig(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetDeviceCapabilitySync(napi_env env, napi_callback_info info)
{
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    return ResourceManagerNapiUtils::CreateJsDeviceCap(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetLocales(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);

    // includeSystem optional parameters
    if (ResourceManagerNapiUtils::GetIncludeSystem(env, argv[ARRAY_SUBCRIPTOR_ZERO],
        dataContext->bValue_) != SUCCESS) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    dataContext->addon_->GetResMgr()->GetLocales(dataContext->arrayValue_, dataContext->bValue_);
    return ResourceManagerNapiUtils::CreateJsArray(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::IsRawDir(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    auto dataContext = std::make_unique<ResMgrDataContext>();

    int32_t ret = InitPathAddon(env, info, dataContext);
    if (ret != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to init para in IsRawDir by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, ret);
        return nullptr;
    }

    RState state = dataContext->addon_->GetResMgr()->IsRawDirFromHap(dataContext->path_,
        dataContext->bValue_);
    if (state != RState::SUCCESS) {
        HiLog::Error(LABEL, "Failed to determine the raw file is directory by %{public}s", dataContext->path_.c_str());
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }
    return ResourceManagerNapiUtils::CreateJsBool(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::GetOverrideResourceManager(napi_env env, napi_callback_info info)
{
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = getAddonAndConfig(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to get param in GetOverrideResourceManager", false);
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    std::shared_ptr<ResourceManager> resMgr = dataContext->addon_->GetResMgr();
    std::shared_ptr<ResourceManager> overrideResMgr = resMgr->GetOverrideResourceManager(
        dataContext->overrideResConfig_);
    if (overrideResMgr == nullptr) {
        dataContext->SetErrorMsg("GetOverrideResourceManager, overrideResMgr is null", false);
        HiLog::Error(LABEL, "GetOverrideResourceManager, overrideResMgr is null");
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return dataContext->addon_->CreateOverrideAddon(env, overrideResMgr);
}

RState ResourceManagerNapiSyncImpl::getAddonAndConfig(napi_env env, napi_callback_info info,
    std::unique_ptr<ResMgrDataContext> &dataContext)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "getAddonAndConfig failed to get addon");
        return ERROR_CODE_INVALID_INPUT_PARAMETER;
    }
    if (ResourceManagerNapiUtils::GetConfigObject(env, argv[ARRAY_SUBCRIPTOR_ZERO], dataContext) != SUCCESS) {
        HiLog::Error(LABEL, "getAddonAndConfig failed to get resConfig");
        return ERROR_CODE_INVALID_INPUT_PARAMETER;
    }
    return SUCCESS;
}

napi_value ResourceManagerNapiSyncImpl::GetOverrideConfiguration(napi_env env, napi_callback_info info)
{
    auto dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        HiLog::Error(LABEL, "GetOverrideConfiguration failed to get addon");
        return nullptr;
    }
    return ResourceManagerNapiUtils::CreateOverrideJsConfig(env, *dataContext);
}

napi_value ResourceManagerNapiSyncImpl::UpdateOverrideConfiguration(napi_env env, napi_callback_info info)
{
    auto dataContext = std::make_unique<ResMgrDataContext>();
    int32_t state = getAddonAndConfig(env, info, dataContext);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to get param in GetOverrideResourceManager", false);
        HiLog::Error(LABEL, "Failed to get param in GetOverrideResourceManager");
        ResourceManagerNapiUtils::NapiThrow(env, state);
        return nullptr;
    }

    std::shared_ptr<ResourceManager> resMgr = dataContext->addon_->GetResMgr();
    state = resMgr->UpdateOverrideResConfig(*dataContext->overrideResConfig_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("UpdateOverrideConfiguration failed due to invalid config", false);
        HiLog::Error(LABEL, "UpdateOverrideConfiguration failed due to invalid config");
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
    }
    return nullptr;
}
} // namespace Resource
} // namespace Global
} // namespace OHOS