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

#include "preload_manager.h"

#include "hilog.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<PreloadManager> PreloadManager::instance_ = nullptr;
std::mutex PreloadManager::mutex_;
std::shared_ptr<PreloadManager> PreloadManager::GetInstance()
{
    if (instance_ == nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            instance_ = std::make_shared<PreloadManager>();
        }
    }

    return instance_;
}

void PreloadManager::PreloadModule(
    const std::unique_ptr<Runtime>& runtime, const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    if (!runtime) {
        HILOG_ERROR("Preload module failed, runtime is nullptr");
        return;
    }

    if (!abilityInfo) {
        HILOG_ERROR("Preload module failed, abilityInfo is nullptr");
        return;
    }

    std::string moduleName(abilityInfo->moduleName);
    std::string modulePath;
    bool esModule = abilityInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    auto abilityBuffer =
        StageAssetManager::GetInstance()->GetModuleAbilityBuffer(moduleName, abilityInfo->name, modulePath, esModule);
    moduleName.append("::").append(abilityInfo->name);
    if (esModule) {
        if (abilityInfo->srcEntrance.empty()) {
            HILOG_ERROR("PreloadManager::PreloadModule abilityInfo srcEntrance is empty");
            return;
        }
        modulePath = abilityInfo->package;
        modulePath.append("/");
        modulePath.append(abilityInfo->srcEntrance);
        modulePath.erase(modulePath.rfind("."));
        modulePath.append(".abc");
    } else {
        HILOG_ERROR("PreloadManager::PreloadModule the module is not esModule");
    }
    static_cast<JsRuntime&>(*runtime).LoadModule(
        moduleName, modulePath, abilityBuffer, abilityInfo->srcEntrance, esModule);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
