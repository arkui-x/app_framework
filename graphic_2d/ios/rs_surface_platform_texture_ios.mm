/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "rs_surface_platform_texture_ios.h"

#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <objc/message.h>
#import <objc/runtime.h>
#include "platform/common/rs_log.h"
#include "platform/common/rs_system_properties.h"
#include "render_context/render_context.h"
#include <dlfcn.h>
#ifdef RS_ENABLE_VK
#include <algorithm>
#endif

namespace OHOS {
namespace Rosen {

RSSurfacePlatformTextureIOS::RSSurfacePlatformTextureIOS(const RSSurfaceExtConfig& config)
    : RSSurfaceExt()
{
    config_ = config;
}

RSSurfacePlatformTextureIOS::~RSSurfacePlatformTextureIOS()
{
    [videoOutput_ release];
    [platformViewFrameProvider_ release];

    if (textureId_ > 0 && attachCallback_ != nullptr) {
        attachCallback_(textureId_, false);
    }

    if (platformEglContext_) {
        [platformEglContext_ release];
        platformEglContext_ = nullptr;
    }

    if (textureId_) {
        glDeleteTextures(1, &textureId_);
    }
#ifdef RS_ENABLE_VK
    if (RSSystemProperties::IsUseVulkan()) {
        CleanupVulkanResources();
    }
#endif
}

void RSSurfacePlatformTextureIOS::EnsureTextureCacheExists()
{
    if (!cache_ref_) {
        CVOpenGLESTextureCacheRef cache;
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL,
                                                    [EAGLContext currentContext], NULL, &cache);
        if (err == noErr) {
            cache_ref_.Reset(cache);
        } else {
            ROSEN_LOGE("RSSurfacePlatformTextureIOS::Failed to create GLES texture cache");
            return;
        }
    }
}

CVPixelBufferRef RSSurfacePlatformTextureIOS::GetPixelBuffer()
{
    if (!videoOutput_) {
        ROSEN_LOGE("RSSurfaceTextureIOS::videoOutput_ is nullptr");
        return nullptr;
    }

    CMTime outputItemTime = [videoOutput_ itemTimeForHostTime:CACurrentMediaTime()];
    if ([videoOutput_ hasNewPixelBufferForItemTime:outputItemTime]) {
        return [videoOutput_ copyPixelBufferForItemTime:outputItemTime itemTimeForDisplay:NULL];
    } else {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::GetPixelBuffer is nullptr");
        return nullptr;
    }
}

CVPixelBufferRef RSSurfacePlatformTextureIOS::GetPlatformViewPixelBuffer()
{
    if (!platformViewFrameProvider_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::platformViewFrameProvider_ is nullptr");
        return nullptr;
    }

    SEL selector = NSSelectorFromString(@"currentPixelBuffer");
    if (![platformViewFrameProvider_ respondsToSelector:selector]) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::platformViewFrameProvider_ has no currentPixelBuffer selector");
        return nullptr;
    }

    typedef CVPixelBufferRef (*CurrentPixelBufferMsgSend)(id, SEL);
    auto msgSend = reinterpret_cast<CurrentPixelBufferMsgSend>(objc_msgSend);
    return msgSend(platformViewFrameProvider_, selector);
}

void RSSurfacePlatformTextureIOS::CreateTextureFromPixelBuffer()
{
    if (buffer_ref_ == nullptr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::buffer_ref_ is nullptr");
        return;
    }
    CVOpenGLESTextureRef texture;
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(
        kCFAllocatorDefault, cache_ref_, buffer_ref_, nullptr, GL_TEXTURE_2D, GL_RGBA,
        static_cast<int>(CVPixelBufferGetWidth(buffer_ref_)),
        static_cast<int>(CVPixelBufferGetHeight(buffer_ref_)), GL_BGRA, GL_UNSIGNED_BYTE, 0,
        &texture);
    if (err != noErr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::Could not create texture from pixel buffer");
    } else {
        texture_ref_.Reset(texture);
    }
}

void RSSurfacePlatformTextureIOS::UpdateSurfaceDefaultSize(float width, float height)
{
}

void RSSurfacePlatformTextureIOS::InitializePlatformEglContext()
{
    platformEglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3
        sharegroup:static_cast<EAGLContext*>(RenderContextGL::GetResourceContext()).sharegroup];
    if (platformEglContext_ == nullptr) {
        platformEglContext_ = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2
            sharegroup:static_cast<EAGLContext*>(RenderContextGL::GetResourceContext()).sharegroup];
    }
}

Drawing::BitmapFormat RSSurfacePlatformTextureIOS::GetBitmapFormatByBackend(bool useVulkan) const
{
    return useVulkan ? Drawing::BitmapFormat{ Drawing::COLORTYPE_BGRA_8888, Drawing::ALPHATYPE_PREMUL }
                     : Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
}

bool RSSurfacePlatformTextureIOS::FillTextureInfoByBackend(Drawing::TextureInfo& textureInfo, int texWidth, int texHeight,
    const char* logPrefix)
{
    const bool useVulkan = RSSystemProperties::IsUseVulkan();
    textureInfo.SetWidth(texWidth);
    textureInfo.SetHeight(texHeight);
    textureInfo.SetIsMipMapped(false);
#ifdef RS_ENABLE_VK
    if (useVulkan) {
        if (!vkImageReady_ || vkImage_ == VK_NULL_HANDLE) {
            ROSEN_LOGE("RSSurfacePlatformTextureIOS %s vkImage not ready vkImageReady_=%{public}d vkImage_=%{public}p",
                logPrefix, vkImageReady_, reinterpret_cast<void*>(vkImage_));
            return false;
        }
        auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
        QueueFamilyIndices indices = vkContext.FindQueueFamilies();
        std::shared_ptr<Drawing::VKTextureInfo> vkTextureInfo = std::make_shared<Drawing::VKTextureInfo>();
        vkTextureInfo->vkImage = vkImage_;
        vkTextureInfo->vkAlloc.memory = vkImageMemory_;
        vkTextureInfo->vkAlloc.offset = 0;
        vkTextureInfo->vkAlloc.size = vkImageAllocSize_;
        vkTextureInfo->vkAlloc.flags = 0;
        vkTextureInfo->imageTiling = VK_IMAGE_TILING_OPTIMAL;
        vkTextureInfo->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        vkTextureInfo->format = vkImageFormat_;
        vkTextureInfo->imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
                                          VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        vkTextureInfo->sampleCount = 1;
        vkTextureInfo->levelCount = 1;
        vkTextureInfo->currentQueueFamily = indices.graphicsFamily;
        vkTextureInfo->vkProtected = false;
        vkTextureInfo->sharingMode = (indices.presentFamily != UINT32_MAX &&
            indices.graphicsFamily != indices.presentFamily) ?
            VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        textureInfo.SetVKTextureInfo(vkTextureInfo);
        return true;
    }
#endif
    if (!texture_ref_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::%s texture_ref_ is nullptr", logPrefix);
        return false;
    }
    textureInfo.SetTarget(CVOpenGLESTextureGetTarget(texture_ref_));
    textureInfo.SetID(CVOpenGLESTextureGetName(texture_ref_));
    textureInfo.SetFormat(GL_RGBA8_OES);
    return true;
}

void RSSurfacePlatformTextureIOS::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    if (active_ == false || currentSharePtr_ != config_.additionalData) {
        currentSharePtr_ = config_.additionalData;
        if (config_.additionalData == nullptr) {
            return;
        }
        if (initTypeCallback_) {
            initTypeCallback_(isVideo_);
        }
        [videoOutput_ release];
        videoOutput_ = nullptr;
        [platformViewFrameProvider_ release];
        platformViewFrameProvider_ = nullptr;
        isPlatformViewFrameProvider_ = false;
        if (IsVideo()) {
            videoOutput_ = [static_cast<AVPlayerItemVideoOutput*>(config_.additionalData) retain];
            active_ = true;
            return;
        }
        if (IsPlatformViewFrameProvider()) {
            platformViewFrameProvider_ = [static_cast<NSObject*>(config_.additionalData) retain];
            isPlatformViewFrameProvider_ = true;
            active_ = true;
            return;
        }
        if (platformEglContext_ == nullptr) {
            InitializePlatformEglContext();
        }
        glGenTextures(1, &textureId_);
        if (attachCallback_) {
            attachCallback_(textureId_, true);
            active_ = true;
        }
        return;
    }

    if (isVideo_) {
        DrawTextureImageForVideo(canvas, freeze, clipRect);
    } else if (isPlatformViewFrameProvider_) {
        DrawTextureImageForPlatformView(canvas, freeze, clipRect);
    } else {
        DrawTextureImageGL(canvas, freeze, clipRect);
    }
}

bool RSSurfacePlatformTextureIOS::IsVideo()
{
    void* additionalData = config_.additionalData;
    if (IsObjectiveCObject(additionalData)) {
        id additionalObj = (__bridge id)additionalData;
        if ([additionalObj isKindOfClass:[AVPlayerItemVideoOutput class]]) {
            isVideo_ = true;
            return isVideo_;
        }
    }
    isVideo_ = false;
    return isVideo_;
}

bool RSSurfacePlatformTextureIOS::IsPlatformViewFrameProvider()
{
    void* additionalData = config_.additionalData;
    if (additionalData == nullptr) {
        return false;
    }
    id additionalObj = reinterpret_cast<id>(additionalData);
    return [additionalObj respondsToSelector:NSSelectorFromString(@"currentPixelBuffer")];
}

void RSSurfacePlatformTextureIOS::DrawTextureImageGL(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
{
    bool bufferAvailable = bufferAvailable_.load();
    if (!freeze && bufferAvailable_.load()) {
        bufferAvailable_.store(false);
    }

    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGI("create Drawing image fail");
        return;
    }
    EAGLContext* context = EAGLContext.currentContext;
    [EAGLContext setCurrentContext:platformEglContext_];
    if(updateCallback_) {
        updateCallback_(matrix);
    }
    glBindTexture(GL_TEXTURE_2D, textureId_);

    Drawing::TextureInfo textureInfo;
    textureInfo.SetWidth((int)clipRect.GetWidth());
    textureInfo.SetHeight((int)clipRect.GetHeight());
    textureInfo.SetIsMipMapped(false);
    textureInfo.SetTarget(GL_TEXTURE_2D);
    textureInfo.SetID(textureId_);
    textureInfo.SetFormat(GL_RGBA8_OES);
    Drawing::BitmapFormat fmt =
        Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        ROSEN_LOGE("BuildFromTexture fail");
        return;
    }
    canvas.DrawImage(*image, clipRect.GetLeft(), clipRect.GetTop(), Drawing::SamplingOptions());
    [EAGLContext setCurrentContext:context];
}

void RSSurfacePlatformTextureIOS::DrawTextureImageForVideo(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    const bool useVulkan = RSSystemProperties::IsUseVulkan();
    if (!useVulkan) {
        EnsureTextureCacheExists();
    }
    if (!freeze) {
        auto pixelBuffer = GetPixelBuffer();
        if (pixelBuffer) {
            buffer_ref_.Reset(pixelBuffer);
#ifdef RS_ENABLE_VK
            if (useVulkan && !UpdateVkImageFromPixelBuffer(buffer_ref_)) {
                ROSEN_LOGE("RSSurfacePlatformTextureIOS::video UpdateVkImageFromPixelBuffer failed");
            }
#endif
            if (!useVulkan) {
                CreateTextureFromPixelBuffer();
            }
        }
    }
    if (!useVulkan && !texture_ref_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::texture_ref_ is nullptr");
        return;
    }
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGE("create Drawing image fail");
        return;
    }
    int texWidth = buffer_ref_ ? static_cast<int>(CVPixelBufferGetWidth(buffer_ref_))
                               : static_cast<int>(clipRect.GetWidth());
    int texHeight = buffer_ref_ ? static_cast<int>(CVPixelBufferGetHeight(buffer_ref_))
                                : static_cast<int>(clipRect.GetHeight());

    Drawing::TextureInfo textureInfo;
    if (!FillTextureInfoByBackend(textureInfo, texWidth, texHeight, "video")) {
        return;
    }
    Drawing::BitmapFormat fmt = GetBitmapFormatByBackend(useVulkan);
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::video BuildFromTexture failed");
        return;
    }
    Drawing::Rect srcRect(0, 0, texWidth, texHeight);
    canvas.DrawImageRect(*image, srcRect, clipRect,
        Drawing::SamplingOptions(Drawing::FilterMode::LINEAR),
        Drawing::SrcRectConstraint::FAST_SRC_RECT_CONSTRAINT);
}

void RSSurfacePlatformTextureIOS::DrawTextureImageForPlatformView(RSPaintFilterCanvas& canvas, bool freeze,
    const Drawing::Rect& clipRect)
{
    const bool useVulkan = RSSystemProperties::IsUseVulkan();
    if (!useVulkan) {
        EnsureTextureCacheExists();
    }
    if (!freeze) {
        auto pixelBuffer = GetPlatformViewPixelBuffer();
        if (pixelBuffer) {
            buffer_ref_.Reset(pixelBuffer);
#ifdef RS_ENABLE_VK
            if (useVulkan && !UpdateVkImageFromPixelBuffer(buffer_ref_)) {
                ROSEN_LOGE("RSSurfacePlatformTextureIOS::UpdateVkImageFromPixelBuffer failed");
            }
#endif
            if (!useVulkan) {
                CreateTextureFromPixelBuffer();
            }
        }
    }
    if (!useVulkan && !texture_ref_) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::platformView texture_ref_ is nullptr");
        return;
    }
    auto image = std::make_shared<Drawing::Image>();
    if (image == nullptr) {
        ROSEN_LOGE("create Drawing image fail");
        return;
    }

    // textureInfo must use actual pixel buffer dimensions for BuildFromTexture
    // to correctly wrap the GL texture.
    int texWidth = buffer_ref_ ? static_cast<int>(CVPixelBufferGetWidth(buffer_ref_))
                               : static_cast<int>(clipRect.GetWidth());
    int texHeight = buffer_ref_ ? static_cast<int>(CVPixelBufferGetHeight(buffer_ref_))
                                : static_cast<int>(clipRect.GetHeight());

    Drawing::TextureInfo textureInfo;
    if (!FillTextureInfoByBackend(textureInfo, texWidth, texHeight, "platformView")) {
        return;
    }
    Drawing::BitmapFormat fmt = GetBitmapFormatByBackend(useVulkan);
    bool ret = image->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::BOTTOM_LEFT, fmt, nullptr);
    if (!ret) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::BuildFromTexture failed");
        return;
    }

    // Scale the full pixel-sized texture to fit the layout-sized clip rect.
    Drawing::Rect srcRect(0, 0, texWidth, texHeight);
    canvas.DrawImageRect(*image, srcRect, clipRect,
        Drawing::SamplingOptions(Drawing::FilterMode::LINEAR),
        Drawing::SrcRectConstraint::FAST_SRC_RECT_CONSTRAINT);
}

#ifdef RS_ENABLE_VK
uint32_t RSSurfacePlatformTextureIOS::FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const
{
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    VkPhysicalDeviceMemoryProperties memProperties {};
    vkContext.GetRsVulkanInterface()->vkGetPhysicalDeviceMemoryProperties(vkContext.GetPhysicalDevice(), &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
        if ((typeFilter & (1u << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return UINT32_MAX;
}

bool RSSurfacePlatformTextureIOS::EnsureVulkanCommandPool()
{
    if (vkCommandPool_ != VK_NULL_HANDLE) {
        return true;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkCommandPoolCreateInfo poolInfo {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = vkContext.FindQueueFamilies().graphicsFamily;
    bool ret = vkInterface->vkCreateCommandPool(vkContext.GetDevice(), &poolInfo, nullptr, &vkCommandPool_) == VK_SUCCESS;
    return ret;
}

void RSSurfacePlatformTextureIOS::DestroyVkStagingBufferResources(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device)
{
    if (vkStagingBuffer_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroyBuffer(device, vkStagingBuffer_, nullptr);
        vkStagingBuffer_ = VK_NULL_HANDLE;
    }
    if (vkStagingMemory_ != VK_NULL_HANDLE) {
        vkInterface->vkFreeMemory(device, vkStagingMemory_, nullptr);
        vkStagingMemory_ = VK_NULL_HANDLE;
    }
}

bool RSSurfacePlatformTextureIOS::CreateAndBindVkStagingBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device,
    size_t size)
{
    VkBufferCreateInfo bufferInfo {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkInterface->vkCreateBuffer(device, &bufferInfo, nullptr, &vkStagingBuffer_) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkCreateBuffer failed");
        return false;
    }

    VkMemoryRequirements memReq {};
    vkInterface->vkGetBufferMemoryRequirements(device, vkStagingBuffer_, &memReq);
    constexpr VkMemoryPropertyFlags kHostFlags =
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    uint32_t typeIndex = FindVulkanMemoryType(memReq.memoryTypeBits, kHostFlags);
    if (typeIndex == UINT32_MAX) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::staging FindVulkanMemoryType failed bits=%{public}u",
            memReq.memoryTypeBits);
        vkInterface->vkDestroyBuffer(device, vkStagingBuffer_, nullptr);
        vkStagingBuffer_ = VK_NULL_HANDLE;
        return false;
    }
    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = typeIndex;
    if (vkInterface->vkAllocateMemory(device, &allocInfo, nullptr, &vkStagingMemory_) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkAllocateMemory for staging failed");
        vkInterface->vkDestroyBuffer(device, vkStagingBuffer_, nullptr);
        vkStagingBuffer_ = VK_NULL_HANDLE;
        return false;
    }
    if (vkInterface->vkBindBufferMemory(device, vkStagingBuffer_, vkStagingMemory_, 0) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkBindBufferMemory failed");
        vkInterface->vkFreeMemory(device, vkStagingMemory_, nullptr);
        vkStagingMemory_ = VK_NULL_HANDLE;
        vkInterface->vkDestroyBuffer(device, vkStagingBuffer_, nullptr);
        vkStagingBuffer_ = VK_NULL_HANDLE;
        return false;
    }
    vkStagingBufferSize_ = size;
    return true;
}

bool RSSurfacePlatformTextureIOS::EnsureVkStagingBuffer(size_t size)
{
    if (size == 0) {
        return false;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    if (vkStagingBuffer_ != VK_NULL_HANDLE && vkStagingBufferSize_ >= size) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::Reuse vk staging buffer size=%{public}zu", vkStagingBufferSize_);
        return true;
    }
    DestroyVkStagingBufferResources(vkInterface, device);
    return CreateAndBindVkStagingBuffer(vkInterface, device, size);
}

void RSSurfacePlatformTextureIOS::DestroyVkImageResources(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device)
{
    if (vkImage_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroyImage(device, vkImage_, nullptr);
        vkImage_ = VK_NULL_HANDLE;
    }
    if (vkImageMemory_ != VK_NULL_HANDLE) {
        vkInterface->vkFreeMemory(device, vkImageMemory_, nullptr);
        vkImageMemory_ = VK_NULL_HANDLE;
    }
}

bool RSSurfacePlatformTextureIOS::CreateVkImage2DForTexture(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device,
    int width, int height)
{
    VkImageCreateInfo imageInfo {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = vkImageFormat_;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage =
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkInterface->vkCreateImage(device, &imageInfo, nullptr, &vkImage_) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkCreateImage failed width=%{public}d height=%{public}d",
            width, height);
        return false;
    }
    return true;
}

bool RSSurfacePlatformTextureIOS::AllocateDeviceLocalMemoryAndBindVkImage(std::shared_ptr<RsVulkanInterface>& vkInterface,
    VkDevice device, int width, int height)
{
    VkMemoryRequirements memReq {};
    vkInterface->vkGetImageMemoryRequirements(device, vkImage_, &memReq);
    uint32_t typeIndex = FindVulkanMemoryType(memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (typeIndex == UINT32_MAX) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::image FindVulkanMemoryType failed bits=%{public}u",
            memReq.memoryTypeBits);
        vkInterface->vkDestroyImage(device, vkImage_, nullptr);
        vkImage_ = VK_NULL_HANDLE;
        return false;
    }
    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = typeIndex;
    if (vkInterface->vkAllocateMemory(device, &allocInfo, nullptr, &vkImageMemory_) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkAllocateMemory for image failed");
        vkInterface->vkDestroyImage(device, vkImage_, nullptr);
        vkImage_ = VK_NULL_HANDLE;
        return false;
    }
    if (vkInterface->vkBindImageMemory(device, vkImage_, vkImageMemory_, 0) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkBindImageMemory failed");
        vkInterface->vkFreeMemory(device, vkImageMemory_, nullptr);
        vkImageMemory_ = VK_NULL_HANDLE;
        vkInterface->vkDestroyImage(device, vkImage_, nullptr);
        vkImage_ = VK_NULL_HANDLE;
        return false;
    }
    vkImageWidth_ = width;
    vkImageHeight_ = height;
    vkImageAllocSize_ = memReq.size;
    vkImageReady_ = false;
    return true;
}

bool RSSurfacePlatformTextureIOS::EnsureVkImage(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return false;
    }
    if (vkImage_ != VK_NULL_HANDLE && width == vkImageWidth_ && height == vkImageHeight_) {
        return true;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    DestroyVkImageResources(vkInterface, device);
    if (!CreateVkImage2DForTexture(vkInterface, device, width, height)) {
        return false;
    }
    return AllocateDeviceLocalMemoryAndBindVkImage(vkInterface, device, width, height);
}

bool RSSurfacePlatformTextureIOS::ValidatePixelBufferForVulkanUpload(CVPixelBufferRef pixelBuffer,
    size_t& outWidth, size_t& outHeight, size_t& outBytesPerRow, size_t& outUploadSize)
{
    if (pixelBuffer == nullptr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::UpdateVkImageFromPixelBuffer pixelBuffer is null");
        return false;
    }
    OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
    if (pixelFormat != kCVPixelFormatType_32BGRA) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::Unsupported pixel format: %{public}u",
            static_cast<uint32_t>(pixelFormat));
        return false;
    }
    outWidth = static_cast<size_t>(CVPixelBufferGetWidth(pixelBuffer));
    outHeight = static_cast<size_t>(CVPixelBufferGetHeight(pixelBuffer));
    outBytesPerRow = static_cast<size_t>(CVPixelBufferGetBytesPerRow(pixelBuffer));
    outUploadSize = outBytesPerRow * outHeight;
    return true;
}

bool RSSurfacePlatformTextureIOS::CopyPixelBufferToVulkanStaging(CVPixelBufferRef pixelBuffer, size_t uploadSize)
{
    CVPixelBufferLockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    auto src = static_cast<const uint8_t*>(CVPixelBufferGetBaseAddress(pixelBuffer));
    if (src == nullptr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::CVPixelBufferGetBaseAddress returned null");
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return false;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    void* mapped = nullptr;
    if (vkInterface->vkMapMemory(vkContext.GetDevice(), vkStagingMemory_, 0, uploadSize, 0, &mapped) != VK_SUCCESS ||
        mapped == nullptr) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkMapMemory failed");
        CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
        return false;
    }
    auto* dst = static_cast<uint8_t*>(mapped);
    std::copy(src, src + uploadSize, dst);
    vkInterface->vkUnmapMemory(vkContext.GetDevice(), vkStagingMemory_);
    CVPixelBufferUnlockBaseAddress(pixelBuffer, kCVPixelBufferLock_ReadOnly);
    return true;
}

bool RSSurfacePlatformTextureIOS::AllocateAndBeginVkUploadCommandBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface,
    VkDevice device, VkCommandBuffer& cmd)
{
    VkCommandBufferAllocateInfo allocInfo {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vkCommandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    cmd = VK_NULL_HANDLE;
    if (vkInterface->vkAllocateCommandBuffers(device, &allocInfo, &cmd) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkAllocateCommandBuffers failed");
        return false;
    }
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkInterface->vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkBeginCommandBuffer failed");
        vkInterface->vkFreeCommandBuffers(device, vkCommandPool_, 1, &cmd);
        cmd = VK_NULL_HANDLE;
        return false;
    }
    return true;
}

void RSSurfacePlatformTextureIOS::RecordVkImageTransferAndSampleBarriers(std::shared_ptr<RsVulkanInterface>& vkInterface,
    VkCommandBuffer cmd, size_t width, size_t height, size_t bytesPerRow)
{
    VkImageMemoryBarrier toTransfer {};
    toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toTransfer.oldLayout = vkImageReady_ ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED;
    toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toTransfer.image = vkImage_;
    toTransfer.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toTransfer.subresourceRange.baseMipLevel = 0;
    toTransfer.subresourceRange.levelCount = 1;
    toTransfer.subresourceRange.baseArrayLayer = 0;
    toTransfer.subresourceRange.layerCount = 1;
    toTransfer.srcAccessMask = vkImageReady_ ? VK_ACCESS_SHADER_READ_BIT : 0;
    toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkInterface->vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
        nullptr, 0, nullptr, 1, &toTransfer);

    VkBufferImageCopy region {};
    region.bufferOffset = 0;
    region.bufferRowLength = static_cast<uint32_t>(bytesPerRow / 4);
    region.bufferImageHeight = static_cast<uint32_t>(height);
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
    vkInterface->vkCmdCopyBufferToImage(cmd, vkStagingBuffer_, vkImage_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
        &region);

    VkImageMemoryBarrier toSampled {};
    toSampled.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toSampled.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toSampled.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toSampled.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toSampled.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toSampled.image = vkImage_;
    toSampled.subresourceRange = toTransfer.subresourceRange;
    toSampled.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toSampled.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkInterface->vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr, 0, nullptr, 1, &toSampled);
}

bool RSSurfacePlatformTextureIOS::EndSubmitWaitAndFreeVkCommandBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface,
    RsVulkanContext& vkContext, VkCommandBuffer cmd)
{
    VkDevice device = vkContext.GetDevice();
    VkQueue graphicsQueue = vkContext.GetGraphicsQueue();
    if (vkInterface->vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkEndCommandBuffer failed");
        vkInterface->vkFreeCommandBuffers(device, vkCommandPool_, 1, &cmd);
        return false;
    }
    VkSubmitInfo submitInfo {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    if (vkInterface->vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::vkQueueSubmit failed");
        vkInterface->vkFreeCommandBuffers(device, vkCommandPool_, 1, &cmd);
        return false;
    }
    vkInterface->vkQueueWaitIdle(graphicsQueue);
    vkInterface->vkFreeCommandBuffers(device, vkCommandPool_, 1, &cmd);
    return true;
}

bool RSSurfacePlatformTextureIOS::UpdateVkImageFromPixelBuffer(CVPixelBufferRef pixelBuffer)
{
    size_t width = 0;
    size_t height = 0;
    size_t bytesPerRow = 0;
    size_t uploadSize = 0;
    if (!ValidatePixelBufferForVulkanUpload(pixelBuffer, width, height, bytesPerRow, uploadSize)) {
        return false;
    }
    if (!EnsureVulkanCommandPool() || !EnsureVkImage(static_cast<int>(width), static_cast<int>(height)) ||
        !EnsureVkStagingBuffer(uploadSize)) {
        ROSEN_LOGE("RSSurfacePlatformTextureIOS::Ensure resources failed");
        return false;
    }
    if (!CopyPixelBufferToVulkanStaging(pixelBuffer, uploadSize)) {
        return false;
    }
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    if (!AllocateAndBeginVkUploadCommandBuffer(vkInterface, device, cmd)) {
        return false;
    }
    RecordVkImageTransferAndSampleBarriers(vkInterface, cmd, width, height, bytesPerRow);
    if (!EndSubmitWaitAndFreeVkCommandBuffer(vkInterface, vkContext, cmd)) {
        return false;
    }
    vkImageReady_ = true;
    return true;
}

void RSSurfacePlatformTextureIOS::CleanupVulkanResources()
{
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkDevice device = vkContext.GetDevice();
    if (device == VK_NULL_HANDLE) {
        return;
    }
    vkInterface->vkDeviceWaitIdle(device);
    if (vkImage_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroyImage(device, vkImage_, nullptr);
        vkImage_ = VK_NULL_HANDLE;
    }
    if (vkImageMemory_ != VK_NULL_HANDLE) {
        vkInterface->vkFreeMemory(device, vkImageMemory_, nullptr);
        vkImageMemory_ = VK_NULL_HANDLE;
    }
    vkImageAllocSize_ = 0;
    if (vkStagingBuffer_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroyBuffer(device, vkStagingBuffer_, nullptr);
        vkStagingBuffer_ = VK_NULL_HANDLE;
    }
    if (vkStagingMemory_ != VK_NULL_HANDLE) {
        vkInterface->vkFreeMemory(device, vkStagingMemory_, nullptr);
        vkStagingMemory_ = VK_NULL_HANDLE;
    }
    vkStagingBufferSize_ = 0;
    if (vkCommandPool_ != VK_NULL_HANDLE) {
        vkInterface->vkDestroyCommandPool(device, vkCommandPool_, nullptr);
        vkCommandPool_ = VK_NULL_HANDLE;
    }
}
#endif

bool RSSurfacePlatformTextureIOS::IsObjectiveCObject(const void* candidate)
{
    if (candidate == nullptr) {
        return false;
    }
    uintptr_t raw = reinterpret_cast<uintptr_t>(candidate);
    if ((raw & (sizeof(void*) - 1)) != 0) {
        return false;
    }
    const void* isa = *reinterpret_cast<const void *const *>(candidate);
    if (isa == nullptr) {
        return false;
    }
    Dl_info info {};
    return dladdr(isa, &info) != 0;
}
} // namespace Rosen
} // namespace OHOS