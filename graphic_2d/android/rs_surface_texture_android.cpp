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

#include "rs_surface_texture_android.h"

#include <cmath>
#include <limits>
#include <vector>

#include "platform/common/rs_log.h"
#include "platform/common/rs_system_properties.h"
#include "gl/rs_surface_texture_android_gl.h"
#ifdef RS_ENABLE_VK
#include "vulkan/rs_surface_texture_android_vulkan.h"
#endif

namespace OHOS {
namespace Rosen {
std::shared_ptr<AndroidSurfaceTexture> AndroidSurfaceTexture::Create(const RSSurfaceExtConfig& config)
{
#ifdef RS_ENABLE_VK
    if (RSSystemProperties::GetGpuApiType() == GpuApiType::VULKAN) {
        return std::make_shared<AndroidSurfaceTextureVK>(config);
    }
#endif
    return std::make_shared<AndroidSurfaceTextureGL>(config);
}

AndroidSurfaceTexture::AndroidSurfaceTexture(const RSSurfaceExtConfig& config)
    : RSSurfaceExt(), config_(config), transform_(Drawing::Matrix())
{
    transform_.SetMatrix(1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

void AndroidSurfaceTexture::MarkUiFrameAvailable(bool available)
{
    bufferAvailable_.store(available);
}

void AndroidSurfaceTexture::UpdateSurfaceDefaultSize(float width, float height)
{
    width_ = width;
    height_ = height;
}

bool AndroidSurfaceTexture::HasValidCallbacks() const
{
    if (attachCallback_ == nullptr || updateCallback_ == nullptr) {
        return false;
    }
#ifdef RS_ENABLE_VK
    if (RSSystemProperties::GetGpuApiType() == GpuApiType::VULKAN && initTypeCallback_ == nullptr) {
        return false;
    }
#endif
    return true;
}

bool AndroidSurfaceTexture::InitializeTextureIfNeeded()
{
    if (state_ == AttachmentState::UNINITIALIZED) {
        if (!bufferAvailable_.load()) {
            return false;
        }
        if (!OnInitializeTexture()) {
            return false;
        }
        state_ = AttachmentState::ATTACHED;
    }
    return true;
}

void AndroidSurfaceTexture::DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect)
{
    if (!CheckPreConditions()) {
        return;
    }
    if (!InitializeTextureIfNeeded()) {
        return;
    }
    ProcessBufferAvailability(freeze);

    if (!OnCreateTextureImage(canvas)) {
        return;
    }

    ApplyClipRectScale(canvas, clipRect);

    canvas.Restore();
    OnPostDraw();
}

bool AndroidSurfaceTexture::CheckPreConditions() const
{
    return state_ != AttachmentState::DETACHED && HasValidCallbacks();
}

void AndroidSurfaceTexture::ProcessBufferAvailability(bool freeze)
{
    bool isBufferAvailable = (!freeze && bufferAvailable_.load());
    if (isBufferAvailable) {
        OnBufferAvailable();
        bufferAvailable_.store(false);
    }
}
} // namespace Rosen
} // namespace OHOS
