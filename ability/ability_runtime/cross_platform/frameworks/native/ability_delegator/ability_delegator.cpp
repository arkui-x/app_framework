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

#include "ability_delegator.h"
#include "ability_delegator_registry.h"
#include "want.h"
#include "hilog.h"
#include "ability_context_adapter.h"
#include "application_context_adapter.h"

namespace OHOS {
namespace AppExecFwk {
AbilityDelegator::AbilityDelegator(const std::shared_ptr<AbilityRuntime::Platform::Context> &context,
    std::unique_ptr<TestRunner> runner)
    : appContext_(context), testRunner_(std::move(runner))
{}

AbilityDelegator::~AbilityDelegator()
{}

void AbilityDelegator::AddAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
}

void AbilityDelegator::AddAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
}

void AbilityDelegator::RemoveAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
}

void AbilityDelegator::ClearAllMonitors()
{
}

size_t AbilityDelegator::GetMonitorsNum()
{
    return 0;
}

size_t AbilityDelegator::GetStageMonitorsNum()
{
    return 0;
}


std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor)
{
    return {};
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    return nullptr;
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    return {};
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor, const int64_t timeoutMs)
{
    return nullptr;
}
std::shared_ptr<AbilityRuntime::Platform::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const std::string &fullname)
{
    return AbilityDelegator::AbilityState::UNINITIALIZED;
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::GetCurrentTopAbility()
{
    return {};
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
}

void AbilityDelegator::OnRun()
{
}

ErrCode AbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    return 0;
}

bool AbilityDelegator::DoAbilityForeground(const std::string &fullname)
{
    return false;
}

bool AbilityDelegator::DoAbilityBackground(const std::string &fullname)
{
    return false;
}

void AbilityDelegator::Print(const std::string &msg)
{
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformStageStart(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage)
{
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

AbilityDelegator::AbilityState AbilityDelegator::ConvertAbilityState(
    const AbilityLifecycleExecutor::LifecycleState lifecycleState)
{
    return AbilityDelegator::AbilityState::UNINITIALIZED;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AppExecFwk::AbilityDelegator::RemoveAbilityProperty(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::FindPropertyByName(const std::string &name)
{
    return {};
}

void AppExecFwk::AbilityDelegator::FinishUserTest(const std::string &msg, const int64_t resultCode)
{
}

void AppExecFwk::AbilityDelegator::RegisterClearFunc(ClearFunc func)
{
}

inline void AppExecFwk::AbilityDelegator::CallClearFunc(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}
}  // namespace AppExecFwk
}  // namespace OHOS
