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

#ifndef CALLBACK_MODULE_LOAD_CALLBACK_H
#define CALLBACK_MODULE_LOAD_CALLBACK_H

#include <functional>
#include <unordered_map>

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

using ModuleLoadCallback = std::function<void()>;

class ModuleLoadCallbackManager {
public:
    static ModuleLoadCallbackManager& GetInstance();

    void RegisterCallback(int32_t instanceId, ModuleLoadCallback callback);

    void UnregisterCallback(int32_t instanceId);

    void NotifyAll();

private:
    ModuleLoadCallbackManager() = default;
    std::unordered_map<int32_t, ModuleLoadCallback> callbacks_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // CALLBACK_MODULE_LOAD_CALLBACK_H