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

#include "rs_surface_android_gl.h"

#include <cstdint>
#include <securec.h>

#include "platform/common/rs_log.h"
#include "rs_surface_frame_android_gl.h"

#include "render_context/new_render_context/render_context_gl.h"

const float TRANSLATE_VALUE = 0.5f;
namespace OHOS {
namespace Rosen {
RSSurfaceAndroidGL::RSSurfaceAndroidGL(ANativeWindow* data)
    : RSSurfaceAndroid(data)
{
    ROSEN_LOGE("RSSurfaceAndroidGL entry with %p", nativeWindow_);
}

RSSurfaceAndroidGL::~RSSurfaceAndroidGL()
{
    if (GetRenderContextGL() != nullptr) {
        GetRenderContextGL()->DestroyEGLSurface(eglSurface_);
    }
    eglSurface_ = EGL_NO_SURFACE;
}

void RSSurfaceAndroidGL::SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
}

std::unique_ptr<RSSurfaceFrame> RSSurfaceAndroidGL::RequestFrame(
    int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC, bool isProtected)
{
    if (nativeWindow_ == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroidGL::RequestFrame, producer is nullptr");
        return nullptr;
    }
    if (GetRenderContextGL() == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroidGL::RequestFrame, renderContext is nullptr");
        return nullptr;
    }
    if (!SetupGrContext()) {
        return nullptr;
    }
    ANativeWindow_setBuffersGeometry(nativeWindow_, width, height, WINDOW_FORMAT_RGBA_8888);
    if (eglSurface_ == nullptr) {
        eglSurface_ = GetRenderContextGL()->CreateEGLSurface(static_cast<EGLNativeWindowType>(nativeWindow_));
    }

    auto frame = std::make_unique<RSSurfaceFrameAndroidGL>(width, height);
    GetRenderContextGL()->MakeCurrent(eglSurface_);

    ROSEN_LOGD("RSSurfaceAndroidGL:RequestFrame, eglsurface is %p, width is %d, height is %d, renderContext_ %p",
        eglSurface_, width, height, GetRenderContextGL().get());

    frame->SetRenderContext(GetRenderContextGL());

    return frame;
}

bool RSSurfaceAndroidGL::FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp)
{
    if (GetRenderContextGL() == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroidGL::FlushFrame, GetRenderContext failed!");
        return false;
    }

    // gpu render flush
    GetRenderContextGL()->RenderFrame();
    GetRenderContextGL()->SwapBuffers(eglSurface_);
    ROSEN_LOGD("RSSurfaceAndroidGL: FlushFrame, SwapBuffers eglsurface is %p", eglSurface_);
    if (auto grContext = GetRenderContextGL()->GetDrGPUContext()) {
        grContext->PurgeUnlockedResources(true);
    }
    return true;
}

std::shared_ptr<RenderContextGL> RSSurfaceAndroidGL::GetRenderContextGL() const
{
    return std::static_pointer_cast<RenderContextGL>(renderContext_);
}
bool RSSurfaceAndroidGL::SetupGrContext()
{
    if (GetRenderContextGL()) {
        GetRenderContextGL()->Init();
        GetRenderContextGL()->SetUpGpuContext();
        GetRenderContextGL()->SetColorSpace(colorSpace_);
    }
    return true;
}

GraphicColorGamut RSSurfaceAndroidGL::GetColorSpace() const
{
    if (GetRenderContextGL() == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroidGL::GetColorSpace, renderContext_ is null  %u", colorSpace_);
        return colorSpace_;
    }
    return GetRenderContextGL()->GetColorSpace();
}

void RSSurfaceAndroidGL::SetColorSpace(GraphicColorGamut colorSpace)
{
    ROSEN_LOGD("RSSurfaceAndroidGL::SetColorSpace colorSpace %u", colorSpace);
    if (GetRenderContextGL() == nullptr) {
        ROSEN_LOGE("RSSurfaceAndroidGL::SetColorSpace renderContext_ is null");
        return;
    }
    colorSpace_ = colorSpace;
    GetRenderContextGL()->SetColorSpace(colorSpace_);
}
} // namespace Rosen
} // namespace OHOS
