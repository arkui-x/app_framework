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
#include "resource_manager.h"
#include "running_process_info.h"

class NativeEngine;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
NativeValue* CreateJsAbilityInfo(NativeEngine& engine, const AppExecFwk::AbilityInfo& abilityInfo);
NativeValue* CreateJsApplicationInfo(NativeEngine& engine, const AppExecFwk::ApplicationInfo& applicationInfo);
NativeValue* CreateJsHapModuleInfo(NativeEngine& engine, const AppExecFwk::HapModuleInfo& hapModuleInfo);
NativeValue* CreateJsConfiguration(NativeEngine& engine, const Platform::Configuration& configuration);
NativeValue* CreateJsResourceManager(NativeEngine& engine,
    const std::shared_ptr<Global::Resource::ResourceManager>& resMgr,
    const std::shared_ptr<Platform::Context>& context);
NativeValue* CreateJsProcessRunningInfoArray(
    NativeEngine& engine, const std::vector<Platform::RunningProcessInfo>& infos);
NativeValue* CreateJsProcessRunningInfo(NativeEngine& engine, const Platform::RunningProcessInfo& info);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_DATA_STRUCT_CONVERTER_H
