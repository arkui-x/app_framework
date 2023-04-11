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

#include "rs_surface_cpu.h"

#include <QuartzCore/CALayer.h>
#include <UIKit/UIKit.h>
#include <__nullptr>

#include <include/core/SkColorSpace.h>
#include <include/gpu/gl/GrGLInterface.h>
#include <memory>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_ios.h"
@class CALayer;

namespace OHOS {
namespace Rosen {
RSSurfaceCPU::RSSurfaceCPU(CALayer* layer)
    : layer_(layer)
{
}

bool RSSurfaceCPU::IsValid() const
{
    return layer_ != nullptr;
}

void RSSurfaceCPU::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceCPU::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC)
{
    if (!IsValid()) {
        ROSEN_LOGE("RSSurfaceCPU::RequestFrame, layer_ is nullptr");
        return nullptr;
    }
    auto frame = std::make_unique<RSSurfaceFrameIOS>(width, height);
    frame->CreateSurface();
    return frame;
}

bool RSSurfaceCPU::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (frame == nullptr) {
        ROSEN_LOGE("RSSurfaceFrame::FlushFrame frame is nullptr");
        return false;
    }
    SkPixmap pixmap;
    if (!frame->GetSurface()->peekPixels(&pixmap)) {
        ROSEN_LOGE("RSSurfaceCPU::peekPixels failed");
        return false;
    }

    CGDataProviderRef dataProvider =
        CGDataProviderCreateWithData(nullptr, pixmap.addr32(), pixmap.computeByteSize(), nullptr);
    if (!dataProvider) {
        return false;
    }
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    // create a CGImage
    CGImageRef cgImage = CGImageCreate(pixmap.width(), pixmap.height(), 8, 32, pixmap.rowBytes(),
                                       colorspace, kCGImageAlphaPremultipliedLast, dataProvider,
                                       nullptr, false, kCGRenderingIntentDefault);
    if (!cgImage) {
        return false;
    }
    CFRelease(colorspace);
    CFRelease(dataProvider);
    dispatch_sync(dispatch_get_main_queue(), ^{
        layer_.contents = reinterpret_cast<id>(static_cast<CGImageRef>(cgImage));       
        CFRelease(cgImage);
    });
    return true;
}

RenderContext* RSSurfaceCPU::GetRenderContext()
{
    return renderContext_;
}

void RSSurfaceCPU::SetRenderContext(RenderContext* context)
{
    renderContext_ = context;
}

void RSSurfaceCPU::YInvert(void *addr, int32_t width, int32_t height)
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

bool RSSurfaceCPU::SetupGrContext()
{
    return false;
}

uint32_t RSSurfaceCPU::GetQueueSize() const
{
    return 0x3;
}

void RSSurfaceCPU::ClearBuffer()
{
}

void RSSurfaceCPU::ClearAllBuffer()
{
}

void RSSurfaceCPU::ResetBufferAge()
{
    ROSEN_LOGD("RSSurfaceCPU: Reset Buffer Age!");
}
} // namespace Rosen
} // namespace OHOS
