/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "memory/rs_tag_tracker.h"
#include "native_buffer_utils.h"
#include "platform/common/rs_log.h"
#include "pipeline/sk_resource_manager.h"
#include <android/hardware_buffer.h>

#ifdef RS_ENABLE_VK
#ifdef USE_M133_SKIA
#include "include/gpu/ganesh/vk/GrVkBackendSurface.h"
#else
#include "include/gpu/GrBackendSurface.h"
#endif
#endif

namespace OHOS::Rosen {
namespace NativeBufferUtils {

void DeleteVkImage(void* context)
{
    VulkanCleanupHelper* cleanupHelper = static_cast<VulkanCleanupHelper*>(context);
    if (cleanupHelper != nullptr) {
        cleanupHelper->UnRef();
    }
}

bool GetNativeBufferFormatProperties(RsVulkanContext& vkContext, VkDevice device,
                                     const struct AHardwareBuffer* nativeBuffer,
                                     VkAndroidHardwareBufferFormatPropertiesANDROID* nbFormatProps,
                                     VkAndroidHardwareBufferPropertiesANDROID* nbProps)
{
    if (!nbFormatProps || !nbProps) {
        RS_LOGE("GetNativeBufferFormatProperties failed!");
        return false;
    }
    nbFormatProps->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID;
    nbFormatProps->pNext = nullptr;

    nbProps->sType = VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID;
    nbProps->pNext = nbFormatProps;

    VkResult err = vkContext.GetRsVulkanInterface().vkGetAndroidHardwareBufferPropertiesANDROID(device,
        nativeBuffer, nbProps);
    if (VK_SUCCESS != err) {
        ROSEN_LOGE("NativeBufferUtils: vkGetAndroidHardwareBufferPropertiesANDROID Failed ! %{public}d", err);
        return false;
    }
    return true;
}

bool CreateVkImage(RsVulkanContext& vkContext, VkImage* image,
    const VkAndroidHardwareBufferFormatPropertiesANDROID& nbFormatProps, const VkExtent3D& imageSize,
    VkImageUsageFlags usageFlags = 0)
{
    VkExternalFormatANDROID externalFormat;
    externalFormat.sType = VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID;
    externalFormat.pNext = nullptr;
    externalFormat.externalFormat = 0;  // 0:externalFormat
    if (nbFormatProps.format == VK_FORMAT_UNDEFINED) {
        externalFormat.externalFormat = nbFormatProps.externalFormat;
    }
    const VkExternalMemoryImageCreateInfo externalMemoryImageInfo {
        VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO,
        &externalFormat,
        VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID,
    };
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;
    VkImageCreateFlags flags = 0; // 0:flags
    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        &externalMemoryImageInfo,
        flags,
        VK_IMAGE_TYPE_2D,
        nbFormatProps.format,
        imageSize,
        1, // 1:mipLevels
        1, // 1:arrayLayers
        VK_SAMPLE_COUNT_1_BIT,
        tiling,
        usageFlags,
        VK_SHARING_MODE_EXCLUSIVE,
        0, // 0:queueFamilyIndexCount
        0, // 0:pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,
    };
    if (imageSize.width == 0 || imageSize.height == 0 || imageSize.depth == 0 ||
        imageSize.depth > VKIMAGE_LIMIT_SIZE / imageSize.width / imageSize.height) {
        return false;
    }
    VkResult err = vkContext.GetRsVulkanInterface().vkCreateImage(vkContext.GetDevice(),
        &imageCreateInfo, nullptr, image);
    if (err != VK_SUCCESS) {
        ROSEN_LOGE("NativeBufferUtils: vkCreateImage failed");
        return false;
    }
    return true;
}

bool AllocateDeviceMemory(RsVulkanContext& vkContext, VkDeviceMemory* memory, VkImage& image,
    struct AHardwareBuffer* nativeBuffer, VkAndroidHardwareBufferPropertiesANDROID& nbProps)
{
    VkPhysicalDeviceMemoryProperties2 physicalDeviceMemProps;
    physicalDeviceMemProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
    physicalDeviceMemProps.pNext = nullptr;
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    uint32_t foundTypeIndex = 0; // 0:foundTypeIndex
    VkDevice device = vkInterface.GetDevice();
    VkPhysicalDevice physicalDevice = vkInterface.GetPhysicalDevice();
    vkInterface.vkGetPhysicalDeviceMemoryProperties2(physicalDevice, &physicalDeviceMemProps);
    uint32_t memTypeCnt = physicalDeviceMemProps.memoryProperties.memoryTypeCount;
    bool found = false;
    for (uint32_t i = 0; i < memTypeCnt; ++i) { // 0:memTypeCnt
        if (nbProps.memoryTypeBits & (1 << i)) { // 0:memoryTypeBits
            const VkPhysicalDeviceMemoryProperties& pdmp = physicalDeviceMemProps.memoryProperties;
            uint32_t supportedFlags = pdmp.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            if (supportedFlags == VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                foundTypeIndex = i;
                found = true;
                break;
            }
        }
    }
    if (!found) {
        ROSEN_LOGE("NativeBufferUtils: no fit memory type, memoryTypeBits is %{public}u", nbProps.memoryTypeBits);
        vkInterface.vkDestroyImage(device, image, nullptr);
        return false;
    }
    VkImportAndroidHardwareBufferInfoANDROID nbImportInfo;
    nbImportInfo.sType = VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID;
    nbImportInfo.pNext = nullptr;
    nbImportInfo.buffer = nativeBuffer;
    VkMemoryDedicatedAllocateInfo dedicatedAllocInfo;
    dedicatedAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO;
    dedicatedAllocInfo.pNext = &nbImportInfo;
    dedicatedAllocInfo.image = image;
    dedicatedAllocInfo.buffer = VK_NULL_HANDLE;
    VkMemoryAllocateInfo allocInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, &dedicatedAllocInfo, nbProps.allocationSize, foundTypeIndex};
    VkResult err = vkInterface.vkAllocateMemory(device, &allocInfo, nullptr, memory);
    if (err != VK_SUCCESS) {
        vkInterface.vkDestroyImage(device, image, nullptr);
        HILOG_COMM_ERROR("NativeBufferUtils: vkAllocateMemory Fail, err:%{public}d", err);
        return false;
    }
    return true;
}

bool BindImageMemory(VkDevice device, RsVulkanContext& vkContext, VkImage& image, VkDeviceMemory& memory)
{
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkBindImageMemoryInfo bindImageInfo;
    bindImageInfo.sType = VK_STRUCTURE_TYPE_BIND_IMAGE_MEMORY_INFO;
    bindImageInfo.pNext = nullptr;
    bindImageInfo.image = image;
    bindImageInfo.memory = memory;
    bindImageInfo.memoryOffset = 0; // 0:memoryOffset

    VkResult err = vkInterface.vkBindImageMemory2(device, 1, &bindImageInfo); // 1:bindInfoCount
    if (err != VK_SUCCESS) {
        ROSEN_LOGE("NativeBufferUtils: vkBindImageMemory2 failed");
        vkInterface.vkDestroyImage(device, image, nullptr);
        vkInterface.vkFreeMemory(device, memory, nullptr);
        return false;
    }
    return true;
}

void SetImageInfo(std::shared_ptr<Drawing::VKTextureInfo> imageInfo,
    VkAndroidHardwareBufferFormatPropertiesANDROID& nbFormatProps)
{
    imageInfo->vkAlloc.source = Drawing::VKMemSource::EXTERNAL;
    imageInfo->imageTiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo->imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo->format = nbFormatProps.format;
    imageInfo->levelCount = 1; // 1:levelCount
    imageInfo->sampleCount = 1; // 1:sampleCount
    imageInfo->currentQueueFamily = VK_QUEUE_FAMILY_EXTERNAL;
    imageInfo->ycbcrConversionInfo.format = nbFormatProps.format,
    imageInfo->ycbcrConversionInfo.externalFormat = nbFormatProps.externalFormat,
    imageInfo->ycbcrConversionInfo.ycbcrModel = nbFormatProps.suggestedYcbcrModel,
    imageInfo->ycbcrConversionInfo.ycbcrRange = nbFormatProps.suggestedYcbcrRange,
    imageInfo->ycbcrConversionInfo.xChromaOffset = nbFormatProps.suggestedXChromaOffset,
    imageInfo->ycbcrConversionInfo.yChromaOffset = nbFormatProps.suggestedYChromaOffset,
    imageInfo->ycbcrConversionInfo.chromaFilter = VK_FILTER_NEAREST,
    imageInfo->ycbcrConversionInfo.forceExplicitReconstruction = VK_FALSE,
    imageInfo->ycbcrConversionInfo.formatFeatures = nbFormatProps.formatFeatures;
    if (VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT & nbFormatProps.formatFeatures) {
        imageInfo->ycbcrConversionInfo.chromaFilter = VK_FILTER_LINEAR;
    }
}

std::shared_ptr<Drawing::VKTextureInfo> MakeVKTextureInfoFromHardwareBuffer(struct AHardwareBuffer* nativeBuffer,
    uint32_t width, uint32_t height)
{
    if (nativeBuffer == nullptr) {
        ROSEN_LOGE("MakeVKTextureInfoFromAHardwareBuffer: AHardwareBuffer is nullptr");
        return {};
    }

    auto& vkContext = RsVulkanContext::GetSingleton();
    VkDevice device = vkContext.GetDevice();
    VkAndroidHardwareBufferFormatPropertiesANDROID nbFormatProps;
    VkAndroidHardwareBufferPropertiesANDROID nbProps;
    if (!GetNativeBufferFormatProperties(vkContext, device, nativeBuffer, &nbFormatProps, &nbProps)) {
        return {};
    }
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT;
    if (nbFormatProps.format != VK_FORMAT_UNDEFINED) {
        usageFlags = usageFlags | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }
    VkImage image;
    const VkExtent3D imageSize = {width, height, 1}; // 1:depth
    if (!CreateVkImage(vkContext, &image, nbFormatProps, imageSize, usageFlags)) {
        return {};
    }
    VkDeviceMemory memory;
    if (!AllocateDeviceMemory(vkContext, &memory, image, nativeBuffer, nbProps)) {
        return {};
    }
    if (!BindImageMemory(device, vkContext, image, memory)) {
        return {};
    }

    std::shared_ptr<Drawing::VKTextureInfo> imageInfo = std::make_shared<Drawing::VKTextureInfo>();
    imageInfo->vkImage = image;
    imageInfo->vkAlloc.memory = memory;
    imageInfo->vkAlloc.size = nbProps.allocationSize;
    imageInfo->vkProtected = false;
    imageInfo->imageUsageFlags = usageFlags;
    SetImageInfo(imageInfo, nbFormatProps);
    QueueFamilyIndices indices = vkContext.FindQueueFamilies();
    imageInfo->sharingMode =
        (indices.graphicsFamily != indices.presentFamily) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
    return imageInfo;
}

} // namespace NativeBufferUtils
} // namespace OHOS::Rosen
