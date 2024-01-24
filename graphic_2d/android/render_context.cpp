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
#include <sys/resource.h>

#include "EGL/egl.h"
#include "rs_trace.h"

#include "platform/common/rs_log.h"
#ifdef RS_ENABLE_GL
#include "platform/common/rs_system_properties.h"
#endif

namespace OHOS {
namespace Rosen {
using GetPlatformDisplayExt = PFNEGLGETPLATFORMDISPLAYEXTPROC;
constexpr int32_t EGL_CONTEXT_CLIENT_VERSION_NUM = 2;
constexpr char CHARACTER_WHITESPACE = ' ';
constexpr const char* CHARACTER_STRING_WHITESPACE = " ";
constexpr const char* EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";

constexpr size_t DEFAULT_SKIA_CACHE_SIZE        = 96 * (1 << 20);
constexpr int DEFAULT_SKIA_CACHE_COUNT          = 2 * (1 << 12);

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
#ifndef USE_ROSEN_DRAWING
    : grContext_(nullptr),
      skSurface_(nullptr),
#else
    : drGPUContext_(nullptr),
      surface_(nullptr),
#endif
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
#ifndef USE_ROSEN_DRAWING
    grContext_ = nullptr;
    skSurface_ = nullptr;
#else
    drGPUContext_ = nullptr;
    surface_ = nullptr;
#endif
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

#ifndef USE_ROSEN_DRAWING
bool RenderContext::SetUpGrContext(sk_sp<GrDirectContext> skContext)
{
    if (grContext_ != nullptr) {
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

#if defined(NEW_SKIA)
    sk_sp<GrDirectContext> grContext(GrDirectContext::MakeGL(std::move(glInterface), options));
#else
    sk_sp<GrContext> grContext(GrContext::MakeGL(std::move(glInterface), options));
#endif

    if (grContext == nullptr) {
        ROSEN_LOGE("SetUpGrContext grContext is null");
        return false;
    }
    int maxResources = 0;
    size_t maxResourcesSize = 0;
    int cacheLimitsTimes = 2;
    grContext->getResourceCacheLimits(&maxResources, &maxResourcesSize);
    if (maxResourcesSize > 0) {
        grContext->setResourceCacheLimits(cacheLimitsTimes * maxResources, cacheLimitsTimes *
            std::fmin(maxResourcesSize, DEFAULT_SKIA_CACHE_SIZE));
    } else {
        grContext->setResourceCacheLimits(DEFAULT_SKIA_CACHE_COUNT, DEFAULT_SKIA_CACHE_SIZE);
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
    sk_sp<SkColorSpace> skColorSpace = nullptr;

    ROSEN_LOGD("RenderContext::AcquireSurface, colorSpace_ =  (%d)", colorSpace_ );
    switch (colorSpace_) {
        // [planning] in order to stay consistant with the colorspace used before, we disabled
        // GRAPHIC_COLOR_GAMUT_SRGB to let the branch to default, then skColorSpace is set to nullptr
        case GRAPHIC_COLOR_GAMUT_DISPLAY_P3:
        case GRAPHIC_COLOR_GAMUT_DCI_P3:
#if defined(NEW_SKIA)
            skColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
#else
            skColorSpace = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);
#endif
            framebufferInfo.fFormat = GL_RGBA16F;
            colorType = kRGBA_F16_SkColorType;
            break;
        default:
            break;
    }
    /* sampleCnt and stencilBits for GrBackendRenderTarget */
    const int stencilBufferSize = 8;
    GrBackendRenderTarget backendRenderTarget(width, height, 0, stencilBufferSize, framebufferInfo);
#if defined(NEW_SKIA)
    SkSurfaceProps surfaceProps(0, kRGB_H_SkPixelGeometry);
#else
    SkSurfaceProps surfaceProps = SkSurfaceProps::kLegacyFontHost_InitType;
#endif

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
#else
bool RenderContext::SetUpGpuContext(std::shared_ptr<Drawing::GPUContext> drawingContext)
{
    if (drGPUContext_ != nullptr) {
        ROSEN_LOGE("Drawing GPUContext has already created!!");
        return true;
    }
    Drawing::GPUContextOptions options;
    auto drGPUContext = std::make_shared<Drawing::GPUContext>();
    if (!drGPUContext->BuildFromGL(options)) {
        ROSEN_LOGE("SetUpGpuContext drGPUContext is null");
        return false;
    }
    int maxResources = 0;
    size_t maxResourcesSize = 0;
    int cacheLimitsTimes = 2;
     drGPUContext->GetResourceCacheLimits(&maxResources, &maxResourcesSize);
    if (maxResourcesSize > 0) {
        drGPUContext->SetResourceCacheLimits(cacheLimitsTimes * maxResources, cacheLimitsTimes *
            std::fmin(maxResourcesSize, DEFAULT_SKIA_CACHE_SIZE));
    } else {
        drGPUContext->SetResourceCacheLimits(DEFAULT_SKIA_CACHE_COUNT, DEFAULT_SKIA_CACHE_SIZE);
    }
    drGPUContext_ = std::move(drGPUContext);
    return true;
}

std::shared_ptr<Drawing::Surface> RenderContext::AcquireSurface(int width, int height)
{
    if (!SetUpGpuContext()) {
        ROSEN_LOGE("GPUContext is not ready!!!");
        return nullptr;
    }
    std::shared_ptr<Drawing::ColorSpace> colorSpace = nullptr;
    struct Drawing::FrameBuffer bufferInfo;
    bufferInfo.width = width;
    bufferInfo.height = height;
    bufferInfo.FBOID = 0;
    bufferInfo.Format = GL_RGBA8;
    bufferInfo.gpuContext = drGPUContext_;
    bufferInfo.colorType = Drawing::COLORTYPE_RGBA_8888;
    switch (colorSpace_) {
        // [planning] in order to stay consistant with the colorspace used before, we disabled
        // COLOR_GAMUT_SRGB to let the branch to default, then skColorSpace is set to nullptr
        case GRAPHIC_COLOR_GAMUT_DISPLAY_P3:
        case GRAPHIC_COLOR_GAMUT_DCI_P3:
            colorSpace = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::DCIP3);
            bufferInfo.Format = GL_RGBA16F;
            bufferInfo.colorType = Drawing::COLORTYPE_RGBA_F16;
            break;
        default:
            break;
    }

    bufferInfo.colorSpace = colorSpace;
    surface_ = std::make_shared<Drawing::Surface>();
    if (!surface_->Bind(bufferInfo)) {
        ROSEN_LOGW("Drawing::Surface surface_ is nullptr");
        surface_ = nullptr;
        return nullptr;
    }

    ROSEN_LOGD("CreateCanvas successfully!!!");
    return surface_;
}

void RenderContext::RenderFrame()
{
    RS_TRACE_FUNC();
    // flush commands
    if (surface_ != nullptr && surface_->GetCanvas() != nullptr) {
        ROSEN_LOGI("RenderFrame: Canvas");
        surface_->GetCanvas()->Flush();
    } else {
        ROSEN_LOGW("canvas is nullptr!!!");
    }
}
#endif

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
    RS_TRACE_FUNC();
    if ((eglDisplay_ == nullptr) || (eglSurface_ == nullptr)) {
        ROSEN_LOGE("eglDisplay or eglSurface is nullptr");
        return;
    }

    EGLint rect[4];
    rect[0] = left;
    rect[1] = top;
    rect[2] = width;
    rect[3] = height;

    EGLBoolean ret = GetEGLSetDamageRegionKHRFunc()(eglDisplay_, eglSurface_, rect, 1);
    if (ret == EGL_FALSE) {
        ROSEN_LOGE("eglSetDamageRegionKHR is failed");
    }
}

void RenderContext::DamageFrame(const std::vector<RectI> &rects)
{
}

void RenderContext::ClearRedundantResources()
{
    RS_TRACE_FUNC();
#ifndef USE_ROSEN_DRAWING
    if (grContext_ != nullptr) {
        ROSEN_LOGD("grContext clear redundant resources");
        grContext_->flush();
        // GPU resources that haven't been used in the past 10 seconds
        grContext_->purgeResourcesNotUsedInMs(std::chrono::seconds(10));
    }
#else
    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("grContext clear redundant resources");
        drGPUContext_->Flush();
        // GPU resources that haven't been used in the past 10 seconds
        drGPUContext_->PerformDeferredCleanup(std::chrono::seconds(10));
    }
#endif
}

RenderContextFactory& RenderContextFactory::GetInstance()
{
    static RenderContextFactory rf;
    return rf;
}
} // namespace Rosen
} // namespace OHOS
