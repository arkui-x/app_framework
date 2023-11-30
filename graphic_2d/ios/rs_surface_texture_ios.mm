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

#include "rs_surface_texture_ios.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {

RSSurfaceTextureIOS::RSSurfaceTextureIOS(const RSSurfaceExtConfig& config)
    : RSSurfaceExt()
{
     videoOutput_ = [static_cast<AVPlayerItemVideoOutput*>(config.additionalData) retain];
}

RSSurfaceTextureIOS::~RSSurfaceTextureIOS()
{
    [videoOutput_ release];
}

void RSSurfaceTextureIOS::EnsureTextureCacheExists()
{
    if (!cache_ref_) {
        CVOpenGLESTextureCacheRef cache;
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL,
                                                    [EAGLContext currentContext], NULL, &cache);
        if (err == noErr) {
            cache_ref_.Reset(cache);
        } else {
            ROSEN_LOGE("RSSurfaceTextureIOS::Failed to create GLES texture cache");
            return;
        }
    }
}

CVPixelBufferRef RSSurfaceTextureIOS::GetPixelBuffer()
{
    if (!videoOutput_) {
        ROSEN_LOGE("RSSurfaceTextureIOS::videoOutput_ is nullptr");
        return nullptr;
    }
    CMTime outputItemTime = [videoOutput_ itemTimeForHostTime:CACurrentMediaTime()];
    if ([videoOutput_ hasNewPixelBufferForItemTime:outputItemTime]) {
        return [videoOutput_ copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
    } else {
            ROSEN_LOGE("RSSurfaceTextureIOS::GetPixelBuffer is nullptr");
        return nullptr;
    }
}

void RSSurfaceTextureIOS::CreateTextureFromPixelBuffer()
{
    if (buffer_ref_ == nullptr) {
        ROSEN_LOGE("RSSurfaceTextureIOS::buffer_ref_ is nullptr");
        return;
    }
    CVOpenGLESTextureRef texture;
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(
        kCFAllocatorDefault, cache_ref_, buffer_ref_, nullptr, GL_TEXTURE_2D, GL_RGBA,
        static_cast<int>(CVPixelBufferGetWidth(buffer_ref_)),
        static_cast<int>(CVPixelBufferGetHeight(buffer_ref_)), GL_BGRA, GL_UNSIGNED_BYTE, 0,
        &texture);
    if (err != noErr) {
        ROSEN_LOGE("RSSurfaceTextureIOS::Could not create texture from pixel buffer");
    } else {
        texture_ref_.Reset(texture);
    }
}

void RSSurfaceTextureIOS::UpdateSurfaceDefaultSize(float width, float height)
{
}

#ifndef USE_ROSEN_DRAWING
void RSSurfaceTextureIOS::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze,
    const SkRect& clipRect)
#else
void RSSurfaceTextureIOS::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
#endif
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
        ROSEN_LOGE("RSSurfaceTextureIOS::texture_ref_ is nullptr");
        return;
    }
    GrGLTextureInfo textureInfo = {CVOpenGLESTextureGetTarget(texture_ref_),
        CVOpenGLESTextureGetName(texture_ref_), GL_RGBA8_OES};
#ifndef USE_ROSEN_DRAWING
    GrBackendTexture backendTexture(clipRect.width(), clipRect.height(), GrMipMapped::kNo, textureInfo);
#ifdef NEW_SKIA
    auto image = SkImage::MakeFromTexture(
        canvas.recordingContext(), backendTexture, kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
#else
    auto image = SkImage::MakeFromTexture(
        canvas.getGrContext(), backendTexture, kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
#endif
    if (image) {
        canvas.drawImage(image, clipRect.x(), clipRect.y());
    }
#else
    GrBackendTexture backendTexture(clipRect.GetWidth(),
        clipRect.GetHeight(), GrMipMapped::kNo, textureInfo);
    auto image = SkImage::MakeFromTexture(
        canvas.GetGPUContext(), backendTexture, kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    if (image) {
        canvas.DrawImage(image, clipRect.GetLeft(), clipRect.GetTop());
    }
#endif
}
} // namespace Rosen
} // namespace OHOS