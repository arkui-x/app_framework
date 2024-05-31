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
#ifndef RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_IOS_H
#define RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_IOS_H

#include "platform/drawing/rs_vsync_client.h"

#include <atomic>
#include <condition_variable>
#include <mutex>

#ifdef __OBJC__
@class RSVsyncIOS;
#else
typedef struct objc_object RSVsyncIOS;
#endif
namespace OHOS {
namespace Rosen {
class RSVsyncClientIOS : public RSVsyncClient {
public:
    RSVsyncClientIOS();
    ~RSVsyncClientIOS() override;

    void RequestNextVsync() override;
    void SetVsyncCallback(VsyncCallback callback) override;

private:
    RSVsyncIOS* vsyncIOS_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS

#endif // RENDER_SERVICE_BASE_ADAPTER_RS_VSYNC_CLIENT_IOS_H
