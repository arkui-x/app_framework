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

#include "rs_surface_gpu.h"

#include <QuartzCore/CALayer.h>
#include <UIKit/UIKit.h>
#include <__nullptr>

#include <include/core/SkColorSpace.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <memory>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_ios.h"


namespace OHOS {
namespace Rosen {
RSSurfaceGPU::RSSurfaceGPU(void* layer)
    : layer_(layer)
{
}

bool RSSurfaceGPU::IsValid() const
{
    return layer_ != nullptr && renderContext_ != nullptr;
}

void RSSurfaceGPU::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceGPU::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC)
{
    if (!IsValid()) {
        ROSEN_LOGE("RSSurfaceGPU::RequestFrame, layer_ is nullptr");
        return nullptr;
    }
    
    renderContext_->CreateEGLSurface(layer_);
    renderContext_->MakeCurrent();
    auto frame = std::make_unique<RSSurfaceFrameIOS>(width, height);
    frame->SetRenderContext(renderContext_);
    frame->CreateSurface();
    return frame;
}

bool RSSurfaceGPU::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (frame == nullptr) {
        ROSEN_LOGE("RSSurfaceFrame::FlushFrame frame is nullptr");
        return false;
    }

    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::FlushFrame, GetRenderContext failed!");
        return false;
    }

    // gpu render flush
    renderContext_->MakeCurrent();
    renderContext_->RenderFrame();
    renderContext_->SwapBuffers();

    return true;
}

RenderContext* RSSurfaceGPU::GetRenderContext()
{
    return renderContext_;
}

void RSSurfaceGPU::SetRenderContext(RenderContext* context)
{
    renderContext_ = context;
}

void RSSurfaceGPU::YInvert(void *addr, int32_t width, int32_t height)
{
    const auto &pixels = reinterpret_cast<uint32_t *>(addr);
    const auto &halfHeight = height / 0x2;
    const auto &tmpPixels = std::make_unique<uint32_t[]>(width * halfHeight);
    for (int32_t i = 0; i < halfHeight; i++) {
        for (int32_t j = 0; j < width; j++) {
            tmpPixels[i * width + j] = pixels[i * width + j];
        }
    }

    for (int32_t i = 0; i < halfHeight; i++) {
        const auto &r = height - 1 - i;
        for (int32_t j = 0; j < width; j++) {
            pixels[i * width + j] = pixels[r * width + j];
        }
    }

    for (int32_t i = 0; i < halfHeight; i++) {
        const auto &r = height - 1 - i;
        for (int32_t j = 0; j < width; j++) {
            pixels[r * width + j] = tmpPixels[i * width + j];
        }
    }
}

bool RSSurfaceGPU::SetupGrContext()
{
    return false;
}

uint32_t RSSurfaceGPU::GetQueueSize() const
{
    return 0x3;
}

void RSSurfaceGPU::ClearBuffer()
{
}

void RSSurfaceGPU::ClearAllBuffer()
{
}

void RSSurfaceGPU::ResetBufferAge()
{
    ROSEN_LOGD("RSSurfaceGPU: Reset Buffer Age!");
}
} // namespace Rosen
} // namespace OHOS
