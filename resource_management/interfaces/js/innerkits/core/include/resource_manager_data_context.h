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
#ifndef RESOURCE_MANAGER_DATA_H
#define RESOURCE_MANAGER_DATA_H
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "resource_manager.h"
#include "resource_manager_addon.h"
#include "hilog/log_cpp.h"
namespace OHOS {
namespace Global {
namespace Resource {
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "ResourceManagerJs" };
using namespace OHOS::HiviewDFX;
struct ResMgrDataContext {
    napi_async_work work_;

    std::string bundleName_;
    int32_t resId_;
    int32_t param_;

    std::string path_;
    std::string resName_;
    int iValue_;
    float fValue_;
    bool bValue_;
    uint32_t colorValue_;

    typedef napi_value (*CreateNapiValue)(napi_env env, ResMgrDataContext &context);
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
    uint32_t iconType_;
    uint32_t symbolValue_;

    ResourceManager::RawFileDescriptor descriptor_;
    std::shared_ptr<ResourceManagerAddon> addon_;
    std::shared_ptr<ResourceManager> resMgr_;
    std::shared_ptr<ResourceManager::Resource> resource_;
    std::shared_ptr<ResConfig> overrideResConfig_;

    ResMgrDataContext() : work_(nullptr), resId_(0), param_(0), iValue_(0), fValue_(0.0f), bValue_(false),
        colorValue_(0), createValueFunc_(nullptr), len_(0), deferred_(nullptr), callbackRef_(nullptr), success_(true),
        errCode_(0), density_(0), iconType_(0), symbolValue_(0) {}

    void SetErrorMsg(const std::string &msg, bool withResId = false, int32_t errCode = 0)
    {
        errMsg_ = msg;
        success_ = false;
        errCode_ = errCode;
        if (withResId) {
            HiLog::Error(LABEL, "%{public}s id = %{public}d", msg.c_str(), resId_);
        } else {
            HiLog::Error(LABEL, "%{public}s name = %{public}s", msg.c_str(), resName_.c_str());
        }
    }

    static std::shared_ptr<ResourceManagerAddon> GetResourceManagerAddon(napi_env env, napi_callback_info info)
    {
        GET_PARAMS(env, info, 2); // 2 means get two params

        std::shared_ptr<ResourceManagerAddon> *addonPtr = nullptr;
        napi_status status = napi_unwrap(env, thisVar, reinterpret_cast<void **>(&addonPtr));
        if (status != napi_ok) {
            HiLog::Error(LABEL, "Failed to unwrap");
            return nullptr;
        }
        return *addonPtr;
    }
};
} // namespace Resource
} // namespace Global
} // namespace OHOS
#endif