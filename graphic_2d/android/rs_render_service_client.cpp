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

#include "transaction/rs_render_service_client.h"

#include <event_handler.h>
#include "platform/common/rs_log.h"
#include "rs_surface_android.h"
#include "rs_vsync_client_android.h"

namespace OHOS {
namespace Rosen {
void RSRenderServiceClient::ExecuteSynchronousTask(const std::shared_ptr<RSSyncTask>& task)
{
}

std::shared_ptr<RSIRenderClient> RSIRenderClient::CreateRenderServiceClient()
{
    static std::shared_ptr<RSIRenderClient> client = std::make_shared<RSRenderServiceClient>();
    return client;
}

void RSRenderServiceClient::CommitTransaction(std::unique_ptr<RSTransactionData>& transactionData)
{
}

bool RSRenderServiceClient::GetUniRenderEnabled()
{
    return {};
}

bool RSRenderServiceClient::CreateNode(const RSSurfaceRenderNodeConfig& config)
{
    return {};
}

void RSRenderServiceClient::SetScreenRefreshRate(ScreenId id, int32_t sceneId, int32_t rate)
{
}

void RSRenderServiceClient::SetRefreshRateMode(int32_t refreshRateMode)
{
}

uint32_t RSRenderServiceClient::GetScreenCurrentRefreshRate(ScreenId id)
{
    return {};
}

std::shared_ptr<RSSurface> RSRenderServiceClient::CreateNodeAndSurface(const RSSurfaceRenderNodeConfig& config,
    bool unobscured)
{
    return std::make_shared<RSSurfaceAndroid>(static_cast<ANativeWindow *>(config.additionalData));
}

class VSyncReceiverAndroid : public VSyncReceiver {
public:
    VSyncReceiverAndroid(const std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler) : handler_(handler) {}
    ~VSyncReceiverAndroid() = default;
    VsyncError Init() override
    {
        client_ = RSVsyncClient::Create();
        if (client_ == nullptr) {
            return GSERROR_NO_MEM;
        }

        return GSERROR_OK;
    }

    VsyncError RequestNextVSync(FrameCallback callback) override
    {
        if (client_ == nullptr) {
            return GSERROR_NOT_INIT;
        }

        auto func = [callback, this](int64_t time, int64_t frameCount) {
            handler_->PostTask([callback, time, frameCount]() {
                if (callback.callbackWithId_) {
                    callback.callbackWithId_(time, frameCount, callback.userData_);
                } else {
                    callback.callback_(time, callback.userData_);
                }
            });
        };
        client_->SetVsyncCallback(func);
        client_->RequestNextVsync();
        return GSERROR_OK;
    }

    VsyncError SetVSyncRate(FrameCallback callback, int32_t rate) override
    {
        if (client_ == nullptr) {
            return GSERROR_NOT_INIT;
        }

        return GSERROR_OK;
    }

private:
    std::unique_ptr<RSVsyncClient> client_ = nullptr;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_ = nullptr;
};

std::shared_ptr<VSyncReceiver> RSRenderServiceClient::CreateVSyncReceiver(
    const std::string& name,
    const std::shared_ptr<OHOS::AppExecFwk::EventHandler> &looper,
    uint64_t id,
    NodeId windowNodeId,
    bool fromXcomponent)
{
    return std::make_shared<VSyncReceiverAndroid>(looper);
}

bool RSRenderServiceClient::TakeSurfaceCapture(NodeId id, std::shared_ptr<SurfaceCaptureCallback> callback,
    const RSSurfaceCaptureConfig& captureConfig, const RSSurfaceCaptureBlurParam& blurParam,
    const Drawing::Rect& specifiedAreaRect)
{
    return false;
}

int32_t RSRenderServiceClient::SetFocusAppInfo(const FocusAppInfo& info)
{
    return false;
}

ScreenId RSRenderServiceClient::GetDefaultScreenId()
{
    return 0;
}

std::vector<ScreenId> RSRenderServiceClient::GetAllScreenIds()
{
    return {0};
}

void RSRenderServiceClient::RemoveVirtualScreen(ScreenId id)
{
}

int32_t RSRenderServiceClient::SetScreenChangeCallback(const ScreenChangeCallback &callback)
{
    return 0;
}

int32_t RSRenderServiceClient::GetScreenBacklight(ScreenId id)
{
    return {};
}

void RSRenderServiceClient::SetScreenBacklight(ScreenId id, uint32_t level)
{
}

bool RSRenderServiceClient::RegisterBufferAvailableListener(NodeId id, const BufferAvailableCallback &callback, bool isFromRenderThread)
{
    return {};
}

bool RSRenderServiceClient::RegisterBufferClearListener(
    NodeId id, const BufferClearCallback &callback)
{
    return {};
}

bool RSRenderServiceClient::UnregisterBufferAvailableListener(NodeId id)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenSupportedColorGamuts(ScreenId id, std::vector<ScreenColorGamut>& mode)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenSupportedMetaDataKeys(ScreenId id, std::vector<ScreenHDRMetadataKey>& keys)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenColorGamut(ScreenId id, ScreenColorGamut& mode)
{
    return {};
}

int32_t RSRenderServiceClient::SetScreenColorGamut(ScreenId id, int32_t modeIdx)
{
    return {};
}

int32_t RSRenderServiceClient::SetScreenGamutMap(ScreenId id, ScreenGamutMap mode)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenGamutMap(ScreenId id, ScreenGamutMap& mode)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenHDRCapability(ScreenId id, RSScreenHDRCapability& screenHdrCapability)
{
    return {};
}

int32_t RSRenderServiceClient::GetScreenType(ScreenId id, RSScreenType& screenType)
{
    return {};
}

bool RSRenderServiceClient::GetBitmap(NodeId id, Drawing::Bitmap& bitmap)
{
    return {};
}

bool RSRenderServiceClient::GetPixelmap(NodeId id, std::shared_ptr<Media::PixelMap> pixelmap,
    const Drawing::Rect* rect, std::shared_ptr<Drawing::DrawCmdList> drawCmdList)
{
    return {};
}

bool RSRenderServiceClient::RegisterTypeface(std::shared_ptr<Drawing::Typeface>& typeface)
{
    return {};
}

bool RSRenderServiceClient::UnRegisterTypeface(std::shared_ptr<Drawing::Typeface>& typeface)
{
    return {};
}

int32_t RSRenderServiceClient::SetScreenSkipFrameInterval(ScreenId id, uint32_t skipFrameInterval)
{
    return {};
}

int32_t RSRenderServiceClient::RegisterOcclusionChangeCallback(const OcclusionChangeCallback& callback)
{
    return {};
}

int32_t RSRenderServiceClient::RegisterHgmConfigChangeCallback(const HgmConfigChangeCallback& callback)
{
    return {};
}

int32_t RSRenderServiceClient::RegisterHgmRefreshRateModeChangeCallback(
    const HgmRefreshRateModeChangeCallback& callback)
{
    return {};
}

void RSRenderServiceClient::SetAppWindowNum(uint32_t num)
{
}

void RSRenderServiceClient::ShowWatermark(const std::shared_ptr<Media::PixelMap> &watermarkImg, bool isShow)
{
}

void RSRenderServiceClient::ReportJankStats()
{
}

void RSRenderServiceClient::ReportEventResponse(DataBaseRs info)
{
}

void RSRenderServiceClient::ReportEventComplete(DataBaseRs info)
{
}

void RSRenderServiceClient::ReportEventJankFrame(DataBaseRs info)
{
}

void RSRenderServiceClient::ReportRsSceneJankStart(AppInfo info)
{
}

void RSRenderServiceClient::ReportRsSceneJankEnd(AppInfo info)
{
}

std::vector<int32_t> RSRenderServiceClient::GetScreenSupportedRefreshRates(ScreenId id)
{
    return {};
}

void RSRenderServiceClient::NotifyXComponentExpectedFrameRate(const std::string& id, int32_t expectedFrameRate)
{
}

void RSRenderServiceClient::SetHardwareEnabled(NodeId id, bool isEnabled, SelfDrawingNodeType selfDrawingType,
    bool dynamicHardwareEnable)
{
}

void RSRenderServiceClient::SetCacheEnabledForRotation(bool isEnabled)
{
}

int32_t RSRenderServiceClient::GetCurrentRefreshRateMode()
{
}

bool RSRenderServiceClient::RegisterTransactionDataCallback(uint64_t token, uint64_t timeStamp,
    std::function<void()> callback)
{
    return false;
}

bool RSRenderServiceClient::TakeUICaptureInRange(
    NodeId id, std::shared_ptr<SurfaceCaptureCallback> callback, const RSSurfaceCaptureConfig& captureConfig)
{
    return false;
}
} // namespace Rosen
} // namespace OHOS
