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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_GL_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_GL_H

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

#include "rs_surface_android.h"

namespace OHOS {
namespace Rosen {
class RenderContextGL;
class AndroidSurfaceTexture;

class RSSurfaceAndroidGL : public RSSurfaceAndroid {
public:
    explicit RSSurfaceAndroidGL(ANativeWindow* data);
    ~RSSurfaceAndroidGL() override;

    void SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp = 0) override;

    std::unique_ptr<RSSurfaceFrame> RequestFrame(
        int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC = true, bool isProtected = false) override;

    bool FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp) override;

    GraphicColorGamut GetColorSpace() const override;
    void SetColorSpace(GraphicColorGamut colorSpace) override;
    
private:
    bool SetupGrContext() override;
    std::shared_ptr<RenderContextGL> GetRenderContextGL() const;
    EGLSurface eglSurface_ = EGL_NO_SURFACE;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_GL_H
