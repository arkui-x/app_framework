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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_PLATFORM_EVENT_HANDLER_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_PLATFORM_EVENT_HANDLER_H

#include "event_inner_runner.h"

namespace OHOS {
namespace AppExecFwk {
class PlatformIoWaiter;

class PlatformEventRunner final : public EventInnerRunner {
public:
    static bool CheckCurrent();

    explicit PlatformEventRunner(const std::shared_ptr<EventRunner>& runner);
    ~PlatformEventRunner() final;

    void Run() final;
    void Stop() final;
    bool Init();

private:
    std::shared_ptr<PlatformIoWaiter> ioWaiter_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_PLATFORM_EVENT_HANDLER_H
