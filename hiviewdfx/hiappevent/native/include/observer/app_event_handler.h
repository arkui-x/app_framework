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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_HANDLER_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_HANDLER_H

#include "event_handler.h"

namespace OHOS {
namespace HiviewDFX {
namespace AppEventType {
const uint32_t WATCHER_TIMEOUT = 0;
}

class AppEventHandler : public AppExecFwk::EventHandler {
public:
    explicit AppEventHandler(const std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~AppEventHandler() override;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_OBSERVER_APP_EVENT_HANDLER_H
