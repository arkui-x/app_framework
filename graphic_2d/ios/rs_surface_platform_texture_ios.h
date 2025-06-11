/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef RS_SURFACE_PLATFORM_TEXTURE_IOS_H
#define RS_SURFACE_PLATFORM_TEXTURE_IOS_H

#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <Foundation/Foundation.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#include <memory>
#include "common/rs_common_def.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "platform/common/rs_surface_ext.h"
#include "platform/ios/cf_ref.h"
#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
class RSSurfacePlatformTextureIOS : public RSSurfaceExt {
public:
    static inline constexpr RSSurfaceExtType Type = RSSurfaceExtType::SURFACE_PLATFORM_TEXTURE;

    RSSurfacePlatformTextureIOS(const RSSurfaceExtConfig& config);
    ~RSSurfacePlatformTextureIOS();
    void DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect) override;
    void DrawTextureImageGL(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect);
    void DrawTextureImageForVideo(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect);
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
        if (initTypeCallback_ == nullptr) {
            initTypeCallback_ = initTypeCallback;
        }
    }
    void MarkUiFrameAvailable(bool available) override
    {
        bufferAvailable_.store(available);
    }
    bool IsUiFrameAvailable() const override
    {
        return bufferAvailable_.load();
    }
    void UpdateSurfaceDefaultSize(float width, float height) override;

    RSSurfaceExtConfig GetSurfaceExtConfig() override
    {
        return config_;
    }
    void UpdateSurfaceExtConfig(const RSSurfaceExtConfig& config) override
    {
        config_.additionalData = config.additionalData;
    }
private:
    void InitializePlatformEglContext();

    std::atomic<bool> bufferAvailable_ = false;
    GLuint textureId_ = 0;
    RSSurfaceTextureAttachCallBack attachCallback_;
    RSSurfaceTextureUpdateCallBack updateCallback_;
    RSSurfaceTextureInitTypeCallBack initTypeCallback_;
    RSSurfaceExtConfig config_;
    bool active_ = false;
    std::vector<float> matrix {};
    EAGLContext* platformEglContext_ = nullptr;

    // for video
    void EnsureTextureCacheExists();
    void CreateTextureFromPixelBuffer();
    CVPixelBufferRef GetPixelBuffer();

    int32_t isVideo_ = 0;
    AVPlayerItemVideoOutput* videoOutput_;
    OHOS::CFRef<CVOpenGLESTextureCacheRef> cache_ref_;
    OHOS::CFRef<CVOpenGLESTextureRef> texture_ref_;
    OHOS::CFRef<CVPixelBufferRef> buffer_ref_;
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_SURFACE_PLATFORM_TEXTURE_IOS_H
