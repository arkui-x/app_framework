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

#include "resource_manager_napi_async_impl.h"

#include <unordered_map>
#if !defined(__ARKUI_CROSS__)
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
#endif
#include "resource_manager_napi_utils.h"
namespace OHOS {
namespace Global {
namespace Resource {
using namespace std::placeholders;
constexpr int PARAMS_NUM_TWO = 2;
constexpr int PARAMS_NUM_THREE = 3;
ResourceManagerNapiAsyncImpl::ResourceManagerNapiAsyncImpl()
{}

ResourceManagerNapiAsyncImpl::~ResourceManagerNapiAsyncImpl()
{}

std::unordered_map<std::string, std::function<napi_value(napi_env&, napi_callback_info&)>>
    ResourceManagerNapiAsyncImpl::asyncFuncMatch {
    /*===========================since 6 but deprecated since 9======================================*/
    {"GetString", std::bind(&ResourceManagerNapiAsyncImpl::GetString, _1, _2)},
    {"GetStringArray", std::bind(&ResourceManagerNapiAsyncImpl::GetStringArray, _1, _2)},
    {"GetMedia", std::bind(&ResourceManagerNapiAsyncImpl::GetMedia, _1, _2)},
    {"GetMediaBase64", std::bind(&ResourceManagerNapiAsyncImpl::GetMediaBase64, _1, _2)},
    {"GetPluralString", std::bind(&ResourceManagerNapiAsyncImpl::GetPluralString, _1, _2)},
    {"GetRawFile", std::bind(&ResourceManagerNapiAsyncImpl::GetRawFile, _1, _2)},
    {"GetRawFileDescriptor", std::bind(&ResourceManagerNapiAsyncImpl::GetRawFileDescriptor, _1, _2)},
    {"CloseRawFileDescriptor", std::bind(&ResourceManagerNapiAsyncImpl::CloseRawFileDescriptor, _1, _2)},
    /*===========================since 6 but deprecated since 9========================================*/

    {"GetStringValue", std::bind(&ResourceManagerNapiAsyncImpl::GetStringValue, _1, _2)},
    {"GetStringArrayValue", std::bind(&ResourceManagerNapiAsyncImpl::GetStringArrayValue, _1, _2)},
    {"GetMediaContent", std::bind(&ResourceManagerNapiAsyncImpl::GetMediaContent, _1, _2)},
    {"GetMediaContentBase64", std::bind(&ResourceManagerNapiAsyncImpl::GetMediaContentBase64, _1, _2)},
    {"GetPluralStringValue", std::bind(&ResourceManagerNapiAsyncImpl::GetPluralStringValue, _1, _2)},
    {"GetRawFileContent", std::bind(&ResourceManagerNapiAsyncImpl::GetRawFileContent, _1, _2)},
    {"GetRawFd", std::bind(&ResourceManagerNapiAsyncImpl::GetRawFd, _1, _2)},
    {"CloseRawFd", std::bind(&ResourceManagerNapiAsyncImpl::CloseRawFd, _1, _2)},
    {"GetConfiguration", std::bind(&ResourceManagerNapiAsyncImpl::GetConfiguration, _1, _2)},
    {"GetDeviceCapability", std::bind(&ResourceManagerNapiAsyncImpl::GetDeviceCapability, _1, _2)},
    {"GetStringByName", std::bind(&ResourceManagerNapiAsyncImpl::GetStringByName, _1, _2)},
    {"GetStringArrayByName", std::bind(&ResourceManagerNapiAsyncImpl::GetStringArrayByName, _1, _2)},
    {"GetMediaByName", std::bind(&ResourceManagerNapiAsyncImpl::GetMediaByName, _1, _2)},
    {"GetMediaBase64ByName", std::bind(&ResourceManagerNapiAsyncImpl::GetMediaBase64ByName, _1, _2)},
    {"GetPluralStringByName", std::bind(&ResourceManagerNapiAsyncImpl::GetPluralStringByName, _1, _2)},
    {"GetRawFileList", std::bind(&ResourceManagerNapiAsyncImpl::GetRawFileList, _1, _2)},
    {"GetColor", std::bind(&ResourceManagerNapiAsyncImpl::GetColor, _1, _2)},
    {"GetColorByName", std::bind(&ResourceManagerNapiAsyncImpl::GetColorByName, _1, _2)},
};

napi_value ResourceManagerNapiAsyncImpl::GetResource(napi_env env, napi_callback_info info,
    const std::string &functionName)
{
    auto functionIndex = asyncFuncMatch.find(functionName);
    if (functionIndex == asyncFuncMatch.end()) {
        HiLog::Info(LABEL, "Invalid functionName, %{public}s", functionName.c_str());
        return nullptr;
    }
    return functionIndex->second(env, info);
}

napi_value GetCallbackErrorCode(napi_env env, const int32_t errCode, const std::string &errMsg)
{
    napi_value error = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    napi_create_int32(env, errCode, &eCode);
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &eMsg);
    napi_create_object(env, &error);
    napi_set_named_property(env, error, "code", eCode);
    napi_set_named_property(env, error, "message", eMsg);
    return error;
}

void ResourceManagerNapiAsyncImpl::Complete(napi_env env, napi_status status, void* data)
{
    ResMgrDataContext* dataContext = static_cast<ResMgrDataContext*>(data);

    napi_value finalResult = nullptr;
    if (dataContext->createValueFunc_ != nullptr) {
        finalResult = dataContext->createValueFunc_(env, *dataContext);
    }

    napi_value result[] = { nullptr, nullptr };
    if (dataContext->success_) {
        napi_get_undefined(env, &result[0]);
        result[1] = finalResult;
    } else {
        result[0] = GetCallbackErrorCode(env, dataContext->errCode_, dataContext->errMsg_.c_str());
        napi_get_undefined(env, &result[1]);
    }
    if (dataContext->deferred_) {
        if (dataContext->success_) {
            if (napi_resolve_deferred(env, dataContext->deferred_, result[1]) != napi_ok) {
                HiLog::Error(LABEL, "napi_resolve_deferred failed");
            }
        } else {
            result[0] = GetCallbackErrorCode(env, dataContext->errCode_, dataContext->errMsg_.c_str());
            if (napi_reject_deferred(env, dataContext->deferred_, result[0]) != napi_ok) {
                HiLog::Error(LABEL, "napi_reject_deferred failed");
            }
        }
    } else {
        do {
            napi_value callback = nullptr;
            napi_status status = napi_get_reference_value(env, dataContext->callbackRef_, &callback);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_get_reference_value failed status=%{public}d", status);
                break;
            }
            napi_value userRet = nullptr;
            status = napi_call_function(env, nullptr, callback, sizeof(result) / sizeof(napi_value), result, &userRet);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_call_function failed status=%{public}d", status);
                break;
            }
            status = napi_delete_reference(env, dataContext->callbackRef_);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "napi_call_function failed status=%{public}d", status);
                break;
            }
        } while (false);
    }
    napi_delete_async_work(env, dataContext->work_);
    delete dataContext;
};

napi_value ResourceManagerNapiAsyncImpl::GetResult(napi_env env, std::unique_ptr<ResMgrDataContext> &dataContext,
    const std::string &name, napi_async_execute_callback &execute)
{
    napi_value result = nullptr;
    if (dataContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &dataContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resource = nullptr;
    napi_create_string_utf8(env, name.c_str(), NAPI_AUTO_LENGTH, &resource);
    if (napi_create_async_work(env, nullptr, resource, execute, ResourceManagerNapiAsyncImpl::Complete,
        static_cast<void*>(dataContext.get()), &dataContext->work_) != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for %{public}s", name.c_str());
        return result;
    }
    if (napi_queue_async_work_with_qos(env, dataContext->work_, napi_qos_user_initiated) != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for %{public}s", name.c_str());
        return result;
    }
    dataContext.release();
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessNameParamV9(napi_env env, napi_callback_info info,
    const std::string &name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);
    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_string) {
            dataContext->resName_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else if (i == 1 && ResourceManagerNapiUtils::GetDataType(env, argv[i], dataContext->density_) != SUCCESS) {
            ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
            return nullptr;
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessIdParamV9(napi_env env, napi_callback_info info,
    const std::string &name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_number) {
            dataContext->resId_ = ResourceManagerNapiUtils::GetResId(env, argc, argv);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else if (i == 1 && ResourceManagerNapiUtils::GetDataType(env, argv[i], dataContext->density_) != SUCCESS) {
            ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
            return nullptr;
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessResourceParamV9(napi_env env, napi_callback_info info,
    const std::string &name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_object) {
            auto resourcePtr = std::make_shared<ResourceManager::Resource>();
            int32_t retCode = ResourceManagerNapiUtils::GetResourceObject(env, resourcePtr, argv[0]);
            if (retCode != SUCCESS) {
                HiLog::Error(LABEL, "Failed to get native Resource object");
                ResourceManagerNapiUtils::NapiThrow(env, retCode);
                return nullptr;
            }
            dataContext->resource_ = resourcePtr;
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else if (i == 1 && ResourceManagerNapiUtils::GetDataType(env, argv[i], dataContext->density_) != SUCCESS) {
            ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
            return nullptr;
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessOnlyIdParam(napi_env env, napi_callback_info info,
    const std::string &name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, PARAMS_NUM_TWO);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    if (dataContext->addon_ == nullptr) {
        return nullptr;
    }
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_number) {
            dataContext->resId_ = ResourceManagerNapiUtils::GetResId(env, argc, argv);
        } else if (i == 1 && valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else if (i == 0 && valueType == napi_string) {
            dataContext->path_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
        } else if (i == 0 && valueType == napi_object) {
            std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
            int32_t retCode = ResourceManagerNapiUtils::GetResourceObject(env, resourcePtr, argv[0]);
            if (retCode != SUCCESS) {
                ResourceManagerNapiUtils::NapiThrow(env, retCode);
                return nullptr;
            }
            dataContext->resource_ = resourcePtr;
        } else {
            // self resourcemanager with promise
        }
    }
    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessIdNameParam(napi_env env, napi_callback_info info,
    const std::string& name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, PARAMS_NUM_THREE);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    dataContext->addon_ = ResMgrDataContext::GetResourceManagerAddon(env, info);
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_number) {
            dataContext->resId_ = ResourceManagerNapiUtils::GetResId(env, argc, argv);
        } else if (i == 0 && valueType == napi_string) {
            dataContext->resName_ = ResourceManagerNapiUtils::GetResNameOrPath(env, argc, argv);
        } else if (i == 0 && valueType == napi_object) {
            std::shared_ptr<ResourceManager::Resource> resourcePtr = std::make_shared<ResourceManager::Resource>();
            int32_t retCode = ResourceManagerNapiUtils::GetResourceObject(env, resourcePtr, argv[0]);
            if (retCode != SUCCESS) {
                HiLog::Error(LABEL, "Failed to get native Resource object");
                return nullptr;
            }
            dataContext->resource_ = resourcePtr;
        } else if (i == 1 && valueType != napi_number) { // the second quantity param
            HiLog::Error(LABEL, "Parameter type is not napi_number");
            ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
            return nullptr;
        } else if (i == 1 && valueType == napi_number) {
            napi_get_value_int32(env, argv[i], &dataContext->param_);
        } else if (i == 2 && valueType == napi_function) { // the third callback param
            napi_create_reference(env, argv[i], 1, &dataContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }
    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

napi_value ResourceManagerNapiAsyncImpl::ProcessNoParam(napi_env env, napi_callback_info info,
    const std::string &name, napi_async_execute_callback execute)
{
    GET_PARAMS(env, info, 1);

    std::unique_ptr<ResMgrDataContext> dataContext = std::make_unique<ResMgrDataContext>();
    std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
    napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to unwrap ProcessNoParam");
        return nullptr;
    }
    dataContext->addon_ = *addonPtr;
    napi_valuetype valueType;
    napi_typeof(env, argv[0], &valueType);
    if (valueType == napi_function) {
        napi_create_reference(env, argv[0], 1, &dataContext->callbackRef_);
    }

    napi_value result = GetResult(env, dataContext, name, execute);
    return result;
}

auto getStringFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;

    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to GetHapResourceManager in getStringFunc");
        return;
    }
    RState state = resMgr->GetStringById(resId, dataContext->value_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetStringById failed state", true, state);
    #if !defined(__ARKUI_CROSS__)
        ReportGetResourceByIdFail(resId, dataContext->value_, "failed in getStringFunc");
    #endif
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsString;
};

napi_value ResourceManagerNapiAsyncImpl::GetStringValue(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        return ProcessIdParamV9(env, info, "getStringValue", getStringFunc);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        return ProcessResourceParamV9(env, info, "getStringValue", getStringFunc);
    } else {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
}

napi_value ResourceManagerNapiAsyncImpl::GetString(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getString", getStringFunc);
}

auto getStringByNameFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state = dataContext->addon_->GetResMgr()->GetStringByName(dataContext->resName_.c_str(),
        dataContext->value_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetStringByName failed state", false, state);
#if !defined(__ARKUI_CROSS__)
        ReportGetResourceByNameFail(dataContext->resName_, dataContext->value_,
            "failed in getStringByNameFunc");
#endif
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsString;
};

napi_value ResourceManagerNapiAsyncImpl::GetStringByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessNameParamV9(env, info, "getStringByName", getStringByNameFunc);
}

auto getStringArrayFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state;
    int32_t resId = 0;
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    if (dataContext->resId_ != 0 || dataContext->resource_ != nullptr) {
        bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId);
        if (!ret) {
            HiLog::Error(LABEL, "Failed to GetHapResourceManager in getStringArrayFunc");
            return;
        }
        state = resMgr->GetStringArrayById(resId, dataContext->arrayValue_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("GetStringArrayById failed state", true, state);
            return;
        }
    } else {
        state = dataContext->addon_->GetResMgr()->GetStringArrayByName(dataContext->resName_.c_str(),
            dataContext->arrayValue_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("GetStringArrayByName failed state", false, state);
            return;
        }
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsArray;
};

napi_value ResourceManagerNapiAsyncImpl::GetStringArray(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getStringArray", getStringArrayFunc);
}

napi_value ResourceManagerNapiAsyncImpl::GetStringArrayValue(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        return ProcessIdParamV9(env, info, "getStringArrayValue", getStringArrayFunc);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        return ProcessResourceParamV9(env, info, "getStringArrayValue", getStringArrayFunc);
    } else {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
}

napi_value ResourceManagerNapiAsyncImpl::GetStringArrayByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessNameParamV9(env, info, "GetStringArrayByName", getStringArrayFunc);
}

auto getMediaFunc = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    std::string path;
    int32_t resId = 0;
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId);
    if (!ret) {
        HiLog::Error(LABEL, "Failed to GetHapResourceManager in getMediaFunc");
        return;
    }
    RState state = resMgr->GetMediaDataById(resId, dataContext->len_, dataContext->mediaData,
        dataContext->density_);
    if (state != SUCCESS) {
        dataContext->SetErrorMsg("Failed to get media data in getMediaFunc", true, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsUint8Array;
};

napi_value ResourceManagerNapiAsyncImpl::GetMediaContent(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        return ProcessIdParamV9(env, info, "getMediaContent", getMediaFunc);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        return ProcessResourceParamV9(env, info, "getMediaContent", getMediaFunc);
    } else {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
}

napi_value ResourceManagerNapiAsyncImpl::GetMedia(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value media = ProcessOnlyIdParam(env, info, "getMedia", getMediaFunc);
#else
    std::string traceVal = "ResourceManagerAddon::GetMedia";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value media = ProcessOnlyIdParam(env, info, "getMedia", getMediaFunc);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return media;
}

auto getMediaBase64Func = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    std::string path;
    RState state;
    if (dataContext->resId_ != 0 || dataContext->resource_ != nullptr) {
        int32_t resId = 0;
        std::shared_ptr<ResourceManager> resMgr = nullptr;
        bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId);
        if (!ret) {
            HiLog::Error(LABEL, "Failed to GetHapResourceManager in getMediaBase64Func");
            return;
        }
        state = resMgr->GetMediaBase64DataById(resId, dataContext->value_, dataContext->density_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("Failed to get media data in getMediaBase64Func", true, state);
            return;
        }
        dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsString;
    } else {
        state = dataContext->addon_->GetResMgr()->GetMediaBase64DataByName(dataContext->resName_.c_str(),
            dataContext->value_, dataContext->density_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("Failed to get media data in getMediaBase64Func", false, state);
            return;
        }
        dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsString;
    }
};

napi_value ResourceManagerNapiAsyncImpl::GetMediaContentBase64(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        return ProcessIdParamV9(env, info, "getMediaContentBase64", getMediaBase64Func);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        return ProcessResourceParamV9(env, info, "getMediaContentBase64", getMediaBase64Func);
    } else {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
}

napi_value ResourceManagerNapiAsyncImpl::GetMediaBase64(napi_env env, napi_callback_info info)
{
#if defined(__ARKUI_CROSS__)
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64", getMediaBase64Func);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaBase64";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value mediaBase64 = ProcessOnlyIdParam(env, info, "GetMediaBase64", getMediaBase64Func);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return mediaBase64;
}

napi_value ResourceManagerNapiAsyncImpl::GetMediaBase64ByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
#if defined(__ARKUI_CROSS__)
    napi_value mediaBase64 = ProcessNameParamV9(env, info, "GetMediaBase64ByName", getMediaBase64Func);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaBase64ByName";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value mediaBase64 = ProcessNameParamV9(env, info, "GetMediaBase64ByName", getMediaBase64Func);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return mediaBase64;
}

auto getPluralCapFunc = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state;
    if (dataContext->resId_ != 0 || dataContext->resource_ != nullptr) {
        int32_t resId = 0;
        std::shared_ptr<ResourceManager> resMgr = nullptr;
        bool ret = ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId);
        if (!ret) {
            HiLog::Error(LABEL, "Failed to GetHapResourceManager in getPluralCapFunc");
            return;
        }
        state = resMgr->GetPluralStringByIdFormat(dataContext->value_,
            resId, dataContext->param_, dataContext->param_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("GetPluralString failed", true, state);
            return;
        }
    } else {
        state = dataContext->addon_->GetResMgr()->GetPluralStringByNameFormat(dataContext->value_,
            dataContext->resName_.c_str(), dataContext->param_, dataContext->param_);
        if (state != RState::SUCCESS) {
            dataContext->SetErrorMsg("GetPluralString failed", false, state);
            return;
        }
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsString;
};

napi_value ResourceManagerNapiAsyncImpl::GetPluralStringValue(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessIdNameParam(env, info, "getPluralStringValue", getPluralCapFunc);
}

napi_value ResourceManagerNapiAsyncImpl::GetPluralStringByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessIdNameParam(env, info, "GetPluralStringByName", getPluralCapFunc);
}

napi_value ResourceManagerNapiAsyncImpl::GetPluralString(napi_env env, napi_callback_info info)
{
    return ProcessIdNameParam(env, info, "GetPluralString", getPluralCapFunc);
}

auto g_getRawFileFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state = dataContext->addon_->GetResMgr()->GetRawFileFromHap(dataContext->path_,
        dataContext->len_, dataContext->mediaData);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetRawFileContent failed", false, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsUint8Array;
};

napi_value ResourceManagerNapiAsyncImpl::GetRawFileContent(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessOnlyIdParam(env, info, "getRawFileContent", g_getRawFileFunc);
}

napi_value ResourceManagerNapiAsyncImpl::GetRawFile(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getRawFile", g_getRawFileFunc);
}

auto g_getRawFileDescriptorFunc = [](napi_env env, void* data) {
    ResMgrDataContext *context = static_cast<ResMgrDataContext*>(data);
    RState state = context->addon_->GetResMgr()->GetRawFileDescriptorFromHap(context->path_, context->descriptor_);
    if (state != RState::SUCCESS) {
        context->SetErrorMsg("Failed to get descriptor", false, state);
        return;
    }
    context->createValueFunc_ = ResourceManagerNapiUtils::CreateJsRawFd;
};

napi_value ResourceManagerNapiAsyncImpl::GetRawFd(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessOnlyIdParam(env, info, "getRawFd", g_getRawFileDescriptorFunc);
}

napi_value ResourceManagerNapiAsyncImpl::GetRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "getRawFileDescriptor", g_getRawFileDescriptorFunc);
}

auto closeRawFileDescriptorFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CloseJsRawFd;
};

napi_value ResourceManagerNapiAsyncImpl::CloseRawFd(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessOnlyIdParam(env, info, "closeRawFd", closeRawFileDescriptorFunc);
}

napi_value ResourceManagerNapiAsyncImpl::CloseRawFileDescriptor(napi_env env, napi_callback_info info)
{
    return ProcessOnlyIdParam(env, info, "closeRawFileDescriptor", closeRawFileDescriptorFunc);
}

auto getMediaByNameFunc = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state = dataContext->addon_->GetResMgr()->GetMediaDataByName(dataContext->resName_.c_str(),
        dataContext->len_, dataContext->mediaData, dataContext->density_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("Failed to get media data in getMediaByNameFunc", false, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsUint8Array;
    return;
};

napi_value ResourceManagerNapiAsyncImpl::GetMediaByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
#if defined(__ARKUI_CROSS__)
    napi_value media = ProcessNameParamV9(env, info, "getMediaByName", getMediaByNameFunc);
#else
    std::string traceVal = "ResourceManagerAddon::GetMediaByName";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_value media = ProcessNameParamV9(env, info, "getMediaByName", getMediaByNameFunc);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    return media;
}

auto getConfigFunc = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsConfig;
};

napi_value ResourceManagerNapiAsyncImpl::GetConfiguration(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetConfiguration", getConfigFunc);
}

auto getDeviceCapFunc = [](napi_env env, void *data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsDeviceCap;
};

napi_value ResourceManagerNapiAsyncImpl::GetDeviceCapability(napi_env env, napi_callback_info info)
{
    return ProcessNoParam(env, info, "GetDeviceCapability", getDeviceCapFunc);
}

auto getRawFileListFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state = dataContext->addon_->GetResMgr()->GetRawFileList(dataContext->path_, dataContext->arrayValue_);
    if (state != SUCCESS) {
        dataContext->SetErrorMsg("GetRawFileList failed state", false, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsArray;
};

napi_value ResourceManagerNapiAsyncImpl::GetRawFileList(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessOnlyIdParam(env, info, "getRawFileList", getRawFileListFunc);
}

auto getColorFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    std::shared_ptr<ResourceManager> resMgr = nullptr;
    int32_t resId = 0;

    if (!ResourceManagerNapiUtils::GetHapResourceManager(dataContext, resMgr, resId)) {
        HiLog::Error(LABEL, "Failed to GetHapResourceManager in getColorFunc");
        return;
    }
    RState state = resMgr->GetColorById(resId, dataContext->colorValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("GetColorById failed state", true, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsColor;
};

napi_value ResourceManagerNapiAsyncImpl::GetColor(napi_env env, napi_callback_info info)
{
    if (ResourceManagerNapiUtils::IsNapiNumber(env, info)) {
        return ProcessIdParamV9(env, info, "getColor", getColorFunc);
    } else if (ResourceManagerNapiUtils::IsNapiObject(env, info)) {
        return ProcessResourceParamV9(env, info, "getColor", getColorFunc);
    } else {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
}

auto getColorByNameFunc = [](napi_env env, void* data) {
    ResMgrDataContext *dataContext = static_cast<ResMgrDataContext*>(data);
    RState state = dataContext->addon_->GetResMgr()->GetColorByName(dataContext->resName_.c_str(),
        dataContext->colorValue_);
    if (state != RState::SUCCESS) {
        dataContext->SetErrorMsg("getColorByNameFunc failed state", false, state);
        return;
    }
    dataContext->createValueFunc_ = ResourceManagerNapiUtils::CreateJsColor;
};

napi_value ResourceManagerNapiAsyncImpl::GetColorByName(napi_env env, napi_callback_info info)
{
    if (!ResourceManagerNapiUtils::IsNapiString(env, info)) {
        ResourceManagerNapiUtils::NapiThrow(env, ERROR_CODE_INVALID_INPUT_PARAMETER);
        return nullptr;
    }
    return ProcessNameParamV9(env, info, "getColorByName", getColorByNameFunc);
}
} // namespace Resource
} // namespace Global
} // namespace OHOS
