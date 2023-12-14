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
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC)
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

    ROSEN_LOGD("RSSurfaceAndroid:RequestFrame, eglsurface is %p, width is %d, height is %d",
        eglSurface_, width, height);

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
    if (auto grContext = renderContext_->GetGrContext()) {
        grContext->purgeUnlockedResources(true);
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
    // if (renderContext_->GetGrContext() != nullptr) {
    //     return true;
    // }
    if (renderContext_) {
        renderContext_->InitializeEglContext();
        renderContext_->SetUpGrContext();
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
} // namespace Rosen
} // namespace OHOS
