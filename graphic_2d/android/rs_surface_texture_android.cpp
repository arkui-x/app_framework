/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "rs_surface_texture_android.h"

#include <cmath>
#include <cstdint>
#include <securec.h>

#include "draw/paint.h"
#include "effect/shader_effect.h"
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
AndroidSurfaceTexture::AndroidSurfaceTexture(const RSSurfaceExtConfig& config)
    : RSSurfaceExt(), config_(std::move(config)), transform_(Drawing::Matrix())
{
    transform_.SetMatrix(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

AndroidSurfaceTexture::~AndroidSurfaceTexture()
{
    ROSEN_LOGE("AndroidSurfaceTexture::~AndroidSurfaceTexture textureId_ %{public}d ", textureId_);
    if (textureId_ > 0 && attachCallback_ != nullptr) {
        attachCallback_(textureId_, false);
    }
}

void AndroidSurfaceTexture::MarkUiFrameAvailable(bool available)
{
    ROSEN_LOGE("AndroidSurfaceTexture::MarkUiFrameAvailable textureId_ %{public}d ", textureId_);
    bufferAvailable_.store(available);
}

void AndroidSurfaceTexture::UpdateTransform()
{
    std::vector<float> matrix {};
    updateCallback_(matrix);
    if (matrix.size() == 16) { // 16 max len
        // the matrix is the same as the matrix in the surface texture, so we need to invert it
        Drawing::Matrix::Buffer matrix3 = {
            matrix[0], matrix[4], matrix[12],
            matrix[1], matrix[5], matrix[13],
            matrix[3], matrix[7], matrix[15]
        };
        Drawing::Matrix transformInvert;
        transformInvert.SetAll(matrix3);
        // invert the matrix to get the transform_
        auto res = transformInvert.Invert(transform_);
        if (!res) {
            transform_.SetMatrix(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
            ROSEN_LOGE("AndroidSurfaceTexture::UpdateTransform Invert failed");
        }
    }
}

void AndroidSurfaceTexture::UpdateSurfaceDefaultSize(float width, float height)
{
    width_ = width;
    height_ = height;
}

bool AndroidSurfaceTexture::InitializeTextureIfNeeded()
{
    if (state_ == AttachmentState::UNINITIALIZED) {
        if (!bufferAvailable_.load()) {
            return false;
        }
        glGenTextures(1, &textureId_);
        ROSEN_LOGE("AndroidSurfaceTexture::DrawTextureImage attachCallback textureId %{public}d", textureId_);
        attachCallback_(textureId_, true);
        state_ = AttachmentState::ATTACHED;
    }
    return true;
}

std::shared_ptr<Drawing::Image> AndroidSurfaceTexture::CreateTextureImage(RSPaintFilterCanvas& canvas)
{
    ROSEN_LOGE("AndroidSurfaceTexture::textureId_ %{public}d", textureId_);
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGE("create Drawing image fail");
        return nullptr;
    }
    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth((int)1);
    textureInfo.SetHeight((int)1);
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(GL_TEXTURE_EXTERNAL_OES);
    textureInfo.SetID(textureId_);
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        return nullptr;
    }
    return image;
}

void AndroidSurfaceTexture::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
{
    if (state_ == AttachmentState::DETACHED || attachCallback_ == nullptr || updateCallback_ == nullptr) {
        return;
    }
    if (!InitializeTextureIfNeeded()) {
        return;
    }

    bool bufferAvailable = bufferAvailable_.load();
    if (!freeze && bufferAvailable_.load()) {
        UpdateTransform();
        bufferAvailable_.store(false);
    }

    auto image = CreateTextureImage(canvas);
    if (image == nullptr) {
        return;
    }
    // transform_ is identity, so no need to clip, just draw the image directly
    if (transform_.IsIdentity()) {
        canvas.DrawImage(*image, 0, 0, Drawing::SamplingOptions());
        return;
    }
    canvas.Save();
    canvas.Translate(clipRect.GetLeft(), clipRect.GetTop() + clipRect.GetHeight());
    canvas.Scale(clipRect.GetWidth(),  -clipRect.GetHeight());
    // draw the image with the transform_, but the image is upside down, so we need to scale the image vertically
    auto imageShader = Drawing::ShaderEffect::CreateImageShader(
        *image, Drawing::TileMode::CLAMP, Drawing::TileMode::CLAMP,
        Drawing::SamplingOptions(), transform_);
    if (imageShader != nullptr) {
        Drawing::Paint paint;
        paint.SetShaderEffect(imageShader);
        paint.SetStyle(Drawing::Paint::PaintStyle::PAINT_FILL);
        canvas.AttachPaint(paint);
        canvas.DrawRect(Drawing::Rect(0, 0, 1, 1));
        canvas.DetachPaint();
    } else {
        ROSEN_LOGE("AndroidSurfaceTexture::DrawTextureImage failed,imageShader == nullptr");
    }
    canvas.Restore();
}
} // namespace Rosen
} // namespace OHOS
