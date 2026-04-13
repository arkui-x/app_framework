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

#include "rs_surface_texture_android_gl.h"

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
AndroidSurfaceTextureGL::AndroidSurfaceTextureGL(const RSSurfaceExtConfig& config)
    : AndroidSurfaceTexture(config)
{
}

AndroidSurfaceTextureGL::~AndroidSurfaceTextureGL()
{
    if (cachedImage_ != nullptr) {
        cachedImage_.reset();
    }
    RSSurfaceTextureAttachCallBack attachCallback = GetAttach();
    if (textureId_ > 0 && attachCallback != nullptr) {
        attachCallback(textureId_, false);
        glDeleteTextures(1, &textureId_); // 1: delete texture id count
    }
}

void AndroidSurfaceTextureGL::UpdateTransform()
{
    RSSurfaceTextureUpdateCallBack updateCallback = GetUpdate();
    if (updateCallback == nullptr) {
        ROSEN_LOGE("AndroidSurfaceTextureGL::UpdateTransform updateCallback is nullptr");
        return;
    }
    std::vector<float> matrix {};
    updateCallback(matrix);
    if (matrix.size() == 16) { // 16: max len
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
            ROSEN_LOGE("AndroidSurfaceTextureGL::UpdateTransform Invert failed");
        }
    }
}

void AndroidSurfaceTextureGL::OnBufferAvailable()
{
    UpdateTransform();
}

bool AndroidSurfaceTextureGL::OnInitializeTexture()
{
    glGenTextures(1, &textureId_); // 1: generate texture id count
    RSSurfaceTextureAttachCallBack attachCallback = GetAttach();
    if (attachCallback == nullptr) {
        ROSEN_LOGE("AndroidSurfaceTextureGL::OnInitializeTexture attachCallback is nullptr");
        return false;
    }
    attachCallback(textureId_, true);
    return true;
}

bool AndroidSurfaceTextureGL::OnCreateTextureImage(RSPaintFilterCanvas& canvas)
{
    if (!cachedImage_) {
        cachedImage_ = std::make_shared<Drawing::Image>();
    }
    if (cachedImage_ == nullptr) {
        ROSEN_LOGE("AndroidSurfaceTextureGL::OnCreateTextureImage create Drawing image fail");
        return false;
    }
    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth(1);
    textureInfo.SetHeight(1);
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(GL_TEXTURE_EXTERNAL_OES);
    textureInfo.SetID(textureId_);
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = cachedImage_->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        ROSEN_LOGE("AndroidSurfaceTextureGL::OnCreateTextureImage BuildFromTexture failed");
        return false;
    }
    return true;
}

void AndroidSurfaceTextureGL::ApplyClipRectScale(RSPaintFilterCanvas& canvas, const Drawing::Rect& clipRect)
{
    if (transform_.IsIdentity()) {
        canvas.DrawImage(*cachedImage_, 0, 0, Drawing::SamplingOptions()); // 0:scalX, 0:scalY
        return;
    }
    canvas.Save();
    canvas.Translate(clipRect.GetLeft(), clipRect.GetTop() + clipRect.GetHeight());
    canvas.Scale(clipRect.GetWidth(), -clipRect.GetHeight());

    // draw the image with the transform_, but the image is upside down, so we need to scale the image vertically
    auto imageShader = Drawing::ShaderEffect::CreateImageShader(
        *cachedImage_, Drawing::TileMode::CLAMP, Drawing::TileMode::CLAMP,
        Drawing::SamplingOptions(), transform_);
    if (imageShader != nullptr) {
        Drawing::Paint paint;
        paint.SetShaderEffect(imageShader);
        paint.SetStyle(Drawing::Paint::PaintStyle::PAINT_FILL);
        canvas.AttachPaint(paint);
        canvas.DrawRect(Drawing::Rect(0, 0, 1, 1)); // 0:left, 0:top, 1:right, 1:bottom
        canvas.DetachPaint();
    } else {
        ROSEN_LOGE("AndroidSurfaceTexture::DrawTextureImage failed,imageShader == nullptr");
    }
}
} // namespace Rosen
} // namespace OHOS
