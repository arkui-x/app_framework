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
#include "iability_monitor.h"

using namespace std::chrono_literals;

namespace OHOS {
namespace AppExecFwk {
IAbilityMonitor::IAbilityMonitor(const std::string &abilityName) : abilityName_(abilityName)
{}

bool IAbilityMonitor::Match(const std::shared_ptr<ADelegatorAbilityProperty> &ability, bool isNotify)
{
    if (!ability) {
        HILOG_WARN("Invalid ability property");
        return false;
    }

    const auto &aName = ability->name_;

    if (abilityName_.empty() || aName.empty()) {
        HILOG_WARN("Invalid name");
        return false;
    }

    if (abilityName_.compare(aName)) {
        HILOG_WARN("Different name");
        return false;
    }

    HILOG_INFO("Matched : ability name : %{public}s, isNotify : %{public}s",
        abilityName_.data(), (isNotify ? "true" : "false"));

    if (isNotify) {
        HILOG_INFO("Matched : notify ability matched");
        {
            std::lock_guard<std::mutex> matchLock(mMatch_);
            matchedAbility_ = ability;
        }
        cvMatch_.notify_one();
    }

    return true;
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::WaitForAbility()
{
    HILOG_INFO("WaitForAbility start");
    return WaitForAbility(MAX_TIME_OUT);
}

std::shared_ptr<ADelegatorAbilityProperty> IAbilityMonitor::WaitForAbility(const int64_t timeoutMs)
{
    HILOG_INFO("WaitForAbility timeout start");
    auto realTime = timeoutMs;
    if (timeoutMs <= 0) {
        HILOG_WARN("Timeout should be a positive number");
        realTime = MAX_TIME_OUT;
    }

    std::unique_lock<std::mutex> matchLock(mMatch_);

    auto condition = [this] { return this->matchedAbility_ != nullptr; };
    if (!cvMatch_.wait_for(matchLock, realTime * 1ms, condition)) {
        HILOG_WARN("Wait ability timeout");
    }
    
    HILOG_INFO("WaitForAbility timeout end");
    return matchedAbility_;
}

void IAbilityMonitor::OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{}

void IAbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{}
}  // namespace AppExecFwk
}  // namespace OHOS
