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

#ifndef RS_SURFACE_PLATFORM_TEXTURE_IOS_H
#define RS_SURFACE_PLATFORM_TEXTURE_IOS_H

#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <Foundation/Foundation.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <memory>
#include "common/rs_common_def.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "platform/common/rs_surface_ext.h"
#include "platform/ios/cf_ref.h"
#include "platform/common/rs_log.h"
#include "render_context/new_render_context/render_context_gl.h"
#ifdef RS_ENABLE_VK
#include "rs_vulkan_context.h"
#endif

namespace OHOS {
namespace Rosen {
class RSSurfacePlatformTextureIOS : public RSSurfaceExt {
public:
    static inline constexpr RSSurfaceExtType Type = RSSurfaceExtType::SURFACE_PLATFORM_TEXTURE;

    RSSurfacePlatformTextureIOS(const RSSurfaceExtConfig& config);
    ~RSSurfacePlatformTextureIOS();
    void DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect) override;
    void DrawTextureImageGL(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect);
    void DrawTextureImageForVideo(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect);
    void DrawTextureImageForPlatformView(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect);
    void SetAttachCallback(const RSSurfaceTextureAttachCallBack& attachCallback) override
    {
        if (attachCallback_ == nullptr) {
            attachCallback_ = attachCallback;
        }
    }
    void SetUpdateCallback(const RSSurfaceTextureUpdateCallBack& updateCallback) override
    {
        if (updateCallback_ == nullptr) {
            updateCallback_ = updateCallback;
        }
    }
    void SetInitTypeCallback(const RSSurfaceTextureInitTypeCallBack& initTypeCallback) override
    {
        if (initTypeCallback_ == nullptr) {
            initTypeCallback_ = initTypeCallback;
        }
    }
    void MarkUiFrameAvailable(bool available) override
    {
        bufferAvailable_.store(available);
    }
    bool IsUiFrameAvailable() const override
    {
        return bufferAvailable_.load();
    }
    void UpdateSurfaceDefaultSize(float width, float height) override;

    RSSurfaceExtConfig GetSurfaceExtConfig() override
    {
        return config_;
    }
    void UpdateSurfaceExtConfig(const RSSurfaceExtConfig& config) override
    {
        if (config.additionalData != config_.additionalData) {
            config_.additionalData = config.additionalData;
        }
    }
private:
    void InitializePlatformEglContext();
    bool FillTextureInfoByBackend(Drawing::TextureInfo& textureInfo, int texWidth, int texHeight,
        const char* logPrefix);
    Drawing::BitmapFormat GetBitmapFormatByBackend(bool useVulkan) const;
    // this function is used to check if the texture is a video or not.
    bool IsVideo();
    bool IsPlatformViewFrameProvider();
    bool IsObjectiveCObject(const void* candidate);
    CVPixelBufferRef GetPlatformViewPixelBuffer();

    std::atomic<bool> bufferAvailable_ = false;
    GLuint textureId_ = 0;
    RSSurfaceTextureAttachCallBack attachCallback_;
    RSSurfaceTextureUpdateCallBack updateCallback_;
    RSSurfaceTextureInitTypeCallBack initTypeCallback_;
    RSSurfaceExtConfig config_;
    bool active_ = false;
    std::vector<float> matrix {};
    EAGLContext* platformEglContext_ = nullptr;
    void *currentSharePtr_ = nullptr;

    // for video
    void EnsureTextureCacheExists();
    void CreateTextureFromPixelBuffer();
    CVPixelBufferRef GetPixelBuffer();

    int32_t isVideo_ = 0;
    bool isPlatformViewFrameProvider_ = false;
    AVPlayerItemVideoOutput* videoOutput_ = nullptr;
    NSObject* platformViewFrameProvider_ = nullptr;
    OHOS::CFRef<CVOpenGLESTextureCacheRef> cache_ref_;
    OHOS::CFRef<CVOpenGLESTextureRef> texture_ref_;
    OHOS::CFRef<CVPixelBufferRef> buffer_ref_;
#ifdef RS_ENABLE_VK
    bool UpdateVkImageFromPixelBuffer(CVPixelBufferRef pixelBuffer);
    bool ValidatePixelBufferForVulkanUpload(CVPixelBufferRef pixelBuffer, size_t& outWidth, size_t& outHeight,
        size_t& outBytesPerRow, size_t& outUploadSize);
    bool CopyPixelBufferToVulkanStaging(CVPixelBufferRef pixelBuffer, size_t uploadSize);
    bool AllocateAndBeginVkUploadCommandBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device, VkCommandBuffer& cmd);
    void RecordVkImageTransferAndSampleBarriers(std::shared_ptr<RsVulkanInterface>& vkInterface, VkCommandBuffer cmd, size_t width,
        size_t height, size_t bytesPerRow);
    bool EndSubmitWaitAndFreeVkCommandBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface, RsVulkanContext& vkContext,
        VkCommandBuffer cmd);
    bool EnsureVulkanCommandPool();
    bool EnsureVkStagingBuffer(size_t size);
    void DestroyVkStagingBufferResources(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device);
    bool CreateAndBindVkStagingBuffer(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device, size_t size);
    bool EnsureVkImage(int width, int height);
    void DestroyVkImageResources(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device);
    bool CreateVkImage2DForTexture(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device, int width, int height);
    bool AllocateDeviceLocalMemoryAndBindVkImage(std::shared_ptr<RsVulkanInterface>& vkInterface, VkDevice device, int width,
        int height);
    uint32_t FindVulkanMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
    void CleanupVulkanResources();

    VkCommandPool vkCommandPool_ = VK_NULL_HANDLE;
    VkBuffer vkStagingBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vkStagingMemory_ = VK_NULL_HANDLE;
    size_t vkStagingBufferSize_ = 0;
    VkImage vkImage_ = VK_NULL_HANDLE;
    VkDeviceMemory vkImageMemory_ = VK_NULL_HANDLE;
    VkDeviceSize vkImageAllocSize_ = 0;
    int vkImageWidth_ = 0;
    int vkImageHeight_ = 0;
    bool vkImageReady_ = false;
    VkFormat vkImageFormat_ = VK_FORMAT_B8G8R8A8_UNORM;
#endif
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_SURFACE_PLATFORM_TEXTURE_IOS_H
