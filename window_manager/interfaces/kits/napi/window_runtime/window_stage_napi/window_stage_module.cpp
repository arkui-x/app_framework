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

extern const char _binary_window_stage_js_start[];
extern const char _binary_window_stage_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_window_stage_js_end[];
extern const char _binary_window_stage_abc_end[];
#else
extern const char* _binary_window_stage_js_end;
extern const char* _binary_window_stage_abc_end;
#endif


extern "C" __attribute__((visibility("default")))
void NAPI_application_WindowStage_GetJSCode(const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_window_stage_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_window_stage_js_end - _binary_window_stage_js_start;
    }
}

// window_stage JS register
extern "C" __attribute__((visibility("default")))
void NAPI_application_WindowStage_GetABCCode(const char **buf, int *buflen)
{
    if (buf != nullptr) {
        *buf = _binary_window_stage_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_window_stage_abc_end - _binary_window_stage_abc_start;
    }
}

static napi_module_with_js _module = {
    .nm_version = 0,
    .nm_register_func = nullptr,
    .nm_modname = "application.WindowStage",
    .nm_filename = "application/libwindowstage.so/window_stage.js",
    .nm_get_abc_code = (NAPIGetJSCode)NAPI_application_WindowStage_GetABCCode,
    .nm_get_js_code = (NAPIGetJSCode)NAPI_application_WindowStage_GetJSCode,
};

extern "C" __attribute__((constructor)) void NAPI_application_WindowStage_AutoRegister(void)
{
    napi_module_with_js_register(&_module);
}
