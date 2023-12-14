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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EVENT_INNER_RUNNER_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EVENT_INNER_RUNNER_H

#include "event_handler_utils.h"
#include "event_queue.h"
#include "event_runner.h"
#include "thread_local_data.h"

namespace OHOS {
namespace AppExecFwk {
class EventInnerRunner {
public:
    explicit EventInnerRunner(const std::shared_ptr<EventRunner>& runner);
    virtual ~EventInnerRunner() = default;

    static std::shared_ptr<EventRunner> GetCurrentEventRunner();

    virtual void Run() = 0;
    virtual void Stop() = 0;

    const std::shared_ptr<EventQueue>& GetEventQueue() const
    {
        return queue_;
    }

    void SetLogger(const std::shared_ptr<Logger>& logger)
    {
        logger_ = logger;
    }

    const std::string& GetThreadName()
    {
        return threadName_;
    }

    const std::thread::id& GetThreadId()
    {
        return threadId_;
    }

protected:
    std::shared_ptr<EventQueue> queue_;
    std::weak_ptr<EventRunner> owner_;
    std::shared_ptr<Logger> logger_;
    static ThreadLocalData<std::weak_ptr<EventRunner>> currentEventRunner;
    std::string threadName_;
    std::thread::id threadId_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_EVENT_INNER_RUNNER_H
