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

#include "rs_surface_frame_android_gl.h"

#include "platform/common/rs_log.h"
#include "render_context/new_render_context/render_context_gl.h"
#include "rs_trace.h"

namespace OHOS {
namespace Rosen {
RSSurfaceFrameAndroidGL::RSSurfaceFrameAndroidGL(int32_t width, int32_t height)
    : RSSurfaceFrameAndroid(width, height)
{
}

void RSSurfaceFrameAndroidGL::SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height)
{
    RS_TRACE_NAME("SetDamageRegion:" + std::to_string(left) + "," +
        std::to_string(top) + "," + std::to_string(width) + "," + std::to_string(height));
    ROSEN_LOGD("SetDamageRegion, left=%d, top=%d, width=%d, height=%d", left, top, width, height);
    std::vector<RectI> rects;
    rects.emplace_back(left, top, width, height);
    GetRenderContextGL()->DamageFrame(rects);
}

int32_t RSSurfaceFrameAndroidGL::GetBufferAge() const
{
    return static_cast<int32_t>(GetRenderContextGL()->QueryEglBufferAge());
}

Drawing::Canvas* RSSurfaceFrameAndroidGL::GetCanvas()
{
    if (surface_ == nullptr) {
        CreateSurface();
    }

    if (surface_ != nullptr) {
        return surface_->GetCanvas().get();
    }
    return nullptr;
}

std::shared_ptr<Drawing::Surface> RSSurfaceFrameAndroidGL::GetSurface()
{
    if (surface_ == nullptr) {
        CreateSurface();
    }
    return surface_;
}
bool RSSurfaceFrameAndroidGL::CreateSurface()
{
    if (GetRenderContextGL() != nullptr) {
        surface_ = GetRenderContextGL()->AcquireSurface(width_, height_);
    }
    return surface_ != nullptr;
}

std::shared_ptr<RenderContextGL> RSSurfaceFrameAndroidGL::GetRenderContextGL() const
{
    return std::static_pointer_cast<RenderContextGL>(renderContext_);
}
} // namespace Rosen
} // namespace OHOS
