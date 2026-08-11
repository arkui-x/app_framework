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
#include <memory>
#include <mutex>
#include <set>
#include <unordered_set>
#include <string_view>
#include <dlfcn.h>
#include <vector>
#include "common/rs_optional_trace.h"
#include "platform/common/rs_log.h"
#ifdef USE_M133_SKIA
#include "include/gpu/vk/VulkanExtensions.h"
#else
#include "include/gpu/vk/GrVkExtensions.h"
#endif
#include "unistd.h"
#include "utils/system_properties.h"

#if defined(RS_ENABLE_GL) || defined(RS_ENABLE_VK)
#include "platform/common/rs_system_properties.h"
#endif

#define ACQUIRE_PROC(name, context)                         \
    if (!(vk##name = AcquireProc("vk" #name, context))) {   \
        ROSEN_LOGE("Could not acquire proc: vk" #name);     \
    }

namespace OHOS {
namespace Rosen {
void* RsVulkanInterface::handle_ = nullptr;
VkInstance RsVulkanInterface::instance_ = VK_NULL_HANDLE;

static std::vector<const char*> gInstanceExtensions = {
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
    VK_KHR_SURFACE_EXTENSION_NAME,
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
};

static std::vector<const char*> gMandatoryDeviceExtensions = {
    VK_KHR_EXTERNAL_SEMAPHORE_FD_EXTENSION_NAME,
    VK_KHR_SAMPLER_YCBCR_CONVERSION_EXTENSION_NAME,
    VK_KHR_MAINTENANCE1_EXTENSION_NAME,
    VK_KHR_MAINTENANCE2_EXTENSION_NAME,
    VK_KHR_MAINTENANCE3_EXTENSION_NAME,
    VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
    VK_KHR_BIND_MEMORY_2_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_ANDROID_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER_EXTENSION_NAME
};

static std::vector<const char*> gOptionalDeviceExtensions = {
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME,
    VK_EXT_DEVICE_FAULT_EXTENSION_NAME,
};

// enabled when persist.sys.graphic.openVkImageMemoryDfx is true
static const std::vector<const char*> gOptionalDeviceExtensionsDebug = {
    VK_KHR_MAINTENANCE_5_EXTENSION_NAME,
    VK_EXT_DEVICE_ADDRESS_BINDING_REPORT_EXTENSION_NAME,
};

std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfofdDupCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfoRSDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineDefensiveDerefCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfoFlushCnt_ = 0;
std::atomic<uint64_t> RsVulkanInterface::callbackSemaphoreInfo2DEngineCallCnt_ = 0;

RsVulkanInterface::RsVulkanInterface(bool isProtected, bool isHtsEnable, RenderEngineType type)
{
    isProtected_ = isProtected;
    type_ = type;
    acquiredMandatoryProcAddresses_ = OpenLibraryHandle() && SetupLoaderProcAddresses();
    if (!acquiredMandatoryProcAddresses_) {
        ROSEN_LOGE("RsVulkanInterface:: failed to acquire mandatory Vulkan proc addresses");
        return;
    }
    if (!CreateInstance()) {
        ROSEN_LOGE("RsVulkanInterface:: CreateInstance failed");
        return;
    }
    if (!SelectPhysicalDevice(isProtected)) {
        ROSEN_LOGE("RsVulkanInterface:: SelectPhysicalDevice failed");
        return;
    }
    if (!CreateDevice(isProtected, isHtsEnable)) {
        ROSEN_LOGE("RsVulkanInterface:: CreateDevice failed");
        return;
    }
    if (!CreateSkiaBackendContext(&backendContext_, isProtected)) {
        ROSEN_LOGE("RsVulkanInterface:: CreateSkiaBackendContext failed");
        return;
    }
    ROSEN_LOGI("RsVulkanInterface:: Init success for type %{public}d", static_cast<int>(type_));
}

RsVulkanInterface::~RsVulkanInterface()
{
    CloseLibraryHandle();
}

bool RsVulkanInterface::IsValid() const
{
    return instance_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE;
}

bool RsVulkanInterface::SetupLoaderProcAddresses()
{
    if (handle_ == nullptr) {
        return true;
    }
    vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(handle_, "vkGetInstanceProcAddr"));
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(dlsym(handle_, "vkGetDeviceProcAddr"));
    vkEnumerateInstanceExtensionProperties = reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          dlsym(handle_, "vkEnumerateInstanceExtensionProperties"));
    vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(dlsym(handle_, "vkCreateInstance"));

    if (!vkGetInstanceProcAddr) {
        ROSEN_LOGE("Could not acquire vkGetInstanceProcAddr");
        return false;
    }

    VkInstance null_instance = VK_NULL_HANDLE;
    ACQUIRE_PROC(EnumerateInstanceLayerProperties, null_instance);
    return true;
}

bool RsVulkanInterface::CreateInstance()
{
    if (!acquiredMandatoryProcAddresses_) {
        return false;
    }

    if (instance_ == VK_NULL_HANDLE) {
        const VkApplicationInfo info = {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "OHOS",
            .applicationVersion = 0,
            .pEngineName = "Rosen",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_3,
        };

        const VkInstanceCreateInfo create_info = {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pApplicationInfo = &info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<uint32_t>(gInstanceExtensions.size()),
            .ppEnabledExtensionNames = gInstanceExtensions.data(),
        };
        if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
            ROSEN_LOGE("Could not create vulkan instance");
            return false;
        }
    }

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
    ACQUIRE_PROC(CreateAndroidSurfaceKHR, instance_);
    ACQUIRE_PROC(DestroySurfaceKHR, instance_);
    return true;
}

SwapChainSupportDetails RsVulkanInterface::QuerySwapChainSupport(VkSurfaceKHR surface)
{
    SwapChainSupportDetails details;
    if (surface == VK_NULL_HANDLE || physicalDevice_ == VK_NULL_HANDLE) {
        return details;
    }
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface, &formatCount, details.formats.data());
    }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice_, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

bool RsVulkanInterface::SelectPhysicalDevice(bool isProtected)
{
    if (!instance_) {
        return false;
    }
    uint32_t deviceCount = 0;
    if (vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr) != VK_SUCCESS) {
        ROSEN_LOGE("vkEnumeratePhysicalDevices failed");
        return false;
    }

    std::vector<VkPhysicalDevice> physicalDevices;
    physicalDevices.resize(deviceCount);
    if (vkEnumeratePhysicalDevices(instance_, &deviceCount, physicalDevices.data()) != VK_SUCCESS) {
        ROSEN_LOGE("vkEnumeratePhysicalDevices failed");
        return false;
    }
    physicalDevice_ = physicalDevices[0];
    return true;
}

void RsVulkanInterface::ConfigureFeatures(bool isProtected)
{
    ycbcrFeature_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_YCBCR_CONVERSION_FEATURES,
    ycbcrFeature_.pNext = nullptr;
    sync2Feature_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2Feature_.pNext = &ycbcrFeature_;
    bindlessFeature_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    bindlessFeature_.pNext = &sync2Feature_;
    timelineFeature_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    timelineFeature_.pNext = &bindlessFeature_;
    physicalDeviceFeatures2_.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2_.pNext = &timelineFeature_;
    void** tailPnext = &ycbcrFeature_.pNext;
    if (protectedMemoryFeatures_) {
        delete protectedMemoryFeatures_;
        protectedMemoryFeatures_ = nullptr;
    }
    protectedMemoryFeatures_ = new VkPhysicalDeviceProtectedMemoryFeatures;
    if (isProtected) {
        protectedMemoryFeatures_->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROTECTED_MEMORY_FEATURES;
        protectedMemoryFeatures_->pNext = nullptr;
        *tailPnext = protectedMemoryFeatures_;
        tailPnext = &protectedMemoryFeatures_->pNext;
    }
}

void RsVulkanInterface::ConfigureExtensions()
{
    deviceExtensions_ = gMandatoryDeviceExtensions;
    uint32_t count = 0;
    std::vector<VkExtensionProperties> supportedExtensions;
    if (vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &count, nullptr) != VK_SUCCESS) {
        ROSEN_LOGE("Failed to get device extension count, try to create device with mandatory extensions only!");
        return;
    }
    supportedExtensions.resize(count);
    if (vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &count,
        supportedExtensions.data()) != VK_SUCCESS) {
        ROSEN_LOGE("Failed to get device extensions, try to create device with mandatory extensions only!");
        return;
    }
    std::unordered_set<std::string_view> extensionNames;
    for (auto& prop: supportedExtensions) {
        extensionNames.emplace(prop.extensionName);
    }
    for (auto& ext: gOptionalDeviceExtensions) {
        if (extensionNames.find(ext) != extensionNames.end()) {
            deviceExtensions_.emplace_back(ext);
        }
    }
    for (auto& ext: gMandatoryDeviceExtensions) {
        if (extensionNames.find(ext) == extensionNames.end()) {
            ROSEN_LOGE("Mandatory device extension %{public}s not found! Try to enable it anyway.", ext);
        }
    }
}

void RsVulkanInterface::BuildDeviceQueueCreateInfos(const QueueFamilyIndices& indices, const float& queuePriority,
    std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos)
{
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily};
    if (pendingPresentQueueFamilyIndex_ != UINT32_MAX) {
        uniqueQueueFamilies.insert(pendingPresentQueueFamilyIndex_);
    }

    queueCreateInfos.clear();
    queueCreateInfos.reserve(uniqueQueueFamilies.size());
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
}

VkDeviceCreateInfo RsVulkanInterface::MakeDeviceCreateInfo(
    const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, bool isHtsEnable)
{
    VkDeviceCreateFlags deviceCreateFlags = isHtsEnable ? VK_DEVICE_CREATE_HTS_ENABLE_BIT : 0;
    return {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &physicalDeviceFeatures2_,
        .flags = deviceCreateFlags,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions_.size()),
        .ppEnabledExtensionNames = deviceExtensions_.data(),
        .pEnabledFeatures = nullptr,
    };
}

bool RsVulkanInterface::InvokeVkCreateDevice(const VkDeviceCreateInfo& createInfo)
{
    queueCount_ = createInfo.queueCreateInfoCount;
    if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
        SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_FAIL);
        ROSEN_LOGE("vkCreateDevice failed");
        return false;
    }
    SetVulkanDeviceStatus(VulkanDeviceStatus::CREATE_SUCCESS);
    return true;
}

void RsVulkanInterface::BindGraphicsAndPresentQueues(const QueueFamilyIndices& indices)
{
    // 99% of the time, the graphicsFamily is the same as the presentFamily
    vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
    presentQueue_ = graphicsQueue_;
    presentQueueFamilyIndex_ = indices.graphicsFamily;

    /*
     * if pendingPresentQueueFamilyIndex_ is not UINT32_MAX,
     * it means the present queue family index is being recreated.
     */
    if (pendingPresentQueueFamilyIndex_ != UINT32_MAX) {
        presentQueueFamilyIndex_ = pendingPresentQueueFamilyIndex_;
        vkGetDeviceQueue(device_, presentQueueFamilyIndex_, 0, &presentQueue_);
        if (presentQueue_ == VK_NULL_HANDLE) {
            ROSEN_LOGE("Failed to get present queue for family %{public}u", presentQueueFamilyIndex_);
            presentQueue_ = graphicsQueue_;
            presentQueueFamilyIndex_ = indices.graphicsFamily;
        }
    }
}

bool RsVulkanInterface::CreateDevice(bool isProtected, bool isHtsEnable)
{
    if (!physicalDevice_) {
        ROSEN_LOGE("CreateDevice physicalDevice_ is VK_NULL_HANDLE ");
        return false;
    }

    QueueFamilyIndices indices = FindQueueFamilies();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    const float queuePriority = 1.0f;
    BuildDeviceQueueCreateInfos(indices, queuePriority, queueCreateInfos);

    ConfigureExtensions();
    ConfigureFeatures(isProtected);
    vkGetPhysicalDeviceFeatures2(physicalDevice_, &physicalDeviceFeatures2_);
    const VkDeviceCreateInfo createInfo = MakeDeviceCreateInfo(queueCreateInfos, isHtsEnable);
    if (!InvokeVkCreateDevice(createInfo)) {
        return false;
    }
    if (!SetupDeviceProcAddresses(device_)) {
        ROSEN_LOGE("SetupDeviceProcAddresses failed");
        return false;
    }
    BindGraphicsAndPresentQueues(indices);
    return true;
}

bool RsVulkanInterface::CreateAndroidSurface(ANativeWindow* window, VkSurfaceKHR& outSurface)
{
    if (instance_ == VK_NULL_HANDLE) {
        ROSEN_LOGE("CreateAndroidSurface instance_ == VK_NULL_HANDLE ");
        return false;
    }

    if (!vkCreateAndroidSurfaceKHR) {
        ROSEN_LOGE("CreateAndroidSurface vkCreateAndroidSurfaceKHR not loaded");
        return false;
    }
    VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo{};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.pNext = nullptr;
    surfaceCreateInfo.flags = 0;
    surfaceCreateInfo.window = window;

    if (vkCreateAndroidSurfaceKHR(instance_, &surfaceCreateInfo, nullptr, &outSurface)) {
        ROSEN_LOGE("CreateAndroidSurface RsVulkanInterface Failed to create Android surface");
        return false;
    }
    QueueFamilyIndices indices = FindQueueFamilies(outSurface);
    if (indices.presentFamily == UINT32_MAX) {
        ROSEN_LOGE("CreateAndroidSurface no present-capable queue family found");
        vkDestroySurfaceKHR(instance_, outSurface, nullptr);
        outSurface = VK_NULL_HANDLE;
        return false;
    }
    /*
     * if the queue count is 1, we can only use the graphics queue for present.
     * so we need to recreate the device for the present family.
     */
    if (indices.presentFamily != indices.graphicsFamily && queueCount_ <= 1) {
        if (!RecreateDeviceForPresentFamily(indices.presentFamily)) {
            ROSEN_LOGE("CreateAndroidSurface failed to recreate device for present family %{public}u",
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
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, queueFamilies.data());

    /*
     * Phase 1: pick first GRAPHICS_BIT family. Used during initial CreateDevice when
     * no surface is available yet. Also serves as fallback if no surface is given later.
     */
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

    /*
     * Phase 2: with a surface available, the queue family ALREADY used by the device
     * at CreateDevice time MUST also support presentation, otherwise we cannot safely
     * present from a queue that wasn't requested at device creation. Verify it.
     */
    if (indices.graphicsFamily != UINT32_MAX) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(
            physicalDevice_, indices.graphicsFamily, surface, &presentSupport);
        if (presentSupport) {
            indices.presentFamily = indices.graphicsFamily;
            presentQueue_ = graphicsQueue_;
            return indices;
        }
    }

    /*
     * Phase 3: if the graphicsFamily does not support present,
     * we need to find a presentFamily that supports present.
     */
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

/*
 * if presentFamily != graphicsQueueFamilyIndex_, we need to recreate the device.
 * because create device only create one queue family for graphics and present.
 */
bool RsVulkanInterface::RecreateDeviceForPresentFamily(uint32_t presentFamily)
{
    if (presentFamily == UINT32_MAX) {
        return false;
    }
    /*
     * if the present family is the same as the graphics family, or the present family is already being recreated,
     * we don't need to recreate the device.
     */
    if (presentFamily == graphicsQueueFamilyIndex_ || pendingPresentQueueFamilyIndex_ == presentFamily) {
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
    return true;
}

#ifndef USE_M133_SKIA
uint32_t RsVulkanInterface::GetGrVkFeatureFlags()
{
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(physicalDevice_, &features);

    uint32_t fFeatures = 0;
    if (features.geometryShader) {
        fFeatures |= kGeometryShader_GrVkFeatureFlag;
    }
    if (features.dualSrcBlend) {
        fFeatures |= kDualSrcBlend_GrVkFeatureFlag;
    }
    if (features.sampleRateShading) {
        fFeatures |= kSampleRateShading_GrVkFeatureFlag;
    }
    return fFeatures;
}
#endif

#ifdef USE_M133_SKIA
void RsVulkanInterface::SetupSkiaBackendContextBasicFields(skgpu::VulkanBackendContext* context, bool isProtected)
#else
void RsVulkanInterface::SetupSkiaBackendContextBasicFields(GrVkBackendContext* context, bool isProtected)
#endif
{
    context->fInstance = instance_;
    context->fPhysicalDevice = physicalDevice_;
    context->fDevice = device_;
    context->fQueue = graphicsQueue_;
    context->fGraphicsQueueIndex = graphicsQueueFamilyIndex_;
#ifndef USE_M133_SKIA
    context->fMinAPIVersion = VK_API_VERSION_1_3;
    uint32_t extensionFlags = kKHR_surface_GrVkExtensionFlag;
    extensionFlags |= kKHR_ohos_surface_GrVkExtensionFlag;
    context->fExtensions = extensionFlags;
#else
    context->fMaxAPIVersion = VK_API_VERSION_1_3;
#endif
}

#ifdef USE_M133_SKIA
bool RsVulkanInterface::CreateSkiaBackendContext(skgpu::VulkanBackendContext* context, bool isProtected)
#else
bool RsVulkanInterface::CreateSkiaBackendContext(GrVkBackendContext* context, bool isProtected)
#endif
{
    auto getProc = CreateSkiaGetProc();
    if (getProc == nullptr) {
        ROSEN_LOGE("CreateSkiaBackendContext getProc is null");
        return false;
    }
    if (instance_ == VK_NULL_HANDLE) {
        ROSEN_LOGE("CreateSkiaBackendContext instance_ == VK_NULL_HANDLE ");
        return false;
    }
    if (graphicsQueue_ == VK_NULL_HANDLE) {
        ROSEN_LOGE("CreateSkiaBackendContext queue_ is VK_NULL_HANDLE ");
        return false;
    }

    SetupSkiaBackendContextBasicFields(context, isProtected);

    skVkExtensions_.init(getProc, instance_, physicalDevice_,
        gInstanceExtensions.size(), gInstanceExtensions.data(),
        deviceExtensions_.size(), deviceExtensions_.data());

    context->fVkExtensions = &skVkExtensions_;
    context->fDeviceFeatures2 = &physicalDeviceFeatures2_;
#ifndef USE_M133_SKIA
    context->fFeatures = GetGrVkFeatureFlags();
#endif
    context->fGetProc = std::move(getProc);
#ifdef USE_M133_SKIA
    context->fProtectedContext = isProtected ? skgpu::Protected::kYes : skgpu::Protected::kNo;
#else
    context->fOwnsInstanceAndDevice = false;
    context->fProtectedContext = isProtected ? GrProtected::kYes : GrProtected::kNo;
#endif

    return true;
}

bool RsVulkanInterface::SetupDeviceProcAddresses(VkDevice device)
{
    ACQUIRE_PROC(AllocateCommandBuffers, device_);
    ACQUIRE_PROC(AllocateMemory, device_);
    ACQUIRE_PROC(BeginCommandBuffer, device_);
    ACQUIRE_PROC(BindImageMemory, device_);
    ACQUIRE_PROC(BindImageMemory2, device_);
    ACQUIRE_PROC(CmdPipelineBarrier, device_);
    ACQUIRE_PROC(CreateCommandPool, device_);
    ACQUIRE_PROC(CreateFence, device_);
    ACQUIRE_PROC(CreateImage, device_);
    ACQUIRE_PROC(CreateImageView, device_);
    ACQUIRE_PROC(CreateSemaphore, device_);
    ACQUIRE_PROC(DestroyCommandPool, device_);
    ACQUIRE_PROC(DestroyFence, device_);
    ACQUIRE_PROC(DestroyImage, device_);
    ACQUIRE_PROC(DestroyImageView, device_);
    ACQUIRE_PROC(DestroySemaphore, device_);
    ACQUIRE_PROC(DeviceWaitIdle, device_);
    ACQUIRE_PROC(EndCommandBuffer, device_);
    ACQUIRE_PROC(FreeCommandBuffers, device_);
    ACQUIRE_PROC(FreeMemory, device_);
    ACQUIRE_PROC(GetDeviceQueue, device_);
    ACQUIRE_PROC(GetImageMemoryRequirements, device_);
    ACQUIRE_PROC(QueueSubmit, device_);
    ACQUIRE_PROC(QueueWaitIdle, device_);
    ACQUIRE_PROC(ResetCommandBuffer, device_);
    ACQUIRE_PROC(ResetFences, device_);
    ACQUIRE_PROC(WaitForFences, device_);
    ACQUIRE_PROC(GetAndroidHardwareBufferPropertiesANDROID, device_);
    ACQUIRE_PROC(ImportSemaphoreFdKHR, device_);
    ACQUIRE_PROC(GetSemaphoreFdKHR, device_);
    ACQUIRE_PROC(AcquireNextImageKHR, device_);
    ACQUIRE_PROC(DestroySwapchainKHR, device_);
    ACQUIRE_PROC(GetSwapchainImagesKHR, device_);
    ACQUIRE_PROC(CreateSwapchainKHR, device_);
    return true;
}

bool RsVulkanInterface::OpenLibraryHandle()
{
    if (handle_) {
        ROSEN_LOGI("RsVulkanInterface OpenLibararyHandle: vk so has already been loaded.");
        return true;
    }
    ROSEN_LOGI("VulkanProcTable OpenLibararyHandle: dlopen libvulkan.so.");
    dlerror();
    handle_ = dlopen("libvulkan.so", RTLD_NOW | RTLD_LOCAL);
    if (handle_ == nullptr) {
        ROSEN_LOGE("Could not open the vulkan library: %{public}s", dlerror());
        return false;
    }
    return true;
}

bool RsVulkanInterface::CloseLibraryHandle()
{
    if (handle_ != nullptr) {
        dlerror();
        if (dlclose(handle_) != 0) {
            ROSEN_LOGE("Could not close the vulkan lib handle. This indicates a leak. %{public}s", dlerror());
        }
        handle_ = nullptr;
    }
    return handle_ == nullptr;
}

PFN_vkVoidFunction RsVulkanInterface::AcquireProc(
    const char* procName,
    const VkInstance& instance) const
{
    if (procName == nullptr || !vkGetInstanceProcAddr) {
        return nullptr;
    }

    return vkGetInstanceProcAddr(instance, procName);
}

PFN_vkVoidFunction RsVulkanInterface::AcquireProc(
    const char* procName,
    const VkDevice& device) const
{
    if (procName == nullptr || !device || !vkGetInstanceProcAddr) {
        return nullptr;
    }
    return vkGetDeviceProcAddr(device, procName);
}

#ifdef USE_M133_SKIA
skgpu::VulkanGetProc RsVulkanInterface::CreateSkiaGetProc() const
#else
GrVkGetProc RsVulkanInterface::CreateSkiaGetProc() const
#endif
{
    if (!IsValid()) {
        return nullptr;
    }

    return [this](const char* proc_name, VkInstance instance, VkDevice device) {
        if (device == VK_NULL_HANDLE) {
            return reinterpret_cast<PFN_vkVoidFunction>(vkGetInstanceProcAddr(instance, proc_name));
        }
        std::string_view s{proc_name};
        if (s.find("vkQueueSubmit") == 0) {
            if (type_ == RenderEngineType::BASIC_RENDER) {
                return (PFN_vkVoidFunction)RsVulkanInterface::HookedVkQueueSubmitForBasicRender;
            } else if (type_ == RenderEngineType::UNPROTECTED_REDRAW) {
                return (PFN_vkVoidFunction)RsVulkanInterface::HookedVkQueueSubmitForUnprotectedRedraw;
            } else {
                return (PFN_vkVoidFunction)RsVulkanInterface::HookedVkQueueSubmitForProtectedRedraw;
            }
        }
        return reinterpret_cast<PFN_vkVoidFunction>(vkGetDeviceProcAddr(device, proc_name));
    };
}

void RsVulkanInterface::DestroyAllSemaphoreFence()
{
}

void RsVulkanInterface::SetVulkanDeviceStatus(VulkanDeviceStatus status)
{
    deviceStatus_ = status;
}

VulkanDeviceStatus RsVulkanInterface::GetVulkanDeviceStatus()
{
    return deviceStatus_.load();
}

void RsVulkanInterface::CleanupUsedSemaphoreFences()
{
}

void RsVulkanInterface::LogSemaphoreFenceStatistics()
{
}

VkSemaphore RsVulkanInterface::RequireSemaphore()
{
    VkSemaphoreCreateInfo semaphoreInfo;
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;
    VkSemaphore semaphore;
    auto err = vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &semaphore);
    if (err != VK_SUCCESS) {
        return VK_NULL_HANDLE;
    }
    return semaphore;
}

RsVulkanContext::RsVulkanContext(bool isProtected, bool isHtsEnable, RenderEngineType type)
{
    RS_TRACE_NAME("Init VulkanContext");
    type_ = type;
    vulkanInterface_ = std::make_shared<RsVulkanInterface>(isProtected, isHtsEnable, type_);

    if (IsValid()) {
        ROSEN_LOGI("RsVulkanContext::Init success for type %{public}u", static_cast<uint32_t>(type_));
    } else {
        ROSEN_LOGE("RsVulkanContext::Init failed for type %{public}u", static_cast<uint32_t>(type_));
    }
}

RsVulkanContext::~RsVulkanContext()
{
    type_ = RenderEngineType::BASIC_RENDER;
}

RsVulkanContext& RsVulkanContext::Get(RenderEngineType type)
{
    switch (type) {
        case RenderEngineType::UNPROTECTED_REDRAW: {
            static RsVulkanContext unprotectedRedrawSingleton = RsVulkanContext(false, false, type);
            return unprotectedRedrawSingleton;
        }
        case RenderEngineType::PROTECTED_REDRAW: {
            static RsVulkanContext protectedRedrawSingleton = RsVulkanContext(true, false, type);
            return protectedRedrawSingleton;
        }
        case RenderEngineType::BASIC_RENDER:
        default:
        {
            static RsVulkanContext basicRenderSingleton = RsVulkanContext(false, true, RenderEngineType::BASIC_RENDER);
            return basicRenderSingleton;
        }
    }
}

VKAPI_ATTR VkResult RsVulkanInterface::HookedVkQueueSubmitForBasicRender(VkQueue queue, uint32_t submitCount,
    VkSubmitInfo* pSubmits, VkFence fence)
{
    auto vkInterface = RsVulkanContext::Get(RenderEngineType::BASIC_RENDER).GetRsVulkanInterface();
    std::lock_guard<std::mutex> lock(vkInterface->graphicsQueueMutex_);
    RS_LOGD_IF(DEBUG_IPC, "%{public}s queue, interfaceType: %{public}d", __func__, 0);
    RS_OPTIONAL_TRACE_NAME_FMT("%s queue, interfaceType: %d", __func__, 0);
    VkResult ret = vkInterface->vkQueueSubmit(queue, submitCount, pSubmits, fence);
#ifdef HETERO_HDR_ENABLE
    RSHDRPatternManager::Instance().MHCSubmitGPUTask(submitCount, pSubmits);
#endif
    return ret;
}

VKAPI_ATTR VkResult RsVulkanInterface::HookedVkQueueSubmitForUnprotectedRedraw(VkQueue queue, uint32_t submitCount,
    VkSubmitInfo* pSubmits, VkFence fence)
{
    auto vkInterface = RsVulkanContext::Get(RenderEngineType::UNPROTECTED_REDRAW).GetRsVulkanInterface();
    std::lock_guard<std::mutex> lock(vkInterface->graphicsQueueMutex_);
    RS_LOGD_IF(DEBUG_IPC, "%{public}s hardware queue, interfaceType: %{public}d", __func__, 1);
    RS_OPTIONAL_TRACE_NAME_FMT("%s hardware queue, interfaceType: %d", __func__, 1);
    return vkInterface->vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VKAPI_ATTR VkResult RsVulkanInterface::HookedVkQueueSubmitForProtectedRedraw(VkQueue queue, uint32_t submitCount,
    VkSubmitInfo* pSubmits, VkFence fence)
{
    auto vkInterface = RsVulkanContext::Get(RenderEngineType::PROTECTED_REDRAW).GetRsVulkanInterface();
    std::lock_guard<std::mutex> lock(vkInterface->graphicsQueueMutex_);
    RS_LOGD_IF(DEBUG_IPC, "%{public}s hardware queue, interfaceType: %{public}d", __func__, 1);
    RS_OPTIONAL_TRACE_NAME_FMT("%s hardware queue, interfaceType: %d", __func__, 1);
    return vkInterface->vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

std::shared_ptr<Drawing::GPUContext> RsVulkanContext::CreateDrawingGPUContext(const std::string& cacheDir)
{
    auto drawingContext = std::make_shared<Drawing::GPUContext>();
    Drawing::GPUContextOptions options;
    std::string version = std::to_string(VK_API_VERSION_1_2);
    drawingContext->BuildFromVK(GetGrVkBackendContext(), options);
    int maxResources = 0;
    size_t maxResourcesSize = 0;
    drawingContext->GetResourceCacheLimits(&maxResources, &maxResourcesSize);
    if (maxResourcesSize > 0) {
        drawingContext->SetResourceCacheLimits(CACHE_LIMITS_TIMES * maxResources,
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
}
}