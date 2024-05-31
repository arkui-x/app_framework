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

#ifndef OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H
#define OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H

#include "ability_info.h"
#include "configuration.h"
#include "context.h"
#include "hap_module_info.h"
#include "launch_param.h"
#include "resource_manager.h"
#include "running_process_info.h"
#include "napi/native_api.h"


namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsAbilityInfo(napi_env env, const AppExecFwk::AbilityInfo& abilityInfo);
napi_value CreateJsApplicationInfo(napi_env env, const AppExecFwk::ApplicationInfo &applicationInfo);
napi_value CreateJsHapModuleInfo(napi_env env, const AppExecFwk::HapModuleInfo& hapModuleInfo);
napi_value CreateJsConfiguration(napi_env env, const Platform::Configuration& configuration);
napi_value CreateJsResourceManager(napi_env env,
    const std::shared_ptr<Global::Resource::ResourceManager>& resMgr,
    const std::shared_ptr<Platform::Context>& context);
napi_value CreateJsProcessRunningInfoArray(
    napi_env env, const std::vector<Platform::RunningProcessInfo>& infos);
napi_value CreateJsProcessRunningInfo(napi_env env, const Platform::RunningProcessInfo& info);
napi_value CreateJsLaunchParam(napi_env env, const Platform::LaunchParam& launchParam);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H
