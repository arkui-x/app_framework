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

#include <CoreFoundation/CoreFoundation.h>
#include <QuartzCore/CAMetalLayer.h>
#include <dispatch/dispatch.h>
#include <atomic>
#include <functional>
#include <memory>
#include <pthread.h>

namespace OHOS {
namespace Rosen {
namespace {
// Avoid indefinite dispatch_sync(main) deadlocks when the UI thread is waiting on the render thread.
// Short timeout for present operations (time-sensitive).
constexpr int64_t MAIN_DISPATCH_TIMEOUT_NS = 50 * 1000000LL;
// Extended timeout for swapchain creation/recreation (can block during startup).
constexpr int64_t SWAPCHAIN_CREATE_TIMEOUT_NS = 500 * 1000000LL;

// Generation counter for discarding stale main queue tasks after timeout.
// When a timeout occurs, we increment this counter so that already-queued but not-yet-executed
// blocks will skip their work, avoiding races like "render thread gave up, but main thread still
// creates swapchain".
static std::atomic<uint64_t> g_mainDispatchGeneration{0};

// iOS 18+ requires pumping the main run loop so Core Animation can recycle IOSurface drawables.
void PumpMainRunLoopForIOS18()
{
    if (@available(iOS 18, *)) {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    }
}

void PumpMainRunLoopOnce()
{
    if (@available(iOS 18, *)) {
        PumpMainRunLoopForIOS18();
    } else {
        CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
    }
}

template<typename T>
bool DispatchOnMainWithTimeout(std::function<T()> work, T& outResult, const char* label, int64_t timeoutNs = MAIN_DISPATCH_TIMEOUT_NS)
{
    if (pthread_main_np() != 0) {
        outResult = work();
        PumpMainRunLoopOnce();
        return true;
    }

    const uint64_t currentGeneration = g_mainDispatchGeneration.load(std::memory_order_relaxed);
    struct Payload {
        std::function<T()> fn;
        T result {};
        std::atomic<bool> executed{false};
    };
    auto payload = std::make_shared<Payload>();
    payload->fn = std::move(work);

    dispatch_semaphore_t sem = dispatch_semaphore_create(0);
    dispatch_retain(sem);  // Extra reference for the block to hold.

    // Capture the retained semaphore; block will release it when done.
    dispatch_async(dispatch_get_main_queue(), [sem, payload, currentGeneration, label]() {
        // Check generation before execution - skip if stale (timeout already occurred).
        if (g_mainDispatchGeneration.load(std::memory_order_relaxed) != currentGeneration) {
            ROSEN_LOGI("%{public}s: skipping stale task (generation mismatch)", label);
            dispatch_semaphore_signal(sem);
            dispatch_release(sem);  // Release block's reference.
            return;
        }
        // Execute work and mark as done.
        payload->result = payload->fn();
        payload->executed.store(true, std::memory_order_release);
        PumpMainRunLoopOnce();
        dispatch_semaphore_signal(sem);
        dispatch_release(sem);  // Release block's reference.
    });

    const bool completed = dispatch_semaphore_wait(
        sem, dispatch_time(DISPATCH_TIME_NOW, timeoutNs)) == 0;
    dispatch_release(sem);  // Release our reference (always, whether success or timeout).

    if (!completed) {
        // Timeout: increment generation to discard any pending tasks.
        g_mainDispatchGeneration.fetch_add(1, std::memory_order_relaxed);
        ROSEN_LOGE("%{public}s: timed out waiting for main queue (avoid deadlock), generation incremented", label);
        return false;
    }

    // If block was skipped due to generation mismatch (should not happen here since we didn't timeout),
    // treat as failure.
    if (!payload->executed.load(std::memory_order_acquire)) {
        ROSEN_LOGE("%{public}s: task was skipped (generation changed)", label);
        return false;
    }

    outResult = payload->result;
    return true;
}

// MoltenVK configures CAMetalLayer during vkCreateSwapchainKHR; layer mutations must run on main thread.
// Uses extended timeout for swapchain creation/recreation which can be slow during startup.
bool RunMetalLayerMutationOnMain(std::function<bool()> work, bool useExtendedTimeout = true)
{
    bool ok = false;
    int64_t timeout = useExtendedTimeout ? SWAPCHAIN_CREATE_TIMEOUT_NS : MAIN_DISPATCH_TIMEOUT_NS;
    if (!DispatchOnMainWithTimeout(std::move(work), ok, "RunMetalLayerMutationOnMain", timeout)) {
        return false;
    }
    return ok;
}

// vkQueuePresentKHR must run on the main thread; pump run loop so CA can recycle IOSurface drawables.
VkResult RunPresentOnMain(std::function<VkResult()> work)
{
    VkResult result = VK_ERROR_UNKNOWN;
    if (!DispatchOnMainWithTimeout(std::move(work), result, "RunPresentOnMain")) {
        return VK_ERROR_DEVICE_LOST;
    }
    return result;
}
} // namespace
static constexpr uint16_t MAX_FRAMES_IN_FLIGHT = 3;
static constexpr uint32_t CONCURRENT_QUEUE_FAMILY_COUNT = 2;

// MoltenVK + CAMetalLayer: VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR forces opaque presentation so
// punch-through (e.g. native WKWebView below) reads as black. Pick first supported non-opaque mode.
static VkCompositeAlphaFlagBitsKHR ChooseSwapChainCompositeAlpha(VkCompositeAlphaFlagsKHR supported)
{
    static constexpr VkCompositeAlphaFlagBitsKHR kPreferenceOrder[] = {
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    };
    for (VkCompositeAlphaFlagBitsKHR candidate : kPreferenceOrder) {
        if ((supported & candidate) != 0U) {
            return candidate;
        }
    }
    return VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
}

RSSurfaceSwapChain::RSSurfaceSwapChain()
{
}

RSSurfaceSwapChain::~RSSurfaceSwapChain()
{
    Cleanup();
    if (metalLayer_ != nullptr) {
        [static_cast<CAMetalLayer*>(metalLayer_)  release];
    }
    ROSEN_LOGI("RSSurfaceSwapChain::release");
}

bool RSSurfaceSwapChain::Initialize(void* metalLayer)
{
    if (metalLayer == nullptr) {
        ROSEN_LOGE("RSSurfaceSwapChain::Initialize: metal layer is null");
        return false;
    }
    if (metalLayer_ != nullptr) {
        [static_cast<CAMetalLayer*>(metalLayer_) release];
    }
    metalLayer_ = [static_cast<CAMetalLayer*>(metalLayer) retain];
    return true;
}

VkSurfaceFormatKHR RSSurfaceSwapChain::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkExtent2D RSSurfaceSwapChain::ChooseSwapExtent(
    const VkSurfaceCapabilitiesKHR& capabilities,
    int32_t width, int32_t height)
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

VkPresentModeKHR RSSurfaceSwapChain::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
    }
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSwapchainCreateInfoKHR RSSurfaceSwapChain::BuildSwapchainCreateInfo(int32_t width, int32_t height,
    const SwapChainSupportDetails& swapChainSupport)
{
    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, width, height);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    
    if (imageCount < MAX_FRAMES_IN_FLIGHT) {
        imageCount = MAX_FRAMES_IN_FLIGHT;
    }
    createInfo.minImageCount = imageCount;
    swapchainFormat_ = surfaceFormat.format;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                            VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    QueueFamilyIndices indices = RsVulkanContext::GetSingleton().FindQueueFamilies(surface_);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.presentFamily != UINT32_MAX && indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = CONCURRENT_QUEUE_FAMILY_COUNT;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    createInfo.compositeAlpha =
    ChooseSwapChainCompositeAlpha(swapChainSupport.capabilities.supportedCompositeAlpha);
    ROSEN_LOGI("RSSurfaceSwapChain::SwapchainCreateInfo compositeAlpha=%{public}u supportedCompositeAlpha=0x%{public}x",
        static_cast<uint32_t>(createInfo.compositeAlpha),
        static_cast<uint32_t>(swapChainSupport.capabilities.supportedCompositeAlpha));
    createInfo.presentMode = presentMode;
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
        ROSEN_LOGE("RSSurfaceSwapChain::Create failed to get swap chain images count");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }
    ROSEN_LOGD("RSSurfaceSwapChain::RetrieveSwapchainImages imageCount=%{public}u", imageCount);
    swapchainImages_.resize(imageCount);
    if (vkInterface.vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, swapchainImages_.data()) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to get swap chain images");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

bool RSSurfaceSwapChain::Create(int32_t width, int32_t height)
{
    // Note: We do NOT check isRecreatingSwapchain_ here because Recreate() calls this method
    // legitimately after setting isRecreatingSwapchain_ = true. The check should only be done
    // by external callers (like RequestFrame) via IsRecreating().
    return RunMetalLayerMutationOnMain([this, width, height]() { return CreateImpl(width, height); });
}

bool RSSurfaceSwapChain::CreateImpl(int32_t width, int32_t height)
{
    if (width <= 0 || height <= 0) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Invalid dimensions: %dx%d", width, height);
        return false;
    }

    if (metalLayer_ == nullptr) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create: metal layer is null");
        return false;
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();

    if (surface_ == VK_NULL_HANDLE && !vkContext.GetRsVulkanInterface().CreateMetalSurface(metalLayer_, surface_)) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to create Vulkan surface via MoltenVK");
        return false;
    }

    SwapChainSupportDetails swapChainSupport = vkContext.GetRsVulkanInterface().QuerySwapChainSupport(surface_);
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        ROSEN_LOGI("RSSurfaceSwapChain::Create Swap chain support details are incomplete");
        return false;
    }
    VkSwapchainCreateInfoKHR createInfo = BuildSwapchainCreateInfo(width, height, swapChainSupport);

    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkResult result = vkInterface.vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_);
    if (result != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to create swap chain, result: %d", result);
        return false;
    }

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
    if (!RetrieveSwapchainImages(imageCount)) {
        return false;
    }

    if (!CreateSyncObjects()) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to create sync objects");
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

void RSSurfaceSwapChain::MarkRecreateFailed()
{
    isRecreatingSwapchain_.store(false, std::memory_order_release);
    needRecreateSwapchain_.store(true, std::memory_order_release);
}

bool RSSurfaceSwapChain::Recreate(int32_t width, int32_t height)
{
    std::lock_guard<std::mutex> lock(swapchainRecreateMutex_);

    if (isRecreatingSwapchain_.load(std::memory_order_acquire)) {
        return false;
    }

    isRecreatingSwapchain_.store(true, std::memory_order_release);
    needRecreateSwapchain_.store(false, std::memory_order_release);

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    vkInterface.vkDeviceWaitIdle(device);

    swapchainGeneration_.fetch_add(1, std::memory_order_release);

    VkSwapchainKHR oldSwapchain = swapchain_;
    VkSurfaceKHR oldSurface = surface_;
    swapchain_ = VK_NULL_HANDLE;
    surface_ = VK_NULL_HANDLE;

    FlushMetalLayerDrawableOnMain();

    if (metalLayer_ == nullptr) {
        ROSEN_LOGE("RSSurfaceSwapChain::Recreate: metal layer is null");
        swapchain_ = oldSwapchain;
        surface_ = oldSurface;
        MarkRecreateFailed();
        return false;
    }
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkInterface.vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }

    if (oldSurface != VK_NULL_HANDLE) {
        vkContext.DestroySurfaceKHR(oldSurface);
    }

    CleanupSyncObjects();

    if (!vkContext.GetRsVulkanInterface().CreateMetalSurface(metalLayer_, surface_)) {
        ROSEN_LOGE("RSSurfaceSwapChain::Recreate: CreateMetalSurface failed");
        MarkRecreateFailed();
        return false;
    }
    if (!Create(width, height)) {
        ROSEN_LOGE("RSSurfaceSwapChain::Recreate: Create swapchain failed");
        if (surface_ != VK_NULL_HANDLE) {
            vkContext.DestroySurfaceKHR(surface_);
            surface_ = VK_NULL_HANDLE;
        }
        MarkRecreateFailed();
        return false;
    }
    swapchainGeneration_.fetch_add(1, std::memory_order_release);
    isRecreatingSwapchain_.store(false, std::memory_order_release);
    return true;
}

void RSSurfaceSwapChain::FlushMetalLayerDrawableOnMain()
{
    if (metalLayer_ == nullptr) {
        return;
    }
    auto* layer = static_cast<CAMetalLayer*>(metalLayer_);
    if (@available(iOS 18, *)) {
        // iOS 18+: layer mutations and run loop pump must happen on the main thread.
        // Use extended timeout since this is called during swapchain recreation.
        bool success = RunMetalLayerMutationOnMain([layer]() {
            layer.drawableSize = CGSizeMake(1, 1);
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
            }
            return true;
        }, true);  // useExtendedTimeout = true
        if (!success) {
            ROSEN_LOGW("FlushMetalLayerDrawableOnMain: failed to flush on main thread");
        }
        return;
    }
    // iOS 17 and below: avoid dispatch_sync to main during render-thread teardown (deadlock with UI).
    layer.drawableSize = CGSizeMake(1, 1);
}

void RSSurfaceSwapChain::SetLayerDrawableSizeOnMain(int32_t width, int32_t height)
{
    if (@available(iOS 18, *)) {
        // iOS 18+: layer mutations must happen on the main thread.
        // Use extended timeout since this may be called during startup.
        bool success = RunMetalLayerMutationOnMain([this, width, height]() {
            if (metalLayer_ == nullptr) {
                return true;
            }
            auto* layer = static_cast<CAMetalLayer*>(metalLayer_);
            layer.drawableSize = CGSizeMake(width, height);
            return true;
        }, true);  // useExtendedTimeout = true
        if (!success) {
            ROSEN_LOGW("SetLayerDrawableSizeOnMain: failed to set size on main thread");
        }
        return;
    }
    if (metalLayer_ == nullptr) {
        return;
    }
    // iOS 17-: post to main without blocking the render thread (sync main can deadlock with UI).
    // Retain the layer to ensure it stays alive until the block completes.
    CAMetalLayer* layer = static_cast<CAMetalLayer*>(metalLayer_);
    CFRetain(layer);
    dispatch_async(dispatch_get_main_queue(), ^{
        layer.drawableSize = CGSizeMake(width, height);
        CFRelease(layer);
    });
}

void RSSurfaceSwapChain::Cleanup()
{
    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    if (device != VK_NULL_HANDLE && vkInterface.vkDeviceWaitIdle) {
        vkInterface.vkDeviceWaitIdle(device);
    }
    FlushMetalLayerDrawableOnMain();
    CleanupSyncObjects();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkInterface.vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }
    if (surface_ != VK_NULL_HANDLE) {
        vkContext.DestroySurfaceKHR(surface_);
        surface_ = VK_NULL_HANDLE;
    }

    swapchainImages_.clear();
    swapchainFormat_ = VK_FORMAT_UNDEFINED;
    swapChainExtent_ = {};
}

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

VkResult RSSurfaceSwapChain::Present(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    // Capture the current swapchain generation to detect if swapchain was recreated
    // while this present was pending on the main queue.
    const uint64_t currentGeneration = swapchainGeneration_.load(std::memory_order_acquire);
    if (swapchain_ == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    return RunPresentOnMain([this, currentGeneration, queue, imageIndex, waitSemaphore]() {
        // Check if swapchain was recreated while we were waiting for main queue.
        if (swapchainGeneration_.load(std::memory_order_acquire) != currentGeneration) {
            ROSEN_LOGI("PresentImpl: swapchain was recreated, skipping present");
            return VK_ERROR_OUT_OF_DATE_KHR;
        }
        return PresentImpl(queue, imageIndex, waitSemaphore);
    });
}

VkResult RSSurfaceSwapChain::PresentImpl(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
{
    if (swapchain_ == VK_NULL_HANDLE || queue == VK_NULL_HANDLE) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

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

    return vkInterface.vkQueuePresentKHR(queue, &presentInfo);
}

bool RSSurfaceSwapChain::CreateSyncObjects()
{
    if (syncObjectsCreated_) {
        CleanupSyncObjects();
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkInterface.vkCreateSemaphore(device, &semaphoreInfo, nullptr,
            &imageAvailableSemaphores_[i]) != VK_SUCCESS || vkInterface.vkCreateSemaphore(device,
            &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS) {
            ROSEN_LOGI("RSSurfaceSwapChain::CreateSyncObjects Failed for frame %zu", i);
            for (size_t j = 0; j < i; j++) {
                vkInterface.vkDestroySemaphore(device, imageAvailableSemaphores_[j], nullptr);
                vkInterface.vkDestroySemaphore(device, renderFinishedSemaphores_[j], nullptr);
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
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    for (size_t i = 0; i < imageAvailableSemaphores_.size(); i++) {
        vkInterface.vkDestroySemaphore(device, imageAvailableSemaphores_[i], nullptr);
    }
    for (size_t i = 0; i < renderFinishedSemaphores_.size(); i++) {
        vkInterface.vkDestroySemaphore(device, renderFinishedSemaphores_[i], nullptr);
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
