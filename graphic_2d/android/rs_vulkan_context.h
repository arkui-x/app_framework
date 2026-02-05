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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_VULKAN_CONTEXT_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_VULKAN_CONTEXT_H

#include <atomic>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include "include/third_party/vulkan/vulkan/vulkan_core.h"
#include "platform/common/rs_log.h"

#define VK_NO_PROTOTYPES 1
#include "vulkan/vulkan.h"
#include "vulkan/vulkan_android.h"
#include "image/gpu_context.h"

#ifdef USE_M133_SKIA
#include "include/gpu/vk/VulkanExtensions.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#else
#include "include/gpu/vk/GrVkExtensions.h"
#include "include/gpu/vk/GrVkBackendContext.h"
#include "include/gpu/GrDirectContext.h"
#endif

namespace OHOS {
namespace Rosen {
enum class VulkanInterfaceType : uint32_t {
    BASIC_RENDER = 0,
    PROTECTED_REDRAW,
    UNPROTECTED_REDRAW,
    MAX_INTERFACE_TYPE,
};

enum class VulkanDeviceStatus : uint32_t {
    UNINITIALIZED = 0,
    CREATE_SUCCESS,
    CREATE_FAIL,
    MAX_DEVICE_STATUS,
};

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily = UINT32_MAX;
    uint32_t presentFamily = UINT32_MAX;
    bool IsComplete()
    {
        return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
    }
} QueueFamilyIndices;

typedef struct {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
} SwapChainSupportDetails;

class MemoryHandler;
class RsVulkanInterface {
public:
    struct CallbackSemaphoreInfo {
        RsVulkanInterface& mVkContext;
        VkSemaphore mSemaphore;
        int mFenceFd;
        
        int mRSRefs = 1; // 1 : rs hold fence fd
        int m2DEngineRefs = 1; // 1 : skia or ddgr hold fence fd
        CallbackSemaphoreInfo(RsVulkanInterface& vkContext, VkSemaphore semaphore, int fenceFd)
            : mVkContext(vkContext),
            mSemaphore(semaphore),
            mFenceFd(fenceFd)
        {
        }
    };
    template <class T>
    class Func {
    public:
        using Proto = T;
        explicit Func(T proc = nullptr) : func_(proc) {}
        ~Func() { func_ = nullptr; }

        Func operator=(T proc)
        {
            func_ = proc;
            return *this;
        }

        Func operator=(PFN_vkVoidFunction proc)
        {
            func_ = reinterpret_cast<Proto>(proc);
            return *this;
        }

        operator bool() const { return func_ != nullptr; }
        operator T() const { return func_; }
    private:
        T func_;
    };

    RsVulkanInterface() {};
    ~RsVulkanInterface();
    void Init(VulkanInterfaceType vulkanInterfaceType, bool isProtected = false, bool isHtsEnable = false);
    bool CreateInstance();
    bool SelectPhysicalDevice(bool isProtected = false);
    bool CreateDevice(bool isProtected = false, bool isHtsEnable = false);
    bool CreateAndroidSurface(ANativeWindow* window);

    QueueFamilyIndices FindQueueFamilies();

    SwapChainSupportDetails QuerySwapChainSupport();

    void DestroySurfaceKHR(VkSurfaceKHR surface)
    {
        vkDestroySurfaceKHR(instance_, surface, nullptr);
    }
#ifdef USE_M133_SKIA
    bool CreateSkiaBackendContext(skgpu::VulkanBackendContext* context, bool isProtected = false);
#else
    bool CreateSkiaBackendContext(GrVkBackendContext* context, bool isProtected = false);
#endif

    bool IsValid() const;
#ifdef USE_M133_SKIA
    skgpu::VulkanGetProc CreateSkiaGetProc() const;
#else
    GrVkGetProc CreateSkiaGetProc() const;
#endif

#define DEFINE_FUNC(name) Func<PFN_vk##name> vk##name

    DEFINE_FUNC(AcquireNextImageKHR);
    DEFINE_FUNC(AllocateCommandBuffers);
    DEFINE_FUNC(AllocateMemory);
    DEFINE_FUNC(BeginCommandBuffer);
    DEFINE_FUNC(BindImageMemory);
    DEFINE_FUNC(BindImageMemory2);
    DEFINE_FUNC(CmdPipelineBarrier);
    DEFINE_FUNC(CreateCommandPool);
    DEFINE_FUNC(CreateDebugReportCallbackEXT);
    DEFINE_FUNC(CreateDevice);
    DEFINE_FUNC(CreateFence);
    DEFINE_FUNC(CreateImage);
    DEFINE_FUNC(CreateImageView);
    DEFINE_FUNC(CreateInstance);
    DEFINE_FUNC(CreateSemaphore);
    DEFINE_FUNC(CreateSwapchainKHR);
    DEFINE_FUNC(DestroyCommandPool);
    DEFINE_FUNC(DestroyDebugReportCallbackEXT);
    DEFINE_FUNC(DestroyDevice);
    DEFINE_FUNC(DestroyFence);
    DEFINE_FUNC(DestroyImage);
    DEFINE_FUNC(DestroyImageView);
    DEFINE_FUNC(DestroyInstance);
    DEFINE_FUNC(DestroySemaphore);
    DEFINE_FUNC(DestroySurfaceKHR);
    DEFINE_FUNC(DestroySwapchainKHR);
    DEFINE_FUNC(DeviceWaitIdle);
    DEFINE_FUNC(EndCommandBuffer);
    DEFINE_FUNC(EnumerateDeviceExtensionProperties);
    DEFINE_FUNC(EnumerateDeviceLayerProperties);
    DEFINE_FUNC(EnumerateInstanceExtensionProperties);
    DEFINE_FUNC(EnumerateInstanceLayerProperties);
    DEFINE_FUNC(EnumeratePhysicalDevices);
    DEFINE_FUNC(FreeCommandBuffers);
    DEFINE_FUNC(FreeMemory);
    DEFINE_FUNC(GetDeviceProcAddr);
    DEFINE_FUNC(GetDeviceQueue);
    DEFINE_FUNC(GetImageMemoryRequirements);
    DEFINE_FUNC(GetInstanceProcAddr);
    DEFINE_FUNC(GetPhysicalDeviceFeatures);
    DEFINE_FUNC(GetPhysicalDeviceQueueFamilyProperties);
    DEFINE_FUNC(QueueSubmit);
    DEFINE_FUNC(QueueWaitIdle);
    DEFINE_FUNC(ResetCommandBuffer);
    DEFINE_FUNC(ResetFences);
    DEFINE_FUNC(WaitForFences);
    DEFINE_FUNC(GetPhysicalDeviceSurfaceCapabilitiesKHR);
    DEFINE_FUNC(GetPhysicalDeviceSurfaceFormatsKHR);
    DEFINE_FUNC(GetPhysicalDeviceSurfacePresentModesKHR);
    DEFINE_FUNC(GetPhysicalDeviceSurfaceSupportKHR);
    DEFINE_FUNC(GetSwapchainImagesKHR);
    DEFINE_FUNC(QueuePresentKHR);
    DEFINE_FUNC(CreateAndroidSurfaceKHR);
    DEFINE_FUNC(GetPhysicalDeviceMemoryProperties);
    DEFINE_FUNC(GetPhysicalDeviceMemoryProperties2);
    DEFINE_FUNC(GetAndroidHardwareBufferPropertiesANDROID);
    DEFINE_FUNC(ImportSemaphoreFdKHR);
    DEFINE_FUNC(GetPhysicalDeviceFeatures2);
    DEFINE_FUNC(GetSemaphoreFdKHR);
#undef DEFINE_FUNC

    VkPhysicalDevice GetPhysicalDevice() const
    {
        return physicalDevice_;
    }

    VkDevice GetDevice() const
    {
        return device_;
    }

    VkQueue GetQueue() const
    {
        return backendContext_.fQueue;
    }

#ifdef USE_M133_SKIA
    inline const skgpu::VulkanBackendContext& GetGrVkBackendContext() const noexcept
#else
    inline const GrVkBackendContext& GetGrVkBackendContext() const noexcept
#endif
    {
        return backendContext_;
    }

    std::shared_ptr<Drawing::GPUContext> CreateDrawingContext(std::string cacheDir = "");
    std::shared_ptr<Drawing::GPUContext> DoCreateDrawingContext(std::string cacheDir = "");
    std::shared_ptr<Drawing::GPUContext> GetDrawingContext();

    VulkanInterfaceType GetInterfaceType() const
    {
        return interfaceType_;
    }

    VkSurfaceKHR GetSurface() const
    {
        return surface_;
    }

    VkQueue GetGraphicsQueue() const
    {
        return graphicsQueue_;
    }

    VkQueue GetPresentQueue() const
    {
        return presentQueue_;
    }

    VkSemaphore RequireSemaphore();
    void SendSemaphoreWithFd(VkSemaphore semaphore, int fenceFd);
    void DestroyAllSemaphoreFence();
    VulkanDeviceStatus GetVulkanDeviceStatus();
    static std::atomic<uint64_t> callbackSemaphoreInfofdDupCnt_;
    static std::atomic<uint64_t> callbackSemaphoreInfoRSDerefCnt_;
    static std::atomic<uint64_t> callbackSemaphoreInfo2DEngineDerefCnt_;
    static std::atomic<uint64_t> callbackSemaphoreInfo2DEngineDefensiveDerefCnt_;
    static std::atomic<uint64_t> callbackSemaphoreInfoFlushCnt_;
    static std::atomic<uint64_t> callbackSemaphoreInfo2DEngineCallCnt_;

friend class RsVulkanContext;
private:
    std::mutex vkMutex_;
    std::mutex graphicsQueueMutex_;
    std::mutex hGraphicsQueueMutex_;
    static void* handle_;
    bool acquiredMandatoryProcAddresses_ = false;
    static VkInstance instance_;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex_ = UINT32_MAX;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue queue_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_;
    VkQueue presentQueue_;
    VkPhysicalDeviceFeatures2 physicalDeviceFeatures2_;
    VkPhysicalDeviceProtectedMemoryFeatures* protectedMemoryFeatures_ = nullptr;
    VkPhysicalDeviceSamplerYcbcrConversionFeatures ycbcrFeature_;
    VkPhysicalDeviceSynchronization2Features sync2Feature_;
    VkPhysicalDeviceDescriptorIndexingFeatures bindlessFeature_;
    VkPhysicalDeviceTimelineSemaphoreFeatures timelineFeature_;
    std::vector<const char*> deviceExtensions_;
#ifdef USE_M133_SKIA
    skgpu::VulkanExtensions skVkExtensions_;
    skgpu::VulkanBackendContext backendContext_;
#else
    GrVkExtensions skVkExtensions_;
    GrVkBackendContext backendContext_;
#endif
    VulkanInterfaceType interfaceType_ = VulkanInterfaceType::BASIC_RENDER;
    RsVulkanInterface(const RsVulkanInterface &) = delete;
    RsVulkanInterface &operator=(const RsVulkanInterface &) = delete;

    RsVulkanInterface(RsVulkanInterface &&) = delete;
    RsVulkanInterface &operator=(RsVulkanInterface &&) = delete;

    void SetVulkanDeviceStatus(VulkanDeviceStatus status);
    bool OpenLibraryHandle();
    bool SetupLoaderProcAddresses();
    bool CloseLibraryHandle();
    bool SetupDeviceProcAddresses(VkDevice device);
    void ConfigureFeatures(bool isProtected);
    void ConfigureExtensions();
#ifndef USE_M133_SKIA
    uint32_t GetGrVkFeatureFlags();
#endif
#ifdef USE_M133_SKIA
    void SetupSkiaBackendContextBasicFields(skgpu::VulkanBackendContext* context, bool isProtected);
#else
    void SetupSkiaBackendContextBasicFields(GrVkBackendContext* context, bool isProtected);
#endif
    void CleanupUsedSemaphoreFences();
    void LogSemaphoreFenceStatistics();
    PFN_vkVoidFunction AcquireProc(
        const char* proc_name,
        const VkInstance& instance) const;
    PFN_vkVoidFunction AcquireProc(const char* proc_name, const VkDevice& device) const;
    std::shared_ptr<Drawing::GPUContext> CreateNewDrawingContext(bool isProtected = false);

    std::atomic<VulkanDeviceStatus> deviceStatus_ = VulkanDeviceStatus::UNINITIALIZED;
};

class RsVulkanContext {
public:
    class DrawContextHolder {
    public:
        explicit DrawContextHolder(std::function<void()> callback) : destructCallback_(std::move(callback)) {}

        ~DrawContextHolder()
        {
            destructCallback_();
        }
    private:
        std::function<void()> destructCallback_;
    };
    static RsVulkanContext& GetSingleton(const std::string& cacheDir = "");
    static void ReleaseRecyclableSingleton();
    explicit RsVulkanContext(std::string cacheDir = "");
    void InitVulkanContextForHybridRender(const std::string& cacheDir);
    void InitVulkanContextForUniRender(const std::string& cacheDir);
    ~RsVulkanContext();

    RsVulkanContext(const RsVulkanContext&) = delete;
    RsVulkanContext &operator=(const RsVulkanContext&) = delete;

    RsVulkanContext(const RsVulkanContext&&) = delete;
    RsVulkanContext &operator=(const RsVulkanContext&&) = delete;

    void SetIsProtected(bool isProtected);

    RsVulkanInterface& GetRsVulkanInterface();

    bool IsValid()
    {
        return GetRsVulkanInterface().IsValid();
    }

#ifdef USE_M133_SKIA
    skgpu::VulkanGetProc CreateSkiaGetProc()
#else
    GrVkGetProc CreateSkiaGetProc()
#endif
    {
        return GetRsVulkanInterface().CreateSkiaGetProc();
    }

    VkPhysicalDevice GetPhysicalDevice()
    {
        return GetRsVulkanInterface().GetPhysicalDevice();
    }

    VkDevice GetDevice()
    {
        return GetRsVulkanInterface().GetDevice();
    }

    VkQueue GetQueue()
    {
        return GetRsVulkanInterface().GetQueue();
    }

    bool CreateAndroidSurface(ANativeWindow* window)
    {
        return GetRsVulkanInterface().CreateAndroidSurface(window);
    }

    QueueFamilyIndices FindQueueFamilies()
    {
        return GetRsVulkanInterface().FindQueueFamilies();
    }

    void DestroySurfaceKHR(VkSurfaceKHR surface)
    {
        return GetRsVulkanInterface().DestroySurfaceKHR(surface);
    }

    VkSurfaceKHR GetSurface()
    {
        return GetRsVulkanInterface().GetSurface();
    }

    VkQueue GetGraphicsQueue()
    {
        return GetRsVulkanInterface().GetGraphicsQueue();
    }

    VkQueue GetPresentQueue()
    {
        return GetRsVulkanInterface().GetPresentQueue();
    }

    SwapChainSupportDetails QuerySwapChainSupport()
    {
        return GetRsVulkanInterface().QuerySwapChainSupport();
    }

#ifdef USE_M133_SKIA
    inline const skgpu::VulkanBackendContext& GetGrVkBackendContext() noexcept
#else
    inline const GrVkBackendContext& GetGrVkBackendContext() noexcept
#endif
    {
        return GetRsVulkanInterface().GetGrVkBackendContext();
    }

    VulkanDeviceStatus GetVulkanDeviceStatus()
    {
        return GetRsVulkanInterface().GetVulkanDeviceStatus();
    }

    std::shared_ptr<Drawing::GPUContext> CreateDrawingContext();
    std::shared_ptr<Drawing::GPUContext> GetDrawingContext(const std::string& cacheDir = "");
    std::shared_ptr<Drawing::GPUContext> GetRecyclableDrawingContext(const std::string& cacheDir = "");
    static void ReleaseDrawingContextMap();
    static void ReleaseRecyclableDrawingContext();
    static void ReleaseDrawingContextForThread(int tid);

    void ClearGrContext(bool isProtected = false);

    static VKAPI_ATTR VkResult HookedVkQueueSubmit(VkQueue queue, uint32_t submitCount,
        VkSubmitInfo* pSubmits, VkFence fence);

    bool GetIsProtected() const;

    static bool IsRecyclable();

    static void SetRecyclable(bool isRecyclable);

    static void SaveNewDrawingContext(int tid, std::shared_ptr<Drawing::GPUContext> drawingContext);

    static bool GetIsInited();

    static bool IsRecyclableSingletonValid();

private:
    static RsVulkanContext& GetRecyclableSingleton(const std::string& cacheDir = "");
    static std::unique_ptr<RsVulkanContext>& GetRecyclableSingletonPtr(const std::string& cacheDir = "");
    static bool CheckDrawingContextRecyclable();
    static thread_local bool isProtected_;
    static thread_local VulkanInterfaceType vulkanInterfaceType_;
    std::vector<std::shared_ptr<RsVulkanInterface>> vulkanInterfaceVec_;
    // drawingContextMap_ : <tid, <drawingContext, isRecyclable>>
    static std::map<int, std::pair<std::shared_ptr<Drawing::GPUContext>, bool>> drawingContextMap_;
    static std::map<int, std::pair<std::shared_ptr<Drawing::GPUContext>, bool>> protectedDrawingContextMap_;
    static std::mutex drawingContextMutex_;
    // use for recyclable singleton
    static std::recursive_mutex recyclableSingletonMutex_;
    static bool isRecyclable_;
    // isRecyclableSingletonValid_ : true -> has been initialized and is valid , false -> has been released
    static std::atomic<bool> isRecyclableSingletonValid_;
    // use to mark current process has created vulkan context at least once
    static std::atomic<bool> isInited_;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_VULKAN_CONTEXT_H