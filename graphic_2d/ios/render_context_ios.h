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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RENDER_CONTEXT_IOS_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RENDER_CONTEXT_IOS_H

// For temporary ios build
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;
typedef unsigned int EGLBoolean;
typedef void *EGLDisplay;

typedef void *EGLNativeDisplayType;
typedef void *EGLNativePixmapType;
typedef void *EGLNativeWindowType;
typedef long long EGLint;

#if defined(__cplusplus)
#define EGL_CAST(type, value) (static_cast<type>(value))
#else
#define EGL_CAST(type, value) ((type) (value))
#endif
#define EGL_NO_CONTEXT                    EGL_CAST(EGLContext,0)
#define EGL_NO_DISPLAY                    EGL_CAST(EGLDisplay,0)
#define EGL_NO_SURFACE                    EGL_CAST(EGLSurface,0)

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RENDER_CONTEXT_IOS_H
