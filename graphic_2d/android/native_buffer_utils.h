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

#ifndef NATIVE_BUFFER_UTILS
#define NATIVE_BUFFER_UTILS

#include <atomic>

#include <cstdint>
#include <vulkan/vulkan_android.h>
#include "sync_fence.h"
#include "rs_vulkan_context.h"

#ifdef USE_M133_SKIA
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrBackendSemaphore.h"
#else
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/GrBackendSemaphore.h"
#endif
#include "include/core/SkSurface.h"
#include "draw/surface.h"
#include "image/image.h"
#include "skia_adapter/skia_color_space.h"
#include "surface_type.h"

namespace OHOS::Rosen {
namespace NativeBufferUtils {
constexpr uint32_t VKIMAGE_LIMIT_SIZE = 10000 * 10000; // Vk-Image Size need less than 10000*10000
constexpr int32_t REFERENCE_COUNT = 1;
void DeleteVkImage(void* context);
class VulkanCleanupHelper {
public:
    VulkanCleanupHelper(RsVulkanContext& vkContext, VkImage image, VkDeviceMemory memory)
        : fDevice_(vkContext.GetDevice()),
          fImage_(image),
          fMemory_(memory),
          fDestroyImage_(vkContext.GetRsVulkanInterface().vkDestroyImage),
          fFreeMemory_(vkContext.GetRsVulkanInterface().vkFreeMemory),
          fRefCnt_(REFERENCE_COUNT) {}
    ~VulkanCleanupHelper()
    {
        fDestroyImage_(fDevice_, fImage_, nullptr);
        fFreeMemory_(fDevice_, fMemory_, nullptr);
    }

    VulkanCleanupHelper* Ref()
    {
        (void)fRefCnt_.fetch_add(+REFERENCE_COUNT, std::memory_order_relaxed);
        return this;
    }

    void UnRef()
    {
        if (fRefCnt_.fetch_add(-REFERENCE_COUNT, std::memory_order_acq_rel) == REFERENCE_COUNT) {
            delete this;
        }
    }

private:
    VkDevice fDevice_;
    VkImage fImage_;
    VkDeviceMemory fMemory_;
    PFN_vkDestroyImage fDestroyImage_;
    PFN_vkFreeMemory fFreeMemory_;
    mutable std::atomic<int32_t> fRefCnt_;
};

std::shared_ptr<Drawing::VKTextureInfo> MakeVKTextureInfoFromHardwareBuffer(struct AHardwareBuffer* nativeWindowBuffer,
    uint32_t width, uint32_t height);

bool AllocateHardwareBuffer(uint32_t width, uint32_t height, AHardwareBuffer** outBuffer);

}
} // OHOS::Rosen
#endif