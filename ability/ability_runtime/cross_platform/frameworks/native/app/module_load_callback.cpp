/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "module_load_callback.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
ModuleLoadCallbackManager& ModuleLoadCallbackManager::GetInstance()
{
    static ModuleLoadCallbackManager instance;
    return instance;
}

void ModuleLoadCallbackManager::RegisterCallback(int32_t instanceId, ModuleLoadCallback callback)
{
    callbacks_[instanceId] = std::move(callback);
}

void ModuleLoadCallbackManager::UnregisterCallback(int32_t instanceId)
{
    callbacks_.erase(instanceId);
}

void ModuleLoadCallbackManager::NotifyAll()
{
    for (auto& pair : callbacks_) {
        if (pair.second) {
            pair.second();
        }
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS