/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "js_window_manager.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

static napi_module _module = {
    .nm_version = 0,
    .nm_modname = "window",
    .nm_filename = "module/libwindow_napi.so/window.js",
    .nm_register_func = OHOS::Rosen::JsWindowManagerInit,
};

extern "C" __attribute__((constructor)) void NAPI_application_windowmanager_AutoRegister(void)
{
    napi_module_register(&_module);
}
