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

#include "rs_surface_frame_ios.h"
#include <queue>
#include <vector>
#include "platform/common/rs_log.h"
#include "render_context/render_context.h"

namespace OHOS {
namespace Rosen {
RSSurfaceFrameIOS::RSSurfaceFrameIOS(int32_t width, int32_t height)
    : width_(width), height_(height)
{
}

void RSSurfaceFrameIOS::SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height)
{
}

int32_t RSSurfaceFrameIOS::GetBufferAge() const
{
    return -1;
}

#ifndef USE_ROSEN_DRAWING
SkCanvas* RSSurfaceFrameIOS::GetCanvas()
{
    if (surface_ == nullptr) {
        CreateSurface();
    }

    if (surface_ != nullptr) {
        return surface_->getCanvas();
    }
    return nullptr;
}

sk_sp<SkSurface> RSSurfaceFrameIOS::GetSurface()
{
    return surface_;
}
#else
Drawing::Canvas* RSSurfaceFrameIOS::GetCanvas()
{
    if (surface_ == nullptr) {
        CreateSurface();
    }

    if (surface_ != nullptr) {
        return surface_->GetCanvas().get();
    }
    return nullptr;
}

std::shared_ptr<Drawing::Surface> RSSurfaceFrameIOS::GetSurface()
{
    if (surface_ == nullptr) {
        CreateSurface();
    }
    return surface_;
}
#endif

void RSSurfaceFrameIOS::SetRenderContext(RenderContext* context)
{
    context_ = context;
}

void RSSurfaceFrameIOS::CreateSurface()
{
#ifdef USE_GPU
    if (context_ == nullptr) {
        ROSEN_LOGE("RSSurfaceFrameIOS::CreateSurface, context_ is null!");
        return;
    }
    surface_ = context_->AcquireSurface(width_, height_);
#else
    if (!surface_) {
        SkImageInfo info = SkImageInfo::MakeN32(width_, height_, kPremul_SkAlphaType, SkColorSpace::MakeSRGB());
        surface_ = SkSurface::MakeRaster(info, nullptr);
        static std::queue<sk_sp<SkSurface>> surfaceQueue;
        if (surface_) {
            surfaceQueue.push(surface_);
        }
        if (surfaceQueue.size() > 2) { // 2 for surfaceQueue max size
            surfaceQueue.pop();
        }
    }
#endif
}
} // namespace Rosen
} // namespace OHOS
