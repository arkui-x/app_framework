/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "js_bundle_manager.h"

#include "bundle_container.h"
#include "js_runtime_utils.h"
#include "napi_common_bundle_info.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace AAFwk {
using namespace OHOS::AbilityRuntime;
using namespace AbilityRuntime::Platform;
using namespace AppExecFwk;
constexpr size_t ARGS_POS_ZERO = 0;
constexpr size_t ARGS_POS_ONE = 1;
constexpr size_t ARGS_POS_TWO = 2;

void JsBundleManager::Finalizer(napi_env env, void* data, void* hint)
{
    std::unique_ptr<JsBundleManager>(static_cast<JsBundleManager*>(data));
}

napi_value JsBundleManager::GetBundleInfoForSelfSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsBundleManager, OnGetBundleInfoForSelfSync);
}

napi_value JsBundleManager::GetBundleInfoForSelf(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsBundleManager, OnGetBundleInfoForSelf);
}

napi_value JsBundleManager::OnGetBundleInfoForSelfSync(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGS_POS_ZERO) {
        LOGE("param count invalid");
        return NapiCommonBundleInfo::ThrowJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS));
    }
    int32_t flag = 0;
    if (!NapiCommonBundleInfo::ParseInt(env, info.argv[ARGS_POS_ZERO], flag)) {
        LOGE("parseInt invalid");
        return NapiCommonBundleInfo::ThrowJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS));
    }
    std::shared_ptr<BundleContainer> bundleContainer = std::make_shared<BundleContainer>();
    if (bundleContainer == nullptr) {
        return CreateJsUndefined(env);
    }
    auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
    if (moduleList.empty()) {
        return CreateJsUndefined(env);
    }
    bundleContainer->LoadBundleInfos(moduleList);
    std::shared_ptr<BundleInfo> bundleInfo = bundleContainer->GetBundleInfoV9(flag);
    napi_value result = nullptr;
    if (bundleInfo != nullptr) {
        NAPI_CALL(env, napi_create_object(env, &result));
        NapiCommonBundleInfo::ConvertBundleInfo(env, *bundleInfo, result, flag);
    }
    return result;
}

napi_value JsBundleManager::OnGetBundleInfoForSelf(napi_env env, NapiCallbackInfo& info)
{
    if (info.argc == ARGS_POS_ZERO) {
        HILOG_ERROR("param count invalid");
        return NapiCommonBundleInfo::ThrowJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS));
    }
    int32_t flag = 0;
    if (!NapiCommonBundleInfo::ParseInt(env, info.argv[ARGS_POS_ZERO], flag)) {
        HILOG_ERROR("parseInt invalid");
        return NapiCommonBundleInfo::ThrowJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS));
    }
    auto complete = [flag](napi_env env, NapiAsyncTask& task, int32_t status) {
        std::shared_ptr<BundleContainer> bundleContainer = std::make_shared<BundleContainer>();
        if (bundleContainer == nullptr) {
            task.Reject(env,
                CreateJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS), "callback is nullptr"));
            return;
        }
        auto moduleList = StageAssetManager::GetInstance()->GetModuleJsonBufferList();
        if (moduleList.empty()) {
            task.Reject(env,
                CreateJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS), "moduleList is nullptr"));
            return;
        }
        bundleContainer->LoadBundleInfos(moduleList);
        std::shared_ptr<BundleInfo> bundleInfo = bundleContainer->GetBundleInfoV9(flag);
        if (bundleInfo != nullptr) {
            napi_value res = nullptr;
            napi_create_object(env, &res);
            NapiCommonBundleInfo::ConvertBundleInfo(env, *bundleInfo, res, flag);
            task.ResolveWithNoError(env, res);
        } else {
            task.Reject(env,
                CreateJsError(env, static_cast<int32_t>(ERROR_CODE::INCORRECT_PARAMETERS), "bundleInfo is nullptr"));
        }
    };
    napi_value lastParam = (info.argc == ARGS_POS_TWO) ? info.argv[ARGS_POS_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsBundleManager::OnGetBundleInfoForSelf", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}
} // namespace AAFwk
} // namespace OHOS