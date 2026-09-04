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

#include "rs_surface_ios_vulkan.h"

#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <atomic>
#include <memory>
#include <mutex>
#include <QuartzCore/CAMetalLayer.h>
#include <UIKit/UIKit.h>

#include "platform/common/rs_log.h"
#include "drawing/engine_adapter/skia_adapter/skia_gpu_context.h"
#include "engine_adapter/skia_adapter/skia_surface.h"
#include "rs_trace.h"
#include "rs_surface_platform_texture_ios.h"
#include "rs_surface_texture_ios.h"
#include "pipeline/rs_render_thread.h"

#ifdef USE_M133_SKIA
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"
#else
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#endif

#include "render_context/new_render_context/render_context_vk.h"

namespace OHOS {
namespace Rosen {

static constexpr uint16_t MAX_FRAMES_IN_FLIGHT = 3;
// ~3 frames at 60Hz; avoids indefinite block when swapchain images are not returned.
static constexpr uint64_t ACQUIRE_SWAPCHAIN_IMAGE_TIMEOUT_NS = 50 * 1000000ULL;

// True while the host iOS app is backgrounded. GPU work submitted in this state is
// aborted by the system (kIOGPUCommandBufferCallbackErrorBackgroundExecutionNotPermitted);
// the rejection is handled non-fatally on the MoltenVK side (no markLost + unstick +
// signalAndUntrack on the skipped present), so no per-frame gate is needed here.


RSSurfaceIOSVulkan::RSSurfaceIOSVulkan(void* metalLayer)
{
    ROSEN_LOGI("RSSurfaceAndroidVulkan entry with %p", metalLayer);
    metalLayer_ = [static_cast<CAMetalLayer*>(metalLayer) retain];
    swapChain_.Initialize(metalLayer_);
}

RSSurfaceIOSVulkan::~RSSurfaceIOSVulkan()
{
    for (size_t i = 0; i < skiaSurfaces_.size(); i++) {
        if (skiaSurfaces_[i]) {
            skiaSurfaces_[i].reset();
        }
    }
    skiaSurfaces_.clear();
    skiaSurfaces_.shrink_to_fit();

    if (mSkContext_) {
        mSkContext_->FlushAndSubmit(true);
        mSkContext_->PurgeUnlockAndSafeCacheGpuResources();
    }

    swapChain_.Cleanup();

    auto renderContextVk = std::static_pointer_cast<RenderContextVK>(renderContext_);
    if (renderContextVk != nullptr) {
        renderContextVk->DeleteSurface();
    }

    [static_cast<CAMetalLayer*>(metalLayer_) release];
    metalLayer_ = nullptr;
    ROSEN_LOGI("RSSurfaceIOSVulkan Destructor");
}

bool RSSurfaceIOSVulkan::IsValid() const
{
    return metalLayer_ != nullptr;
}

std::shared_ptr<Drawing::ColorSpace> ConvertColorGamutToColorSpace(GraphicColorGamut colorGamut)
{
    std::shared_ptr<Drawing::ColorSpace> colorSpace = nullptr;
    switch (colorGamut) {
        case GRAPHIC_COLOR_GAMUT_DISPLAY_P3:
        case GRAPHIC_COLOR_GAMUT_DCI_P3:
            colorSpace = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::DCIP3);
            break;
        case GRAPHIC_COLOR_GAMUT_ADOBE_RGB:
            colorSpace = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::ADOBE_RGB);
            break;
        case GRAPHIC_COLOR_GAMUT_BT2020:
            colorSpace = Drawing::ColorSpace::CreateRGB(Drawing::CMSTransferFuncType::SRGB,
                Drawing::CMSMatrixType::REC2020);
            break;
        default:
            colorSpace = Drawing::ColorSpace::CreateSRGB();
            break;
    }
    return colorSpace;
}

static void DeleteVkImage(void* context)
{
}

std::shared_ptr<Drawing::Surface> RSSurfaceIOSVulkan::CreateSkiaSurfaceFromSwapchainImage(
    uint32_t imageIndex, int32_t width, int32_t height, bool isProtected)
{
    const auto& swapchainImages = swapChain_.GetImages();
    if (imageIndex >= swapchainImages.size()) {
        ROSEN_LOGI("Invalid image index: %u", imageIndex);
        return nullptr;
    }

    VkImage vkImage = swapchainImages[imageIndex];
    VkFormat imageFormat = swapChain_.GetFormat();
    auto colorSpace = ConvertColorGamutToColorSpace(colorSpace_);

    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    QueueFamilyIndices indices = vkContext.FindQueueFamilies(swapChain_.GetSurface());

    Drawing::TextureInfo texture_info;
    texture_info.SetWidth(width);
    texture_info.SetHeight(height);
    std::shared_ptr<Drawing::VKTextureInfo> vkTextureInfo = std::make_shared<Drawing::VKTextureInfo>();
    vkTextureInfo->vkImage = vkImage;
    vkTextureInfo->vkAlloc.memory = VK_NULL_HANDLE;
    vkTextureInfo->vkAlloc.offset = 0;
    vkTextureInfo->vkAlloc.size = 0;
    vkTextureInfo->vkAlloc.flags = 0;
    vkTextureInfo->imageTiling = VK_IMAGE_TILING_OPTIMAL;
    vkTextureInfo->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkTextureInfo->format = imageFormat;
    vkTextureInfo->imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                      VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    vkTextureInfo->sampleCount = 1;
    vkTextureInfo->levelCount = 1;
    vkTextureInfo->currentQueueFamily = VK_QUEUE_FAMILY_IGNORED;
    vkTextureInfo->vkProtected = isProtected;
    vkTextureInfo->sharingMode = (indices.presentFamily != UINT32_MAX &&
        indices.graphicsFamily != indices.presentFamily) ?
        VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    texture_info.SetVKTextureInfo(vkTextureInfo);

    Drawing::ColorType colorType = ConvertVkFormatToSkiaColorType(imageFormat);
    auto skSurface = Drawing::Surface::MakeFromBackendRenderTarget(
        mSkContext_.get(),
        texture_info,
        Drawing::TextureOrigin::TOP_LEFT,
        colorType,
        colorSpace,
        DeleteVkImage,
        nullptr);
    if (!skSurface) {
        ROSEN_LOGE("Failed to create Skia surface from Vulkan image, index: %u", imageIndex);
        return nullptr;
    }
    return skSurface;
}

Drawing::ColorType RSSurfaceIOSVulkan::ConvertVkFormatToSkiaColorType(VkFormat imageFormat)
{
    switch (imageFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return Drawing::ColorType::COLORTYPE_RGBA_8888;
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_SRGB:
            return Drawing::ColorType::COLORTYPE_BGRA_8888;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return Drawing::ColorType::COLORTYPE_RGB_565;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return Drawing::ColorType::COLORTYPE_RGBA_1010102;
        default:
            ROSEN_LOGW("Unsupported Vulkan format: %d, defaulting to RGBA_8888", imageFormat);
            return Drawing::ColorType::COLORTYPE_RGBA_8888;
    }
}

bool RSSurfaceIOSVulkan::SetupGrContext()
{
    if (mSkContext_) {
        return true;
    }
    auto renderCtxVk = std::static_pointer_cast<RenderContextVK>(renderContext_);
    if (renderCtxVk) {
        renderCtxVk->Init();
        mSkContext_ = renderCtxVk->GetSharedDrGPUContext();
        return mSkContext_ != nullptr;
    }
    return false;
}

bool RSSurfaceIOSVulkan::RecreateSwapchainIfNeeded(int32_t width, int32_t height)
{
    if (!swapChain_.NeedRecreate() && swapChain_.GetSwapchain() != VK_NULL_HANDLE) {
        return true;
    }

    static const bool disablePresentGate = getenv("ARKUI_DISABLE_PRESENT_GATE") != nullptr;
    if (!disablePresentGate && swapChain_.HasPendingPresentsOnMain()) {
        // Recreate starts with an unbounded vkDeviceWaitIdle and destroys the old swapchain while
        // a queued present may still reference it. Defer until the main queue drained it.
        static std::atomic<uint32_t> deferRecreateLogCount { 0 };
        uint32_t deferCount = deferRecreateLogCount.fetch_add(1, std::memory_order_relaxed);
        if (deferCount < 5 || deferCount % 100 == 0) {
            ROSEN_LOGW("RecreateSwapchainIfNeeded: deferring recreate, presents still queued on main"
                " (%{public}u deferred)", deferCount + 1);
        }
        return false;
    }

    if (swapChain_.IsRecreating()) {
        ROSEN_LOGI("RequestFrame: Swapchain is being recreated, waiting...");
        return false;
    }

    int32_t recreateWidth = width;
    int32_t recreateHeight = height;
    swapChain_.GetPendingSize(&recreateWidth, &recreateHeight);
    if (recreateWidth == 0) {
        recreateWidth = width;
    }
    if (recreateHeight == 0) {
        recreateHeight = height;
    }

    for (size_t i = 0; i < skiaSurfaces_.size(); i++) {
        if (skiaSurfaces_[i]) {
            skiaSurfaces_[i].reset();
        }
    }
    skiaSurfaces_.clear();
    skiaSurfaces_.shrink_to_fit();

    if (mSkContext_) {
        mSkContext_->Submit();
    }
    swapChain_.Recreate(recreateWidth, recreateHeight);
    swapChain_.SetPendingSize(0, 0);
    lastPresentedImageIndex_ = UINT32_MAX;
    currentFrame_ = 0;
    return true;
}

uint32_t RSSurfaceIOSVulkan::AcquireSwapchainImage()
{
    uint32_t imageIndex;
    VkSemaphore imageAvailableSemaphore = swapChain_.GetImageAvailableSemaphore(currentFrame_);
    VkResult result = swapChain_.AcquireNextImage(
        ACQUIRE_SWAPCHAIN_IMAGE_TIMEOUT_NS, imageAvailableSemaphore, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ROSEN_LOGW("RequestFrame Swapchain out of date, will recreate next frame");
        swapChain_.SetNeedRecreate(true);
        return UINT32_MAX;
    }
    if (result == VK_SUBOPTIMAL_KHR) {
        // Per Vulkan spec the image WAS acquired; only presentation is suboptimal. On iOS this
        // fires permanently when the CAMetalLayer's natural extent (bounds * contentsScale)
        // differs from drawableSize — recreating cannot fix a scale mismatch (the new swapchain
        // comes back suboptimal too), and attempting it per frame floods the main queue with
        // layer-mutation dispatches. Present the acquired image as-is.
        // ARKUI_DISABLE_SCALE_FIXES=1 restores the pre-fix recreate behaviour for A/B tests.
        static const bool disableScaleFix = getenv("ARKUI_DISABLE_SCALE_FIXES") != nullptr;
        if (disableScaleFix) {
            ROSEN_LOGW("RequestFrame Swapchain suboptimal, will recreate next frame (fix disabled)");
            swapChain_.SetNeedRecreate(true);
            return UINT32_MAX;
        }
        static std::atomic<uint32_t> suboptimalLogCount { 0 };
        if (suboptimalLogCount.fetch_add(1, std::memory_order_relaxed) < 5) {
            ROSEN_LOGW("RequestFrame Swapchain suboptimal (contentsScale mismatch?), present as-is");
        }
    }
    if (result == VK_TIMEOUT) {
        static const bool disablePresentGate = getenv("ARKUI_DISABLE_PRESENT_GATE") != nullptr;
        if (!disablePresentGate && swapChain_.HasPendingPresentsOnMain()) {
            // Drawable pool starved by our own presents stuck on the busy main queue — not a
            // broken swapchain. Recreating would destroy a healthy swapchain and wedge the
            // unbounded vkDeviceWaitIdle in Recreate. Just drop this frame.
            ROSEN_LOGW("RequestFrame AcquireNextImage timed out while presents are queued on main, defer recreate");
            return UINT32_MAX;
        }
        ROSEN_LOGW("RequestFrame AcquireNextImage timed out, will recreate next frame");
        swapChain_.SetNeedRecreate(true);
        return UINT32_MAX;
    }
    if (result != VK_SUCCESS) {
        ROSEN_LOGW("RequestFrame Failed to acquire swapchain image: %d", result);
        return UINT32_MAX;
    }
    return imageIndex;
}

std::shared_ptr<Drawing::Surface> RSSurfaceIOSVulkan::GetOrCreateSkiaSurface(
    uint32_t imageIndex, int32_t swapchainWidth, int32_t swapchainHeight, bool isProtected)
{
    if (imageIndex >= skiaSurfaces_.size()) {
        skiaSurfaces_.resize(imageIndex + 1);
    }

    if (!skiaSurfaces_[imageIndex] ||
        skiaSurfaces_[imageIndex]->Width() != swapchainWidth ||
        skiaSurfaces_[imageIndex]->Height() != swapchainHeight) {
        if (skiaSurfaces_[imageIndex]) {
            skiaSurfaces_[imageIndex].reset();
        }
        skiaSurfaces_[imageIndex] = CreateSkiaSurfaceFromSwapchainImage(
            imageIndex, swapchainWidth, swapchainHeight, isProtected);
        if (!skiaSurfaces_[imageIndex]) {
            ROSEN_LOGE("RequestFrame Failed to create surface for image index: %u", imageIndex);
            return nullptr;
        }
    }
    return skiaSurfaces_[imageIndex];
}

bool RSSurfaceIOSVulkan::CheckLayerAndContext()
{
    if (metalLayer_ == nullptr) {
        ROSEN_LOGE("RSSurfaceIOSVulkan::RequestFrame, metal layer is nullptr");
        return false;
    }
    if (!SetupGrContext()) {
        ROSEN_LOGE("RSSurfaceIOSVulkan::RequestFrame, failed to setup GPU context");
        return false;
    }
    return true;
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceIOSVulkan::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC, bool isProtected)
{
    if (!CheckLayerAndContext()) {
        return nullptr;
    }
    // No background gate here: backgrounding is rare, the rare rejected frame is absorbed
    // by the MoltenVK-side recovery (Resumed VkDevice + skipped present signalAndUntrack),
    // and a per-frame applicationState poll in the hot path costs more than it saves.
    // ARKUI_DISABLE_PRESENT_GATE=1 restores the pre-fix behaviour (acquire despite pending
    // presents, acquire-timeout triggers recreate, recreate not deferred) for A/B tests.
    static const bool disablePresentGate = getenv("ARKUI_DISABLE_PRESENT_GATE") != nullptr;
    if (!disablePresentGate && swapChain_.HasPendingPresentsOnMain()) {
        // A previous present is still queued on the busy main queue, so its drawable has not
        // returned to the pool. Acquiring now only starves the pool, and the resulting acquire
        // timeout would be misread as a broken swapchain and trigger a destructive recreate
        // whose unbounded vkDeviceWaitIdle wedges the render thread against the stalled main
        // thread (GPU command-buffer timeout -> VK_ERROR_DEVICE_LOST). Drop the frame instead;
        // rendering resumes as soon as the main queue drains the queued present.
        static std::atomic<uint32_t> presentPendingSkipCount { 0 };
        uint32_t skipCount = presentPendingSkipCount.fetch_add(1, std::memory_order_relaxed);
        if (skipCount < 5 || skipCount % 100 == 0) {
            ROSEN_LOGW("RequestFrame: previous present still queued on main, drop frame (%{public}u skipped)",
                skipCount + 1);
        }
        return nullptr;
    }
    if (width != currentWidth_ || height != currentHeight_) {
        swapChain_.SetLayerDrawableSizeOnMain(width, height);
        swapChain_.SetNeedRecreate(true);
        swapChain_.SetPendingSize(width, height);
        currentWidth_ = width;
        currentHeight_ = height;
    }
    if (!RecreateSwapchainIfNeeded(width, height)) {
        return nullptr;
    }
    if (swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGE("RequestFrame: Swapchain is not available after recreation attempt");
        return nullptr;
    }
    uint32_t imageIndex = AcquireSwapchainImage();
    if (imageIndex == UINT32_MAX) {
        return nullptr;
    }
    VkExtent2D extent = swapChain_.GetExtent();
    int32_t swapchainWidth = static_cast<int32_t>(extent.width);
    int32_t swapchainHeight = static_cast<int32_t>(extent.height);
    auto surface = GetOrCreateSkiaSurface(imageIndex, swapchainWidth, swapchainHeight, isProtected);
    if (!surface) {
        ReleaseAcquiredSwapchainImage(imageIndex);
        return nullptr;
    }
    surface->ClearDrawingArea();
    int32_t bufferAge = 0;
    if (lastPresentedImageIndex_ != UINT32_MAX) {
        bufferAge = (imageIndex == lastPresentedImageIndex_) ? 0 : 1;
    }
    std::unique_ptr<RSSurfaceFrameIOSVulkan> frame = std::make_unique<RSSurfaceFrameIOSVulkan>(surface,
        width, height, bufferAge);
    frame->SetSwapchainImageIndex(imageIndex);
    frame->SetSurfacePhysicalSize(swapchainWidth, swapchainHeight);
    return frame;
}

bool RSSurfaceIOSVulkan::FlushSkiaSurface(
    std::shared_ptr<Drawing::Surface> surface, VkSemaphore renderFinishedSemaphore)
{
    RS_TRACE_NAME("SkiaFlush");
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
#ifdef USE_M133_SKIA
    GrBackendSemaphore backendSignalSemaphore = GrBackendSemaphores::MakeVk(signalSemaphores[0]);
#else
    GrBackendSemaphore backendSignalSemaphore;
    backendSignalSemaphore.initVulkan(signalSemaphores[0]);
#endif
    std::vector<GrBackendSemaphore> signalSemaphoreVec = {backendSignalSemaphore};
    Drawing::FlushInfo flushInfo;
    flushInfo.backendSurfaceAccess = true;
    flushInfo.numSemaphores = signalSemaphoreVec.size();
    flushInfo.backendSemaphore = static_cast<void*>(signalSemaphoreVec.data());
    flushInfo.finishedProc = nullptr;
    flushInfo.finishedContext = nullptr;
    auto res = surface->Flush(&flushInfo);
    if (res == Drawing::SemaphoresSubmited::DRAWING_ENGINE_SUBMIT_NO) {
        ROSEN_LOGE("FlushFrame flush stage failed: submit_no, currentFrame=%{public}zu", currentFrame_);
        return false;
    }
    return true;
}

void RSSurfaceIOSVulkan::WaitAndSubmitSkiaContext(VkSemaphore waitSemaphore)
{
    RS_TRACE_NAME("Submit");
#ifdef USE_M133_SKIA
    GrBackendSemaphore backendWaitSemaphore = GrBackendSemaphores::MakeVk(waitSemaphore);
#else
    GrBackendSemaphore backendWaitSemaphore;
    backendWaitSemaphore.initVulkan(waitSemaphore);
#endif
    Drawing::SkiaGPUContext* skiaGpuContext = mSkContext_->GetImpl<Drawing::SkiaGPUContext>();
    if (skiaGpuContext) {
        sk_sp<GrDirectContext> grContext = skiaGpuContext->GetGrContext();
        if (grContext) {
            grContext->wait(1, &backendWaitSemaphore, false);
        }
    }
    mSkContext_->Submit();
}

bool RSSurfaceIOSVulkan::PresentSwapchainImage(
    VkQueue queue, uint32_t imageIndex, VkSemaphore renderFinishedSemaphore)
{
    VkResult result = swapChain_.Present(queue, imageIndex, renderFinishedSemaphore);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ROSEN_LOGE("FlushFrame Swapchain out of date, will recreate next frame");
        swapChain_.SetNeedRecreate(true);
        return false;
    }
    if (result == VK_SUBOPTIMAL_KHR) {
        ROSEN_LOGE("FlushFrame Swapchain suboptimal");
    }
    if (result == VK_TIMEOUT) {
        // Main queue is busy (window creation / page navigation); the queued present is still pending
        // and will run when the main thread frees up. Skip this frame WITHOUT recreating: a recreate
        // during a busy main thread only produces a failed-recreate storm (its main dispatch times out
        // too) and tears down a healthy swapchain.
        static std::atomic<uint32_t> presentTimeoutLogCount { 0 };
        uint32_t timeoutCount = presentTimeoutLogCount.fetch_add(1, std::memory_order_relaxed);
        if (timeoutCount < 5 || timeoutCount % 100 == 0) {
            ROSEN_LOGW("FlushFrame Present main dispatch timed out, skip frame without recreate"
                " (%{public}u skipped)", timeoutCount + 1);
        }
        return false;
    }
    if (result == VK_ERROR_DEVICE_LOST) {
        ROSEN_LOGE("FlushFrame Present failed: device lost, will recreate");
        swapChain_.SetNeedRecreate(true);
        return false;
    }
    if (result != VK_SUCCESS) {
        ROSEN_LOGE("FlushFrame present stage failed: VkResult=%{public}d,imageIndex=%{public}u"
            "currentFrame=%{public}zu",
            static_cast<int32_t>(result), imageIndex, currentFrame_);
        swapChain_.SetNeedRecreate(true);
        return false;
    }
    lastPresentedImageIndex_ = imageIndex;
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

void RSSurfaceIOSVulkan::ReleaseAcquiredSwapchainImage(uint32_t imageIndex)
{
    if (imageIndex == UINT32_MAX || swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        return;
    }
    if (imageIndex >= swapChain_.GetImageCount()) {
        return;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkQueue queue = vkContext.GetPresentQueue();
    if (queue == VK_NULL_HANDLE) {
        queue = vkContext.GetGraphicsQueue();
    }
    if (queue == VK_NULL_HANDLE) {
        return;
    }
    VkSemaphore waitSemaphore = swapChain_.GetImageAvailableSemaphore(currentFrame_);
    ROSEN_LOGW("ReleaseAcquiredSwapchainImage: present to return image %{public}u frame %{public}zu",
        imageIndex, currentFrame_);
    PresentSwapchainImage(queue, imageIndex, waitSemaphore);
}

bool RSSurfaceIOSVulkan::CheckSwapchainValidity(uint32_t imageIndex)
{
    if (swapChain_.IsRecreating() || swapChain_.NeedRecreate()) {
        ROSEN_LOGE("CheckSwapchainValidity: Swapchain is being recreated, dropping frame");
        ReleaseAcquiredSwapchainImage(imageIndex);
        return false;
    }

    if (imageIndex >= swapChain_.GetImageCount()) {
        ROSEN_LOGE("CheckSwapchainValidity: Invalid image index %u (swapchain has %zu images)",
                   imageIndex, swapChain_.GetImageCount());
        return false;
    }

    if (swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGE("CheckSwapchainValidity: Swapchain is null");
        return false;
    }
    return true;
}

bool RSSurfaceIOSVulkan::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (!frame) {
        ROSEN_LOGE("RSSurfaceIOSVulkan::FlushFrame, frame is null");
        return false;
    }

    auto frameVulkan = static_cast<RSSurfaceFrameIOSVulkan*>(frame.get());
    if (!frameVulkan) {
        ROSEN_LOGE("RSSurfaceIOSVulkan::FlushFrame, invalid frame type");
        return false;
    }
    uint32_t imageIndex = frameVulkan->GetSwapchainImageIndex();
    if (!CheckSwapchainValidity(imageIndex)) {
        return false;
    }
    auto surface = frame->GetSurface();
    if (!surface) {
        ROSEN_LOGE("RSSurfaceIOSVulkan::FlushFrame Invalid surface");
        ReleaseAcquiredSwapchainImage(imageIndex);
        return false;
    }

    VkSemaphore waitSemaphore = swapChain_.GetImageAvailableSemaphore(currentFrame_);
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto queue = vkContext.GetPresentQueue();
    if (queue == VK_NULL_HANDLE) {
        ROSEN_LOGI("RSSurfaceIOSVulkan::FlushFrame, presentQueue is null, fallback to graphicsQueue");
        queue = vkContext.GetGraphicsQueue();
    }

    VkSemaphore renderFinishedSemaphore = swapChain_.GetRenderFinishedSemaphore(currentFrame_);
    if (!FlushSkiaSurface(surface, renderFinishedSemaphore)) {
        ROSEN_LOGW("FlushFrame: flush failed, fallback present with acquire semaphore to drain frame state");
        return PresentSwapchainImage(queue, imageIndex, waitSemaphore);
    }

    // Check again before submitting - swapchain may have been recreated during FlushSkiaSurface.
    if (swapChain_.IsRecreating() || swapChain_.NeedRecreate() || swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGW("FlushFrame: Swapchain became invalid after flush, dropping frame");
        return false;
    }

    WaitAndSubmitSkiaContext(waitSemaphore);

    // Final check before present - swapchain may have been recreated during submit.
    if (swapChain_.IsRecreating() || swapChain_.NeedRecreate() || swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGW("FlushFrame: Swapchain became invalid after submit, dropping frame");
        return false;
    }

    bool result = PresentSwapchainImage(queue, imageIndex, renderFinishedSemaphore);
    if (mSkContext_) {
        mSkContext_->PurgeUnlockedResources(true);
    }
    return result;
}

void RSSurfaceIOSVulkan::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGI("RSSurfaceIOSVulkan::SetColorSpace %{public}d", colorSpace);
    if (colorSpace_ == colorSpace) {
        return;
    }
    colorSpace_ = colorSpace;
    for (size_t i = 0; i < skiaSurfaces_.size(); i++) {
        if (skiaSurfaces_[i]) {
            skiaSurfaces_[i].reset();
        }
    }
    skiaSurfaces_.clear();
    skiaSurfaces_.shrink_to_fit();
}

void RSSurfaceIOSVulkan::ClearBuffer()
{
    // TODO: implement for Vulkan backend
}

void RSSurfaceIOSVulkan::ClearAllBuffer()
{
    // TODO: implement for Vulkan backend
}

void RSSurfaceIOSVulkan::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    // TODO: implement for Vulkan backend
}

void RSSurfaceIOSVulkan::ResetBufferAge()
{
    // TODO: implement for Vulkan backend
}

uint32_t RSSurfaceIOSVulkan::GetQueueSize() const
{
    return MAX_FRAMES_IN_FLIGHT;
}

GraphicColorGamut RSSurfaceIOSVulkan::GetColorSpace() const
{
    return colorSpace_;
}

std::shared_ptr<RenderContext> RSSurfaceIOSVulkan::GetRenderContext()
{
    return renderContext_;
}

void RSSurfaceIOSVulkan::SetRenderContext(std::shared_ptr<RenderContext> context)
{
    auto newContextVk = std::static_pointer_cast<RenderContextVK>(context);
    auto oldContextVk = std::static_pointer_cast<RenderContextVK>(renderContext_);
    if (oldContextVk != newContextVk) {
        if (oldContextVk != nullptr) {
            oldContextVk->DeleteSurface();
        }
        renderContext_ = context;
        if (newContextVk != nullptr) {
            newContextVk->AddSurface();
        }
    }
}

RSSurfaceExtPtr RSSurfaceIOSVulkan::CreateSurfaceExt(const RSSurfaceExtConfig& config)
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

RSSurfaceExtPtr RSSurfaceIOSVulkan::GetSurfaceExt(const RSSurfaceExtConfig& config)
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
