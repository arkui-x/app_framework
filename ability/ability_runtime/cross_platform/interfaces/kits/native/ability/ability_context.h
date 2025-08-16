/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITYKIT_ABILITY_CONTEXT_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITYKIT_ABILITY_CONTEXT_H

#include "context.h"
#include "native_engine/native_reference.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
using ErrCode = int32_t;
using RuntimeTask = std::function<void(int, const AAFwk::Want&)>;

class AbilityContext : public Context {
public:
    virtual ~AbilityContext() = default;

    virtual int32_t StartAbility(const AAFwk::Want& want, int requestCode) = 0;

    virtual std::shared_ptr<AppExecFwk::AbilityInfo> GetAbilityInfo() const = 0;

    virtual int32_t TerminateSelf() = 0;

    virtual int32_t CloseAbility() = 0;

    virtual std::shared_ptr<Configuration> GetConfiguration() override;

    virtual std::shared_ptr<NativeReference> GetJsWindowStage() = 0;

    virtual void SetJsWindowStage(const std::shared_ptr<NativeReference> jsWindowStage) = 0;

    virtual int32_t StartAbilityForResult(const AAFwk::Want& want, int32_t requestCode, RuntimeTask&& task) = 0;

    virtual int32_t TerminateAbilityWithResult(const AAFwk::Want& want, int32_t resultCode) = 0;

    virtual void OnAbilityResult(int32_t requestCode, int32_t resultCode, const AAFwk::Want& resultWant) = 0;

    virtual int32_t ReportDrawnCompleted() = 0;

    using SelfType = AbilityContext;
    static const size_t CONTEXT_TYPE_ID;

protected:
    bool IsContext(size_t contextTypeId) override
    {
        return contextTypeId == CONTEXT_TYPE_ID || Context::IsContext(contextTypeId);
    }
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITYKIT_ABILITY_CONTEXT_H