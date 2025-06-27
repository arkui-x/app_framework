/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "surface/surface_type.h"

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

    std::unique_ptr<RSSurfaceFrame> RequestFrame(
        int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC = true, bool isProtected = false) override;

    bool FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp) override;
    RenderContext* GetRenderContext() override;
    void SetRenderContext(RenderContext* context) override;
    uint32_t GetQueueSize() const override;
    void ClearBuffer() override;
    void ClearAllBuffer() override;
    void ResetBufferAge() override;
    GraphicColorGamut GetColorSpace() const override;
    void SetColorSpace(GraphicColorGamut colorSpace) override;
    RSSurfaceExtPtr CreateSurfaceExt(const RSSurfaceExtConfig& config) override;
    RSSurfaceExtPtr GetSurfaceExt(const RSSurfaceExtConfig& config) override;
private:
    void YInvert(void *addr, int32_t width, int32_t height);
    bool SetupGrContext();

    RenderContext* renderContext_ = nullptr;
    ANativeWindow* nativeWindow_ = nullptr;
    EGLSurface eglSurface_ = EGL_NO_SURFACE;
    GraphicColorGamut colorSpace_ = GraphicColorGamut::GRAPHIC_COLOR_GAMUT_SRGB;
    std::shared_ptr<AndroidSurfaceTexture> texture_;
};

class AndroidSurfaceTexture : public RSSurfaceExt {
public:
    static inline constexpr RSSurfaceExtType Type = RSSurfaceExtType::SURFACE_TEXTURE;

    AndroidSurfaceTexture(RSSurfaceAndroid* surface, const RSSurfaceExtConfig& config);
    ~AndroidSurfaceTexture();
    void DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect) override;

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
    }
    void MarkUiFrameAvailable(bool available) override;
    bool IsUiFrameAvailable() const override
    {
        return bufferAvailable_.load();
    }

    void UpdateSurfaceDefaultSize(float width, float height) override;
    RSSurfaceExtConfig GetSurfaceExtConfig() override
    {
        return RSSurfaceExtConfig{};
    }
    void UpdateSurfaceExtConfig(const RSSurfaceExtConfig& config) override
    {
    }
private:
    void updateTransform();
    enum class AttachmentState { uninitialized, attached, detached };
    AttachmentState state_ = AttachmentState::uninitialized;
    GLuint textureId_ = 0;
    RSSurfaceTextureAttachCallBack attachCallback_;
    RSSurfaceTextureUpdateCallBack updateCallback_;
    std::atomic<bool> bufferAvailable_ = false;
    RSSurfaceExtConfig config_;
    Drawing::Matrix transform_;
    float width_ = 0;
    float height_ = 0;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_ANDROID_H
