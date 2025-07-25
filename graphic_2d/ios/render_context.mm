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
    : drGPUContext_(nullptr),
      surface_(nullptr),
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
    if (@available(iOS 18, *)) {
        @autoreleasepool {
            UpdateStorageSizeIfNecessary();
        }
    } else {
        UpdateStorageSizeIfNecessary();
    }
}

bool RenderContext::UpdateStorageSizeIfNecessary()
{
    [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];
    static_cast<CAEAGLLayer*>(layer_).drawableProperties = @{
        kEAGLDrawablePropertyColorFormat : kEAGLColorFormatRGBA8,
        kEAGLDrawablePropertyRetainedBacking : @(NO),
    };

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
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = [static_cast<CAEAGLLayer*>(eglNativeWindow) retain];
    if (static_cast<CAEAGLLayer*>(layer_) == nullptr) {
       ROSEN_LOGE("RenderContextEAGL layer_ is null");
       return nullptr;
    }
    return layer_;
}

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
