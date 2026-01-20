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

#include "rs_surface_frame_android_vulkan.h"
#include "platform/common/rs_log.h"

#include "platform/common/rs_system_properties.h"
#include "rs_trace.h"

namespace OHOS {
namespace Rosen {

RSSurfaceFrameAndroidVulkan::RSSurfaceFrameAndroidVulkan(std::shared_ptr<Drawing::Surface> surface, int32_t width,
    int32_t height, int32_t bufferAge)
    : RSSurfaceFrameAndroid(width, height), surface_(surface), bufferAge_(bufferAge)
{
}

void RSSurfaceFrameAndroidVulkan::SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height)
{
    if (!RSSystemProperties::IsUseVulkan()) {
        return;
    }
    RS_TRACE_FUNC();
    std::vector<Drawing::RectI> rects;
    Drawing::RectI rect = {left, top, left + width, top + height};
    rects.push_back(rect);
    surface_->SetDrawingArea(rects);
}

void RSSurfaceFrameAndroidVulkan::SetDamageRegion(const std::vector<RectI>& rects)
{
    if (!RSSystemProperties::IsUseVulkan()) {
        return;
    }
    RS_TRACE_FUNC();
    std::vector<Drawing::RectI> iRects;
    for (auto &rect : rects) {
        Drawing::RectI iRect = {rect.GetLeft(), rect.GetTop(), rect.GetRight(), rect.GetBottom()};
        iRects.push_back(iRect);
    }
    surface_->SetDrawingArea(iRects);
}

Drawing::Canvas* RSSurfaceFrameAndroidVulkan::GetCanvas()
{
    if (surface_ == nullptr) {
        return nullptr;
    }

    Drawing::Canvas* canvas = surface_->GetCanvas().get();
    if (canvas == nullptr) {
        return nullptr;
    }
  
    return canvas;
}

std::shared_ptr<Drawing::Surface> RSSurfaceFrameAndroidVulkan::GetSurface()
{
    return surface_;
}

int32_t RSSurfaceFrameAndroidVulkan::GetReleaseFence() const
{
    return releaseFence_;
}

void RSSurfaceFrameAndroidVulkan::SetReleaseFence(const int32_t& fence)
{
    releaseFence_ = fence;
}

int32_t RSSurfaceFrameAndroidVulkan::GetBufferAge() const
{
    return bufferAge_;
}
} // namespace Rosen
} // namespace OHOS
