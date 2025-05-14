/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

/**
 * @addtogroup NativeWindow
 *
 * @brief Provides the native window capability for connection to the EGL.
 *
 * @syscap SystemCapability.Graphic.Graphic2D.NativeWindow
 */

/**
 * @file external_window.h
 *
 * @brief Defines the functions for obtaining and using a native window.
 *
 */
 
#ifndef NDK_INCLUDE_EXTERNAL_NATIVE_WINDOW_H_
#define NDK_INCLUDE_EXTERNAL_NATIVE_WINDOW_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct NativeWindow OHNativeWindow;
/**
 * @brief Creates an <b>OHNativeWindow</b> instance.\n
 *
 * @syscap SystemCapability.Graphic.Graphic2D.NativeWindow
 * @param surfaceId Indicates the surfaceId to a surface.
 * @param window indicates the pointer to an <b>OHNativeWindow</b> instance.
 * @return Returns an error code, 0 is Success, otherwise, failed.
 */
int32_t OH_NativeWindow_CreateNativeWindowFromSurfaceId(uint64_t surfaceId, OHNativeWindow **window);
#ifdef __cplusplus
}
#endif
#endif