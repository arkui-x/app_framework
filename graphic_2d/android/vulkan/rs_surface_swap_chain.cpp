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

#include "rs_surface_swap_chain.h"
#include "platform/common/rs_log.h"
#include "rs_vulkan_context.h"

namespace OHOS {
namespace Rosen {

static constexpr uint16_t MAX_FRAMES_IN_FLIGHT = 2;
// Number of queue families when graphics and present queues are different
static constexpr uint32_t CONCURRENT_QUEUE_FAMILY_COUNT = 2;

RSSurfaceSwapChain::RSSurfaceSwapChain()
{
}

RSSurfaceSwapChain::~RSSurfaceSwapChain()
{
    Cleanup();
}

bool RSSurfaceSwapChain::Initialize(ANativeWindow* nativeWindow)
{
    if (nativeWindow == nullptr) {
        ROSEN_LOGD("RSSurfaceSwapChain::Initialize: native window is null");
        return false;
    }
    nativeWindow_ = nativeWindow;
    return true;
}

// Choose optimal Vulkan surface format from available formats
VkSurfaceFormatKHR RSSurfaceSwapChain::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

// Choose swapchain extent within Vulkan surface capabilities
VkExtent2D RSSurfaceSwapChain::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    int32_t width, int32_t height,
    VkSurfaceTransformFlagBitsKHR preTransform)
{
    VkExtent2D logicalExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
    logicalExtent.width = std::max(capabilities.minImageExtent.width,
                                   std::min(capabilities.maxImageExtent.width, logicalExtent.width));
    logicalExtent.height = std::max(capabilities.minImageExtent.height,
                                    std::min(capabilities.maxImageExtent.height, logicalExtent.height));

    return logicalExtent;
}

VkSwapchainCreateInfoKHR RSSurfaceSwapChain::BuildSwapchainCreateInfo(int32_t width, int32_t height,
    const SwapChainSupportDetails& swapChainSupport)
{
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkSurfaceTransformFlagBitsKHR preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, width, height, preTransform);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = RsVulkanContext::GetSingleton().GetSurface();
    createInfo.minImageCount = imageCount;
    swapchainFormat_ = surfaceFormat.format;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    QueueFamilyIndices indices = RsVulkanContext::GetSingleton().FindQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = CONCURRENT_QUEUE_FAMILY_COUNT;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = preTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    createInfo.clipped = VK_TRUE;
    swapChainExtent_ = extent;
    return createInfo;
}

bool RSSurfaceSwapChain::RetrieveSwapchainImages(uint32_t& imageCount)
{
    auto& vkContext = RsVulkanContext::GetSingleton();
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    if (vkInterface.vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, nullptr) != VK_SUCCESS) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create failed to get swap chain images count");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }
    swapchainImages_.resize(imageCount);
    if (vkInterface.vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, swapchainImages_.data()) != VK_SUCCESS) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create Failed to get swap chain images");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool RSSurfaceSwapChain::Create(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create Invalid dimensions for swapchain: %dx%d", width, height);
        return false;
    }

    if (nativeWindow_ == nullptr) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create: native window is null");
        return false;
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();

    if (!vkContext.GetSurface() && !vkContext.CreateAndroidSurface(nativeWindow_)) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create Failed to create Vulkan surface");
        return false;
    }
    // Query Vulkan swapchain capabilities
    SwapChainSupportDetails swapChainSupport = vkContext.QuerySwapChainSupport();
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        ROSEN_LOGI("RSSurfaceSwapChain::Create Swap chain support details are incomplete");
        return false;
    }
    VkSwapchainCreateInfoKHR createInfo = BuildSwapchainCreateInfo(width, height, swapChainSupport);

    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkResult result = vkInterface.vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_);
    if (result != VK_SUCCESS) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create Failed to create swap chain");
        return false;
    }

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (!RetrieveSwapchainImages(imageCount)) {
        return false;
    }

    if (!CreateSyncObjects()) {
        ROSEN_LOGD("RSSurfaceSwapChain::Create Failed to create sync objects");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

bool RSSurfaceSwapChain::Recreate(int32_t width, int32_t height)
{
    std::lock_guard<std::mutex> lock(swapchainRecreateMutex_);

    if (isRecreatingSwapchain_) {
        return false;
    }

    isRecreatingSwapchain_ = true;
    needRecreateSwapchain_ = false;

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    // Wait for all Vulkan operations to complete before recreation
    vkDeviceWaitIdle(device);

    if (nativeWindow_ == nullptr) {
        ROSEN_LOGD("RSSurfaceSwapChain::Recreate: native window is null, cannot recreate swapchain");
        isRecreatingSwapchain_ = false;
        return false;
    }

    auto& vkInterface = vkContext.GetRsVulkanInterface();
    if (swapchain_ != VK_NULL_HANDLE) {
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    VkSurfaceKHR oldSurface = vkContext.GetSurface();
    if (oldSurface != VK_NULL_HANDLE) {
        vkContext.DestroySurfaceKHR(oldSurface);
    }

    CleanupSyncObjects();

    // Recreate Vulkan surface from native window
    vkContext.CreateAndroidSurface(nativeWindow_);
    if (!Create(width, height)) {
        VkSurfaceKHR newSurface = vkContext.GetSurface();
        if (newSurface != VK_NULL_HANDLE) {
            vkContext.DestroySurfaceKHR(newSurface);
        }
        isRecreatingSwapchain_ = false;
        return false;
    }

    isRecreatingSwapchain_ = false;
    return true;
}

void RSSurfaceSwapChain::Cleanup()
{
    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    CleanupSyncObjects();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    swapchainImages_.clear();
    swapchainFormat_ = VK_FORMAT_UNDEFINED;
    swapChainExtent_ = {};
}

// Acquire next image from Vulkan swapchain for rendering
VkResult RSSurfaceSwapChain::AcquireNextImage(uint64_t timeout, VkSemaphore semaphore, uint32_t* imageIndex)
{
    if (swapchain_ == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    return vkInterface.vkAcquireNextImageKHR(
        vkContext.GetDevice(), swapchain_, timeout, semaphore, VK_NULL_HANDLE, imageIndex);
}

// Present rendered image to Vulkan swapchain
VkResult RSSurfaceSwapChain::Present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    if (swapchain_ == VK_NULL_HANDLE || queue == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // Wait for render completion semaphore before presenting
    if (waitSemaphore != VK_NULL_HANDLE) {
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &waitSemaphore;
    }

    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    auto& vkContext = RsVulkanContext::GetSingleton();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    // Queue present operation
    return vkInterface.vkQueuePresentKHR(queue, &presentInfo);
}

// Create Vulkan semaphores for frame synchronization
bool RSSurfaceSwapChain::CreateSyncObjects()
{
    if (syncObjectsCreated_) {
        CleanupSyncObjects();
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();

    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS) {
            ROSEN_LOGI("RSSurfaceSwapChain::CreateSyncObjects Failed to create sync objects for frame %zu", i);
            for (size_t j = 0; j < i; j++) {
                vkDestroySemaphore(device, imageAvailableSemaphores_[j], nullptr);
                vkDestroySemaphore(device, renderFinishedSemaphores_[j], nullptr);
            }
            imageAvailableSemaphores_.clear();
            renderFinishedSemaphores_.clear();
            return false;
        }
    }

    syncObjectsCreated_ = true;
    return true;
}

void RSSurfaceSwapChain::CleanupSyncObjects()
{
    if (!syncObjectsCreated_) {
        return;
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();

    for (size_t i = 0; i < imageAvailableSemaphores_.size(); i++) {
        vkDestroySemaphore(device, imageAvailableSemaphores_[i], nullptr);
    }
    for (size_t i = 0; i < renderFinishedSemaphores_.size(); i++) {
        vkDestroySemaphore(device, renderFinishedSemaphores_[i], nullptr);
    }

    imageAvailableSemaphores_.clear();
    renderFinishedSemaphores_.clear();
    syncObjectsCreated_ = false;
}

VkSemaphore RSSurfaceSwapChain::GetImageAvailableSemaphore(size_t frameIndex) const
{
    if (frameIndex >= imageAvailableSemaphores_.size()) {
        return VK_NULL_HANDLE;
    }
    return imageAvailableSemaphores_[frameIndex];
}

VkSemaphore RSSurfaceSwapChain::GetRenderFinishedSemaphore(size_t frameIndex) const
{
    if (frameIndex >= renderFinishedSemaphores_.size()) {
        return VK_NULL_HANDLE;
    }
    return renderFinishedSemaphores_[frameIndex];
}

void RSSurfaceSwapChain::SetPendingSize(int32_t width, int32_t height)
{
    pendingWidth_ = width;
    pendingHeight_ = height;
}

void RSSurfaceSwapChain::GetPendingSize(int32_t* width, int32_t* height) const
{
    if (width) {
        *width = pendingWidth_;
    }
    if (height) {
        *height = pendingHeight_;
    }
}

} // namespace Rosen
} // namespace OHOS
