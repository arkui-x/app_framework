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

#include <MacTypes.h>
#include <__nullptr>
#include <sstream>
#include <UIKit/UIKit.h>
#include "OpenGLES/EAGL.h"
#include "OpenGLES/ES2/gl.h"
#include "OpenGLES/ES2/glext.h"
#include "QuartzCore/CAEAGLLayer.h"
#include "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "rs_trace.h"
#include "memory/rs_tag_tracker.h"
#include "platform/common/rs_log.h"
#ifdef RS_ENABLE_GL
#include "platform/common/rs_system_properties.h"
#endif

namespace OHOS {
namespace Rosen {
std::mutex RenderContext::resourceContextMutex;
EGLContext RenderContext::resourceContext = nullptr;
constexpr int32_t EGL_CONTEXT_CLIENT_VERSION_NUM = 2;
constexpr char CHARACTER_WHITESPACE = ' ';
constexpr const char* CHARACTER_STRING_WHITESPACE = " ";
constexpr const char* EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";

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
{
}

RenderContext::~RenderContext()
{
    EAGLContext* context = EAGLContext.currentContext;
    [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];

    glDeleteFramebuffers(1, &framebuffer_);
    glDeleteRenderbuffers(1, &colorbuffer_);

    [EAGLContext setCurrentContext:context];
}

void RenderContext::CreatePbufferSurface()
{
}

const EGLContext RenderContext::GetResourceContext()
{
    std::lock_guard<std::mutex> lock(RenderContext::resourceContextMutex);
    if (!RenderContext::resourceContext) {
        RenderContext::resourceContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    }
    if (!RenderContext::resourceContext) {
        RenderContext::resourceContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    return RenderContext::resourceContext;
}

void RenderContext::InitializeEglContext()
{   
    if (IsEglContextReady()) {
        return;
    }
    eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
            sharegroup:static_cast<EAGLContext*>(GetResourceContext()).sharegroup];
    if (eglContext_ == nullptr) {
        eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
            sharegroup:static_cast<EAGLContext*>(GetResourceContext()).sharegroup];
    }

    if (eglContext_ == nullptr) {
        ROSEN_LOGE("eglContext_ is null");
        return;
    }
    color_space_ = Drawing::ColorSpace::CreateSRGB();
    if (@available(iOS 10, *)) {
        UIDisplayGamut displayGamut = [UIScreen mainScreen].traitCollection.displayGamut;
        switch (displayGamut) {
            case UIDisplayGamutP3:
            color_space_ = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::DCIP3);
                break;
            default:
                break;
        }
    }

    if (valid_) {
        return;
    }
    
    bool isContentCurrent = [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];
    if (!isContentCurrent){
        ROSEN_LOGE("Failed to set current OpenGL context");
        return;
    }

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glGenRenderbuffers(1, &colorbuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer_);
    valid_ = true;
}

void RenderContext::MakeCurrent(EGLSurface surface, EGLContext context) 
{
    [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];
    CAEAGLLayer* layer = static_cast<CAEAGLLayer*>(surface);
    const CGSize layer_size = [layer bounds].size;
    const CGFloat contents_scale = layer.contentsScale;
    const GLint size_width = layer_size.width * contents_scale;
    const GLint size_height = layer_size.height * contents_scale;

    if (layer == layer_ && size_width == storage_width_ && size_height == storage_height_) {
        // Nothing to since the stoage size is already consistent with the layer.
        return;
    }
    ROSEN_LOGD("renderbufferStorage");
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);
    __block bool res = false;
    if (@available(iOS 18, *)) {
        dispatch_sync(dispatch_get_main_queue(), ^{
            res = [eglContext_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(layer_)];
        });
    } else {
       res = [eglContext_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(layer_)];
    }
    if (!res) {
        ROSEN_LOGE("eglContext_ renderbufferStorage:GL_RENDERBUFFER Failed");
        return;
    }
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &storage_width_);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &storage_height_);
}

bool RenderContext::UpdateStorageSizeIfNecessary()
{
    return true;
}

bool RenderContext::ResourceMakeCurrent() 
{
    return [EAGLContext setCurrentContext:static_cast<EAGLContext*>(resourceContext)];
}

void RenderContext::ShareMakeCurrent(EGLContext shareContext)
{
}

void RenderContext::ShareMakeCurrentNoSurface(EGLContext shareContext)
{
}

void RenderContext::MakeSelfCurrent()
{
}

EGLContext RenderContext::CreateShareContext()
{
    return nullptr;
}

void RenderContext::SwapBuffers(EGLSurface surface) const
{
    const GLenum discards[] = {
        GL_DEPTH_ATTACHMENT,
        GL_STENCIL_ATTACHMENT,
    };

    glDiscardFramebufferEXT(GL_FRAMEBUFFER, sizeof(discards) / sizeof(GLenum), discards);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];
    [CATransaction flush];
}

void RenderContext::DestroyEGLSurface(EGLSurface surface)
{
    ROSEN_LOGD("RenderContext::DestroyEGLSurface");
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = nullptr;
}

EGLSurface RenderContext::CreateEGLSurface(EGLNativeWindowType eglNativeWindow)
{
    if (layer_ == eglNativeWindow) {
        return layer_;
    }
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = [static_cast<CAEAGLLayer*>(eglNativeWindow) retain];
    if (static_cast<CAEAGLLayer*>(layer_) == nullptr) {
       ROSEN_LOGE("RenderContextEAGL layer_ is null");
       return nullptr;
    }
    // use new layer, reset width and height
    storage_width_ = 0;
    storage_height_ = 0;
    return layer_;
}

#ifndef USE_ROSEN_DRAWING
bool RenderContext::SetUpGrContext(sk_sp<GrDirectContext> skContext)
{
    if (grContext_ != nullptr) {
        return true;
    }

    sk_sp<const GrGLInterface> glInterface(GrGLMakeNativeInterface());
    if (glInterface.get() == nullptr) {
        ROSEN_LOGE("SetUpGrContext failed to make native interface");
        return false;
    }

    GrContextOptions options;
    options.fAvoidStencilBuffers = true;
    options.fPreferExternalImagesOverES3 = true;
    options.fDisableGpuYUVConversion = true;
#if defined(NEW_SKIA)
    sk_sp<GrDirectContext> grContext(GrDirectContext::MakeGL(std::move(glInterface), options));
#else
    sk_sp<GrContext> grContext(GrContext::MakeGL(std::move(glInterface), options));
#endif

    if (grContext == nullptr) {
        ROSEN_LOGE("SetUpGrContext grContext is null");
        return false;
    }
    grContext_ = std::move(grContext);
    return true;
}

sk_sp<SkSurface> RenderContext::AcquireSurface(int width, int height)
{
    GrGLFramebufferInfo framebufferInfo;
    framebufferInfo.fFBOID = framebuffer_;
    framebufferInfo.fFormat = 0x8058; // GL_RGBA8

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
            framebufferInfo.fFormat = 0x881A; // GL_RGBA16F
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
        grContext_.get(), backendRenderTarget, kBottomLeft_GrSurfaceOrigin, colorType, skColorSpace, &surfaceProps);
    if (skSurface_ == nullptr) {
        ROSEN_LOGW("skSurface is nullptr");
        return nullptr;
    }

    return skSurface_;
}

void RenderContext::RenderFrame()
{
    if (skSurface_ == nullptr) {
        ROSEN_LOGE("skSurface_ is nullptr!!!");
        return;
    }
    if (skSurface_->getCanvas() != nullptr) {
        skSurface_->getCanvas()->flush();
    } else {
        ROSEN_LOGE("canvas is nullptr!!!");
    }
}
#else
bool RenderContext::SetUpGpuContext(std::shared_ptr<Drawing::GPUContext> drawingContext)
{
    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("Drawing GPUContext has already created");
        return true;
    }
    Drawing::GPUContextOptions options;
    auto drGPUContext = std::make_shared<Drawing::GPUContext>();
    if (!drGPUContext->BuildFromGL(options)) {
        ROSEN_LOGE("SetUpGpuContext drGPUContext is null");
        return false;
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
    bufferInfo.FBOID = framebuffer_;
    bufferInfo.Format = 0x8058;
    bufferInfo.gpuContext = drGPUContext_;
    bufferInfo.colorType = Drawing::COLORTYPE_RGBA_8888;
    switch (colorSpace_) {
        // [planning] in order to stay consistant with the colorspace used before, we disabled
        // COLOR_GAMUT_SRGB to let the branch to default, then skColorSpace is set to nullptr
        case GRAPHIC_COLOR_GAMUT_DISPLAY_P3:
        case GRAPHIC_COLOR_GAMUT_DCI_P3:
            colorSpace = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::DCIP3);
            bufferInfo.Format = 0x881A;
            bufferInfo.colorType = Drawing::COLORTYPE_RGBA_F16;
            break;
        default:
            break;
    }
    bufferInfo.colorSpace = colorSpace;
    surface_ = std::make_shared<Drawing::Surface>();
    if (!surface_->Bind(bufferInfo)) {
        ROSEN_LOGE("Drawing::Surface surface_ is nullptr");
        surface_ = nullptr;
        return nullptr;
    }
    return surface_;
}

void RenderContext::RenderFrame()
{
    RS_TRACE_FUNC();
    // flush commands
    if (surface_ != nullptr && surface_->GetCanvas() != nullptr) {
        ROSEN_LOGD("RenderFrame: Canvas");
        surface_->GetCanvas()->Flush();
    } else {
        ROSEN_LOGE("canvas is nullptr!!!");
    }
}
#endif

EGLint RenderContext::QueryEglBufferAge()
{
    return EGL_UNKNOWN;
}

void RenderContext::DamageFrame(int32_t left, int32_t top, int32_t width, int32_t height)
{
}

void RenderContext::DamageFrame(const std::vector<RectI> &rects)
{
}

void RenderContext::ClearRedundantResources()
{
}

#if defined(RS_ENABLE_GL) || defined(RS_ENABLE_VK)
std::string RenderContext::GetShaderCacheSize() const
{
    return "";
}

std::string RenderContext::CleanAllShaderCache() const
{
    return "";
}
#endif

RenderContextFactory& RenderContextFactory::GetInstance()
{
    static RenderContextFactory rf;
    return rf;
}
} // namespace Rosen
} // namespace OHOS
