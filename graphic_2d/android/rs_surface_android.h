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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_H

#include <android/native_window.h>
#include "EGL/egl.h"
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <memory>

#include "common/rs_common_def.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "platform/common/rs_surface_ext.h"
#include "platform/drawing/rs_surface.h"
#include "platform/drawing/rs_surface_frame.h"
#include "surface_type.h"
#include "render_context/new_render_context/render_context_gl.h"
#include "rs_surface_texture_android.h"

namespace OHOS {
namespace Rosen {
class RenderContext;
class AndroidSurfaceTexture;

class RSSurfaceAndroid : public RSSurface {
public:
    RSSurfaceAndroid(ANativeWindow* data);
    ~RSSurfaceAndroid() override;

    bool IsValid() const override;

    void SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp = 0) override;
    std::shared_ptr<RenderContext> GetRenderContext() override;
    void SetRenderContext(std::shared_ptr<RenderContext> context) override;
    uint32_t GetQueueSize() const override;
    void ClearBuffer() override;
    void ClearAllBuffer() override;
    void ResetBufferAge() override;
    GraphicColorGamut GetColorSpace() const override;
    void SetColorSpace(GraphicColorGamut colorSpace) override;
    RSSurfaceExtPtr CreateSurfaceExt(const RSSurfaceExtConfig& config) override;
    RSSurfaceExtPtr GetSurfaceExt(const RSSurfaceExtConfig& config) override;
protected:
    virtual bool SetupGrContext() = 0;
    void YInvert(void *addr, int32_t width, int32_t height);
    std::shared_ptr<RenderContext> renderContext_;
    ANativeWindow* nativeWindow_ = nullptr;
    GraphicColorGamut colorSpace_ = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    std::shared_ptr<AndroidSurfaceTexture> texture_;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_H
