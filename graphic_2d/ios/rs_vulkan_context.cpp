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

#include "rs_vulkan_context.h"

#include <algorithm>
#include <dlfcn.h>
#include <cmath>
#include <set>
#include <unordered_set>
#include <map>
#include <thread>
#include <vector>

namespace OHOS {
namespace Rosen {

#define ACQUIRE_PROC(name, context)                       \
    if (!(vk##name = AcquireProc("vk" #name, context))) { \
        ROSEN_LOGW("Could not acquire proc: vk" #name);   \
    }

static std::vector<const char*> gInstanceExtensions = {
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_MVK_IOS_SURFACE_EXTENSION_NAME,
#ifdef VK_EXT_METAL_SURFACE_EXTENSION_NAME
    VK_EXT_METAL_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
    VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME,
#endif
#ifdef VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
#endif
};

static std::vector<const char*> gMandatoryDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static std::vector<const char*> gOptionalDeviceExtensions = {
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME,
#endif
};

static std::vector<const char*> ResolveEnabledInstanceExtensions(
    PFN_vkEnumerateInstanceExtensionProperties enumerateInstanceExts)
{
    if (enumerateInstanceExts == nullptr) {
        ROSEN_LOGW("vkEnumerateInstanceExtensionProperties is null, use configured instance extensions directly.");
        return gInstanceExtensions;
    }
    uint32_t extCount = 0;
    if (enumerateInstanceExts(nullptr, &extCount, nullptr) != VK_SUCCESS || extCount == 0) {
        ROSEN_LOGW("Failed to enumerate instance extensions, use configured instance extensions directly.");
        return gInstanceExtensions;
    }
    std::vector<VkExtensionProperties> props(extCount);
    if (enumerateInstanceExts(nullptr, &extCount, props.data()) != VK_SUCCESS) {
        ROSEN_LOGW("Failed to query instance extensions detail, use configured instance extensions directly.");
        return gInstanceExtensions;
    }
    std::unordered_set<std::string_view> supportedExts;
    supportedExts.reserve(extCount);
    for (const auto& ext : props) {
        supportedExts.emplace(ext.extensionName);
    }
    std::vector<const char*> enabledExts;
    enabledExts.reserve(gInstanceExtensions.size());
    for (const char* ext : gInstanceExtensions) {
        if (supportedExts.find(ext) != supportedExts.end()) {
            enabledExts.push_back(ext);
        } else {
            ROSEN_LOGW("Instance extension unavailable and skipped: %{public}s", ext);
        }
    }
    return enabledExts;
}

namespace {
inline int GetDrawingContextThreadKey()
{
    static thread_local const int key = static_cast<int>(
        std::hash<std::thread::id>{}(std::this_thread::get_id()));
    return key;
}
} // namespace

void* RsVulkanInterface::handle_ = nullptr;
VkInstance RsVulkanInterface::instance_ = VK_NULL_HANDLE;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfofdDupCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfoRSDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineDefensiveDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfoFlushCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineCallCnt_ = 0;

RsVulkanInterface::~RsVulkanInterface()
{
    CloseLibraryHandle();
}

RsVulkanInterface::RsVulkanInterface(bool isProtected, bool isHtsEnable, RenderEngineType type)
{
    type_ = type;
    deviceIsProtected_ = isProtected;
    deviceIsHtsEnable_ = isHtsEnable;
    acquiredMandatoryProcAddresses_ = OpenLibraryHandle() && SetupLoaderProcAddresses();
    if (!acquiredMandatoryProcAddresses_) {
        SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_FAIL);
        return;
    }
    if (!CreateInstance() || !SelectPhysicalDevice(isProtected) || !CreateDevice(isProtected, isHtsEnable)) {
        SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_FAIL);
        return;
    }
    std::unique_lock<std::mutex> lock(vkMutex_);
    if (!CreateSkiaBackendContext(&backendContext_, isProtected)) {
        SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_FAIL);
        return;
    }
    SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_SUCCESS);
}

void RsVulkanInterface::AcquireProcAddresses()
{
    ACQUIRE_PROC(CreateDevice, instance_);
    ACQUIRE_PROC(DestroyDevice, instance_);
    ACQUIRE_PROC(DestroyInstance, instance_);
    ACQUIRE_PROC(EnumerateDeviceExtensionProperties, instance_);
    ACQUIRE_PROC(EnumerateDeviceLayerProperties, instance_);
    ACQUIRE_PROC(EnumeratePhysicalDevices, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceFeatures, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceQueueFamilyProperties, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceSurfaceSupportKHR, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceMemoryProperties, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceMemoryProperties2, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceFeatures2, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceSurfaceCapabilitiesKHR, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceSurfaceFormatsKHR, instance_);
    ACQUIRE_PROC(GetPhysicalDeviceSurfacePresentModesKHR, instance_);
    ACQUIRE_PROC(QueuePresentKHR, instance_);
    ACQUIRE_PROC(DestroySurfaceKHR, instance_);
}

bool RsVulkanInterface::CreateInstance()
{
    if (!acquiredMandatoryProcAddresses_) {
        return false;
    }
    if (instance_ != VK_NULL_HANDLE) {
        return true;
    }
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "OHOS";
    appInfo.pEngineName = "Rosen";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    auto enabledInstanceExtensions = ResolveEnabledInstanceExtensions(vkEnumerateInstanceExtensionProperties);
    auto hasRequiredExt = [&enabledInstanceExtensions](const char* extName) {
        return std::find(enabledInstanceExtensions.begin(), enabledInstanceExtensions.end(), extName) !=
            enabledInstanceExtensions.end();
    };
    if (!hasRequiredExt(VK_KHR_SURFACE_EXTENSION_NAME) || !hasRequiredExt(VK_MVK_IOS_SURFACE_EXTENSION_NAME)) {
        ROSEN_LOGE("Required Vulkan instance extensions are missing.");
        return false;
    }
    createInfo.enabledExtensionCount = static_cast<uint32_t>(enabledInstanceExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledInstanceExtensions.data();
#ifdef VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR
    if (hasRequiredExt(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {
        createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        ROSEN_LOGE("CreateInstance failed, result=%{public}d.", static_cast<int32_t>(result));
        return false;
    }
    AcquireProcAddresses();
    return true;
}

bool RsVulkanInterface::SelectPhysicalDevice(bool isProtected)
{
    (void)isProtected;
    if (instance_ == VK_NULL_HANDLE) {
        return false;
    }
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr) != VK_SUCCESS || deviceCount == 0) {
        ROSEN_LOGE("No Vulkan physical device found.");
        return false;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    if (vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data()) != VK_SUCCESS) {
        ROSEN_LOGE("Enumerate physical devices failed.");
        return false;
    }
    physicalDevice_ = devices[0];
    return true;
}

bool RsVulkanInterface::CreateDevice(bool isProtected, bool isHtsEnable)
{
    if (physicalDevice_ == VK_NULL_HANDLE) {
        return false;
    }
    QueueFamilyIndices indices = FindQueueFamilies();
    if (indices.graphicsFamily == UINT32_MAX) {
        ROSEN_LOGE("No graphics queue family found.");
        return false;
    }
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily };
    if (pendingPresentQueueFamilyIndex_ != UINT32_MAX) {
        uniqueQueueFamilies.insert(pendingPresentQueueFamilyIndex_);
    }
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float priority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.queueFamilyIndex = queueFamily;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &priority;
        queueCreateInfos.push_back(queueInfo);
    }
    ConfigureExtensions();
    ConfigureFeatures(isProtected);
    vkGetPhysicalDeviceFeatures2(physicalDevice_, &physicalDeviceFeatures2_);
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = &physicalDeviceFeatures2_;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions_.data();
    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        ROSEN_LOGE("CreateDevice failed.");
        return false;
    }
    if (!SetupDeviceProcAddresses(device_)) {
        return false;
    }
    graphicsQueueFamilyIndex_ = indices.graphicsFamily;
    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    presentQueue_ = graphicsQueue_;
    presentQueueFamilyIndex_ = indices.graphicsFamily;
    if (pendingPresentQueueFamilyIndex_ != UINT32_MAX) {
        presentQueueFamilyIndex_ = pendingPresentQueueFamilyIndex_;
        vkGetDeviceQueue(device_, presentQueueFamilyIndex_, 0, &presentQueue_);
    }
    return graphicsQueue_ != VK_NULL_HANDLE;
}

bool RsVulkanInterface::CreateMetalSurface(void* metalLayer, VkSurfaceKHR& outSurface)
{
    if (instance_ == VK_NULL_HANDLE || metalLayer == nullptr) {
        return false;
    }
    auto createIOSSurface = reinterpret_cast<PFN_vkCreateIOSSurfaceMVK>(
        AcquireProc("vkCreateIOSSurfaceMVK", instance_));
    if (createIOSSurface == nullptr) {
        ROSEN_LOGE("vkCreateIOSSurfaceMVK is unavailable.");
        return false;
    }
    VkIOSSurfaceCreateInfoMVK createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IOS_SURFACE_CREATE_INFO_MVK;
    createInfo.pView = metalLayer;
    if (createIOSSurface(instance_, &createInfo, nullptr, &outSurface) != VK_SUCCESS) {
        ROSEN_LOGE("CreateIOSSurfaceMVK failed.");
        return false;
    }
    QueueFamilyIndices indices = FindQueueFamilies(outSurface);
    if (indices.presentFamily == UINT32_MAX) {
        ROSEN_LOGE("CreateMetalSurface no present-capable queue family found");
        vkDestroySurfaceKHR(instance_, outSurface, nullptr);
        outSurface = VK_NULL_HANDLE;
        return false;
    }
    if (indices.presentFamily != indices.graphicsFamily) {
        if (!RecreateDeviceForPresentFamily(indices.presentFamily)) {
            ROSEN_LOGE("CreateMetalSurface failed to recreate device for present family %{public}u",
                indices.presentFamily);
            vkDestroySurfaceKHR(instance_, outSurface, nullptr);
            outSurface = VK_NULL_HANDLE;
            return false;
        }
    }
    return true;
}

QueueFamilyIndices RsVulkanInterface::FindQueueFamilies(VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;
    if (physicalDevice_ == VK_NULL_HANDLE) {
        return indices;
    }

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

    if (graphicsQueueFamilyIndex_ == UINT32_MAX) {
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphicsQueueFamilyIndex_ = i;
                break;
            }
        }
    }
    indices.graphicsFamily = graphicsQueueFamilyIndex_;

    if (surface == VK_NULL_HANDLE) {
        return indices;
    }

    if (indices.graphicsFamily != UINT32_MAX) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice_, indices.graphicsFamily, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = indices.graphicsFamily;
            return indices;
        }
    }

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = i;
            ROSEN_LOGW("FindQueueFamilies graphicsFamily=%{public}u lacks present support on surface=%{public}p,"
                " fallback presentFamily=%{public}u requires device recreate",
                indices.graphicsFamily, reinterpret_cast<void*>(surface), i);
            return indices;
        }
    }
    ROSEN_LOGE("FindQueueFamilies: graphicsFamily=%{public}u does not support present on"
               " surface=%{public}p and no fallback present family found",
        indices.graphicsFamily, reinterpret_cast<void*>(surface));
    return indices;
}

bool RsVulkanInterface::RecreateDeviceForPresentFamily(uint32_t presentFamily)
{
    if (presentFamily == UINT32_MAX) {
        return true;
    }
    if (presentFamily == graphicsQueueFamilyIndex_) {
        return true;
    }
    pendingPresentQueueFamilyIndex_ = presentFamily;

    if (device_ != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(device_);
        vkDestroyDevice(device_, nullptr);
        device_ = VK_NULL_HANDLE;
    }
    graphicsQueue_ = VK_NULL_HANDLE;
    presentQueue_ = VK_NULL_HANDLE;
    if (!CreateDevice(deviceIsProtected_, deviceIsHtsEnable_)) {
        pendingPresentQueueFamilyIndex_ = UINT32_MAX;
        return false;
    }
    std::unique_lock<std::mutex> lock(vkMutex_);
    if (!CreateSkiaBackendContext(&backendContext_, deviceIsProtected_)) {
        pendingPresentQueueFamilyIndex_ = UINT32_MAX;
        return false;
    }
    pendingPresentQueueFamilyIndex_ = UINT32_MAX;
    return true;
}

SwapChainSupportDetails RsVulkanInterface::QuerySwapChainSupport(VkSurfaceKHR surface)
{
    SwapChainSupportDetails details = {};
    if (surface == VK_NULL_HANDLE || physicalDevice_ == VK_NULL_HANDLE) {
        return details;
    }
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &formatCount, nullptr);
    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice_, surface, &presentModeCount, details.presentModes.data());
    }
    return details;
}

#ifdef USE_M133_SKIA
bool RsVulkanInterface::CreateSkiaBackendContext(skgpu::VulkanBackendContext* context, bool isProtected)
{
    auto getProc = CreateSkiaGetProc();
    if (getProc == nullptr || context == nullptr || !IsValid()) {
        return false;
    }
    if (graphicsQueue_ == VK_NULL_HANDLE) {
        ROSEN_LOGE("CreateSkiaBackendContext graphicsQueue_ is VK_NULL_HANDLE ");
        return false;
    }
    SetupSkiaBackendContextBasicFields(context, isProtected);
    skVkExtensions_.init(getProc, instance_, physicalDevice_,
        gInstanceExtensions.size(), gInstanceExtensions.data(),
        deviceExtensions_.size(), deviceExtensions_.data());
    context->fVkExtensions = &skVkExtensions_;
    context->fDeviceFeatures2 = &physicalDeviceFeatures2_;
    context->fGetProc = std::move(getProc);
    context->fProtectedContext = isProtected ? skgpu::Protected::kYes : skgpu::Protected::kNo;
    return true;
}

skgpu::VulkanGetProc RsVulkanInterface::CreateSkiaGetProc() const
{
    if (!IsValid()) {
        return nullptr;
    }
    return [this](const char* procName, VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            auto proc = AcquireProc(procName, device);
            if (proc != nullptr) {
                return proc;
            }
        }
        return AcquireProc(procName, instance);
    };
}
#else
bool RsVulkanInterface::CreateSkiaBackendContext(GrVkBackendContext* context, bool isProtected)
{
    auto getProc = CreateSkiaGetProc();
    if (getProc == nullptr || context == nullptr || !IsValid()) {
        return false;
    }
    if (graphicsQueue_ == VK_NULL_HANDLE) {
        ROSEN_LOGE("CreateSkiaBackendContext graphicsQueue_ is VK_NULL_HANDLE ");
        return false;
    }
    SetupSkiaBackendContextBasicFields(context, isProtected);
    skVkExtensions_.init(getProc, instance_, physicalDevice_,
        gInstanceExtensions.size(), gInstanceExtensions.data(),
        deviceExtensions_.size(), deviceExtensions_.data());
    context->fVkExtensions = &skVkExtensions_;
    context->fDeviceFeatures2 = &physicalDeviceFeatures2_;
    context->fFeatures = GetGrVkFeatureFlags();
    context->fGetProc = std::move(getProc);
    context->fOwnsInstanceAndDevice = false;
    context->fProtectedContext = isProtected ? GrProtected::kYes : GrProtected::kNo;
    return true;
}

GrVkGetProc RsVulkanInterface::CreateSkiaGetProc() const
{
    if (!IsValid()) {
        return nullptr;
    }
    return [this](const char* procName, VkInstance instance, VkDevice device) {
        if (device != VK_NULL_HANDLE) {
            auto proc = AcquireProc(procName, device);
            if (proc != nullptr) {
                return proc;
            }
        }
        return AcquireProc(procName, instance);
    };
}
#endif

bool RsVulkanInterface::IsValid() const
{
    return instance_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE;
}

std::shared_ptr<Drawing::GPUContext> RsVulkanInterface::GetDrawingContext()
{
    return CreateDrawingContext();
}

VkSemaphore RsVulkanInterface::RequireSemaphore()
{
    return VK_NULL_HANDLE;
}

void RsVulkanInterface::SendSemaphoreWithFd(VkSemaphore semaphore, int fenceFd)
{
    (void)semaphore;
    (void)fenceFd;
}

void RsVulkanInterface::DestroyAllSemaphoreFence()
{
}

VulkanDeviceStatus RsVulkanInterface::GetVulkanDeviceStatus()
{
    return deviceStatus_.load();
}

RsVulkanContext& RsVulkanContext::Get(RenderEngineType type)
{
    static RsVulkanContext basicRenderSingleton = RsVulkanContext(false, true, RenderEngineType::BASIC_RENDER);
    return basicRenderSingleton;
}

RsVulkanContext::RsVulkanContext(bool isProtected, bool isHtsEnable, RenderEngineType type)
{
    type_ = type;
    vulkanInterface_ = std::make_shared<RsVulkanInterface>(isProtected, isHtsEnable, type_);

    if (IsValid()) {
        ROSEN_LOGI("RsVulkanContext::Init success for type %{public}u", static_cast<uint32_t>(type_));
    } else {
        ROSEN_LOGE("RsVulkanContext::Init failed for type %{public}u", static_cast<uint32_t>(type_));
    }
}

RsVulkanContext::~RsVulkanContext() = default;

VKAPI_ATTR VkResult RsVulkanContext::HookedVkQueueSubmit(VkQueue queue, uint32_t submitCount,
    VkSubmitInfo* pSubmits, VkFence fence)
{
    (void)queue;
    (void)submitCount;
    (void)pSubmits;
    (void)fence;
    return VK_SUCCESS;
}

void RsVulkanInterface::SetVulkanDeviceStatus(VulkanDeviceStatus status)
{
    deviceStatus_.store(status);
}

bool RsVulkanInterface::OpenLibraryHandle()
{
    if (handle_ != nullptr) {
        return true;
    }
    handle_ = dlopen("libMoltenVK.dylib", RTLD_NOW | RTLD_LOCAL);
    if (handle_ == nullptr) {
        // GN links libMoltenVK.a statically; there is no dylib in the app bundle.
        // Resolve Vulkan entry points from the main image via RTLD_DEFAULT.
        if (dlsym(RTLD_DEFAULT, "vkGetInstanceProcAddr") != nullptr) {
            handle_ = RTLD_DEFAULT;
            return true;
        }
        ROSEN_LOGE("Could not open MoltenVK dylib: %{public}s", dlerror());
        return false;
    }
    return true;
}

bool RsVulkanInterface::SetupLoaderProcAddresses()
{
    if (handle_ == nullptr) {
        return false;
    }
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(handle_, "vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(handle_, "vkGetDeviceProcAddr"));
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
        dlsym(handle_, "vkEnumerateInstanceExtensionProperties"));
    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(handle_, "vkCreateInstance"));
    if (!vkGetInstanceProcAddr || !vkCreateInstance) {
        ROSEN_LOGE("SetupLoaderProcAddresses failed.");
        return false;
    }
    VkInstance nullInstance = VK_NULL_HANDLE;
    ACQUIRE_PROC(EnumerateInstanceLayerProperties, nullInstance);
    return true;
}

bool RsVulkanInterface::CloseLibraryHandle()
{
    if (handle_ != nullptr && handle_ != RTLD_DEFAULT) {
        dlclose(handle_);
    }
    handle_ = nullptr;
    return true;
}

bool RsVulkanInterface::SetupDeviceProcAddresses(VkDevice device)
{
    (void)device;
    ACQUIRE_PROC(AllocateCommandBuffers, device_);
    ACQUIRE_PROC(AllocateMemory, device_);
    ACQUIRE_PROC(BeginCommandBuffer, device_);
    ACQUIRE_PROC(BindImageMemory, device_);
    ACQUIRE_PROC(BindImageMemory2, device_);
    ACQUIRE_PROC(CmdPipelineBarrier, device_);
    ACQUIRE_PROC(CreateCommandPool, device_);
    ACQUIRE_PROC(CreateBuffer, device_);
    ACQUIRE_PROC(CreateFence, device_);
    ACQUIRE_PROC(CreateImage, device_);
    ACQUIRE_PROC(CreateImageView, device_);
    ACQUIRE_PROC(CreateSemaphore, device_);
    ACQUIRE_PROC(DestroyCommandPool, device_);
    ACQUIRE_PROC(DestroyBuffer, device_);
    ACQUIRE_PROC(DestroyFence, device_);
    ACQUIRE_PROC(DestroyImage, device_);
    ACQUIRE_PROC(DestroyImageView, device_);
    ACQUIRE_PROC(DestroySemaphore, device_);
    ACQUIRE_PROC(DeviceWaitIdle, device_);
    ACQUIRE_PROC(EndCommandBuffer, device_);
    ACQUIRE_PROC(FreeCommandBuffers, device_);
    ACQUIRE_PROC(FreeMemory, device_);
    ACQUIRE_PROC(GetDeviceQueue, device_);
    ACQUIRE_PROC(GetPhysicalDeviceProperties, instance_);
    ACQUIRE_PROC(GetBufferMemoryRequirements, device_);
    ACQUIRE_PROC(GetImageMemoryRequirements, device_);
    ACQUIRE_PROC(QueueSubmit, device_);
    ACQUIRE_PROC(QueueWaitIdle, device_);
    ACQUIRE_PROC(ResetCommandBuffer, device_);
    ACQUIRE_PROC(ResetFences, device_);
    ACQUIRE_PROC(WaitForFences, device_);
    ACQUIRE_PROC(GetFenceStatus, device_);
    ACQUIRE_PROC(AcquireNextImageKHR, device_);
    ACQUIRE_PROC(DestroySwapchainKHR, device_);
    ACQUIRE_PROC(GetSwapchainImagesKHR, device_);
    ACQUIRE_PROC(CreateSwapchainKHR, device_);
    ACQUIRE_PROC(BindBufferMemory, device_);
    ACQUIRE_PROC(MapMemory, device_);
    ACQUIRE_PROC(UnmapMemory, device_);
    ACQUIRE_PROC(CmdCopyBufferToImage, device_);
    return true;
}

void RsVulkanInterface::ConfigureFeatures(bool isProtected)
{
    (void)isProtected;
    ycbcrFeature_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES;
    ycbcrFeature_.pNext = nullptr;
    physicalDeviceFeatures2_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2_.pNext = &ycbcrFeature_;
}

void RsVulkanInterface::ConfigureExtensions()
{
    deviceExtensions_ = gMandatoryDeviceExtensions;
    uint32_t count = 0;
    if (vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &count, nullptr) != VK_SUCCESS) {
        return;
    }
    std::vector<VkExtensionProperties> props(count);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &count, props.data()) != VK_SUCCESS) {
        return;
    }
    std::unordered_set<std::string_view> extNames;
    for (const auto& ext : props) {
        extNames.emplace(ext.extensionName);
    }
    for (const auto& ext : gOptionalDeviceExtensions) {
        if (extNames.find(ext) != extNames.end()) {
            deviceExtensions_.push_back(ext);
        }
    }
}

#ifndef USE_M133_SKIA
uint32_t RsVulkanInterface::GetGrVkFeatureFlags()
{
    return 0;
}
#endif

#ifdef USE_M133_SKIA
void RsVulkanInterface::SetupSkiaBackendContextBasicFields(skgpu::VulkanBackendContext* context, bool isProtected)
#else
void RsVulkanInterface::SetupSkiaBackendContextBasicFields(GrVkBackendContext* context, bool isProtected)
#endif
{
    (void)isProtected;
    context->fInstance = instance_;
    context->fPhysicalDevice = physicalDevice_;
    context->fDevice = device_;
    context->fQueue = graphicsQueue_;
    context->fGraphicsQueueIndex = graphicsQueueFamilyIndex_;
#ifdef USE_M133_SKIA
    context->fMaxAPIVersion = VK_API_VERSION_1_2;
#else
    context->fMinAPIVersion = VK_API_VERSION_1_2;
    context->fExtensions = kKHR_surface_GrVkExtensionFlag;
#endif
}

void RsVulkanInterface::CleanupUsedSemaphoreFences()
{
}

void RsVulkanInterface::LogSemaphoreFenceStatistics()
{
}

PFN_vkVoidFunction RsVulkanInterface::AcquireProc(const char* procName, const VkInstance& instance) const
{
    if (procName == nullptr || vkGetInstanceProcAddr == nullptr) {
        return nullptr;
    }
    return vkGetInstanceProcAddr(instance, procName);
}

PFN_vkVoidFunction RsVulkanInterface::AcquireProc(const char* procName, const VkDevice& device) const
{
    if (procName == nullptr || device == VK_NULL_HANDLE || vkGetDeviceProcAddr == nullptr) {
        return nullptr;
    }
    return vkGetDeviceProcAddr(device, procName);
}

std::shared_ptr<Drawing::GPUContext> RsVulkanContext::CreateDrawingGPUContext(const std::string& cacheDir)
{
    (void)cacheDir;
    if (!IsValid()) {
        return nullptr;
    }
    auto drawingContext = std::make_shared<Drawing::GPUContext>();
    Drawing::GPUContextOptions options;
    if (!drawingContext->BuildFromVK(GetGrVkBackendContext(), options)) {
        ROSEN_LOGE("nullptr gpuContext is null");
        return nullptr;
    }
    int maxResources = 0;
    size_t maxResourcesSize = 0;
    drawingContext->GetResourceCacheLimits(&maxResources, &maxResourcesSize);
    if (maxResourcesSize > 0) {
        drawingContext->SetResourceCacheLimits(
            CACHE_LIMITS_TIMES * maxResources,
            CACHE_LIMITS_TIMES * std::fmin(maxResourcesSize, GR_CACHE_MAX_BYTE_SIZE));
    } else {
        drawingContext->SetResourceCacheLimits(GR_CACHE_MAX_COUNT, GR_CACHE_MAX_BYTE_SIZE);
    }
    return drawingContext;
}

void RsVulkanContext::ReleaseDrawingGPUContext(std::shared_ptr<Drawing::GPUContext>& gpuContext)
{
    if (gpuContext) {
        if (IsValid()) {
            gpuContext->FlushAndSubmit(true);
            gpuContext->PurgeUnlockAndSafeCacheGpuResources();
        }
        gpuContext.reset();
    }
}
} // namespace Rosen
} // namespace OHOS

