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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_VULKAN_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_VULKAN_H

#include <surface.h>

#ifdef USE_M133_SKIA
#include "include/gpu/ganesh/GrDirectContext.h"
#else
#include "include/gpu/GrDirectContext.h"
#endif

#include "rs_surface_frame_android.h"

namespace OHOS {
namespace Rosen {

class RSSurfaceFrameAndroidVulkan : public RSSurfaceFrameAndroid {
public:
    RSSurfaceFrameType GetType() const override
    {
        return RSSurfaceFrameType::RS_SURFACE_FRAME_OHOS_VULKAN;
    }

    RSSurfaceFrameAndroidVulkan(std::shared_ptr<Drawing::Surface> surface, int32_t width,
        int32_t height, int32_t bufferAge);
    ~RSSurfaceFrameAndroidVulkan() override = default;

    Drawing::Canvas* GetCanvas() override;
    std::shared_ptr<Drawing::Surface> GetSurface() override;
    void SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height) override;
    void SetDamageRegion(const std::vector<RectI>& rects) override;
    int32_t GetReleaseFence() const;
    void SetReleaseFence(const int32_t& fence);
    int32_t GetBufferAge() const override;
    void SetSwapchainImageIndex(uint32_t index) { imageIndex_ = index; }
    uint32_t GetSwapchainImageIndex() const { return imageIndex_; }
    void SetSurfacePhysicalSize(int32_t physicalWidth, int32_t physicalHeight)
    {
        surfacePhysicalWidth_ = physicalWidth;
        surfacePhysicalHeight_ = physicalHeight;
    }

private:

    std::shared_ptr<Drawing::Surface> surface_;
    int width_ = 0;
    int height_ = 0;
    int32_t surfacePhysicalWidth_ = 0;   // Surface 的实际物理尺寸
    int32_t surfacePhysicalHeight_ = 0;
    uint32_t imageIndex_ = 0;
    int32_t bufferAge_ = -1;
    int32_t releaseFence_ = -1;
    void CreateCanvas();
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_VULKAN_H
