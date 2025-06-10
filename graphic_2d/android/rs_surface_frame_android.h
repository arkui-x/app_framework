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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_H

#include "platform/drawing/rs_surface_frame.h"

#include <android/native_window.h>

namespace OHOS {
namespace Rosen {
class RSSurfaceFrameAndroid : public RSSurfaceFrame {
public:
    RSSurfaceFrameAndroid(int32_t width, int32_t height);
    ~RSSurfaceFrameAndroid() override = default;

    void SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height) override;
    int32_t GetBufferAge() const override;
    Drawing::Canvas* GetCanvas() override;
    std::shared_ptr<Drawing::Surface> GetSurface() override;
    void SetRenderContext(RenderContext* context) override;

private:
    friend class RSSurfaceAndroid;

    bool CreateSurface();

    RenderContext *renderContext_ = nullptr;
    std::unique_ptr<uint32_t[]> addr_ = nullptr;
    int32_t width_ = 0;
    int32_t height_ = 0;
    int32_t bpp_ = 0;
    std::shared_ptr<Drawing::Surface> surface_ = nullptr;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_H
