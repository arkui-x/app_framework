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

#include "render_context/render_context_log.h"
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

bool RenderContextVK::Init(RenderEngineType type, const std::string& cacheDir)
{
    contextType_ = type;
    return SetUpGpuContext(cacheDir);
}

bool RenderContextVK::AbandonContext()
{
    if (drGPUContext_ == nullptr) {
        LOGD("grContext is nullptr.");
        return false;
    }
    drGPUContext_->FlushAndSubmit(true);
    drGPUContext_->PurgeUnlockAndSafeCacheGpuResources();
    return true;
}

bool RenderContextVK::SetUpGpuContext(const std::string& cacheDir)
{
    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("Drawing GPUContext has already created!!");
        return true;
    }
    drGPUContext_ = CreateDrawingGPUContext(cacheDir);
    return true;    if (drGPUContext_ != nullptr) {
        ROSEN_LOGD("Drawing GPUContext has already created!!");
        return true;
    }
    drGPUContext_ = CreateDrawingGPUContext(cacheDir);
    return true;
}

std::shared_ptr<Drawing::GPUContext> RenderContextVK::CreateDrawingGPUContext(const std::string& cacheDir)
{
    return RsVulkanContext::Get(contextType_).CreateDrawingGPUContext();
}

void RenderContextVK::ReleaseDrawingGPUContext(std::shared_ptr<Drawing::GPUContext> gpuContext)
{
    // RsVulkanContext singleton owns the Vulkan GPUContext lifecycle on cross-platform.
}

bool RenderContextVK::QueryMaxGpuBufferSize(uint32_t& maxWidth, uint32_t& maxHeight)
{
    LOGI("RenderContextVK::QueryMaxGpuBufferSize: using Vulkan backend");
    auto& vkContext = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER);
    auto& vkInterface = vkContext.GetRsVulkanInterface();
    VkPhysicalDevice physicalDevice = vkContext.GetPhysicalDevice();
    if (physicalDevice == VK_NULL_HANDLE) {
        LOGE("RenderContextVK::QueryMaxGpuBufferSize: Vulkan physical device is null");
        return false;
    }

    if (!vkInterface->vkGetPhysicalDeviceProperties) {
        LOGE("RenderContextVK::QueryMaxGpuBufferSize: vkGetPhysicalDeviceProperties is null");
        return false;
    }
    VkPhysicalDeviceProperties deviceProperties {};
    vkInterface->vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    maxWidth = deviceProperties.limits.maxImageDimension2D;
    maxHeight = deviceProperties.limits.maxImageDimension2D;

    LOGI("RenderContextVK::QueryMaxGpuBufferSize: Vulkan max image dimension = %u", maxWidth);
    return true;
}

void RenderContextVK::AddSurface()
{
    int32_t count = surface_count_.fetch_add(1, std::memory_order_relaxed) + 1;
    ROSEN_LOGD("RenderContextVK::AddSurface surface_count_=%{public}d", count);
}

void RenderContextVK::DeleteSurface()
{
    int32_t count = surface_count_.fetch_sub(1, std::memory_order_acq_rel) - 1;
    ROSEN_LOGD("RenderContextVK::DeleteSurface surface_count_=%{public}d", count);

    if (cleanUpHelper_ != nullptr && count <= 0) {
        ROSEN_LOGD("RenderContextVK::DeleteSurface all surfaces gone, cleanUpHelper_");
        cleanUpHelper_();
    }
}

void RenderContextVK::SetCleanUpHelper(std::function<void()> func)
{
    cleanUpHelper_ = std::move(func);
}

void RenderContextVK::DestroySharedSource()
{
    int32_t count = surface_count_.load(std::memory_order_acquire);
    ROSEN_LOGD("RenderContextVK::DestroySharedSource surface_count_=%{public}d", count);
    if (count > 0) {
        return;
    }
    AbandonContext();
}

} // namespace Rosen
} // namespace OHOS
