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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_VULKAN_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_VULKAN_H

#include <cstdint>

#include <android/native_window.h>
#include "include/third_party/vulkan/vulkan/vulkan_core.h"
#include "render_context/new_render_context/render_context_vk.h"
#include "image/image.h"
#include "rs_surface_android.h"
#include "rs_surface_frame_android_vulkan.h"
#include "rs_surface_swap_chain.h"

namespace OHOS {
namespace Rosen {
class RSSurfaceAndroidVulkan : public RSSurfaceAndroid {
public:
    explicit RSSurfaceAndroidVulkan(ANativeWindow* data);
    ~RSSurfaceAndroidVulkan() override;

    std::unique_ptr<RSSurfaceFrame> RequestFrame(
        int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC = true, bool isProtected = false) override;
    bool FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp) override;
    void SetColorSpace(GraphicColorGamut colorSpace) override;
    void ClearBuffer() override;
    void SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp) override;
    std::shared_ptr<Drawing::Surface> CreateSkiaSurfaceFromSwapchainImage(
        uint32_t imageIndex, int32_t width, int32_t height, bool isProtected);

private:
    std::shared_ptr<RenderContextVK> GetRenderContextVulkan() const;
    bool SetupGrContext() override;
    void SetNativeWindowInfo(int32_t width, int32_t height, bool useAFBC, bool isProtected = false);
    bool RecreateSwapchainIfNeeded(int32_t width, int32_t height);
    uint32_t AcquireSwapchainImage();
    std::shared_ptr<Drawing::Surface> GetOrCreateSkiaSurface(
        uint32_t imageIndex, int32_t swapchainWidth, int32_t swapchainHeight, bool isProtected);
    bool FlushSkiaSurface(std::shared_ptr<Drawing::Surface> surface, VkSemaphore renderFinishedSemaphore);
    void WaitAndSubmitSkiaContext(VkSemaphore waitSemaphore);
    bool PresentSwapchainImage(VkQueue queue, uint32_t imageIndex, VkSemaphore renderFinishedSemaphore);
    Drawing::ColorType ConvertVkFormatToSkiaColorType(VkFormat imageFormat);
    RSSurfaceSwapChain swapChain_;
    std::vector<std::shared_ptr<Drawing::Surface>> skiaSurfaces_;
    uint32_t lastPresentedImageIndex_ = UINT32_MAX;
    size_t currentFrame_ = 0;
    int32_t currentWidth_ = -1;
    int32_t currentHeight_ = 0;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_VULKAN_H