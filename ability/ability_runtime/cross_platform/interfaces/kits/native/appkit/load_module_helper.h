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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_LOAD_MODULE_HELPER_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_LOAD_MODULE_HELPER_H

#include <memory>
#include <string>

#include "hap_module_info.h"
#include "napi/native_api.h"
#include "native_reference.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class LoadModuleHelper final {
public:
    LoadModuleHelper(const LoadModuleHelper&) = delete;
    LoadModuleHelper& operator=(const LoadModuleHelper&) = delete;

    static void LoadModule(const std::unique_ptr<Runtime>& runtime,
        const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo, const std::string& entryPath);

    static void CallObjectMethod(napi_env env, NativeReference* moduleRef, const std::string& methodName);
private:
    LoadModuleHelper() = delete;
    ~LoadModuleHelper() = delete;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS

#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_LOAD_MODULE_HELPER_H
