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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_SWAP_CHAIN_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_SWAP_CHAIN_H

#include <atomic>
#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include "include/third_party/vulkan/vulkan/vulkan_core.h"
#include "rs_vulkan_context.h"

namespace OHOS {
namespace Rosen {

class RSSurfaceSwapChain {
public:
    RSSurfaceSwapChain();
    ~RSSurfaceSwapChain();
    bool Initialize(void* metalLayer);
    bool Create(int32_t width, int32_t height);
    bool CreateImpl(int32_t width, int32_t height);
    bool Recreate(int32_t width, int32_t height);
    VkSurfaceKHR GetSurface() const { return surface_; }
    void Cleanup();
    VkResult AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, uint32_t* imageIndex);
    VkResult Present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);
    void SetLayerDrawableSizeOnMain(int32_t width, int32_t height);
    bool NeedRecreate() const { return needRecreateSwapchain_.load(std::memory_order_acquire); }
    void SetNeedRecreate(bool need) { needRecreateSwapchain_.store(need, std::memory_order_release); }
    bool IsRecreating() const { return isRecreatingSwapchain_.load(std::memory_order_acquire); }
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
    // True while at least one present dispatched to the main queue timed out and its queued block
    // has not run yet: the acquired drawable of that frame has not returned to the pool.
    bool HasPendingPresentsOnMain() const { return pendingPresentsOnMain_->load(std::memory_order_acquire) > 0; }

private:
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                                int32_t width, int32_t height);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    void CleanupSyncObjects();
    VkSwapchainCreateInfoKHR BuildSwapchainCreateInfo(int32_t width, int32_t height,
        const SwapChainSupportDetails& swapChainSupport);
    bool RetrieveSwapchainImages(uint32_t& imageCount);
    VkResult PresentImpl(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore);
    void MarkRecreateFailed();
    // isTeardown: give CA a real time budget to commit the {1,1} resize — at final teardown
    // nothing will spin the run loop afterwards, and the resize-triggered completion of
    // presentations stuck on the iOS 18 Metal regression would never be processed.
    void FlushMetalLayerDrawableOnMain(bool isTeardown = false);

    void* metalLayer_ = nullptr;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchainFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent_ = {};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    bool syncObjectsCreated_ = false;
    std::atomic<bool> needRecreateSwapchain_{false};
    std::atomic<bool> isRecreatingSwapchain_{false};
    // Presents whose main-queue dispatch timed out and whose queued block has not run yet.
    // While > 0 the drawable pool is starved: acquiring more images only times out, and that
    // timeout must NOT be misread as a broken swapchain (a recreate would destroy a healthy
    // swapchain while main is busy and wedge the unbounded vkDeviceWaitIdle in Recreate).
    // shared_ptr for the same reason as mainDispatchGeneration_: the queued block's done-
    // callback decrements it and may run after this object is destroyed (busy main thread at
    // teardown) — the callback must hold only shared state, never `this`.
    std::shared_ptr<std::atomic<int32_t>> pendingPresentsOnMain_ =
        std::make_shared<std::atomic<int32_t>>(0);
    std::atomic<uint64_t> swapchainGeneration_{0};  // Incremented on each swapchain create/destroy.
    // Per-swapchain generation for discarding stale main-queue tasks (after a dispatch timeout or
    // during teardown). shared_ptr so blocks already queued on the main queue can outlive this
    // object: they read the counter through the shared state instead of touching freed members.
    // Two independent counters: layer mutations/swapchain create use mainDispatchGeneration_
    // (bumped on their timeout AND in Cleanup), presents use presentDispatchGeneration_ (bumped
    // ONLY in Cleanup) — a layer-mutation timeout must not discard a still-queued present: its
    // drawable is already acquired and would never return to the pool, forcing a pointless
    // recreate of a healthy swapchain. Presents are self-guarded by the swapchain generation
    // check inside their block.
    std::shared_ptr<std::atomic<uint64_t>> mainDispatchGeneration_;
    std::shared_ptr<std::atomic<uint64_t>> presentDispatchGeneration_ =
        std::make_shared<std::atomic<uint64_t>>(0);
    std::mutex swapchainRecreateMutex_;
    int32_t pendingWidth_ = 0;
    int32_t pendingHeight_ = 0;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_SWAP_CHAIN_H
