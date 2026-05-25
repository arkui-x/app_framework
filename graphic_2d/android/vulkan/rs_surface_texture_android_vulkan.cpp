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

#include "rs_surface_texture_android_vulkan.h"

#include "image_texture_jni.h"
#include "../native_buffer_utils.h"
#include "platform/common/rs_log.h"

const float TRANSLATE_VALUE = 0.5f;
const int MAX_RETAINED_FRAMES = 1;
namespace OHOS {
namespace Rosen {
AndroidSurfaceTextureVK::AndroidSurfaceTextureVK(const RSSurfaceExtConfig& config)
    : AndroidSurfaceTexture(config)
{
}

AndroidSurfaceTextureVK::~AndroidSurfaceTextureVK()
{
    if (cachedImage_ != nullptr) {
        cachedImage_.reset();
    }

    ReleaseTextureInfoMap(vkTextureInfoMap_);
    ReleaseTextureInfoMap(vkTextureInfoOldMap_);
    ReleaseTextureInfoMap(vkTextureInfoClearMap_);
    if (lastHardwareBuffer_ != nullptr) {
        lastHardwareBuffer_.reset();
    }
}

bool AndroidSurfaceTextureVK::OnInitializeTexture()
{
    RSSurfaceTextureInitTypeCallBack initTypeCallback = GetInitType();
    if (initTypeCallback == nullptr) {
        ROSEN_LOGE("AndroidSurfaceTextureVK::OnInitializeTexture initTypeCallback is nullptr");
        return false;
    }
    initTypeCallback(textureId_);
    return true;
}

bool AndroidSurfaceTextureVK::GetHardwareBuffer()
{
    auto imageTexture = Ace::Platform::ImageTextureJni::GetImageTexture(textureId_);
    if (!imageTexture) {
        ROSEN_LOGE("AndroidSurfaceTextureVK::OnCreateTextureImage imageTexture is invalid");
        return false;
    }

    std::shared_ptr<Ace::Platform::AcquiredFrame> bufferHandle = imageTexture->AcquireLatestHardwareBuffer();
    if (bufferHandle != nullptr) {
        if (lastHardwareBuffer_ != nullptr) {
            retainedFrames_.push(std::move(lastHardwareBuffer_));
            while (retainedFrames_.size() > MAX_RETAINED_FRAMES) {
                retainedFrames_.pop();
            }
        }
        lastHardwareBuffer_ = bufferHandle;
    }

    if (imageTexture->GetUpdateState()) {
        for (auto& it : vkTextureInfoMap_) {
            vkTextureInfoOldMap_.emplace(it.first, it.second);
        }
        vkTextureInfoMap_.clear();
        imageTexture->SetUpdateState(false);
    }

    if (lastHardwareBuffer_ == nullptr) {
        ROSEN_LOGE("AndroidSurfaceTextureVK: AHardwareBuffer is nullptr");
        return false;
    }

    textureWidth_ = lastHardwareBuffer_->bufferWidth;
    textureHeight_ = lastHardwareBuffer_->bufferHeight;
    return true;
}

bool AndroidSurfaceTextureVK::OnCreateTextureImage(RSPaintFilterCanvas& canvas)
{
    if (!GetHardwareBuffer()) {
        return false;
    }
    Drawing::TextureInfo textureInfo;
    Drawing::BitmapFormat fmt = Drawing::BitmapFormat{ Drawing::COLORTYPE_RGBA_8888, Drawing::ALPHATYPE_PREMUL };
    uintptr_t hardwareBufferId = reinterpret_cast<uintptr_t>(lastHardwareBuffer_->buffer);

    if (vkTextureInfoMap_.find(hardwareBufferId) == vkTextureInfoMap_.end()) {
        auto vkTextureInfo = NativeBufferUtils::MakeVKTextureInfoFromHardwareBuffer(lastHardwareBuffer_->buffer,
            textureWidth_, textureHeight_);
        if (!vkTextureInfo) {
            ROSEN_LOGE("AndroidSurfaceTextureVK: MakeVKTextureInfoFromHardwareBuffer failed");
            return false;
        }
        vkTextureInfoMap_.emplace(hardwareBufferId, vkTextureInfo);
    }
    textureInfo.SetVKTextureInfo(vkTextureInfoMap_[hardwareBufferId]);
    textureInfo.SetWidth(textureWidth_);
    textureInfo.SetHeight(textureHeight_);

    if (!cachedImage_) {
        cachedImage_ = std::make_shared<Drawing::Image>();
    }

    bool ret = cachedImage_->BuildFromTexture(*canvas.GetGPUContext(), textureInfo,
        Drawing::TextureOrigin::TOP_LEFT, fmt, nullptr);
    if (!ret) {
        vkTextureInfoOldMap_.emplace(hardwareBufferId, vkTextureInfoMap_[hardwareBufferId]);
        vkTextureInfoMap_.erase(hardwareBufferId);
        ROSEN_LOGE("AndroidSurfaceTextureVK::OnCreateTextureImage BuildFromTexture failed");
        return false;
    }
    return true;
}

void AndroidSurfaceTextureVK::ApplyClipRectScale(RSPaintFilterCanvas& canvas, const Drawing::Rect& clipRect)
{
    canvas.Save();
    canvas.Translate(clipRect.GetLeft(), clipRect.GetTop());
    if (textureWidth_ > 0 && textureHeight_ > 0) {
        canvas.Scale(clipRect.GetWidth() / textureWidth_, clipRect.GetHeight() / textureHeight_);
    }

    if (!transform_.IsIdentity()) {
        Drawing::Matrix transformAroundCenter(transform_);
        transformAroundCenter.PreTranslate(-TRANSLATE_VALUE, -TRANSLATE_VALUE);
        transformAroundCenter.PostScale(1, -1); // 1:scaleY, -1:scaleY
        transformAroundCenter.PostTranslate(TRANSLATE_VALUE, TRANSLATE_VALUE);
        canvas.ConcatMatrix(transformAroundCenter);
    }
    canvas.DrawImage(*cachedImage_, 0, 0, Drawing::SamplingOptions()); // 0:scalarX, 0:scalarY
}

void AndroidSurfaceTextureVK::OnPostDraw()
{
    ReleaseTextureInfoMap(vkTextureInfoClearMap_);
    for (auto& it : vkTextureInfoOldMap_) {
        vkTextureInfoClearMap_.emplace(it.first, it.second);
    }
    vkTextureInfoOldMap_.clear();
}

void AndroidSurfaceTextureVK::ReleaseTextureInfoMap(
    std::unordered_map<uintptr_t, std::shared_ptr<Drawing::VKTextureInfo>>& infoMap)
{
    auto& vkContext = RsVulkanContext::GetSingleton();
    for (const auto& it : infoMap) {
        auto info = it.second;
        if (info) {
            vkContext.GetRsVulkanInterface().vkDestroyImage(vkContext.GetDevice(), info->vkImage, nullptr);
            vkContext.GetRsVulkanInterface().vkFreeMemory(vkContext.GetDevice(), info->vkAlloc.memory, nullptr);
        }
    }
    infoMap.clear();
}
} // namespace Rosen
} // namespace OHOS
