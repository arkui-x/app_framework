/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025-2025. All rights reserved.
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

extern const char _binary_module_loader_js_start[];
extern const char _binary_module_loader_abc_start[];

#if !defined(IOS_PLATFORM)
extern const char _binary_module_loader_js_end[];
extern const char _binary_module_loader_abc_end[];
#else
extern const char* _binary_module_loader_js_end;
extern const char* _binary_module_loader_abc_end;
#endif

extern "C" __attribute__((visibility("default"))) void NAPI_Module_Loader_GetJSCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_module_loader_js_start;
    }
    if (bufLen != nullptr) {
        *bufLen = _binary_module_loader_js_end - _binary_module_loader_js_start;
    }
}

extern "C" __attribute__((visibility("default"))) void NAPI_Module_Loader_GetABCCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_module_loader_abc_start;
    }
    if (bufLen != nullptr) {
        *bufLen = _binary_module_loader_abc_end - _binary_module_loader_abc_start;
    }
}

static napi_module_with_js _module = {
    .nm_version = 0,
    .nm_modname = "ModuleLoader",
    .nm_filename = "app/ability/libmoduleload.so/module_loader.js",
    .nm_get_js_code = NAPI_Module_Loader_GetJSCode,
    .nm_get_abc_code = NAPI_Module_Loader_GetABCCode,
};

extern "C" __attribute__((constructor)) void NAPI_Module_Loader_AutoRegister()
{
    napi_module_with_js_register(&_module);
}
