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
 
#include "native_engine/native_engine.h"
#include "js_ability_delegator_registry.h"

extern "C" __attribute__((constructor)) void NAPI_application_AbilityDelegatorRegistry_AutoRegister()
{
    auto moduleManager = NativeModuleManager::GetInstance();
    NativeModule newModuleInfo = {
        .name = "app.ability.abilityDelegatorRegistry",
        .fileName = "app/ability/libabilitydelegator.so/ability_delegator_registry.js",
        .registerCallback = OHOS::AbilityDelegatorJs::JsAbilityDelegatorRegistryInit,
    };
    moduleManager->Register(&newModuleInfo);
}
