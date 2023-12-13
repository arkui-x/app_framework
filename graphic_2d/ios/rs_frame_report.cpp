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

#include "pipeline/rs_frame_report.h"

namespace OHOS::Rosen {
RsFrameReport& RsFrameReport::GetInstance()
{
    static RsFrameReport instance;
    return instance;
}

RsFrameReport::RsFrameReport()
{
}

RsFrameReport::~RsFrameReport()
{
}

void RsFrameReport::Init()
{
}

int RsFrameReport::GetEnable()
{
    return 0;
}

void RsFrameReport::ProcessCommandsStart()
{
}

void RsFrameReport::AnimateStart()
{
}

void RsFrameReport::RenderStart()
{
}

void RsFrameReport::SendCommandsStart()
{
}
} // namespace OHOS::Rosen
