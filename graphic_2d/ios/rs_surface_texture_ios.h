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

#ifndef RS_SURFACE_TEXTURE_IOS_H
#define RS_SURFACE_TEXTURE_IOS_H

#include <AVFoundation/AVFoundation.h>
#include <CoreMedia/CoreMedia.h>
#include <Foundation/Foundation.h>
#include <memory>
#include "common/rs_common_def.h"
#include "pipeline/rs_paint_filter_canvas.h"
#include "platform/common/rs_surface_ext.h"
#include "platform/ios/cf_ref.h"

namespace OHOS {
namespace Rosen {
class RSSurfaceTextureIOS : public RSSurfaceExt {
public:
    static inline constexpr RSSurfaceExtType Type = RSSurfaceExtType::SURFACE_TEXTURE;

    RSSurfaceTextureIOS(const RSSurfaceExtConfig& config);
    ~RSSurfaceTextureIOS();
    void DrawTextureImage(RSPaintFilterCanvas& canvas, bool freeze, const Drawing::Rect& clipRect) override;
    void SetAttachCallback(const RSSurfaceTextureAttachCallBack& attachCallback) override
    {
    }
    void SetUpdateCallback(const RSSurfaceTextureUpdateCallBack& updateCallback) override
    {
    }
    void SetInitTypeCallback(const RSSurfaceTextureInitTypeCallBack& initTypeCallback) override
    {
    }
    void MarkUiFrameAvailable(bool available) override
    {
    }
    bool IsUiFrameAvailable() const override
    {   
        return false;
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
    void EnsureTextureCacheExists();
    void CreateTextureFromPixelBuffer();
    CVPixelBufferRef GetPixelBuffer();

    AVPlayerItemVideoOutput* videoOutput_;
    OHOS::CFRef<CVOpenGLESTextureCacheRef> cache_ref_;
    OHOS::CFRef<CVOpenGLESTextureRef> texture_ref_;
    OHOS::CFRef<CVPixelBufferRef> buffer_ref_;
};
} // namespace Rosen
} // namespace OHOS

#endif // RS_SURFACE_TEXTURE_IOS_H
