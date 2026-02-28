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

#include <cstdint>
#include <securec.h>
#include <cmath>
#include <limits>

#include "platform/common/rs_log.h"
const float TRANSLATE_VALUE = 0.5f;
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

static inline Drawing::SizeF ScaleToFill(float scaleX, float scaleY)
{
    const double epsilon = std::numeric_limits<double>::epsilon();
    /* scaleY is negative. */
    const double minScale = fmin(scaleX, fabs(scaleY));
    const double rescale = 1.0f / (minScale + epsilon);
    return Drawing::SizeF(scaleX * rescale, scaleY * rescale);
}

void AndroidSurfaceTexture::UpdateTransform()
{
    std::vector<float> matrix {};
    updateCallback_(matrix);
    if (matrix.size() == 16) { // 16 max len
        const Drawing::SizeF scaled = ScaleToFill(matrix[0], matrix[5]); // 5 index
        Drawing::Matrix::Buffer matrix3 = {
            scaled.Width(), matrix[1], matrix[2], matrix[4], // 2 4 index
            scaled.Height(), matrix[6], matrix[8], matrix[9], matrix[10]}; // 6,8,9,10 index
        transform_.SetAll(matrix3);
    }
}

void AndroidSurfaceTexture::UpdateSurfaceDefaultSize(float width, float height)
{
    const int pixelAlignment = 16;
    int widthInt = static_cast<int>(width + 0.5f);
    int heightInt = static_cast<int>(height + 0.5f);

    bool widthNeedsAlignment = (widthInt % pixelAlignment != 0);
    bool heightNeedsAlignment = (heightInt % pixelAlignment != 0);
    bool incomingHasUnaligned = (widthNeedsAlignment || heightNeedsAlignment);

    int savedWidthInt = static_cast<int>(width_ + 0.5f);
    int savedHeightInt = static_cast<int>(height_ + 0.5f);
    bool savedWidthAligned = (savedWidthInt % pixelAlignment == 0);
    bool savedHeightAligned = (savedHeightInt % pixelAlignment == 0);
    bool savedIsFullyAligned = (savedWidthAligned && savedHeightAligned);

    bool shouldSave = (width_ == 0 && height_ == 0) ||
        (incomingHasUnaligned && savedIsFullyAligned);

    if (shouldSave) {
        width_ = width;
        height_ = height;
    } else {
        return;
    }
}

bool AndroidSurfaceTexture::InitializeTextureIfNeeded()
{
    if (state_ == AttachmentState::UNINITIALIZED) {
        if (!bufferAvailable_.load()) {
            return false;
        }
        glGenTextures(1, &textureId_);
        attachCallback_(textureId_, true);
        state_ = AttachmentState::ATTACHED;
    }
    return true;
}

std::shared_ptr<Drawing::Image> AndroidSurfaceTexture::CreateTextureImage(RSPaintFilterCanvas& canvas)
{
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
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

bool AndroidSurfaceTexture::ApplyClipForAlignment(RSPaintFilterCanvas& canvas)
{
    if (width_ <= 0 || height_ <= 0) {
        return true;
    }
    const int pixelAlignment = 16;
    int checkWidth = static_cast<int>(width_ + 0.5f);
    int checkHeight = static_cast<int>(height_ + 0.5f);
    int alignedWidth = ((checkWidth + pixelAlignment - 1) / pixelAlignment) * pixelAlignment;
    int alignedHeight = ((checkHeight + pixelAlignment - 1) / pixelAlignment) * pixelAlignment;
    bool widthNeedsPadding = (alignedWidth > checkWidth);
    bool heightNeedsPadding = (alignedHeight > checkHeight);
    if (widthNeedsPadding || heightNeedsPadding) {
        float rightClip = widthNeedsPadding ? static_cast<float>(checkWidth) / alignedWidth : 1.0f;
        float bottomClip = heightNeedsPadding ? static_cast<float>(checkHeight) / alignedHeight : 1.0f;
        Drawing::Rect videoRect(0.0f, 0.0f, rightClip, bottomClip);
        canvas.ClipRect(videoRect, Drawing::ClipOp::INTERSECT, false);
        return true;
    }
    return false;
}

bool AndroidSurfaceTexture::ApplyTransformMatrix(RSPaintFilterCanvas& canvas)
{
    if (!transform_.IsIdentity()) {
        Drawing::Matrix::Buffer matrixBuffer;
        transform_.GetAll(matrixBuffer);
        Drawing::Matrix transformAroundCenter(transform_);
        transformAroundCenter.PreTranslate(-TRANSLATE_VALUE, -TRANSLATE_VALUE);
        transformAroundCenter.PostScale(1, -1);
        transformAroundCenter.PostTranslate(TRANSLATE_VALUE, TRANSLATE_VALUE);
        canvas.ConcatMatrix(transformAroundCenter);
        return true;
    }
    return false;
}

void AndroidSurfaceTexture::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
{
    if (state_ == AttachmentState::DETACHED || attachCallback_ == nullptr || updateCallback_ == nullptr) {
        return;
    }
    auto x = clipRect.GetLeft();
    auto y = clipRect.GetTop();
    auto width = clipRect.GetWidth();
    auto height = clipRect.GetHeight();
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
    canvas.Save();
    canvas.Translate(x, y);
    canvas.Scale(width, height);
    if (!ApplyClipForAlignment(canvas)) {
        return;
    }
    if (!ApplyTransformMatrix(canvas)) {
        return;
    }
    canvas.DrawImage(*image, 0, 0, Drawing::SamplingOptions());
    canvas.Restore();
}
} // namespace Rosen
} // namespace OHOS
