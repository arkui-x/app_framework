/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ENVIRONMENT_CALLBACK_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_JS_ENVIRONMENT_CALLBACK_H

#include <map>
#include <memory>

#include "environment_callback.h"
#include "napi/native_api.h"

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class JsEnvironmentCallback : public EnvironmentCallback,
                               public std::enable_shared_from_this<JsEnvironmentCallback> {
public:
    explicit JsEnvironmentCallback(napi_env env);
    ~JsEnvironmentCallback() override = default;

    void OnConfigurationUpdated(const Configuration& config) override;
    void OnMemoryLevel(int32_t level) override;

    int32_t Register(napi_value jsCallback);
    bool UnRegister(int32_t callbackId);
    bool IsEmpty() const;

private:
    void CallConfigurationUpdatedInner(
        const Configuration& config,
        const std::map<int32_t, std::shared_ptr<NativeReference>>& callbacks);
    void CallMemoryLevelInner(
        int32_t level,
        const std::map<int32_t, std::shared_ptr<NativeReference>>& callbacks);

    napi_env env_ = nullptr;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacks_;
    static int32_t serialNumber_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif
