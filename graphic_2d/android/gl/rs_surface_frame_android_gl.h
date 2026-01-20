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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_GL_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_GL_H

#include "rs_surface_frame_android.h"

#include <android/native_window.h>

namespace OHOS {
namespace Rosen {
class RSSurfaceFrameAndroidGL : public RSSurfaceFrameAndroid {
public:
    RSSurfaceFrameAndroidGL(int32_t width, int32_t height);
    ~RSSurfaceFrameAndroidGL() override = default;

    void SetDamageRegion(int32_t left, int32_t top, int32_t width, int32_t height) override;
    int32_t GetBufferAge() const override;
    Drawing::Canvas* GetCanvas() override;
    std::shared_ptr<Drawing::Surface> GetSurface() override;

private:
    bool CreateSurface();
    std::shared_ptr<RenderContextGL> GetRenderContextGL() const;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_ANDROID_RS_SURFACE_FRAME_ANDROID_GL_H
