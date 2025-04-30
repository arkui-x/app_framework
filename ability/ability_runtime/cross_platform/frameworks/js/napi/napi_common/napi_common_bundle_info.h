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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_BUNDLE_INFO_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_BUNDLE_INFO_H

#include "ability_info.h"
#include "bundle_info.h"
#include "hap_module_info.h"
#include "inner_bundle_info.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AppExecFwk {
enum class ERROR_CODE : int32_t {
    INCORRECT_PARAMETERS = 401,
};

class NapiCommonBundleInfo {
public:
    static napi_value ThrowJsError(napi_env env, int32_t errCode);
    static bool ParseInt(napi_env env, napi_value args, int32_t& param);
    static void ConvertWindowSize(napi_env env, const AbilityInfo& abilityInfo, napi_value value);
    static void ConvertMetadata(napi_env env, const Metadata& metadata, napi_value value);
    static void ConvertApplicationInfo(napi_env env, napi_value objAppInfo, const ApplicationInfo& appInfo);
    static void ConvertAbilityInfo(napi_env env, const AbilityInfo& abilityInfo, napi_value objAbilityInfo);
    static void ConvertResource(napi_env env, const Resource& resource, napi_value objResource);
    static void ConvertModuleMetaInfos(
        napi_env env, const std::map<std::string, std::vector<Metadata>>& metadata, napi_value objInfos);
    static void ConvertHapModuleInfo(
        napi_env env, const HapModuleInfo& hapModuleInfo, napi_value objHapModuleInfo, int32_t flags);
    static void ConvertRequestPermissionUsedScene(
        napi_env env, const RequestPermissionUsedScene& requestPermissionUsedScene, napi_value result);
    static void ConvertSignatureInfo(napi_env env, const SignatureInfo& signatureInfo, napi_value value);
    static void ConvertRequestPermission(napi_env env, const RequestPermission& requestPermission, napi_value result);
    static void ConvertBundleInfo(napi_env env, const BundleInfo& bundleInfo, napi_value objBundleInfo, int32_t flags);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_NAPI_COMMON_BUNDLE_INFO_H
