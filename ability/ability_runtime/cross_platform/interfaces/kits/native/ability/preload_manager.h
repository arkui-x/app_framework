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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_PRELOAD_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_PRELOAD_MANAGER_H

#include <memory>
#include <string>

#include "ability.h"
#include "ability_info.h"

namespace OHOS {
namespace AbilityRuntime {
class JsRuntime;
namespace Platform {

class PreloadManager {
public:
    PreloadManager() = default;
    ~PreloadManager() = default;

    static std::shared_ptr<PreloadManager> GetInstance();
    void PreloadModule(
        const std::unique_ptr<Runtime>& runtime, const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo);

private:
    static std::shared_ptr<PreloadManager> instance_;
    static std::mutex mutex_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_PRELOAD_MANAGER_H
