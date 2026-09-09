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
#include <UIKit/UIScreen.h>
#include <dispatch/dispatch.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
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
// Flood-control counter for slow-but-successful main-queue waits (see DispatchOnMainWithTimeout).
std::atomic<uint32_t> gSlowWaitLogged { 0 };

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

// Dispatches work onto the main queue and waits with a bounded timeout, so a busy main thread
// (window creation, page navigation) can never deadlock the render thread.
// `generation` is PER swapchain and PER task class (shared_ptr: queued blocks must outlive the
// swapchain object). Blocks skip their work when the counter no longer matches the value captured
// at dispatch time.
//  - cancelQueuedOnTimeout = true (layer mutation / swapchain create, mainDispatchGeneration_):
//    on timeout the counter is bumped so already-queued blocks of THIS swapchain are discarded —
//    the render thread gave up, and a late CreateImpl would race with the retry (e.g. overwrite
//    swapchain_).
//  - cancelQueuedOnTimeout = false (present, presentDispatchGeneration_): the queued present
//    remains valid — it is self-guarded by the swapchain generation check inside, and cancelling
//    it would leak the acquired image. Its generation is bumped ONLY by Cleanup.
// onQueuedBlockDone runs on BOTH the executed and the stale-skip path, i.e. possibly after this
// swapchain object is destroyed — it must only touch shared state (shared_ptr captures), never
// members of a possibly-dead object.
template<typename T>
bool DispatchOnMainWithTimeout(const std::shared_ptr<std::atomic<uint64_t>>& generation,
    std::function<T()> work, T& outResult, const char* label, int64_t timeoutNs,
    bool cancelQueuedOnTimeout, const std::function<void()>& onQueuedBlockDone = {})
{
    if (pthread_main_np() != 0) {
        outResult = work();
        PumpMainRunLoopOnce();
        return true;
    }

    const uint64_t currentGeneration = generation->load(std::memory_order_relaxed);
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
    dispatch_async(dispatch_get_main_queue(), [sem, payload, generation, currentGeneration, label,
        onQueuedBlockDone]() {
        // Check generation before execution - skip if stale (timeout or teardown already occurred).
        if (generation->load(std::memory_order_relaxed) != currentGeneration) {
            ROSEN_LOGI("%{public}s: skipping stale task (generation mismatch)", label);
            if (onQueuedBlockDone) {
                onQueuedBlockDone();
            }
            dispatch_semaphore_signal(sem);
            dispatch_release(sem);  // Release block's reference.
            return;
        }
        // Execute work and mark as done.
        payload->result = payload->fn();
        payload->executed.store(true, std::memory_order_release);
        PumpMainRunLoopOnce();
        if (onQueuedBlockDone) {
            onQueuedBlockDone();
        }
        dispatch_semaphore_signal(sem);
        dispatch_release(sem);  // Release block's reference.
    });

    const auto waitStart = std::chrono::steady_clock::now();
    const bool completed = dispatch_semaphore_wait(
        sem, dispatch_time(DISPATCH_TIME_NOW, timeoutNs)) == 0;
    // A successful but slow wait (< timeout) is invisible in every other log, yet it is
    // the per-frame cost this dispatch design imposes on the render thread whenever the
    // main thread is busy (page/window transitions, drag gesture storms). Surface it with
    // flood control: first 20 occurrences, then every 200th. Timeouts are excluded — they
    // already log below. Counter is shared by all template instantiations (per call site
    // would multiply the "first 20" budget).
    const auto waitMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - waitStart).count();
    if (completed && waitMs >= 8) {
        const uint32_t logged = gSlowWaitLogged.fetch_add(1, std::memory_order_relaxed);
        if (logged < 20 || logged % 200 == 0) {
            ROSEN_LOGW("%{public}s: main queue busy, render thread waited %{public}lldms (of %{public}lldms bound)",
                label, static_cast<long long>(waitMs), static_cast<long long>(timeoutNs / 1000000LL));
        }
    }
    dispatch_release(sem);  // Release our reference (always, whether success or timeout).

    if (!completed) {
        if (cancelQueuedOnTimeout) {
            // Timeout: bump generation to discard this swapchain's pending tasks.
            generation->fetch_add(1, std::memory_order_relaxed);
        }
        ROSEN_LOGE("%{public}s: timed out waiting for main queue (avoid deadlock), queued task %{public}s",
            label, cancelQueuedOnTimeout ? "cancelled" : "kept");
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
bool RunMetalLayerMutationOnMain(const std::shared_ptr<std::atomic<uint64_t>>& generation,
    std::function<bool()> work, bool useExtendedTimeout = true)
{
    bool ok = false;
    int64_t timeout = useExtendedTimeout ? SWAPCHAIN_CREATE_TIMEOUT_NS : MAIN_DISPATCH_TIMEOUT_NS;
    if (!DispatchOnMainWithTimeout(generation, std::move(work), ok, "RunMetalLayerMutationOnMain",
        timeout, true)) {
        return false;
    }
    return ok;
}

// vkQueuePresentKHR must run on the main thread; pump run loop so CA can recycle IOSurface drawables.
// A dispatch timeout means the main queue is busy — it is NOT a device-lost condition, so report
// VK_TIMEOUT (not VK_ERROR_DEVICE_LOST) and let the caller skip the frame without recreating.
VkResult RunPresentOnMain(const std::shared_ptr<std::atomic<uint64_t>>& generation,
    std::function<VkResult()> work, const std::function<void()>& onQueuedBlockDone = {})
{
    VkResult result = VK_ERROR_UNKNOWN;
    if (!DispatchOnMainWithTimeout(generation, std::move(work), result, "RunPresentOnMain",
        MAIN_DISPATCH_TIMEOUT_NS, false, onQueuedBlockDone)) {
        return VK_TIMEOUT;
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
    : mainDispatchGeneration_(std::make_shared<std::atomic<uint64_t>>(0))
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

    QueueFamilyIndices indices = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER).FindQueueFamilies(surface_);
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
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    if (vkInterface->vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, nullptr) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create failed to get swap chain images count");
        vkInterface->vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
        return false;
    }
    ROSEN_LOGD("RSSurfaceSwapChain::RetrieveSwapchainImages imageCount=%{public}u", imageCount);
    swapchainImages_.resize(imageCount);
    if (vkInterface->vkGetSwapchainImagesKHR(device, swapchain_, &imageCount, swapchainImages_.data()) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to get swap chain images");
        vkInterface->vkDestroySwapchainKHR(device, swapchain_, nullptr);
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
    return RunMetalLayerMutationOnMain(mainDispatchGeneration_,
        [this, width, height]() { return CreateImpl(width, height); });
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

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkDevice device = vkContext.GetDevice();

    if (surface_ == VK_NULL_HANDLE && !vkContext.GetRsVulkanInterface()->CreateMetalSurface(metalLayer_, surface_)) {
        ROSEN_LOGE("RSSurfaceSwapChain::Create Failed to create Vulkan surface via MoltenVK");
        return false;
    }

    SwapChainSupportDetails swapChainSupport = vkContext.GetRsVulkanInterface()->QuerySwapChainSupport(surface_);
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        ROSEN_LOGI("RSSurfaceSwapChain::Create Swap chain support details are incomplete");
        return false;
    }
    VkSwapchainCreateInfoKHR createInfo = BuildSwapchainCreateInfo(width, height, swapChainSupport);

    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkResult result = vkInterface->vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain_);
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
        vkInterface->vkDestroySwapchainKHR(device, swapchain_, nullptr);
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

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    vkInterface->vkDeviceWaitIdle(device);

    swapchainGeneration_.fetch_add(1, std::memory_order_release);

    VkSwapchainKHR oldSwapchain = swapchain_;
    VkSurfaceKHR oldSurface = surface_;
    swapchain_ = VK_NULL_HANDLE;
    surface_ = VK_NULL_HANDLE;

    // Light flush only here — the layer outlives this swapchain; the full teardown
    // treatment (contents=nil + real-budget pumps) is reserved for Cleanup.
    FlushMetalLayerDrawableOnMain();

    if (metalLayer_ == nullptr) {
        ROSEN_LOGE("RSSurfaceSwapChain::Recreate: metal layer is null");
        swapchain_ = oldSwapchain;
        surface_ = oldSurface;
        MarkRecreateFailed();
        return false;
    }
    if (oldSwapchain != VK_NULL_HANDLE) {
        vkInterface->vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
    }

    if (oldSurface != VK_NULL_HANDLE) {
        vkContext.DestroySurfaceKHR(oldSurface);
    }

    CleanupSyncObjects();

    if (!vkContext.GetRsVulkanInterface()->CreateMetalSurface(metalLayer_, surface_)) {
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

void RSSurfaceSwapChain::FlushMetalLayerDrawableOnMain(bool isTeardown)
{
    if (metalLayer_ == nullptr) {
        return;
    }
    auto* layer = static_cast<CAMetalLayer*>(metalLayer_);
    if (isTeardown) {
        // Final teardown (Cleanup only — the layer will never render again): on the main
        // thread, resize to {1,1}, clear the contents reference, and give CA real runloop
        // turns to commit and process the change, releasing the layer's drawable pool.
        bool success = RunMetalLayerMutationOnMain(mainDispatchGeneration_, [layer]() {
            layer.drawableSize = CGSizeMake(1, 1);
            layer.contents = nil;
            for (int i = 0; i < 3; ++i) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.05, false);
            }
            return true;
        }, true);
        if (!success) {
            ROSEN_LOGW("FlushMetalLayerDrawableOnMain: failed to flush on main thread (teardown)");
        }
        return;
    }
    // Mid-stream flush (swapchain recreate — the layer stays alive): light flush only.
    // Never clear contents on a layer that keeps rendering. iOS 18: {1,1} on main with
    // zero-budget drains; iOS 17 and below: render-thread {1,1} that is overwritten by
    // the real size before CA ever sees it (it must stay uncommitted).
    if (@available(iOS 18, *)) {
        bool success = RunMetalLayerMutationOnMain(mainDispatchGeneration_, [layer]() {
            layer.drawableSize = CGSizeMake(1, 1);
            for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
                CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0, true);
            }
            return true;
        }, true);
        if (!success) {
            ROSEN_LOGW("FlushMetalLayerDrawableOnMain: failed to flush on main thread (recreate)");
        }
        return;
    }
    layer.drawableSize = CGSizeMake(1, 1);
}

void RSSurfaceSwapChain::SetLayerDrawableSizeOnMain(int32_t width, int32_t height)
{
    if (@available(iOS 18, *)) {
        // iOS 18+: layer mutations must happen on the main thread.
        // Use extended timeout since this may be called during startup.
        bool success = RunMetalLayerMutationOnMain(mainDispatchGeneration_, [this, width, height]() {
            if (metalLayer_ == nullptr) {
                return true;
            }
            auto* layer = static_cast<CAMetalLayer*>(metalLayer_);
            layer.drawableSize = CGSizeMake(width, height);
            // MoltenVK expects drawableSize to equal bounds * contentsScale (naturalDrawableSizeMVK).
            // If the layer never ran layout (e.g. the drag-preview subwindow), contentsScale stays
            // at the default 1.0 and every acquire returns VK_SUBOPTIMAL_KHR forever. Keep the
            // scale consistent with the requested drawable size.
            {
                CGSize boundsSize = layer.bounds.size;
                if (boundsSize.width > 0.0 && boundsSize.height > 0.0) {
                    CGFloat scaleX = static_cast<CGFloat>(width) / boundsSize.width;
                    CGFloat scaleY = static_cast<CGFloat>(height) / boundsSize.height;
                    CGFloat derivedScale = (scaleX + scaleY) * 0.5;
                    if (std::abs(scaleX - scaleY) < 0.01 &&
                        std::abs(derivedScale - layer.contentsScale) > 0.01) {
                        ROSEN_LOGI("SetLayerDrawableSizeOnMain: fix contentsScale %{public}.2f -> %{public}.2f",
                            layer.contentsScale, derivedScale);
                        layer.contentsScale = derivedScale;
                    }
                } else if (layer.contentsScale <= 1.0) {
                    // Bounds not laid out yet; assume the main screen scale rather than the 1.0 default.
                    layer.contentsScale = [UIScreen mainScreen].scale;
                }
            }
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
    // Cancel any of our tasks still queued on the main queue BEFORE touching Vulkan state:
    // after this object is destroyed, a late CreateImpl/layer mutation would be use-after-free.
    // Blocks read the generations through the shared counters, so they skip safely; the present
    // done-callback likewise only holds shared state. Both counters must be bumped: presents
    // deliberately use their own so a layer-mutation timeout cannot discard them, but Cleanup
    // invalidates everything.
    mainDispatchGeneration_->fetch_add(1, std::memory_order_relaxed);
    presentDispatchGeneration_->fetch_add(1, std::memory_order_relaxed);

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    if (device != VK_NULL_HANDLE && vkInterface->vkDeviceWaitIdle) {
        vkInterface->vkDeviceWaitIdle(device);
    }
    // Teardown flush: force completion of presentations stuck on the iOS 18 Metal regression
    // while the layer is still attached, with a real runloop budget (see implementation).
    ROSEN_LOGI("RSSurfaceSwapChain::Cleanup: teardown flush, pendingPresents=%{public}d, swapchain=%{public}s",
        pendingPresentsOnMain_->load(std::memory_order_acquire),
        swapchain_ != VK_NULL_HANDLE ? "live" : "null");
    FlushMetalLayerDrawableOnMain(true);
    CleanupSyncObjects();

    if (swapchain_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroySwapchainKHR(device, swapchain_, nullptr);
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

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    return vkInterface->vkAcquireNextImageKHR(
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
    // Track queued presents: while one has not run on main yet, its drawable is still held and
    // the caller must not acquire more images (pool starvation) or recreate (destroys a healthy
    // swapchain while main is busy). Only counted when actually dispatched: on the main thread
    // the work runs inline, so no queued block will ever fire the done-callback.
    const bool calledFromMainThread = pthread_main_np() != 0;
    // Present runs directly on this (render) thread: [MTLDrawable present] is
    // thread-safe, so the render thread never waits on the main queue to present;
    // a busy main thread only delays drawable recycling, which the 3-image pool
    // absorbs.
    if (!calledFromMainThread) {
        // Runs synchronously here — no queued block, so pendingPresentsOnMain_ stays
        // untouched (it only tracks presents still waiting for the main queue).
        const VkResult presentResult = PresentImpl(queue, imageIndex, waitSemaphore);
        // Flush this thread's implicit CA transaction: an off-main present rides a
        // thread-local transaction that only a flush on this very thread commits.
        [CATransaction flush];
        return presentResult;
    }
    if (!calledFromMainThread) {
        pendingPresentsOnMain_->fetch_add(1, std::memory_order_relaxed);
    }
    // The done-callback below runs whenever the queued block finally executes — including the
    // generation-mismatch skip after Cleanup bumped the counters, i.e. potentially AFTER this
    // object was destroyed (the teardown flush can itself time out on a busy main thread). It
    // must therefore capture only shared state; capturing `this` here was a use-after-free.
    const std::shared_ptr<std::atomic<int32_t>> pendingPresents = pendingPresentsOnMain_;
    return RunPresentOnMain(presentDispatchGeneration_,
        [this, currentGeneration, queue, imageIndex, waitSemaphore]() {
        // Check if swapchain was recreated while we were waiting for main queue.
        if (swapchainGeneration_.load(std::memory_order_acquire) != currentGeneration) {
            ROSEN_LOGI("PresentImpl: swapchain was recreated, skipping present");
            return VK_ERROR_OUT_OF_DATE_KHR;
        }
        return PresentImpl(queue, imageIndex, waitSemaphore);
    },
        [pendingPresents, calledFromMainThread]() {
            if (!calledFromMainThread) {
                pendingPresents->fetch_sub(1, std::memory_order_relaxed);
            }
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

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    return vkInterface->vkQueuePresentKHR(queue, &presentInfo);
}

bool RSSurfaceSwapChain::CreateSyncObjects()
{
    if (syncObjectsCreated_) {
        CleanupSyncObjects();
    }

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkInterface->vkCreateSemaphore(device, &semaphoreInfo, nullptr,
            &imageAvailableSemaphores_[i]) != VK_SUCCESS || vkInterface->vkCreateSemaphore(device,
            &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS) {
            ROSEN_LOGI("RSSurfaceSwapChain::CreateSyncObjects Failed for frame %zu", i);
            for (size_t j = 0; j < i; j++) {
                vkInterface->vkDestroySemaphore(device, imageAvailableSemaphores_[j], nullptr);
                vkInterface->vkDestroySemaphore(device, renderFinishedSemaphores_[j], nullptr);
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

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkDevice device = vkContext.GetDevice();
    auto& vkInterface = vkContext.GetRsVulkanInterface();

    for (size_t i = 0; i < imageAvailableSemaphores_.size(); i++) {
        vkInterface->vkDestroySemaphore(device, imageAvailableSemaphores_[i], nullptr);
    }
    for (size_t i = 0; i < renderFinishedSemaphores_.size(); i++) {
        vkInterface->vkDestroySemaphore(device, renderFinishedSemaphores_[i], nullptr);
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
