/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "rs_surface_platform_texture_ios.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#include "platform/common/rs_log.h"
#include "render_context/render_context.h"

namespace OHOS {
namespace Rosen {

RSSurfacePlatformTextureIOS::RSSurfacePlatformTextureIOS(const RSSurfaceExtConfig& config)
    : RSSurfaceExt()
{
    config_ = config;
}

RSSurfacePlatformTextureIOS::~RSSurfacePlatformTextureIOS()
{
    if (isVideo_) {
        [videoOutput_ release];
    }

    if (textureId_ > 0 && attachCallback_ != nullptr) {
        attachCallback_(textureId_, false);
    }

    if (platformEglContext_) {
        [platformEglContext_ release];
        platformEglContext_ = nullptr;
    }

    if (textureId_) {
        glDeleteTextures(1, &textureId_);
    }
}

void RSSurfacePlatformTextureIOS::EnsureTextureCacheExists()
{
    if (!cache_ref_) {
        CVOpenGLESTextureCacheRef cache;
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL,
                                                    [EAGLContext currentContext], NULL, &cache);
        if (err == noErr) {
            cache_ref_.Reset(cache);
        } else {
            ROSEN_LOGE("RSSurfacePlatformTextureIOS::Failed to create GLES texture cache");
            return;
        }
    }
}

CVPixelBufferRef RSSurfacePlatformTextureIOS::GetPixelBuffer()
{
    if (!videoOutput_) {
        ROSEN_LOGE("RSSurfaceTextureIOS::videoOutput_ is nullptr");
        return nullptr;
    }

    CMTime outputItemTime = [videoOutput_ itemTimeForHostTime:CACurrentMediaTime()];
    if ([videoOutput_ hasNewPixelBufferForItemTime:outputItemTime]) {
        return [videoOutput_ copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
    } else {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::GetPixelBuffer is nullptr");
        return nullptr;
    }
}

void RSSurfacePlatformTextureIOS::CreateTextureFromPixelBuffer()
{
    if (buffer_ref_ == nullptr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::buffer_ref_ is nullptr");
        return;
    }
    CVOpenGLESTextureRef texture;
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(
        kCFAllocatorDefault, cache_ref_, buffer_ref_, nullptr, GL_TEXTURE_2D, GL_RGBA,
        static_cast<int>(CVPixelBufferGetWidth(buffer_ref_)),
        static_cast<int>(CVPixelBufferGetHeight(buffer_ref_)), GL_BGRA, GL_UNSIGNED_BYTE, 0,
        &texture);
    if (err != noErr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::Could not create texture from pixel buffer");
    } else {
        texture_ref_.Reset(texture);
    }
}

void RSSurfacePlatformTextureIOS::UpdateSurfaceDefaultSize(float width, float height)
{
}

void RSSurfacePlatformTextureIOS::InitializePlatformEglContext()
{
    platformEglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
        sharegroup:static_cast<EAGLContext*>(RenderContext::GetResourceContext()).sharegroup];
    if (platformEglContext_ == nullptr) {
        platformEglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
            sharegroup:static_cast<EAGLContext*>(RenderContext::GetResourceContext()).sharegroup];
    }
}

void RSSurfacePlatformTextureIOS::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    if (active_ == false) {
        if (!bufferAvailable_.load()){
            return;
        }
        if (config_.additionalData == nullptr) {
            return;
        }
        if (initTypeCallback_) {
            initTypeCallback_(isVideo_);
        }
        if (isVideo_) {
            videoOutput_ = [static_cast<AVPlayerItemVideoOutput*>(config_.additionalData) retain];
            active_ = true;
            return;
        }
        if (platformEglContext_ == nullptr) {
            InitializePlatformEglContext();
            void ** sharePtr = static_cast<void **>(config_.additionalData);
            *sharePtr = platformEglContext_;
        }
        glGenTextures(1, &textureId_);
        if (attachCallback_) {
            attachCallback_(textureId_, true);
            active_ = true;
        }
        return;
    }

    if (isVideo_) {
        DrawTextureImageForVideo(canvas, freeze, clipRect);
    } else {
        DrawTextureImageGL(canvas, freeze, clipRect);
    }
}

void RSSurfacePlatformTextureIOS::DrawTextureImageGL(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
{
    bool bufferAvailable = bufferAvailable_.load();
    if (!freeze && bufferAvailable_.load()) {
        bufferAvailable_.store(false);
    }

    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGI("create Drawing image fail");
        return;
    }
    EAGLContext* context = EAGLContext.currentContext;
    [EAGLContext setCurrentContext:platformEglContext_];
    if(updateCallback_) {
        updateCallback_(matrix);
    }
    glBindTexture(GL_TEXTURE_2D, textureId_);

    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth((int)clipRect.GetWidth());
    textureInfo.SetHeight((int)clipRect.GetHeight());
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(GL_TEXTURE_2D);
    textureInfo.SetID(textureId_);
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        ROSEN_LOGE("BuildFromTexture fail");
        return;
    }
    canvas.DrawImage(*image, clipRect.GetLeft(), clipRect.GetTop(), Drawing::SamplingOptions());
    [EAGLContext setCurrentContext:context];
}

void RSSurfacePlatformTextureIOS::DrawTextureImageForVideo(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    EnsureTextureCacheExists();
    if (!freeze) {
        auto pixelBuffer = GetPixelBuffer();
        if (pixelBuffer) {
            buffer_ref_.Reset(pixelBuffer);
        }
        CreateTextureFromPixelBuffer();
    }
    if (!texture_ref_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::texture_ref_ is nullptr");
        return;
    }
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGE("create Drawing image fail");
        return;
    }
    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth((int)clipRect.GetWidth());
    textureInfo.SetHeight((int)clipRect.GetHeight());
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(CVOpenGLESTextureGetTarget(texture_ref_));
    textureInfo.SetID(CVOpenGLESTextureGetName(texture_ref_));
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        return;
    }
    canvas.DrawImage(*image, clipRect.GetLeft(), clipRect.GetTop(), Drawing::SamplingOptions());
}
} // namespace Rosen
} // namespace OHOS