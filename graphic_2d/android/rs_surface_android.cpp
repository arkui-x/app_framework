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

#include "rs_surface_android.h"

#include <include/core/SkColorSpace.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <include/gpu/GrBackendSurface.h>
#include <stdint.h>
#include <securec.h>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_android.h"
#include "render_context/render_context.h"

namespace OHOS {
namespace Rosen {
RSSurfaceAndroid::RSSurfaceAndroid(ANativeWindow* data)
    : nativeWindow_(data)
{
    ROSEN_LOGD("RSSurfaceAndroid entry with %p", nativeWindow_);
}

RSSurfaceAndroid::~RSSurfaceAndroid()
{
    if (renderContext_ != nullptr) {
        renderContext_->DestroyEGLSurface(eglSurface_);
    }
    nativeWindow_ = nullptr;
    eglSurface_ = EGL_NO_SURFACE;
}

bool RSSurfaceAndroid::IsValid() const
{
    return nativeWindow_ != nullptr;
}

void RSSurfaceAndroid::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceAndroid::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC, bool isProtected)
{
    if (nativeWindow_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::RequestFrame, producer is nullptr");
        return nullptr;
    }
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::RequestFrame, renderContext is nullptr");
        return nullptr;
    }
    if (!SetupGrContext()) {
        return nullptr;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow_, width, height, WINDOW_FORMAT_RGBA_8888);
    if (eglSurface_ == nullptr) {
        eglSurface_ = renderContext_->CreateEGLSurface(static_cast<EGLNativeWindowType>(nativeWindow_));
    }

    auto frame = std::make_unique<RSSurfaceFrameAndroid>(width, height);
    renderContext_->MakeCurrent(eglSurface_);

    ROSEN_LOGD("RSSurfaceAndroid:RequestFrame, eglsurface is %p, width is %d, height is %d, renderContext_ %p",
        eglSurface_, width, height, renderContext_);

    frame->SetRenderContext(renderContext_);

    return frame;
}

bool RSSurfaceAndroid::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::FlushFrame, GetRenderContext failed!");
        return false;
    }

    // gpu render flush
    renderContext_->RenderFrame();
    renderContext_->SwapBuffers(eglSurface_);
    ROSEN_LOGD("RSSurfaceAndroid: FlushFrame, SwapBuffers eglsurface is %p", eglSurface_);
    if (auto grContext = renderContext_->GetDrGPUContext()) {
        grContext->PurgeUnlockedResources(true);
    }
    return true;
}

RenderContext* RSSurfaceAndroid::GetRenderContext()
{
    return renderContext_;
}

void RSSurfaceAndroid::SetRenderContext(RenderContext* context)
{
    renderContext_ = context;
}

void RSSurfaceAndroid::YInvert(void *addr, int32_t width, int32_t height)
{
}

bool RSSurfaceAndroid::SetupGrContext()
{
    if (renderContext_) {
        renderContext_->InitializeEglContext();
        renderContext_->SetUpGpuContext();
        renderContext_->SetColorSpace(colorSpace_);
    }
    return true;
}

uint32_t RSSurfaceAndroid::GetQueueSize() const
{
    return 0x3;
}

void RSSurfaceAndroid::ClearBuffer()
{
}

void RSSurfaceAndroid::ClearAllBuffer()
{
}

void RSSurfaceAndroid::ResetBufferAge()
{
    ROSEN_LOGD("RSSurfaceAndroid: Reset Buffer Age!");
}

GraphicColorGamut RSSurfaceAndroid::GetColorSpace() const
{
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::GetColorSpace, renderContext_ is null  %u", colorSpace_);
        return colorSpace_;
    }
     return renderContext_->GetColorSpace();
}

void RSSurfaceAndroid::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGD("RSSurfaceAndroid::SetColorSpace colorSpace %u", colorSpace);
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::SetColorSpace renderContext_ is null");
        return;
    }
    colorSpace_ = colorSpace;
    renderContext_->SetColorSpace(colorSpace_);
}

RSSurfaceExtPtr RSSurfaceAndroid::CreateSurfaceExt(const RSSurfaceExtConfig& config)
{
    ROSEN_LOGD("RSSurfaceAndroid::CreateSurfaceExt %{public}u", config.type);
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            if (texture_ == nullptr) {
                texture_ = std::make_shared<AndroidSurfaceTexture>(this, config);
            }
            return texture_;
        }
        default:
            return nullptr;
    }
}

RSSurfaceExtPtr RSSurfaceAndroid::GetSurfaceExt(const RSSurfaceExtConfig& config)
{
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            return texture_;
        }
        default:
            return nullptr;
    }
}

AndroidSurfaceTexture::AndroidSurfaceTexture(RSSurfaceAndroid* surface, const RSSurfaceExtConfig& config)
    : RSSurfaceExt(), config_(std::move(config)), transform_(SkMatrix::I())
{
}

AndroidSurfaceTexture::~AndroidSurfaceTexture()
{
    ROSEN_LOGD("AndroidSurfaceTexture::~AndroidSurfaceTexture textureId_ %{public}d ", textureId_);
    if (textureId_ > 0 && attachCallback_ != nullptr) {
        attachCallback_(textureId_, false);
    }
}

void AndroidSurfaceTexture::MarkUiFrameAvailable(bool available)
{
    ROSEN_LOGD("AndroidSurfaceTexture::MarkUiFrameAvailable textureId_ %{public}d ", textureId_);
    bufferAvailable_.store(available);
}

static inline SkSize ScaleToFill(float scaleX, float scaleY)
{
    const double epsilon = std::numeric_limits<double>::epsilon();
    /* scaleY is negative. */ 
    const double minScale = fmin(scaleX, fabs(scaleY));
    const double rescale = 1.0f / (minScale + epsilon);
    return SkSize::Make(scaleX * rescale, scaleY * rescale);
}

void AndroidSurfaceTexture::updateTransform()
{
    std::vector<float> matrix {};
    updateCallback_(matrix);
    if (matrix.size() == 16) { // 16 max len
        const SkSize scaled = ScaleToFill(matrix[0], matrix[5]); // 5 index
        SkScalar matrix3[] = {
            scaled.fWidth, matrix[1], matrix[2], matrix[4], // 2 4 index
            scaled.fHeight, matrix[6], matrix[8], matrix[9], matrix[10]}; // 6,8,9,10 index
        transform_.set9(matrix3);
    }
}

void AndroidSurfaceTexture::UpdateSurfaceDefaultSize(float width, float height)
{
    width_ = width;
    height_ = height;
}

#ifndef USE_ROSEN_DRAWING
void AndroidSurfaceTexture::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const SkRect& clipRect)
#else
void AndroidSurfaceTexture::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
#endif
{
    if (state_ == AttachmentState::detached || attachCallback_ == nullptr || updateCallback_ == nullptr) {
        return;
    }
#ifndef USE_ROSEN_DRAWING
    auto x = clipRect.x();
    auto y = clipRect.y();
    auto width = clipRect.width();
    auto height = clipRect.height();
#else
    auto x = clipRect.GetLeft();
    auto y = clipRect.GetTop();
    auto width = clipRect.GetWidth();
    auto height = clipRect.GetHeight();
#endif
    if (state_ == AttachmentState::uninitialized) {
        if (!bufferAvailable_.load()){
            return;
        }
        glGenTextures(1, &textureId_);
        ROSEN_LOGD("AndroidSurfaceTexture::DrawTextureImage attachCallback textureId %{public}d", textureId_);
        attachCallback_(textureId_, true);
        state_ = AttachmentState::attached;
    }
    bool bufferAvailable = bufferAvailable_.load();
    if (!freeze && bufferAvailable_.load()) {
        updateTransform();
        bufferAvailable_.store(false);
    }

    ROSEN_LOGD("AndroidSurfaceTexture::textureId_ %{public}d %{public}d %{public}d",
        textureId_, bufferAvailable, transform_.isIdentity());
#ifndef USE_ROSEN_DRAWING
    GrGLTextureInfo textureInfo = {GL_TEXTURE_EXTERNAL_OES, textureId_, GL_RGBA8_OES};
    GrBackendTexture backendTexture((int)width_, (int)height_, GrMipMapped::kNo, textureInfo);
    auto image = SkImage::MakeFromTexture(
        canvas.recordingContext(), backendTexture, kTopLeft_GrSurfaceOrigin,
        kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr);
    if (!image) {
        return;
    }
    canvas.drawImage(image, x, y);
#else
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGD("create Drawing image fail");
        return;
    }
    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth((int)width_);
    textureInfo.SetHeight((int)height_);
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(GL_TEXTURE_EXTERNAL_OES);
    textureInfo.SetID(textureId_);
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        return;
    }
    canvas.DrawImage(*image, x, y, Drawing::SamplingOptions());
#endif
}
} // namespace Rosen
} // namespace OHOS
