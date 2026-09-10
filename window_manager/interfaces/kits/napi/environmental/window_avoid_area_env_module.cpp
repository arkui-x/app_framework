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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "window_avoid_area_env_module.h"

extern "C" __attribute__((visibility("default"))) void NapiWindowWindowAvoidAreaEnvGetJSCode(
    const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = BINARY_WINDOW_AVOID_AREA_ENV_JS_START_PTR;
    }
    if (bufLen != nullptr) {
        *bufLen = static_cast<int>(
            BINARY_WINDOW_AVOID_AREA_ENV_JS_END_PTR - BINARY_WINDOW_AVOID_AREA_ENV_JS_START_PTR);
    }
}

extern "C" __attribute__((visibility("default"))) void NapiWindowWindowAvoidAreaEnvGetABCCode(
    const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = BINARY_WINDOW_AVOID_AREA_ENV_ABC_START_PTR;
    }
    if (bufLen != nullptr) {
        *bufLen = static_cast<int>(
            BINARY_WINDOW_AVOID_AREA_ENV_ABC_END_PTR - BINARY_WINDOW_AVOID_AREA_ENV_ABC_START_PTR);
    }
}

static napi_module_with_js g_windowAvoidAreaEnvModule = {
    .nm_version = 0,
    .nm_register_func = nullptr,
    .nm_modname = "window.windowavoidareaenv",
    .nm_filename = "window/libwindowavoidareaenv.so/windowavoidareaenv.js",
    .nm_get_abc_code = (NAPIGetJSCode)NapiWindowWindowAvoidAreaEnvGetABCCode,
    .nm_get_js_code = (NAPIGetJSCode)NapiWindowWindowAvoidAreaEnvGetJSCode,
};

extern "C" __attribute__((constructor)) void NapiWindowWindowAvoidAreaEnvAutoRegister(void)
{
    napi_module_with_js_register(&g_windowAvoidAreaEnvModule);
}
