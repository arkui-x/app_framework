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

#include "platform/common/rs_system_properties.h"

#include "transaction/rs_render_service_client.h"

namespace OHOS {
namespace Rosen {
const GpuApiType RSSystemProperties::systemGpuApiType_ = GpuApiType::OPENGL;

int RSSystemProperties::GetDumpFrameNum()
{
    return 0;
}

int RSSystemProperties::GetRecordingEnabled()
{
    return 0;
}

void RSSystemProperties::SetRecordingDisenabled()
{
    return;
}

std::string RSSystemProperties::GetRecordingFile()
{
    return "";
}

bool RSSystemProperties::GetUniRenderEnabled()
{
    return isUniRenderEnabled_;
}

bool RSSystemProperties::GetRenderNodeTraceEnabled()
{
    return {};
}

bool RSSystemProperties::GetAnimationTraceEnabled()
{
    return {};
}

bool RSSystemProperties::GetRenderNodePurgeEnabled()
{
    return false;
}

bool RSSystemProperties::GetDrawOpTraceEnabled()
{
    return false;
}

bool RSSystemProperties::IsSceneBoardEnabled()
{
    return {};
}

DirtyRegionDebugType RSSystemProperties::GetDirtyRegionDebugType()
{
    return {};
}

PartialRenderType RSSystemProperties::GetPartialRenderEnabled()
{
    return {};
}

PartialRenderType RSSystemProperties::GetUniPartialRenderEnabled()
{
    return {};
}

bool RSSystemProperties::GetOcclusionEnabled()
{
    return {};
}

std::string RSSystemProperties::GetRSEventProperty(const std::string &paraName)
{
    return {};
}

bool RSSystemProperties::GetHighContrastStatus()
{
    return {};
}

bool RSSystemProperties::IsPhoneType()
{
    return true;
}

bool RSSystemProperties::GetCacheEnabledForRotation()
{
    return {};
}

#ifndef NEW_SKIA
ReleaseGpuResourceType RSSystemProperties::GetReleaseGpuResourceEnabled()
{
    return {};
}
#endif

uint32_t RSSystemProperties::GetCorrectionMode()
{
    return {};
}

DumpSurfaceType RSSystemProperties::GetDumpSurfaceType()
{
    return {};
}

long long int RSSystemProperties::GetDumpSurfaceId()
{
    return {};
}

void RSSystemProperties::SetDrawTextAsBitmap(bool flag)
{
}

bool RSSystemProperties::GetDrawTextAsBitmap()
{
    return {};
}

ParallelRenderingType RSSystemProperties::GetPrepareParallelRenderingEnabled()
{
    return {};
}

ParallelRenderingType RSSystemProperties::GetParallelRenderingEnabled()
{
    return {};
}

HgmRefreshRates RSSystemProperties::GetHgmRefreshRatesEnabled()
{
    return {};
}

void RSSystemProperties::SetHgmRefreshRateModesEnabled(std::string param)
{
    return;
}

HgmRefreshRateModes RSSystemProperties::GetHgmRefreshRateModesEnabled()
{
    return {};
}

bool RSSystemProperties::GetSkipForAlphaZeroEnabled()
{
    return {};
}

bool RSSystemProperties::GetSkipGeometryNotChangeEnabled()
{
    return {};
}

bool RSSystemProperties::GetAnimationCacheEnabled()
{
    return {};
}

float RSSystemProperties::GetAnimationScale()
{
    return 1.f;
}

bool RSSystemProperties::GetProxyNodeDebugEnabled()
{
    return false;
}

bool RSSystemProperties::GetFilterCacheEnabled()
{
    return false;
}

int RSSystemProperties::GetFilterCacheUpdateInterval()
{
    return 0;
}

int RSSystemProperties::GetFilterCacheSizeThreshold()
{
    return 0;
}

bool RSSystemProperties::GetKawaseEnabled()
{
    return false;
}

bool RSSystemProperties::GetHpsBlurEnabled()
{
    return false;
}

bool RSSystemProperties::GetMESABlurFuzedEnabled()
{
    return false;
}


bool RSSystemProperties::GetBoolSystemProperty(const char* name, bool defaultValue)
{
    return {};
}

int RSSystemProperties::WatchSystemProperty(const char* name, OnSystemPropertyChanged func, void* context)
{
    return {};
}

bool RSSystemProperties::GetAFBCEnabled()
{
    return {};
}

bool RSSystemProperties::GetASTCEnabled()
{
    return false;
}

bool RSSystemProperties::GetImageGpuResourceCacheEnable(int, int)
{
    return false;
}

float RSSystemProperties::GetKawaseRandomColorFactor()
{
    return 0.f;
}

bool RSSystemProperties::GetRandomColorEnabled()
{
    return false;
}

bool RSSystemProperties::GetKawaseOriginalEnabled()
{
    return false;
}

bool RSSystemProperties::GetBlurEnabled()
{
    return true;
}

bool RSSystemProperties::GetForegroundFilterEnabled()
{
    return true;
}


const std::vector<float>& RSSystemProperties::GetAiInvertCoef()
{
    // Configure AiInvertCoef: Low, High, Threshold, Opacity, Saturation, Filter Radius.
    std::vector<float> aiInvertCoef = {0.0, 1.0, 0.55, 0.4, 1.6, 45.0};
    return aiInvertCoef;
}

bool RSSystemProperties::GetDebugTraceEnabled()
{
    return false;
}

int RSSystemProperties::GetDebugTraceLevel()
{
    return 0;
}

bool RSSystemProperties::GetSyncTransactionEnabled()
{
    return false;
}

int RSSystemProperties::GetSyncTransactionWaitDelay()
{
    return 0;
}

bool RSSystemProperties::GetSingleFrameComposerEnabled()
{
    return false;
}

bool RSSystemProperties::GetSubSurfaceEnabled()
{
    return false;
}

bool RSSystemProperties::GetSingleFrameComposerCanvasNodeEnabled()
{
    return false;
}

bool RSSystemProperties::GetDrawFilterWithoutSnapshotEnabled()
{
    return false;
}

bool RSSystemProperties::GetBlurExtraFilterEnabled()
{
    return false;
}

bool RSSystemProperties::GetDiscardCanvasBeforeFilterEnabled()
{
    return false;
}

bool RSSystemProperties::GetMaskLinearBlurEnabled(){
    return false;
}

bool RSSystemProperties::GetMotionBlurEnabled()
{
    return true;
}

bool RSSystemProperties::GetDynamicBrightnessEnabled()
{
    return true;
}

bool RSSystemProperties::IsTabletType()
{
    return false;
}

bool RSSystemProperties::IsPcType()
{
    return false;
}

bool RSSystemProperties::GetEffectMergeEnabled()
{
    return true;
}

bool RSSystemProperties::GetDumpUICaptureEnabled()
{
    return false;
}

bool RSSystemProperties::GetDumpUIPixelmapEnabled()
{
    return false;
}

SubTreePrepareCheckType RSSystemProperties::GetSubTreePrepareCheckType()
{
    return SubTreePrepareCheckType::ENABLED;
}

bool RSSystemProperties::GetRenderParallelEnabled()
{
    return false;
}

bool RSSystemProperties::GetUnmarshParallelFlag()
{
    return false;
}

uint32_t RSSystemProperties::GetUnMarshParallelSize()
{
    return UINT32_MAX;
}
} // namespace Rosen
} // namespace OHOS
