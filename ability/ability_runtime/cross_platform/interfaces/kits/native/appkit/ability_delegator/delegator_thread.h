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

#ifndef OHOS_ABILITY_RUNTIME_DELEGATOR_THREAD_H
#define OHOS_ABILITY_RUNTIME_DELEGATOR_THREAD_H

#include <functional>
#include <memory>
#include <string>
#include "event_handler.h"
#include "event_runner.h"

namespace OHOS {
namespace AppExecFwk {
class DelegatorThread {
public:
    /**
     * Definition of thread task.
     */
    using DTask = std::function<void()>;

public:
    /**
     * A constructor used to create a DelegatorThread instance with the input parameter passed.
     *
     * @param isMain Indicates whether to use main thread.
     */
    explicit DelegatorThread(bool isMain = false);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~DelegatorThread() = default;

    /**
     * Runs specified task.
     *
     * @param task, Indicates the specified task.
     * @return true if succeed, false otherwise.
     */
    bool Run(const DTask &task);

    /**
     * Obtains the name of the thread.
     *
     * @return the name of the thread.
     */
    std::string GetThreadName() const;

private:
    std::string threadName_;
    std::shared_ptr<EventRunner> runner_;
    std::shared_ptr<EventHandler> handler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_DELEGATOR_THREAD_H
