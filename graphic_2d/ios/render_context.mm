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

namespace OHOS {
namespace Rosen {
constexpr int32_t EGL_CONTEXT_CLIENT_VERSION_NUM = 2;
constexpr char CHARACTER_WHITESPACE = ' ';
constexpr const char* CHARACTER_STRING_WHITESPACE = " ";
constexpr const char* EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";

RenderContext::RenderContext()
    : grContext_(nullptr),
      skSurface_(nullptr),
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

void RenderContext::InitializeEglContext()
{   
    if (IsEglContextReady()) {
        return;
    }
    resource_context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    if (resource_context_ != nullptr) {
        eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
            sharegroup:static_cast<EAGLContext*>(resource_context_).sharegroup];
    } else {
        resource_context_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
        eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
            sharegroup:static_cast<EAGLContext*>(resource_context_).sharegroup];
    }

    if (resource_context_ == nullptr || eglContext_ == nullptr) {
        ROSEN_LOGE("eglContext_ is null");
        return;
    }
    color_space_ = SkColorSpace::MakeSRGB();
    if (@available(iOS 10, *)) {
        UIDisplayGamut displayGamut = [UIScreen mainScreen].traitCollection.displayGamut;
        switch (displayGamut) {
            case UIDisplayGamutP3:
#if defined(NEW_SKIA)
                color_space_ = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);
#else
                color_space_ = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);
#endif
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
    UpdateStorageSizeIfNecessary();    
}

bool RenderContext::UpdateStorageSizeIfNecessary()
{
    const CGSize layer_size = [static_cast<CAEAGLLayer*>(layer_)bounds].size;
    const CGFloat contents_scale = static_cast<CAEAGLLayer*>(layer_).contentsScale;
    const int size_width = layer_size.width * contents_scale;
    const int size_height = layer_size.height * contents_scale;

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);

    if (![eglContext_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(layer_)]) {
        ROSEN_LOGE("eglContext_ renderbufferStorage:GL_RENDERBUFFER Failed");
        return false;
    }

    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &storage_width_);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &storage_height_);
    return true;
}

bool RenderContext::ResourceMakeCurrent() 
{
    return [EAGLContext setCurrentContext:static_cast<EAGLContext*>(resource_context_)];
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
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = [static_cast<CAEAGLLayer*>(eglNativeWindow) retain];
    if (static_cast<CAEAGLLayer*>(layer_) == nullptr) {
       ROSEN_LOGE("RenderContextEAGL layer_ is null");
       return nullptr;
    }

    NSString* drawableColorFormat = kEAGLColorFormatRGBA8;

    static_cast<CAEAGLLayer*>(layer_).drawableProperties = @{
        kEAGLDrawablePropertyColorFormat : drawableColorFormat,
        kEAGLDrawablePropertyRetainedBacking : @(NO),
    };
    return layer_;
}

bool RenderContext::SetUpGrContext()
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

void RenderContext::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGD("RenderContext::SetColorSpace %{public}d", colorSpace);
    colorSpace_ = colorSpace;
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

RenderContextFactory& RenderContextFactory::GetInstance()
{
    static RenderContextFactory rf;
    return rf;
}
} // namespace Rosen
} // namespace OHOS
