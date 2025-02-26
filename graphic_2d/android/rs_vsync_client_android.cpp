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

#include <android/choreographer.h>
#include <android/looper.h>
#include <chrono>
#include <sys/prctl.h>

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
std::unique_ptr<RSVsyncClient> RSVsyncClient::Create()
{
    return std::make_unique<RSVsyncClientAndroid>();
}

RSVsyncClientAndroid::RSVsyncClientAndroid() : running_(true)
{
    auto func = std::bind(&RSVsyncClientAndroid::VsyncThreadMain, this);
    vsyncThread_ = std::make_unique<std::thread>(func);
}

RSVsyncClientAndroid::~RSVsyncClientAndroid()
{
    running_ = false;
    if (vsyncThread_) {
        if (looper_) {
            ALooper_wake(looper_);
        }
        vsyncThread_->join();
    }
}

void RSVsyncClientAndroid::OnVsync(long frameTimeNanos, void* data)
{
    auto client = static_cast<RSVsyncClientAndroid*>(data);
    if (client && client->running_) {
        client->having_ = false;
        int64_t now = static_cast<int64_t>(frameTimeNanos);
        std::unique_lock lock(client->mutex_);
        client->vsyncCallback_(now, 0);
    }
}

void RSVsyncClientAndroid::RequestNextVsync()
{
    if (!running_) {
        return;
    }
#if defined(__ANDROID_API__ ) && __ANDROID_API__ >= 24
    if (!having_ && grapher_) {
        having_ = true;
        AChoreographer_postFrameCallback(grapher_, OnVsync, this);
    } else {
        needVsyncOnce_ = true;
    }
#else
    having_ = true;
#endif
}

void RSVsyncClientAndroid::SetVsyncCallback(VsyncCallback callback)
{
    std::unique_lock lock(mutex_);
    vsyncCallback_ = callback;
}

void RSVsyncClientAndroid::VsyncThreadMain()
{
    prctl(PR_SET_NAME, "RSVsyncClientAndroid");
#if defined(__ANDROID_API__ ) && __ANDROID_API__ >= 24
    ROSEN_LOGD("VsyncThreadMain Aosp");
    looper_ = ALooper_prepare(0);
    grapher_ = AChoreographer_getInstance();
    if (needVsyncOnce_) {
        RS_LOGE("RSVsyncClientAndroid need RequestNextVsync at first time");
        RequestNextVsync();
    }
    while (running_ && looper_) {
        ALooper_pollOnce(-1, nullptr, nullptr, nullptr);
    }
#else
    ROSEN_LOGD("VsyncThreadMain Soft");
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 16ms for 60 fps vsync rate
        OnVsync(0, this);
    }
#endif
}
} // namespace Rosen
} // namespace OHOS
