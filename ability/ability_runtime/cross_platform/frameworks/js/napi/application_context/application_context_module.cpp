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

extern const char _binary_application_context_js_start[];
extern const char _binary_application_context_abc_start[];
#if !defined(IOS_PLATFORM)
extern const char _binary_application_context_js_end[];
extern const char _binary_application_context_abc_end[];
#else
extern const char* _binary_application_context_js_end;
extern const char* _binary_application_context_abc_end;
#endif

extern "C" __attribute__((visibility("default")))
void NAPI_application_ApplicationContext_GetJSCode(const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_application_context_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_application_context_js_end - _binary_application_context_js_start;
    }
}

// context JS register
extern "C" __attribute__((visibility("default")))
void NAPI_application_ApplicationContext_GetABCCode(const char **buf, int *buflen)
{
    if (buf != nullptr) {
        *buf = _binary_application_context_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_application_context_abc_end - _binary_application_context_abc_start;
    }
}

static napi_module_with_js _module = {
    .nm_version = 0,
    .nm_filename = "application/libapplication_applicationcontext_napi.so/application_context.js",
    .nm_modname = "application.ApplicationContext",
    .nm_get_js_code = (GetJSCodeCallback)NAPI_application_ApplicationContext_GetJSCode,
    .nm_get_abc_code = (GetJSCodeCallback)NAPI_application_ApplicationContext_GetABCCode,
};

extern "C" __attribute__((constructor))
void NAPI_application_ApplicationContext_AutoRegister()
{
    napi_module_with_js_register(&_module);
}