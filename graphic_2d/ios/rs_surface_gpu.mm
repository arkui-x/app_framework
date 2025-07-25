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

#include <memory>
#include <__nullptr>
#include <QuartzCore/CALayer.h>
#include <UIKit/UIKit.h>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_ios.h"
#include "rs_surface_platform_texture_ios.h"
#include "rs_surface_texture_ios.h"

namespace OHOS {
namespace Rosen {
RSSurfaceGPU::RSSurfaceGPU(void* layer)
{
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = [layer retain];
}

RSSurfaceGPU::~RSSurfaceGPU()
{
    ROSEN_LOGD("RSSurfaceGPU::release");
    if (renderContext_ != nullptr) {
        renderContext_->DestroyEGLSurface(layer_);
    }
    [static_cast<CAEAGLLayer*>(layer_) release];
    layer_ = nullptr;
}

bool RSSurfaceGPU::IsValid() const
{
    return layer_ != nullptr && renderContext_ != nullptr;
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceGPU::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC, bool isProtected)
{
    if (!IsValid()) {
        ROSEN_LOGE("RSSurfaceGPU::RequestFrame, layer_ is nullptr");
        return nullptr;
    }
    if (!SetupGrContext()) {
        return nullptr;
    }
    renderContext_->CreateEGLSurface(layer_);
    renderContext_->MakeCurrent(layer_, nullptr);
    auto frame = std::make_unique<RSSurfaceFrameIOS>(width, height);
    frame->SetRenderContext(renderContext_);
    frame->CreateSurface();
    return frame;
}

bool RSSurfaceGPU::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (frame == nullptr) {
        ROSEN_LOGE("RSSurfaceGPU::FlushFrame frame is nullptr");
        return false;
    }
    if (!IsValid()) {
        ROSEN_LOGE("RSSurfaceGPU::FlushFrame, layer_ or renderContent_ is nullptr");
        return false;
    }

    /* gpu render flush */
    renderContext_->MakeCurrent(layer_, nullptr);
    renderContext_->RenderFrame();
    renderContext_->SwapBuffers(nullptr);
    if (auto grContext = renderContext_->GetDrGPUContext()) {
        grContext->PurgeUnlockedResources(true);
    }
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

bool RSSurfaceGPU::SetupGrContext()
{
    if (renderContext_) {
        renderContext_->InitializeEglContext();
        renderContext_->SetUpGpuContext();
    }
    return true;
}

uint32_t RSSurfaceGPU::GetQueueSize() const
{
    /* cache buffer count */
    return 0x3;
}

GraphicColorGamut RSSurfaceGPU::GetColorSpace() const
{
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceGPU::GetColorSpace renderContext_ is nullptr");
        return GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    }
    return renderContext_->GetColorSpace();
}

void RSSurfaceGPU::SetColorSpace(GraphicColorGamut colorSpace)
{
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceGPU::SetColorSpace renderContext_ is nullptr");
        return;
    }
    renderContext_->SetColorSpace(colorSpace);
}

RSSurfaceExtPtr RSSurfaceGPU::CreateSurfaceExt(const RSSurfaceExtConfig& config)
{
    ROSEN_LOGD("RSSurfaceGPU::CreateSurfaceExt");
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            if (texture_ == nullptr) {
                texture_ = std::dynamic_pointer_cast<RSSurfaceExt>(std::make_shared<RSSurfaceTextureIOS>(config));
            }
            return texture_;
        }
        case RSSurfaceExtType::SURFACE_PLATFORM_TEXTURE: {
            if (texture_ == nullptr) {
                auto texture = std::make_shared<RSSurfacePlatformTextureIOS>(config);
                texture_ = std::dynamic_pointer_cast<RSSurfaceExt>(texture);
            }
            return texture_;
        }
        default:
            return nullptr;
    }
}

RSSurfaceExtPtr RSSurfaceGPU::GetSurfaceExt(const RSSurfaceExtConfig& config)
{
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            return texture_;
        }
        case RSSurfaceExtType::SURFACE_PLATFORM_TEXTURE: {
            return texture_;
        }
        default:
            return nullptr;
    }
}
} // namespace Rosen
} // namespace OHOS
