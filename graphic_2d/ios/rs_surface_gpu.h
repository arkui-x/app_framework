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

#ifndef RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_GPU_H
#define RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_GPU_H

#include <memory>

#include "common/rs_common_def.h"
#include "platform/common/rs_surface_ext.h"
#include "platform/drawing/rs_surface.h"
#include "platform/drawing/rs_surface_frame.h"
#include "platform/ios/cf_ref.h"
#include "render_context/render_context.h"

#ifdef __OBJC__
@class CAEAGLLayer;
#else
typedef struct objc_object CAEAGLLayer;
#endif

namespace OHOS {
namespace Rosen {
class RenderContext;
class RSSurfaceGPU : public RSSurface {
public:
    RSSurfaceGPU(void* layer);
    ~RSSurfaceGPU() override;

    bool IsValid() const override;

    void SetUiTimeStamp(const std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp = 0) override
    {
    }

    std::unique_ptr<RSSurfaceFrame> RequestFrame(
        int32_t width, int32_t height, uint64_t uiTimestamp, bool useAFBC = true, bool isProtected = false) override;

    bool FlushFrame(std::unique_ptr<RSSurfaceFrame>& frame, uint64_t uiTimestamp) override;
    uint32_t GetQueueSize() const override;
    void ClearBuffer() override
    {
    }
    void ClearAllBuffer() override
    {
    }
    RenderContext* GetRenderContext() override;
    void SetRenderContext(RenderContext* context) override;
    void ResetBufferAge() override
    {
    }
    void SetCleanUpHelper(std::function<void()> func) override;
    GraphicColorGamut GetColorSpace() const override;
    void SetColorSpace(GraphicColorGamut colorSpace) override;
    RSSurfaceExtPtr CreateSurfaceExt(const RSSurfaceExtConfig& config) override;
    RSSurfaceExtPtr GetSurfaceExt(const RSSurfaceExtConfig& config) override;  
private:
    bool SetupGrContext();

    CAEAGLLayer* layer_ = nullptr;
    RenderContext *renderContext_ = nullptr;
    RSSurfaceExtPtr texture_;
};

} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_SRC_PLATFORM_IOS_RS_SURFACE_GPU_H
