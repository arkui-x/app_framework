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

#include "rs_vsync_client_ios.h"

#include <chrono>

#include "platform/common/rs_log.h"

namespace OHOS {
namespace Rosen {
std::unique_ptr<RSVsyncClient> RSVsyncClient::Create()
{
    return std::make_unique<RSVsyncClientIOS>();
}

RSVsyncClientIOS::~RSVsyncClientIOS()
{
    running_ = false;
    if (vsyncThread_) {
        vsyncThread_->join();
    }
}

void RSVsyncClientIOS::RequestNextVsync()
{
    if (vsyncThread_ == nullptr) {
        running_ = true;
        auto func = std::bind(&RSVsyncClientIOS::VsyncThreadMain, this);
        vsyncThread_ = std::make_unique<std::thread>(func);
    }

    having_ = true;
}

void RSVsyncClientIOS::SetVsyncCallback(VsyncCallback callback)
{
    std::unique_lock lock(mutex_);
    vsyncCallback_ = callback;
}

void RSVsyncClientIOS::VsyncThreadMain()
{
    ROSEN_LOGI("RSVsyncClientIOS");
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // wait for 16 ms
        int64_t now = std::chrono::duration_cast<std::chrono::nanoseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        if (having_.load()) {
            having_ = false;
            VsyncCallback vsyncCallbackTmp = nullptr;
            {
                std::unique_lock lock(mutex_);
                vsyncCallbackTmp = vsyncCallback_;
            }
            if (vsyncCallbackTmp) {
                vsyncCallbackTmp(now);
            }
        }
    }
}
} // namespace Rosen
} // namespace OHOS