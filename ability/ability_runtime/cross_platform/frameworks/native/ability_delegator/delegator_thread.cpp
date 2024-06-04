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

#include "delegator_thread.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
DelegatorThread::DelegatorThread(bool isMain)
{
    if (isMain) {
        runner_ = EventRunner::Current();
    } else {
        runner_ = EventRunner::Create();
    }

    handler_ = std::make_shared<EventHandler>(runner_);
}

bool DelegatorThread::Run(const DTask &task)
{
    if (!task) {
        RESMGR_HILOGW(RESMGR_TAG, "Invalid input parameter");
        return false;
    }

    if (!handler_) {
        RESMGR_HILOGW(RESMGR_TAG, "Invalid EventHandler");
        return false;
    }

    return handler_->PostTask(task);
}

std::string DelegatorThread::GetThreadName() const
{
    return threadName_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
