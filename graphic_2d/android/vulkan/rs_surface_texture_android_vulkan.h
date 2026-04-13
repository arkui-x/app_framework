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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_VULKAN_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_VULKAN_H

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <queue>
#include <android/hardware_buffer.h>

#include "../rs_surface_texture_android.h"

namespace OHOS {
namespace Ace {
namespace Platform {
class AcquiredFrame;
}
} // namespace Ace
} // namespace OHOS

namespace OHOS {
namespace Rosen {
class AndroidSurfaceTextureVK final : public AndroidSurfaceTexture {
public:
    explicit AndroidSurfaceTextureVK(const RSSurfaceExtConfig& config);
    ~AndroidSurfaceTextureVK() override;

protected:
    bool OnInitializeTexture() override;
    bool OnCreateTextureImage(RSPaintFilterCanvas& canvas) override;
    void ApplyClipRectScale(RSPaintFilterCanvas& canvas, const Drawing::Rect& clipRect) override;
    void OnPostDraw() override;

private:
    bool GetHardwareBuffer();
    void ReleaseTextureInfoMap(std::unordered_map<uintptr_t, std::shared_ptr<Drawing::VKTextureInfo>>& infoMap);

    int32_t textureId_ = 0;
    std::unordered_map<uintptr_t, std::shared_ptr<Drawing::VKTextureInfo>> vkTextureInfoMap_;
    std::unordered_map<uintptr_t, std::shared_ptr<Drawing::VKTextureInfo>> vkTextureInfoOldMap_;
    std::unordered_map<uintptr_t, std::shared_ptr<Drawing::VKTextureInfo>> vkTextureInfoClearMap_;
    std::shared_ptr<Ace::Platform::AcquiredFrame> lastHardwareBuffer_ = nullptr;
    std::queue<std::shared_ptr<Ace::Platform::AcquiredFrame>> retainedFrames_;
    std::shared_ptr<Drawing::Image> cachedImage_;
    uint32_t textureWidth_ = 0;
    uint32_t textureHeight_ = 0;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_VULKAN_H
