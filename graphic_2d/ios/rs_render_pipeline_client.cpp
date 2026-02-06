/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "transaction/rs_render_pipeline_client.h"
#include "platform/common/rs_log.h"
#include "rs_surface_darwin.h"
#include "rs_vsync_client_darwin.h"
namespace OHOS {
namespace Rosen {


void RSRenderPipelineClient::CommitTransaction(std::unique_ptr<RSTransactionData>& transactionData)
{
}

void RSRenderPipelineClient::ExecuteSynchronousTask(const std::shared_ptr<RSSyncTask>& task)
{
}

void RSRenderPipelineClient::TriggerSurfaceCaptureCallback(NodeId id, const RSSurfaceCaptureConfig& captureConfig,
    std::shared_ptr<Media::PixelMap> pixelmap, CaptureError captureErrorCode,
    std::shared_ptr<Media::PixelMap> pixelmapHDR)
{
}

bool RSRenderPipelineClient::TakeSurfaceCapture(NodeId id, std::shared_ptr<SurfaceCaptureCallback> callback,
    const RSSurfaceCaptureConfig& captureConfig, const RSSurfaceCaptureBlurParam& blurParam,
    const Drawing::Rect& specifiedAreaRect)
{
    return true;
}

std::vector<std::pair<NodeId, std::shared_ptr<Media::PixelMap>>> RSRenderPipelineClient::TakeSurfaceCaptureSoloNode(
    NodeId id, const RSSurfaceCaptureConfig& captureConfig)
{
    std::vector<std::pair<NodeId, std::shared_ptr<Media::PixelMap>>> pixelMapIdPairVector;
    return pixelMapIdPairVector;
}

bool RSRenderPipelineClient::TakeSelfSurfaceCapture(NodeId id, std::shared_ptr<SurfaceCaptureCallback> callback,
    const RSSurfaceCaptureConfig& captureConfig)
{
    return true;
}

bool RSRenderPipelineClient::SetWindowFreezeImmediately(NodeId id, bool isFreeze,
    std::shared_ptr<SurfaceCaptureCallback> callback, const RSSurfaceCaptureConfig& captureConfig,
    const RSSurfaceCaptureBlurParam& blurParam)
{
    return true;
}

bool RSRenderPipelineClient::TakeSurfaceCaptureWithAllWindows(NodeId id,
    std::shared_ptr<SurfaceCaptureCallback> callback, const RSSurfaceCaptureConfig& captureConfig,
    bool checkDrmAndSurfaceLock)
{
    return true;
}

bool RSRenderPipelineClient::FreezeScreen(NodeId id, bool isFreeze)
{
    return true;
}

bool RSRenderPipelineClient::TakeUICaptureInRange(
    NodeId id, std::shared_ptr<SurfaceCaptureCallback> callback, const RSSurfaceCaptureConfig& captureConfig)
{
    return true;
}

bool RSRenderPipelineClient::SetHwcNodeBounds(int64_t rsNodeId, float positionX, float positionY,
    float positionZ, float positionW)
{
    return true;
}

int32_t RSRenderPipelineClient::SetFocusAppInfo(const FocusAppInfo& info)
{
    return 0;
}

int32_t RSRenderPipelineClient::GetScreenHDRStatus(ScreenId id, HdrStatus& hdrStatus)
{
    return 0;
}

void RSRenderPipelineClient::DropFrameByPid(const std::vector<int32_t>& pidList, int32_t dropFrameLevel)
{
}

bool RSRenderPipelineClient::SetAncoForceDoDirect(bool direct)
{
    return true;
}

bool RSRenderPipelineClient::RegisterSurfaceBufferCallback(
    pid_t pid, uint64_t uid, std::shared_ptr<SurfaceBufferCallback> callback)
{
    return true;
}

bool RSRenderPipelineClient::UnregisterSurfaceBufferCallback(pid_t pid, uint64_t uid)
{
    return true;
}

void RSRenderPipelineClient::SetLayerTopForHWC(NodeId nodeId, bool isTop, uint32_t zOrder)
{
}

bool RSRenderPipelineClient::RegisterTransactionDataCallback(uint64_t token, uint64_t timeStamp,
    std::function<void()> callback)
{
    return true;
}

void RSRenderPipelineClient::TriggerTransactionDataCallbackAndErase(uint64_t token, uint64_t timeStamp)
{
}

void RSRenderPipelineClient::SetWindowContainer(NodeId nodeId, bool value)
{
}

void RSRenderPipelineClient::ClearUifirstCache(NodeId id)
{
}

void RSRenderPipelineClient::SetScreenFrameGravity(ScreenId id, int32_t gravity)
{
}
} // namespace Rosen
} // namespace OHOS
