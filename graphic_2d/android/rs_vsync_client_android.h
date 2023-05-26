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
#ifndef RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_ANDROID_H
#define RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_ANDROID_H

#include "platform/drawing/rs_vsync_client.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

struct ALooper;
struct AChoreographer;

namespace OHOS {
namespace Rosen {
class RSVsyncClientAndroid : public RSVsyncClient {
public:
    RSVsyncClientAndroid();
    ~RSVsyncClientAndroid() override;

    void RequestNextVsync() override;
    void SetVsyncCallback(VsyncCallback callback) override;
    static void OnVsync(long frameTimeNanos, void* data);

private:
    void VsyncThreadMain();

    VsyncCallback vsyncCallback_ = nullptr;
    std::unique_ptr<std::thread> vsyncThread_ = nullptr;
    std::atomic<bool> running_ {false};
    std::atomic<bool> having_ {false};
    ALooper* looper_ = nullptr;
    AChoreographer* grapher_ = nullptr;
    std::mutex mutex_;
    bool needVsyncOnce_ = false;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_ANDROID_H
