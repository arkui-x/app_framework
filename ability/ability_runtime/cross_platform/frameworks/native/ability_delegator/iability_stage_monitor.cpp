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

#include "hilog.h"
#include "iability_stage_monitor.h"

namespace OHOS {
namespace AppExecFwk {
IAbilityStageMonitor::IAbilityStageMonitor(const std::string &moduleName, const std::string &srcEntrance)
    : moduleName_(moduleName), srcEntrance_(srcEntrance)
{}

bool IAbilityStageMonitor::Match(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage, bool isNotify)
{
    return false;
}

std::shared_ptr<DelegatorAbilityStageProperty> IAbilityStageMonitor::WaitForAbilityStage()
{
    return WaitForAbilityStage(MAX_TIME_OUT);
}

std::shared_ptr<DelegatorAbilityStageProperty> IAbilityStageMonitor::WaitForAbilityStage(const int64_t timeoutMs)
{
    return matchedAbilityStage_;
}
}  // namespace AppExecFwk
}  // namespace OHOS
