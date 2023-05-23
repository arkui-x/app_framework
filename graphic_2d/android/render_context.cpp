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

#include "render_context/render_context.h"

#include <sstream>
#include <string>

#include "EGL/egl.h"
#include "rs_trace.h"

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
using GetPlatformDisplayExt = PFNEGLGETPLATFORMDISPLAYEXTPROC;
constexpr int32_t EGL_CONTEXT_CLIENT_VERSION_NUM = 2;
constexpr char CHARACTER_WHITESPACE = ' ';
constexpr const char* CHARACTER_STRING_WHITESPACE = " ";
constexpr const char* EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";

// use functor to call gel*KHR API
static PFNEGLSETDAMAGEREGIONKHRPROC GetEGLSetDamageRegionKHRFunc()
{
    static auto func = reinterpret_cast<PFNEGLSETDAMAGEREGIONKHRPROC>(eglGetProcAddress("eglSetDamageRegionKHR"));
    return func;
}

static bool CheckEglExtension(const char* extensions, const char* extension)
{
    size_t extlen = strlen(extension);
    const char* end = extensions + strlen(extensions);

    while (extensions < end) {
        size_t n = 0;
        /* Skip whitespaces, if any */
        if (*extensions == CHARACTER_WHITESPACE) {
            extensions++;
            continue;
        }
        n = strcspn(extensions, CHARACTER_STRING_WHITESPACE);
        /* Compare strings */
        if (n == extlen && strncmp(extension, extensions, n) == 0) {
            return true; /* Found */
        }
        extensions += n;
    }
    /* Not found */
    return false;
}

static EGLDisplay GetPlatformEglDisplay(EGLenum platform, void* native_display, const EGLint* attrib_list)
{
    return eglGetDisplay((EGLNativeDisplayType)native_display);
}

RenderContext::RenderContext()
    : grContext_(nullptr),
      skSurface_(nullptr),
      nativeWindow_(nullptr),
      eglDisplay_(EGL_NO_DISPLAY),
      eglContext_(EGL_NO_CONTEXT),
      eglSurface_(EGL_NO_SURFACE),
      config_(nullptr),
      mHandler_(nullptr)
{}

RenderContext::~RenderContext()
{
    if (eglDisplay_ == EGL_NO_DISPLAY) {
        return;
    }

    eglDestroyContext(eglDisplay_, eglContext_);
    if (pbufferSurface_ != EGL_NO_SURFACE) {
        eglDestroySurface(eglDisplay_, pbufferSurface_);
    }
    eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(eglDisplay_);
    eglReleaseThread();

    eglDisplay_ = EGL_NO_DISPLAY;
    eglContext_ = EGL_NO_CONTEXT;
    eglSurface_ = EGL_NO_SURFACE;
    pbufferSurface_ = EGL_NO_SURFACE;
    grContext_ = nullptr;
    skSurface_ = nullptr;
    mHandler_ = nullptr;
}

void RenderContext::CreatePbufferSurface()
{
    const char* extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);

    if ((extensions != nullptr) &&
        (!CheckEglExtension(extensions, EGL_KHR_SURFACELESS_CONTEXT)) &&
        (pbufferSurface_ == EGL_NO_SURFACE)) {
        EGLint attribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE};
        pbufferSurface_ = eglCreatePbufferSurface(eglDisplay_, config_, attribs);
        if (pbufferSurface_ == EGL_NO_SURFACE) {
            ROSEN_LOGE("Failed to create pbuffer surface");
            return;
        }
    }
}

void RenderContext::InitializeEglContext()
{
    if (IsEglContextReady()) {
        return;
    }

    ROSEN_LOGD("RenderContext Creating EGLContext!!!");
    eglDisplay_ = GetPlatformEglDisplay(0, EGL_DEFAULT_DISPLAY, NULL);
    if (eglDisplay_ == EGL_NO_DISPLAY) {
        ROSEN_LOGW("RenderContext Failed to create EGLDisplay gl errno : %x", eglGetError());
        return;
    }

    EGLint major, minor;
    if (eglInitialize(eglDisplay_, &major, &minor) == EGL_FALSE) {
        ROSEN_LOGE("RenderContext Failed to initialize EGLDisplay");
        return;
    }

    if (eglBindAPI(EGL_OPENGL_ES_API) == EGL_FALSE) {
        ROSEN_LOGE("RenderContext Failed to bind OpenGL ES API");
        return;
    }

    unsigned int ret;
    EGLint count;
    EGLint config_attribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8, EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, EGL_NONE };

    ret = eglChooseConfig(eglDisplay_, config_attribs, &config_, 1, &count);
    if (!(ret && static_cast<unsigned int>(count) >= 1)) {
        ROSEN_LOGE("RenderContext Failed to eglChooseConfig");
        return;
    }

    static const EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, EGL_CONTEXT_CLIENT_VERSION_NUM, EGL_NONE };

    eglContext_ = eglCreateContext(eglDisplay_, config_, EGL_NO_CONTEXT, context_attribs);
    if (eglContext_ == EGL_NO_CONTEXT) {
        ROSEN_LOGE("RenderContext Failed to create egl context %x", eglGetError());
        return;
    }
    CreatePbufferSurface();
    if (!eglMakeCurrent(eglDisplay_, pbufferSurface_, pbufferSurface_, eglContext_)) {
        ROSEN_LOGE("RenderContext Failed to make current on surface %p, error is %x", pbufferSurface_, eglGetError());
        return;
    }

    ROSEN_LOGD("RenderContext Create EGL context successfully, version %d.%d", major, minor);
}

void RenderContext::MakeCurrent(EGLSurface surface, EGLContext context)
{
    if (surface == EGL_NO_SURFACE) {
        if (!eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
            ROSEN_LOGE("Failed to make current on surface %p, error is %x", surface, eglGetError());
        }
    } else {
        if (!eglMakeCurrent(eglDisplay_, surface, surface, eglContext_)) {
            ROSEN_LOGE("Failed to make current on surface %p, error is %x", surface, eglGetError());
        }
    }
}

void RenderContext::ShareMakeCurrent(EGLContext shareContext)
{
    eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, shareContext);
}

void RenderContext::ShareMakeCurrentNoSurface(EGLContext shareContext)
{
    eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, shareContext);
}

void RenderContext::MakeSelfCurrent()
{
    eglMakeCurrent(eglDisplay_, eglSurface_, eglSurface_, eglContext_);
}

EGLContext RenderContext::CreateShareContext()
{
    std::unique_lock<std::mutex> lock(shareContextMutex_);
    static const EGLint context_attribs[] = {EGL_CONTEXT_CLIENT_VERSION, EGL_CONTEXT_CLIENT_VERSION_NUM, EGL_NONE};
    auto eglShareContext = eglCreateContext(eglDisplay_, config_, eglContext_, context_attribs);
    lock.unlock();
    return eglShareContext;
}

void RenderContext::SwapBuffers(EGLSurface surface) const
{
    RS_TRACE_FUNC();
    if (!eglSwapBuffers(eglDisplay_, surface)) {
        ROSEN_LOGE("Failed to SwapBuffers on surface %p, error is %x", surface, eglGetError());
    } else {
        ROSEN_LOGD("SwapBuffers successfully, surface is %p", surface);
    }
}

void RenderContext::DestroyEGLSurface(EGLSurface surface)
{
    if (!eglDestroySurface(eglDisplay_, surface)) {
        ROSEN_LOGE("Failed to DestroyEGLSurface surface %p, error is %x", surface, eglGetError());
    }
}

EGLSurface RenderContext::CreateEGLSurface(EGLNativeWindowType eglNativeWindow)
{
    if (!IsEglContextReady()) {
        ROSEN_LOGE("EGL context has not initialized");
        return EGL_NO_SURFACE;
    }
    nativeWindow_ = eglNativeWindow;

    eglMakeCurrent(eglDisplay_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    EGLSurface surface = eglCreateWindowSurface(eglDisplay_, config_, nativeWindow_, NULL);
    if (surface == EGL_NO_SURFACE) {
        ROSEN_LOGW("Failed to create eglsurface!!! %x", eglGetError());
        return EGL_NO_SURFACE;
    }

    ROSEN_LOGD("CreateEGLSurface: %p", surface);

    eglSurface_ = surface;
    return surface;
}

bool RenderContext::SetUpGrContext()
{
    if (grContext_ != nullptr) {
        ROSEN_LOGD("grContext has already created!!");
        return true;
    }

    sk_sp<const GrGLInterface> glInterface(GrGLCreateNativeInterface());
    if (glInterface.get() == nullptr) {
        ROSEN_LOGE("SetUpGrContext failed to make native interface");
        return false;
    }

    GrContextOptions options;
    options.fGpuPathRenderers &= ~GpuPathRenderers::kCoverageCounting;
    options.fPreferExternalImagesOverES3 = true;
    options.fDisableDistanceFieldPaths = true;

    sk_sp<GrContext> grContext(GrContext::MakeGL(std::move(glInterface), options));
    if (grContext == nullptr) {
        ROSEN_LOGE("SetUpGrContext grContext is null");
        return false;
    }
    grContext_ = std::move(grContext);
    return true;
}

sk_sp<SkSurface> RenderContext::AcquireSurface(int width, int height)
{
    if (!SetUpGrContext()) {
        ROSEN_LOGE("GrContext is not ready!!!");
        return nullptr;
    }

    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = 0;
    framebufferInfo.fFormat = GL_RGBA8;

    SkColorType colorType = kRGBA_8888_SkColorType;
    /* sampleCnt and stencilBits for GrBackendRenderTarget */
    const int stencilBufferSize = 8;
    GrBackendRenderTarget backendRenderTarget(width, height, 0, stencilBufferSize, framebufferInfo);
    SkSurfaceProps surfaceProps = SkSurfaceProps::kLegacyFontHost_InitType;
    sk_sp<SkColorSpace> skColorSpace = nullptr;

    skSurface_ = SkSurface::MakeFromBackendRenderTarget(
        GetGrContext(), backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType, skColorSpace, &surfaceProps);
    if (skSurface_ == nullptr) {
        ROSEN_LOGW("skSurface is nullptr");
        return nullptr;
    }

    ROSEN_LOGD("CreateCanvas successfully!!! (%p)", skSurface_->getCanvas());
    return skSurface_;
}

void RenderContext::RenderFrame()
{
    RS_TRACE_FUNC();
    // flush commands
    if (skSurface_->getCanvas() != nullptr) {
        ROSEN_LOGD("RenderFrame: Canvas is %p", skSurface_->getCanvas());
        skSurface_->getCanvas()->flush();
    } else {
        ROSEN_LOGW("canvas is nullptr!!!");
    }
}

EGLint RenderContext::QueryEglBufferAge()
{
    if ((eglDisplay_ == nullptr) || (eglSurface_ == nullptr)) {
        ROSEN_LOGE("eglDisplay or eglSurface is nullptr");
        return EGL_UNKNOWN;
    }
    EGLint bufferAge = EGL_UNKNOWN;
    EGLBoolean ret = eglQuerySurface(eglDisplay_, eglSurface_, EGL_BUFFER_AGE_KHR, &bufferAge);
    if (ret == EGL_FALSE) {
        ROSEN_LOGE("eglQuerySurface is failed");
        return EGL_UNKNOWN;
    }
    return bufferAge;
}

void RenderContext::DamageFrame(int32_t left, int32_t top, int32_t width, int32_t height)
{
}

void RenderContext::DamageFrame(const std::vector<RectI> &rects)
{
}

void RenderContext::ClearRedundantResources()
{
    RS_TRACE_FUNC();
    if (grContext_ != nullptr) {
        ROSEN_LOGD("grContext clear redundant resources");
        grContext_->flush();
        // GPU resources that haven't been used in the past 10 seconds
        grContext_->purgeResourcesNotUsedInMs(std::chrono::seconds(10));
    }
}

RenderContextFactory& RenderContextFactory::GetInstance()
{
    static RenderContextFactory rf;
    return rf;
}
} // namespace Rosen
} // namespace OHOS
