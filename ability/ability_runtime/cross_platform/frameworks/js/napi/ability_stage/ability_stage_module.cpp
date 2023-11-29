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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "native_engine/native_engine.h"

extern const char _binary_ability_stage_js_start[];
extern const char _binary_ability_stage_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_ability_stage_js_end[];
extern const char _binary_ability_stage_abc_end[];
#else
extern const char* _binary_ability_stage_js_end;
extern const char* _binary_ability_stage_abc_end;
#endif

extern "C" __attribute__((visibility("default"))) void NAPI_app_ability_AbilityStage_GetJSCode(
    const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_ability_stage_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_ability_stage_js_end - _binary_ability_stage_js_start;
    }
}

// ability_stage JS register
extern "C" __attribute__((visibility("default"))) void NAPI_app_ability_AbilityStage_GetABCCode(
    const char** buf, int* buflen)
{
    if (buf != nullptr) {
        *buf = _binary_ability_stage_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_ability_stage_abc_end - _binary_ability_stage_abc_start;
    }
}

static napi_module_with_js _module = {
    .nm_version = 0,
    .nm_filename = "app/ability/libabilitystage.so/ability_stage.js",
    .nm_modname = "app.ability.AbilityStage",
    .nm_get_js_code =NAPI_app_ability_AbilityStage_GetJSCode,
    .nm_get_abc_code = (GetJSCodeCallback)NAPI_app_ability_AbilityStage_GetABCCode,
};

extern "C" __attribute__((constructor)) void NAPI_app_ability_AbilityStage_AutoRegister()
{
    napi_module_with_js_register(&_module);
}