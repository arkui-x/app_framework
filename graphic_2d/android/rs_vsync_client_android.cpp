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

#include "rs_vsync_client_android.h"

#include <chrono>
#include <android/choreographer.h>
#include <android/looper.h>
#include <sys/prctl.h>

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
std::unique_ptr<RSVsyncClient> RSVsyncClient::Create()
{
    return std::make_unique<RSVsyncClientAndroid>();
}

RSVsyncClientAndroid::RSVsyncClientAndroid()
{
    running_ = true;
    auto func = std::bind(&RSVsyncClientAndroid::VsyncThreadMain, this);
    vsyncThread_ = std::make_unique<std::thread>(func);
}

RSVsyncClientAndroid::~RSVsyncClientAndroid()
{
    running_ = false;
    if (vsyncThread_) {
        vsyncThread_->join();
    }
}

void RSVsyncClientAndroid::OnVsync(int64_t frameTimeNanos, void* data)
{
    auto client = static_cast<RSVsyncClientAndroid*>(data);
    if (client) {
        client->having_ = false;
        client->vsyncCallback_(frameTimeNanos);
    }
}

void RSVsyncClientAndroid::RequestNextVsync()
{
    if (!having_ && grapher_) {
        AChoreographer_postFrameCallbackDelayed64(grapher_, OnVsync, this, 0);
        having_ = true;
    }
}

void RSVsyncClientAndroid::SetVsyncCallback(VsyncCallback callback)
{
    vsyncCallback_ = callback;
}

void RSVsyncClientAndroid::VsyncThreadMain()
{
    prctl(PR_SET_NAME, "RSVsyncClientAndroid");
    looper_ = ALooper_prepare(0);
    grapher_ = AChoreographer_getInstance();
    int id;
    while (running_ && looper_) {
        ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
    }
}
} // namespace Rosen
} // namespace OHOS
