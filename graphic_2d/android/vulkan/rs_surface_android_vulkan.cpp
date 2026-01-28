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

#include "rs_surface_android_vulkan.h"

#include <memory>
#include "platform/common/rs_log.h"
#include "drawing/engine_adapter/skia_adapter/skia_gpu_context.h"
#include "engine_adapter/skia_adapter/skia_surface.h"
#include "rs_trace.h"

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

static constexpr uint16_t MAX_FRAMES_IN_FLIGHT = 2;

RSSurfaceAndroidVulkan::RSSurfaceAndroidVulkan(ANativeWindow* data) : RSSurfaceAndroid(data)
{
    ROSEN_LOGE("RSSurfaceAndroidVulkan entry with %p", nativeWindow_);
    swapChain_.Initialize(nativeWindow_);
}

RSSurfaceAndroidVulkan::~RSSurfaceAndroidVulkan()
{
    if (mSkContext) {
        Drawing::SkiaGPUContext* skiaGpuContext = mSkContext->GetImpl<Drawing::SkiaGPUContext>();
        if (skiaGpuContext) {
            sk_sp<GrDirectContext> grContext = skiaGpuContext->GetGrContext();
            if (grContext) {
                grContext->flushAndSubmit();
                grContext->freeGpuResources();
            }
        }
        mSkContext->FlushAndSubmit(true);
    }
    
    for (size_t i = 0; i < skiaSurfaces_.size(); i++) {
        if (skiaSurfaces_[i]) {
            skiaSurfaces_[i].reset();
        }
    }
    skiaSurfaces_.clear();
    skiaSurfaces_.shrink_to_fit();
    
    swapChain_.Cleanup();
    
    ROSEN_LOGD("RSSurfaceAndroidVulkan Destructor");
}

std::shared_ptr<Drawing::ColorSpace> ConvertColorGamutToColorSpace(GraphicColorGamut colorGamut)
{
    std::shared_ptr<Drawing::ColorSpace> colorSpace = nullptr;
    switch (colorGamut) {
        // [planning] in order to stay consistant with the colorspace used before, we disabled
        // GRAPHIC_COLOR_GAMUT_SRGB to let the branch to default, then skColorSpace is set to nullptr
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

void DeleteVkImage(void* context)
{
}

// Create Skia surface from Vulkan swapchain image
std::shared_ptr<Drawing::Surface> RSSurfaceAndroidVulkan::CreateSkiaSurfaceFromSwapchainImage(
    uint32_t imageIndex, int32_t width, int32_t height, bool isProtected)
{
    const auto& swapchainImages = swapChain_.GetImages();
    if (imageIndex >= swapchainImages.size()) {
        ROSEN_LOGI("Invalid image index: %u", imageIndex);
        return nullptr;
    }

    // Get Vulkan image and format from swapchain
    VkImage vkImage = swapchainImages[imageIndex];
    VkFormat imageFormat = swapChain_.GetFormat();
    auto colorSpace = ConvertColorGamutToColorSpace(colorSpace_);

    auto& vkContext = RsVulkanContext::GetSingleton();
    QueueFamilyIndices indices = vkContext.FindQueueFamilies();

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
    // Configure sharing mode based on queue families
    vkTextureInfo->sharingMode = (indices.graphicsFamily != indices.presentFamily) ?
                                  VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    texture_info.SetVKTextureInfo(vkTextureInfo);

    Drawing::ColorType colorType = ConvertVkFormatToSkiaColorType(imageFormat);

    // Create Skia surface from Vulkan render target (swapchain manages image lifecycle)
    auto skSurface = Drawing::Surface::MakeFromBackendRenderTarget(
        mSkContext.get(),
        texture_info,
        Drawing::TextureOrigin::TOP_LEFT,
        colorType,
        colorSpace,
        DeleteVkImage,
        nullptr);
    if (!skSurface) {
        ROSEN_LOGD("Failed to create Skia surface from Vulkan image, index: %u", imageIndex);
        return nullptr;
    }
    return skSurface;
}

Drawing::ColorType RSSurfaceAndroidVulkan::ConvertVkFormatToSkiaColorType(VkFormat imageFormat)
{
    switch (imageFormat) {
        case VK_FORMAT_R8G8B8A8_UNORM:
        case VK_FORMAT_B8G8R8A8_UNORM:
            return Drawing::ColorType::COLORTYPE_RGBA_8888;
        case VK_FORMAT_R5G6B5_UNORM_PACK16:
            return Drawing::ColorType::COLORTYPE_RGB_565;
        case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
            return Drawing::ColorType::COLORTYPE_RGBA_1010102;
        default:
            ROSEN_LOGW("Unsupported Vulkan format for Skia: %d, defaulting to RGBA_8888", imageFormat);
            return Drawing::ColorType::COLORTYPE_RGBA_8888;
    }
}

std::shared_ptr<RenderContextVK> RSSurfaceAndroidVulkan::GetRenderContextVulkan() const
{
    return std::static_pointer_cast<RenderContextVK>(renderContext_);
}

bool RSSurfaceAndroidVulkan::RecreateSwapchainIfNeeded(int32_t width, int32_t height)
{
    if (!swapChain_.NeedRecreate() && swapChain_.GetSwapchain() != VK_NULL_HANDLE) {
        return true;
    }

    if (swapChain_.IsRecreating()) {
        ROSEN_LOGD("RequestFrame: Swapchain is being recreated, waiting...");
        return false;
    }

    ROSEN_LOGD("RequestFrame: Need to recreate swapchain");
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

    if (mSkContext) {
        Drawing::SkiaGPUContext* skiaGpuContext = mSkContext->GetImpl<Drawing::SkiaGPUContext>();
        if (skiaGpuContext) {
            sk_sp<GrDirectContext> grContext = skiaGpuContext->GetGrContext();
            if (grContext) {
                grContext->freeGpuResources();
            }
        }
    }
    swapChain_.Recreate(recreateWidth, recreateHeight);
    swapChain_.SetPendingSize(0, 0);
    lastPresentedImageIndex_ = UINT32_MAX;
    currentFrame_ = 0;
    return true;
}

uint32_t RSSurfaceAndroidVulkan::AcquireSwapchainImage()
{
    uint32_t imageIndex;
    VkSemaphore imageAvailableSemaphore = swapChain_.GetImageAvailableSemaphore(currentFrame_);
    VkResult result = swapChain_.AcquireNextImage(UINT64_MAX, imageAvailableSemaphore, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        ROSEN_LOGE("RequestFrame Swapchain out of date or suboptimal, will recreate next frame");
        swapChain_.SetNeedRecreate(true);
        return UINT32_MAX;
    } else if (result != VK_SUCCESS) {
        ROSEN_LOGD("RequestFrame Failed to acquire swapchain image: %d", result);
        return UINT32_MAX;
    }
    return imageIndex;
}

std::shared_ptr<Drawing::Surface> RSSurfaceAndroidVulkan::GetOrCreateSkiaSurface(
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
            ROSEN_LOGD("RequestFrame Failed to create surface for image index: %u", imageIndex);
            return nullptr;
        }
    }
    return skiaSurfaces_[imageIndex];
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceAndroidVulkan::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC, bool isProtected)
{
    if (nativeWindow_ == nullptr) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::RequestFrame, native window is nullptr");
        return nullptr;
    }

    if (!mSkContext) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::RequestFrame, Skia context is nullptr");
        return nullptr;
    }
    SetNativeWindowInfo(width, height, useAFBC, isProtected);

    if (!RecreateSwapchainIfNeeded(width, height)) {
        return nullptr;
    }

    if (swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGD("RequestFrame: Swapchain is not available after recreation attempt");
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
        return nullptr;
    }

    surface->ClearDrawingArea();

    int32_t bufferAge = 0;
    if (lastPresentedImageIndex_ != UINT32_MAX) {
        bufferAge = (imageIndex == lastPresentedImageIndex_) ? 0 : 1;
    } else {
        bufferAge = 0;
    }

    std::unique_ptr<RSSurfaceFrameAndroidVulkan> frame =
        std::make_unique<RSSurfaceFrameAndroidVulkan>(
            surface, width, height, bufferAge);
    frame->SetSwapchainImageIndex(imageIndex);
    frame->SetSurfacePhysicalSize(swapchainWidth, swapchainHeight);

    return frame;
}

bool RSSurfaceAndroidVulkan::FlushSkiaSurface(
    std::shared_ptr<Drawing::Surface> surface, VkSemaphore renderFinishedSemaphore)
{
    auto& vkContext = RsVulkanContext::GetSingleton();
    auto callbackInfoPtr = std::make_unique<RsVulkanInterface::CallbackSemaphoreInfo>(
        vkContext.GetRsVulkanInterface(), renderFinishedSemaphore, -1);
    auto* callbackInfo = callbackInfoPtr.get();

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
    flushInfo.finishedProc = [](void *context) {
        if (context != nullptr) {
            delete reinterpret_cast<RsVulkanInterface::CallbackSemaphoreInfo*>(context);
        }
    };
    flushInfo.finishedContext = callbackInfo;
    auto res = surface->Flush(&flushInfo);
    if (res == Drawing::SemaphoresSubmited::DRAWING_ENGINE_SUBMIT_NO) {
        ROSEN_LOGI("FlushFrame Surface flush returned DRAWING_ENGINE_SUBMIT_NO, semaphores may not be submitted");
        callbackInfoPtr.reset();
        return false;
    } else {
        callbackInfoPtr.release();
    }
    return true;
}

void RSSurfaceAndroidVulkan::WaitAndSubmitSkiaContext(VkSemaphore waitSemaphore)
{
    RS_TRACE_NAME("Submit");
#ifdef USE_M133_SKIA
    GrBackendSemaphore backendWaitSemaphore = GrBackendSemaphores::MakeVk(waitSemaphore);
#else
    GrBackendSemaphore backendWaitSemaphore;
    backendWaitSemaphore.initVulkan(waitSemaphore);
#endif
    Drawing::SkiaGPUContext* skiaGpuContext = mSkContext->GetImpl<Drawing::SkiaGPUContext>();
    if (skiaGpuContext) {
        sk_sp<GrDirectContext> grContext = skiaGpuContext->GetGrContext();
        if (grContext) {
            grContext->wait(1, &backendWaitSemaphore, false);
        }
    }
    mSkContext->Submit();
}

bool RSSurfaceAndroidVulkan::PresentSwapchainImage(
    VkQueue queue, uint32_t imageIndex, VkSemaphore renderFinishedSemaphore)
{
    VkResult result = swapChain_.Present(queue, imageIndex, renderFinishedSemaphore);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ROSEN_LOGD("FlushFrame Swapchain out of date, will recreate next frame");
        swapChain_.SetNeedRecreate(true);
        return false;
    } else if (result == VK_SUBOPTIMAL_KHR) {
        ROSEN_LOGD("FlushFrame Swapchain suboptimal");
    } else if (result != VK_SUCCESS) {
        ROSEN_LOGD("FlushFrame Present failed: %d", result);
        return false;
    } else {
        lastPresentedImageIndex_ = imageIndex;
    }
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

bool RSSurfaceAndroidVulkan::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (!frame) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::FlushFrame, frame is null");
        return false;
    }

    auto frameVulkan = static_cast<RSSurfaceFrameAndroidVulkan*>(frame.get());
    if (!frameVulkan) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::FlushFrame, invalid frame type");
        return false;
    }
    if (swapChain_.IsRecreating() || swapChain_.NeedRecreate()) {
        ROSEN_LOGD("FlushFrame: Swapchain is being recreated, dropping frame");
        return false;
    }

    uint32_t imageIndex = frameVulkan->GetSwapchainImageIndex();
    if (imageIndex >= swapChain_.GetImageCount()) {
        ROSEN_LOGD("FlushFrame: Invalid image index %u (swapchain has %zu images), dropping frame",
                   imageIndex, swapChain_.GetImageCount());
        return false;
    }

    if (swapChain_.GetSwapchain() == VK_NULL_HANDLE) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::FlushFrame Swapchain is null");
        return false;
    }
    auto surface = frame->GetSurface();
    if (!surface) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::FlushFrame Invalid surface");
        return false;
    }

    VkSemaphore renderFinishedSemaphore = swapChain_.GetRenderFinishedSemaphore(currentFrame_);
    if (!FlushSkiaSurface(surface, renderFinishedSemaphore)) {
        return false;
    }

    VkSemaphore waitSemaphore = swapChain_.GetImageAvailableSemaphore(currentFrame_);
    WaitAndSubmitSkiaContext(waitSemaphore);

    auto& vkContext = RsVulkanContext::GetSingleton();
    auto queue = vkContext.GetQueue();
    if (queue == VK_NULL_HANDLE) {
        ROSEN_LOGE("RSSurfaceAndroidVulkan::FlushFrame, queue is null");
        QueueFamilyIndices indices = vkContext.FindQueueFamilies();
        VkDevice device = vkContext.GetDevice();
        vkGetDeviceQueue(device, indices.presentFamily, 0, &queue);
    }

    return PresentSwapchainImage(queue, imageIndex, renderFinishedSemaphore);
}

void RSSurfaceAndroidVulkan::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGD("RSSurfaceAndroidVulkan::SetColorSpace colorSpace %{public}d", colorSpace);
    if (colorSpace_ == colorSpace) {
        ROSEN_LOGD("RSSurfaceAndroidVulkan::SetColorSpace colorspace unchanged, skip");
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

void RSSurfaceAndroidVulkan::ClearBuffer()
{
}

void RSSurfaceAndroidVulkan::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}


void RSSurfaceAndroidVulkan::SetNativeWindowInfo(int32_t width, int32_t height, bool useAFBC, bool isProtected)
{
    if (nativeWindow_ == nullptr) {
        ROSEN_LOGE("SetNativeWindowInfo: native window is null!");
        return;
    }

    int32_t actualWidth = width;
    int32_t actualHeight = height;

    if (actualWidth != currentWidth_ || actualHeight != currentHeight_) {
        int32_t result = ANativeWindow_setBuffersGeometry(
            nativeWindow_, actualWidth, actualHeight, WINDOW_FORMAT_RGBA_8888);
        if (result != 0) {
            ROSEN_LOGE("SetNativeWindowInfo: setBuffersGeometry failed! error: %d", result);
        }
        if (!swapChain_.NeedRecreate()) {
            swapChain_.SetNeedRecreate(true);
            swapChain_.SetPendingSize(actualWidth, actualHeight);
        }
        currentWidth_ = actualWidth;
        currentHeight_ = actualHeight;
    }
}


bool RSSurfaceAndroidVulkan::SetupGrContext()
{
    if (GetRenderContextVulkan()) {
        GetRenderContextVulkan()->Init();
        GetRenderContextVulkan()->SetUpGpuContext();
        GetRenderContextVulkan()->SetColorSpace(colorSpace_);
    }
    return true;
}
} // namespace Rosen
} // namespace OHOS