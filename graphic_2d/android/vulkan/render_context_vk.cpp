/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "render_context/new_render_context/render_context_vk.h"

#include "platform/common/rs_log.h"
#include "rs_vulkan_context.h"

#ifdef USE_M133_SKIA
#include "include/gpu/ganesh/vk/GrVkBackendSemaphore.h"
#endif

namespace OHOS {
namespace Rosen {

RenderContextVK::~RenderContextVK()
{
    AbandonContext();
}

bool RenderContextVK::Init()
{
    return true;
}

bool RenderContextVK::AbandonContext()
{
    if (drGPUContext_ == nullptr) {
        ROSEN_LOGD("grContext is nullptr.");
        return true;
    }
    drGPUContext_->FlushAndSubmit(true);
    drGPUContext_->PurgeUnlockAndSafeCacheGpuResources();
    return true;
}

bool RenderContextVK::SetUpGpuContext(std::shared_ptr<Drawing::GPUContext> drawingContext)
{
    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("Drawing GPUContext has already created!!");
        return true;
    }
    if (drawingContext == nullptr) {
        drawingContext = RsVulkanContext::GetSingleton(cacheDir_).CreateDrawingContext();
        if (drawingContext == nullptr) {
            ROSEN_LOGE("Fail to create vulkan GPUContext");
            return false;
        }
    }
    std::shared_ptr<Drawing::GPUContext> drGPUContext(drawingContext);
    drGPUContext_ = std::move(drGPUContext);
    return true;
}

void RenderContextVK::SetRenderContextType(uint8_t type)
{
    RenderContextVKType renderContextType = static_cast<RenderContextVKType>(type);
    if (renderContextType == RenderContextVKType::PROTECTED_REDRAW) {
        isProtected_.store(true, std::memory_order_release);
        RsVulkanContext::GetSingleton().SetIsProtected(true);
    } else if (renderContextType == RenderContextVKType::UNPROTECTED_REDRAW) {
        isProtected_.store(false, std::memory_order_release);
        RsVulkanContext::GetSingleton().SetIsProtected(false);
    }
    // if RsVulkanContext Singleton not call SetIsProtected, it means RenderContextVKType is BASIC_RENDER.
}

void RenderContextVK::ChangeProtectedState(bool isProtected)
{
    RsVulkanContext::GetSingleton().SetIsProtected(isProtected);

    // Use compare_exchange_strong to avoid race condition
    // Only update drGPUContext_ if the value actually changes
    bool expected = !isProtected;
    if (isProtected_.compare_exchange_strong(expected, isProtected, std::memory_order_acq_rel,
                                             std::memory_order_acquire)) {
        // The value was different from expected, so it actually changed
        drGPUContext_ = RsVulkanContext::GetSingleton().GetDrawingContext();
    }
    // If compare_exchange_strong returns false, expected now contains the current value
    // In this case, we just need to ensure isProtected_ is set to the correct value
    isProtected_.store(isProtected, std::memory_order_release);
}

bool RenderContextVK::QueryMaxGpuBufferSize(uint32_t& maxWidth, uint32_t& maxHeight)
{
    ROSEN_LOGI("RenderContextVK::QueryMaxGpuBufferSize: using Vulkan backend");
    auto& vkContext = RsVulkanContext::GetSingleton();
    VkPhysicalDevice physicalDevice = vkContext.GetPhysicalDevice();
    if (physicalDevice == VK_NULL_HANDLE) {
        ROSEN_LOGE("RenderContextVK::QueryMaxGpuBufferSize: Vulkan physical device is null");
        return false;
    }

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    maxWidth = deviceProperties.limits.maxImageDimension2D;
    maxHeight = deviceProperties.limits.maxImageDimension2D;

    ROSEN_LOGI("RenderContextVK::QueryMaxGpuBufferSize: Vulkan max image dimension = %u", maxWidth);
    return true;
}
} // namespace Rosen
} // namespace OHOS
