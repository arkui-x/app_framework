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

namespace OHOS {
namespace Rosen {

RSSurfacePlatformTextureIOS::RSSurfacePlatformTextureIOS(const RSSurfaceExtConfig& config)
    : RSSurfaceExt()
{
    ref_ = static_cast<CVPixelBufferRef*>(config.additionalData);
}

RSSurfacePlatformTextureIOS::~RSSurfacePlatformTextureIOS()
{
    ref_ = nullptr;
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
    if (!ref_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::ref_ is nullptr");
        return nullptr;
    }
    if (*ref_) {
        CFRetain(*ref_);
    }
    return *ref_;
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

void RSSurfacePlatformTextureIOS::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    EnsureTextureCacheExists();
    if (!freeze && bufferAvailable_.load()) {
        auto pixelBuffer = GetPixelBuffer();
        bufferAvailable_.store(false);
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
        ROSEN_LOGD("create Drawing image fail");
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