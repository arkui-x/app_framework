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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_GL_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_GL_H

#include "EGL/egl.h"
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "../rs_surface_texture_android.h"

namespace OHOS {
namespace Rosen {
class AndroidSurfaceTextureGL final : public AndroidSurfaceTexture {
public:
    explicit AndroidSurfaceTextureGL(const RSSurfaceExtConfig& config);
    ~AndroidSurfaceTextureGL() override;

protected:
    void UpdateTransform() override;
    void OnBufferAvailable() override;
    bool OnInitializeTexture() override;
    bool OnCreateTextureImage(RSPaintFilterCanvas& canvas) override;
    void ApplyClipRectScale(RSPaintFilterCanvas& canvas, const Drawing::Rect& clipRect) override;

private:
    GLuint textureId_ = 0;
    std::shared_ptr<Drawing::Image> cachedImage_;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_TEXTURE_ANDROID_GL_H
