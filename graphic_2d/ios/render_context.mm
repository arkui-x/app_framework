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
#include "render_context/new_render_context/render_context_gl.h"

#include <MacTypes.h>
#include <__config>
#include <__nullptr>
#include <sstream>
#include <chrono>
#include <UIKit/UIKit.h>
#include <CoreFoundation/CoreFoundation.h>
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
std::mutex RenderContextGL::resourceContextMutex;
EGLContext RenderContextGL::resourceContext = nullptr;
constexpr int32_t EGL_CONTEXT_CLIENT_VERSION_NUM = 2;
constexpr char CHARACTER_WHITESPACE = ' ';
constexpr const char* CHARACTER_STRING_WHITESPACE = " ";
constexpr const char* EGL_KHR_SURFACELESS_CONTEXT = "EGL_KHR_surfaceless_context";

std::shared_ptr<RenderContext> RenderContext::Create()
{
#ifdef RS_ENABLE_VK
    if (RSSystemProperties::IsUseVulkan()) {
        ROSEN_LOGE("arkui-x is use vulkan is true, RenderContext::Create() return nullptr!!")
        return nullptr;
    }
#endif
    return std::make_shared<RenderContextGL>();
}

RenderContextGL::RenderContextGL()
{
    surface_ = nullptr;
    drGPUContext_ = nullptr;
    nativeWindow_ = nullptr;
    eglDisplay_ = EGL_NO_DISPLAY;
    eglContext_ = EGL_NO_CONTEXT;
    eglSurface_ = EGL_NO_SURFACE;
    config_ = nullptr;
    mHandler_ = nullptr;
}

RenderContextGL::~RenderContextGL()
{
    EAGLContext* context = EAGLContext.currentContext;
    [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];

    glDeleteFramebuffers(1, &framebuffer_);
    glDeleteRenderbuffers(1, &colorbuffer_);

    [EAGLContext setCurrentContext:context];
}

void RenderContextGL::CreatePbufferSurface()
{
}

const EGLContext RenderContextGL::GetResourceContext()
{
    std::lock_guard<std::mutex> lock(RenderContextGL::resourceContextMutex);
    if (!RenderContextGL::resourceContext) {
        RenderContextGL::resourceContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
    }
    if (!RenderContextGL::resourceContext) {
        RenderContextGL::resourceContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    }
    return RenderContextGL::resourceContext;
}

bool RenderContextGL::Init()
{   
    if (IsEglContextReady()) {
         return true;
    }
    eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
            sharegroup:static_cast<EAGLContext*>(GetResourceContext()).sharegroup];
    if (eglContext_ == nullptr) {
        eglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
            sharegroup:static_cast<EAGLContext*>(GetResourceContext()).sharegroup];
    }

    if (eglContext_ == nullptr) {
        ROSEN_LOGE("eglContext_ is null");
        return false;
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
        return false;
    }
    
    bool isContentCurrent = [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];
    if (!isContentCurrent){
        ROSEN_LOGE("Failed to set current OpenGL context");
        return false;
    }

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    glGenRenderbuffers(1, &colorbuffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorbuffer_);
    valid_ = true;
    return true;
}

void RenderContextGL::MakeCurrent(EGLSurface surface, EGLContext context)
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
            res = [eglContext_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(layer)];
        });
    } else {
       res = [eglContext_ renderbufferStorage:GL_RENDERBUFFER fromDrawable:static_cast<CAEAGLLayer *>(layer)];
    }
    if (!res) {
        ROSEN_LOGE("eglContext_ renderbufferStorage:GL_RENDERBUFFER Failed");
        storage_width_ = 0;
        storage_height_ = 0;
        [static_cast<CAEAGLLayer *>(layer_) release];
        layer_ = nullptr;
        return;
    }
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &storage_width_);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &storage_height_);
    if (layer_ != layer) {
        [static_cast<CAEAGLLayer *>(layer_) release];
        layer_ = [layer retain];
    }
}

bool RenderContextGL::UpdateStorageSizeIfNecessary()
{

    return true;
}

bool RenderContextGL::ResourceMakeCurrent()
{
    return [EAGLContext setCurrentContext:static_cast<EAGLContext*>(resourceContext)];
}

void RenderContextGL::CreateShareContext()
{
    return;
}

bool RenderContextGL::SwapBuffers(EGLSurface surface) const
{
    const GLenum discards[] = {
        GL_DEPTH_ATTACHMENT,
        GL_STENCIL_ATTACHMENT,
    };

    glDiscardFramebufferEXT(GL_FRAMEBUFFER, sizeof(discards) / sizeof(GLenum), discards);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer_);
    [[EAGLContext currentContext] presentRenderbuffer:GL_RENDERBUFFER];
    [CATransaction flush];
    return true;
}

void RenderContextGL::AddSurface()
{
    surface_count_++;
}

void RenderContextGL::SetCleanUpHelper(std::function<void()> func)
{
    cleanUpHelper_ = func;
}

void RenderContextGL::DestroyEGLSurface(EGLSurface surface)
{
    if (layer_ != nullptr) {
        [static_cast<CAEAGLLayer*>(layer_) release];
        layer_ = nullptr;
    }
    ROSEN_LOGD("DestroyEGLSurface");
    surface_count_--;
    storage_width_ = 0;
    storage_height_ = 0;

    if (cleanUpHelper_ != nullptr && surface_count_ == 0) {
        ROSEN_LOGD("DestroyEGLSurface cleanUpHelper_ - All surfaces destroyed, cleaning up shared resources");
        cleanUpHelper_();
    }
}

/* 
 *  Destroy the shared source when all rssurface are destroyed, this must be called on the render thread, 
 *  otherwise it will cause memory leak,iosurface will not be released.
 */
void RenderContextGL::DestroySharedSource()
{
    ROSEN_LOGD("DestroySharedSource surface_count_ %{public}d", surface_count_);

    if (surface_count_ > 0) {
        return;
    }

    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("DestroySharedSource: Cleaning GPUContext resources to release ioaccelerator");
        drGPUContext_->PurgeUnlockedResources(false);
        drGPUContext_->FreeGpuResources();
        drGPUContext_->PerformDeferredCleanup(std::chrono::milliseconds(0));
        drGPUContext_ = nullptr;
    }
    surface_ = nullptr;

    if (eglContext_ != nullptr) {
        ROSEN_LOGD("DestroySharedSource eglContext_ is not nullptr");
        [EAGLContext setCurrentContext:static_cast<EAGLContext*>(eglContext_)];      
        if (framebuffer_ != 0) {
            glDeleteFramebuffers(1, &framebuffer_);
            glFinish();
            framebuffer_ = 0; 
        }

        if (colorbuffer_ != 0) {
            glDeleteRenderbuffers(1, &colorbuffer_);
            glFinish();
            colorbuffer_ = 0;
        }
        [EAGLContext setCurrentContext:nil];
        glFinish();
        [static_cast<EAGLContext*>(eglContext_) release];
        eglContext_ = nullptr;
    } else {
        ROSEN_LOGD("DestroySharedSource eglContext_ is nullptr");
    }
    
    storage_width_ = 0;
    storage_height_ = 0;
    valid_ = false;
}

EGLSurface RenderContextGL::CreateEGLSurface(EGLNativeWindowType eglNativeWindow)
{
    CAEAGLLayer* layer = static_cast<CAEAGLLayer*>(eglNativeWindow);
    const CGSize layer_size = [layer bounds].size;
    const CGFloat contents_scale = layer.contentsScale;
    const GLint size_width = layer_size.width * contents_scale;
    const GLint size_height = layer_size.height * contents_scale;

    if (size_width == 0 || size_height == 0) {
        return nullptr;
    }
    return eglNativeWindow;
}

bool RenderContextGL::SetUpGpuContext(std::shared_ptr<Drawing::GPUContext> drawingContext)
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

std::shared_ptr<Drawing::Surface> RenderContextGL::AcquireSurface(int width, int height)
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

void RenderContextGL::RenderFrame()
{
    RS_TRACE_FUNC();
    // flush commands
    if (surface_ != nullptr && surface_->GetCanvas() != nullptr) {
        surface_->GetCanvas()->Flush();
    } else {
        ROSEN_LOGE("canvas is nullptr!!!");
    }
}

int32_t RenderContextGL::QueryEglBufferAge()
{
    return static_cast<int32_t>(EGL_UNKNOWN);
}

void RenderContextGL::DamageFrame(const std::vector<RectI> &rects)
{
}

void RenderContextGL::ClearRedundantResources()
{
}

std::string RenderContextGL::GetShaderCacheSize() const
{
    return "";
}

std::string RenderContextGL::CleanAllShaderCache() const
{
    return "";
}

bool RenderContextGL::AbandonContext()
{
    return true;
}

void RenderContextGL::DestroyShareContext()
{
}
} // namespace Rosen
} // namespace OHOS
