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

#include "js_ability_delegator_registry.h"

#include <memory>
#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator.h"
#include "js_ability_delegator_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
namespace {
static thread_local std::unique_ptr<NativeReference> reference;

class JsAbilityDelegatorRegistry {
public:
    JsAbilityDelegatorRegistry() = default;
    ~JsAbilityDelegatorRegistry() = default;

    static void Finalizer(NativeEngine *engine, void *data, void *hint)
    {
        HILOG_INFO("enter");
        reference.reset();
        std::unique_ptr<JsAbilityDelegatorRegistry>(static_cast<JsAbilityDelegatorRegistry *>(data));
    }

    static NativeValue *GetAbilityDelegator(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsAbilityDelegatorRegistry *me = CheckParamsAndGetThis<JsAbilityDelegatorRegistry>(engine, info);
        return (me != nullptr) ? me->OnGetAbilityDelegator(*engine, *info) : nullptr;
    }

    static NativeValue *GetArguments(NativeEngine *engine, NativeCallbackInfo *info)
    {
        JsAbilityDelegatorRegistry *me = CheckParamsAndGetThis<JsAbilityDelegatorRegistry>(engine, info);
        return (me != nullptr) ? me->OnGetArguments(*engine, *info) : nullptr;
    }

private:
    NativeValue *OnGetAbilityDelegator(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("enter");
        if (!AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator()) {
            HILOG_ERROR("Failed to get delegator object");
            return engine.CreateNull();
        }

        if (!reference) {
            auto value = CreateJsAbilityDelegator(engine);
            reference.reset(engine.CreateReference(value, 1));
        }

        return reference->Get();
    }

    NativeValue *OnGetArguments(NativeEngine &engine, const NativeCallbackInfo &info)
    {
        HILOG_INFO("enter");

        std::shared_ptr<AppExecFwk::AbilityDelegatorArgs> abilityDelegatorArgs =
            AppExecFwk::AbilityDelegatorRegistry::GetArguments();
        if (!abilityDelegatorArgs) {
            HILOG_ERROR("Failed to get delegator args object");
            return engine.CreateNull();
        }
        return CreateJsAbilityDelegatorArguments(engine, abilityDelegatorArgs);
    }
};
} // namespace

NativeValue *JsAbilityDelegatorRegistryInit(NativeEngine *engine, NativeValue *exportObj)
{
    HILOG_INFO("enter");
    if (engine == nullptr || exportObj == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(exportObj);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::unique_ptr<JsAbilityDelegatorRegistry> jsDelegatorManager =
        std::make_unique<JsAbilityDelegatorRegistry>();
    object->SetNativePointer(jsDelegatorManager.release(), JsAbilityDelegatorRegistry::Finalizer, nullptr);

    const char *moduleName = "JsAbilityDelegatorRegistry";
    BindNativeFunction(*engine, *object, "getAbilityDelegator", moduleName,
        JsAbilityDelegatorRegistry::GetAbilityDelegator);
    BindNativeFunction(*engine, *object, "getArguments", moduleName, JsAbilityDelegatorRegistry::GetArguments);

    object->SetProperty("AbilityLifecycleState", AbilityLifecycleStateInit(engine));

    return engine->CreateUndefined();
}

NativeValue *AbilityLifecycleStateInit(NativeEngine *engine)
{
    HILOG_INFO("enter");

    if (engine == nullptr) {
        HILOG_ERROR("Invalid input parameters");
        return nullptr;
    }

    NativeValue *objValue = engine->CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);

    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    object->SetProperty("UNINITIALIZED", CreateJsValue(*engine,
        static_cast<int32_t>(AbilityLifecycleState::UNINITIALIZED)));
    object->SetProperty("CREATE", CreateJsValue(*engine, static_cast<int32_t>(AbilityLifecycleState::CREATE)));
    object->SetProperty("FOREGROUND", CreateJsValue(*engine, static_cast<int32_t>(AbilityLifecycleState::FOREGROUND)));
    object->SetProperty("BACKGROUND", CreateJsValue(*engine, static_cast<int32_t>(AbilityLifecycleState::BACKGROUND)));
    object->SetProperty("DESTROY", CreateJsValue(*engine, static_cast<int32_t>(AbilityLifecycleState::DESTROY)));

    return objValue;
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
