/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "rs_surface_android.h"

#include <stdint.h>
#include <securec.h>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_android.h"
#include "render_context/render_context.h"

const float TRANSLATE_VALUE = 0.5f;
namespace OHOS {
namespace Rosen {
RSSurfaceAndroid::RSSurfaceAndroid(ANativeWindow* data)
    : nativeWindow_(data)
{
    ROSEN_LOGD("RSSurfaceAndroid entry with %p", nativeWindow_);
}

RSSurfaceAndroid::~RSSurfaceAndroid()
{
    nativeWindow_ = nullptr;
}

bool RSSurfaceAndroid::IsValid() const
{
    return nativeWindow_ != nullptr;
}

void RSSurfaceAndroid::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}

std::shared_ptr<RenderContext> RSSurfaceAndroid::GetRenderContext()
{
    return renderContext_;
}

void RSSurfaceAndroid::SetRenderContext(std::shared_ptr<RenderContext> context)
{
    renderContext_ = context;
}

void RSSurfaceAndroid::YInvert(void *addr, int32_t width, int32_t height)
{
}

uint32_t RSSurfaceAndroid::GetQueueSize() const
{
    return 0x3;
}

void RSSurfaceAndroid::ClearBuffer()
{
}

void RSSurfaceAndroid::ClearAllBuffer()
{
}

void RSSurfaceAndroid::ResetBufferAge()
{
    ROSEN_LOGD("RSSurfaceAndroid: Reset Buffer Age!");
}

void RSSurfaceAndroid::SetCleanUpHelper(std::function<void()> func) {}

GraphicColorGamut RSSurfaceAndroid::GetColorSpace() const
{
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::GetColorSpace, renderContext_ is null  %u", colorSpace_);
        return colorSpace_;
    }
     return renderContext_->GetColorSpace();
}

void RSSurfaceAndroid::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGD("RSSurfaceAndroid::SetColorSpace colorSpace %u", colorSpace);
    if (renderContext_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroid::SetColorSpace renderContext_ is null");
        return;
    }
    colorSpace_ = colorSpace;
    renderContext_->SetColorSpace(colorSpace_);
}

RSSurfaceExtPtr RSSurfaceAndroid::CreateSurfaceExt(const RSSurfaceExtConfig& config)
{
    ROSEN_LOGD("RSSurfaceAndroid::CreateSurfaceExt %{public}u", config.type);
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            if (texture_ == nullptr) {
                texture_ = std::make_shared<AndroidSurfaceTexture>(config);
            }
            return texture_;
        }
        default:
            return nullptr;
    }
}

RSSurfaceExtPtr RSSurfaceAndroid::GetSurfaceExt(const RSSurfaceExtConfig& config)
{
    switch(config.type) {
        case RSSurfaceExtType::SURFACE_TEXTURE: {
            return texture_;
        }
        default:
            return nullptr;
    }
}
} // namespace Rosen
} // namespace OHOS
