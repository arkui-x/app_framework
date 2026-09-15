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

#ifndef OHOS_WINDOW_AVOID_AREA_ENV_MODULE_H
#define OHOS_WINDOW_AVOID_AREA_ENV_MODULE_H

extern "C" {
#if defined(IOS_PLATFORM)
extern const char BINARY_WINDOW_AVOID_AREA_ENV_JS_START
    asm("__binary_WindowAvoidAreaEnv_js_start");
extern const char BINARY_WINDOW_AVOID_AREA_ENV_ABC_START
    asm("__binary_windowavoidareaenv_abc_start");
extern const char* BINARY_WINDOW_AVOID_AREA_ENV_JS_END
    asm("__binary_WindowAvoidAreaEnv_js_end");
extern const char* BINARY_WINDOW_AVOID_AREA_ENV_ABC_END
    asm("__binary_windowavoidareaenv_abc_end");
#else
extern const char BINARY_WINDOW_AVOID_AREA_ENV_JS_START
    asm("_binary_WindowAvoidAreaEnv_js_start");
extern const char BINARY_WINDOW_AVOID_AREA_ENV_ABC_START
    asm("_binary_windowavoidareaenv_abc_start");
extern const char BINARY_WINDOW_AVOID_AREA_ENV_JS_END
    asm("_binary_WindowAvoidAreaEnv_js_end");
extern const char BINARY_WINDOW_AVOID_AREA_ENV_ABC_END
    asm("_binary_windowavoidareaenv_abc_end");
#endif
}

#define BINARY_WINDOW_AVOID_AREA_ENV_JS_START_PTR (&BINARY_WINDOW_AVOID_AREA_ENV_JS_START)
#define BINARY_WINDOW_AVOID_AREA_ENV_ABC_START_PTR (&BINARY_WINDOW_AVOID_AREA_ENV_ABC_START)
#if defined(IOS_PLATFORM)
#define BINARY_WINDOW_AVOID_AREA_ENV_JS_END_PTR BINARY_WINDOW_AVOID_AREA_ENV_JS_END
#define BINARY_WINDOW_AVOID_AREA_ENV_ABC_END_PTR BINARY_WINDOW_AVOID_AREA_ENV_ABC_END
#else
#define BINARY_WINDOW_AVOID_AREA_ENV_JS_END_PTR (&BINARY_WINDOW_AVOID_AREA_ENV_JS_END)
#define BINARY_WINDOW_AVOID_AREA_ENV_ABC_END_PTR (&BINARY_WINDOW_AVOID_AREA_ENV_ABC_END)
#endif

#endif // OHOS_WINDOW_AVOID_AREA_ENV_MODULE_H
