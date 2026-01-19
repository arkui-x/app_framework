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

#include "rs_surface_frame_android.h"

#include "platform/common/rs_log.h"
#include "render_context/render_context.h"
#include "rs_trace.h"

namespace OHOS {
namespace Rosen {
RSSurfaceFrameAndroid::RSSurfaceFrameAndroid(int32_t width, int32_t height)
    : width_(width), height_(height)
{
}

void RSSurfaceFrameAndroid::SetRenderContext(std::shared_ptr<RenderContext> context)
{
    renderContext_ = context;
}

} // namespace Rosen
} // namespace OHOS
