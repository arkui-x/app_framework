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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_SWAP_CHAIN_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_SWAP_CHAIN_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <android/native_window.h>
#include "include/third_party/vulkan/vulkan/vulkan_core.h"
#include "rs_vulkan_context.h"

namespace OHOS {
namespace Rosen {

class RSSurfaceSwapChain {
public:
    RSSurfaceSwapChain();
    ~RSSurfaceSwapChain();
    bool Initialize(ANativeWindow* nativeWindow);
    bool Create(int32_t width, int32_t height);
    bool Recreate(int32_t width, int32_t height);
    void Cleanup();
    VkResult AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, uint32_t* imageIndex);
    VkResult Present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);
    bool NeedRecreate() const { return needRecreateSwapchain_; }
    void SetNeedRecreate(bool need) { needRecreateSwapchain_ = need; }
    bool IsRecreating() const { return isRecreatingSwapchain_; }
    VkSwapchainKHR GetSwapchain() const { return swapchain_; }
    VkFormat GetFormat() const { return swapchainFormat_; }
    VkExtent2D GetExtent() const { return swapChainExtent_; }
    const std::vector<VkImage>& GetImages() const { return swapchainImages_; }
    size_t GetImageCount() const { return swapchainImages_.size(); }
    bool CreateSyncObjects();
    VkSemaphore GetImageAvailableSemaphore(size_t frameIndex) const;
    VkSemaphore GetRenderFinishedSemaphore(size_t frameIndex) const;
    void SetPendingSize(int32_t width, int32_t height);
    void GetPendingSize(int32_t* width, int32_t* height) const;

private:
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                int32_t width, int32_t height,
                                VkSurfaceTransformFlagBitsKHR preTransform);
    void CleanupSyncObjects();
    VkSwapchainCreateInfoKHR BuildSwapchainCreateInfo(int32_t width, int32_t height,
        const SwapChainSupportDetails& swapChainSupport);
    bool RetrieveSwapchainImages(uint32_t& imageCount);

private:
    ANativeWindow* nativeWindow_ = nullptr;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchainFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent_ = {};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    bool syncObjectsCreated_ = false;
    bool needRecreateSwapchain_ = false;
    bool isRecreatingSwapchain_ = false;
    std::mutex swapchainRecreateMutex_;
    int32_t pendingWidth_ = 0;
    int32_t pendingHeight_ = 0;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_SWAP_CHAIN_H
