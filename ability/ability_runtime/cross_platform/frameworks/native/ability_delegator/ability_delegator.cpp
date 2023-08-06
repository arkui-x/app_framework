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
#include "ability_manager_errors.h"
#include "want.h"
#include "hilog.h"
#include "ability_context_adapter.h"
#include "application_context_adapter.h"
#include "base/utils/string_utils.h"

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
    HILOG_INFO("AddAbilityMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        HILOG_WARN("Monitor has been added");
        return;
    }

    HILOG_DEBUG("before AddAbilityMonitor count: %{public}zu", abilityMonitors_.size());
    abilityMonitors_.emplace_back(monitor);
    HILOG_DEBUG("after AddAbilityMonitor count: %{public}zu", abilityMonitors_.size());
}

void AbilityDelegator::AddAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    HILOG_INFO("AddAbilityStageMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return;
    }
	
    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
        HILOG_WARN("Stage monitor has been added");
        return;
    }

    HILOG_DEBUG("before AddAbilityStageMonitor count: %{public}zu", abilityStageMonitors_.size());
    abilityStageMonitors_.emplace_back(monitor);
    HILOG_DEBUG("after AddAbilityStageMonitor count: %{public}zu", abilityStageMonitors_.size());
}

void AbilityDelegator::RemoveAbilityMonitor(const std::shared_ptr<IAbilityMonitor> &monitor)
{
    HILOG_INFO("RemoveAbilityMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    auto pos = std::find(abilityMonitors_.begin(), abilityMonitors_.end(), monitor);
    if (pos != abilityMonitors_.end()) {
        abilityMonitors_.erase(pos);
    }

    HILOG_DEBUG("RemoveAbilityMonitor count: %{public}zu", abilityMonitors_.size());
}

void AbilityDelegator::RemoveAbilityStageMonitor(const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    HILOG_INFO("RemoveAbilityStageMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    auto pos = std::find(abilityStageMonitors_.begin(), abilityStageMonitors_.end(), monitor);
    if (pos != abilityStageMonitors_.end()) {
        abilityStageMonitors_.erase(pos);
    }

    HILOG_DEBUG("RemoveAbilityStageMonitor count: %{public}zu", abilityStageMonitors_.size());
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
    HILOG_INFO("WaitAbilityMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return nullptr;
    }

    AddAbilityMonitor(monitor);
    auto obtainedAbility = monitor->WaitForAbility();
    if (!obtainedAbility) {
        HILOG_WARN("Invalid obtained ability");
        return nullptr;
    }

    HILOG_INFO("WaitAbilityMonitor end");
    return obtainedAbility;
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor)
{
    HILOG_INFO("WaitAbilityStageMonitor start");
    if (!monitor) {
        HILOG_ERROR("Invalid monitor for waiting abilityStage");
        return nullptr;
    }

    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage();
    if (!stage) {
        HILOG_WARN("Invalid obtained abilityStage");
        return nullptr;
    }

    HILOG_INFO("WaitAbilityStageMonitor end");
    return stage;
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::WaitAbilityMonitor(
    const std::shared_ptr<IAbilityMonitor> &monitor, const int64_t timeoutMs)
{
    HILOG_INFO("WaitAbilityMonitor timeout start");
    if (!monitor) {
        HILOG_ERROR("Invalid input parameter");
        return nullptr;
    }

    AddAbilityMonitor(monitor);
    auto obtainedAbility = monitor->WaitForAbility(timeoutMs);
    if (!obtainedAbility) {
        HILOG_WARN("Invalid obtained ability");
        return nullptr;
    }

    HILOG_INFO("WaitAbilityMonitor timeout end");
    return obtainedAbility;
}

std::shared_ptr<DelegatorAbilityStageProperty> AbilityDelegator::WaitAbilityStageMonitor(
    const std::shared_ptr<IAbilityStageMonitor> &monitor, const int64_t timeoutMs)
{
    HILOG_INFO("WaitAbilityStageMonitor timeout start");
    if (!monitor) {
        HILOG_ERROR("Invalid monitor for waiting abilityStage with timeout");
        return nullptr;
    }

    AddAbilityStageMonitor(monitor);
    auto stage = monitor->WaitForAbilityStage(timeoutMs);
    if (!stage) {
        HILOG_WARN("Invalid obtained abilityStage");
        return nullptr;
    }

    HILOG_INFO("WaitAbilityStageMonitor timeout end");
    return stage;
}
std::shared_ptr<AbilityRuntime::Platform::Context> AbilityDelegator::GetAppContext() const
{
    return appContext_;
}

AbilityDelegator::AbilityState AbilityDelegator::GetAbilityState(const std::string &fullName)
{
    HILOG_INFO("AbilityDelegator::GetAbilityState");
    if (fullName == "") {
        HILOG_WARN("Invalid input parameter");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByName(fullName);
    if (!existedProperty) {
        HILOG_WARN("Unknown ability token");
        return AbilityDelegator::AbilityState::UNINITIALIZED;
    }

    return ConvertAbilityState(existedProperty->lifecycleState_);
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::GetCurrentTopAbility()
{
    HILOG_INFO("GetCurrentTopAbility Enter");
    std::string abilityName;
#ifdef ANDROID_PLATFORM
    abilityName = AbilityRuntime::Platform::ApplicationContextAdapter::GetInstance()->GetTopAbility();
#else
    abilityName = AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->GetTopAbility();
#endif
    if (abilityName.empty()) {
        HILOG_ERROR("Failed to get top ability");
        return {};
    }

    std::vector<std::string> vecList;
    Ace::StringUtils::StringSplitter(abilityName, ':', vecList);
    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByName(abilityName);
    if (!existedProperty) {
        HILOG_WARN("Unknown ability name");
        return {};
    }

    existedProperty->instanceId_ = std::stoi(vecList.back());

    return existedProperty;
}

std::string AbilityDelegator::GetThreadName() const
{
    return {};
}

void AbilityDelegator::Prepare()
{
    HILOG_INFO("AbilityDelegator Prepare enter.");
    if (!testRunner_) {
        HILOG_WARN("Invalid TestRunner");
        return;
    }

    HILOG_INFO("Call TestRunner::Prepare()");
    testRunner_->Prepare();

    if (!delegatorThread_) {
        delegatorThread_ = std::make_unique<DelegatorThread>(true);
        if (!delegatorThread_) {
            HILOG_ERROR("Create delegatorThread failed");
            return;
        }
    }

    auto runTask = [this]() { this->OnRun(); };
    if (!delegatorThread_->Run(runTask)) {
        HILOG_ERROR("Run task on delegatorThread failed");
    }
}

void AbilityDelegator::OnRun()
{
    HILOG_INFO("Enter");
    if (!testRunner_) {
        HILOG_WARN("Invalid TestRunner");
        return;
    }

    HILOG_INFO("Call TestRunner::Run(), Start run");
    testRunner_->Run();
    HILOG_INFO("Run finished");
}

ErrCode AbilityDelegator::StartAbility(const AAFwk::Want &want)
{
    HILOG_INFO("Enter");

    std::string bundleName = want.GetBundleName();
    std::string abilityName = want.GetAbilityName();
    std::string moduleName = want.GetModuleName();

    HILOG_INFO("AbilityDelegator: bundleName is %{public}s, abilityName is %{public}s, moduleName is %{public}s",
        bundleName.c_str(), abilityName.c_str(), moduleName.c_str());
   
    if (want.IsEmpty()) {
        return AAFwk::ERR_IMPLICIT_START_ABILITY_FAIL;
    }
    if (bundleName.empty()) {
        return AAFwk::RESOLVE_ABILITY_ERR;
    }
    if (abilityName.empty()) {
        return AAFwk::ERR_IMPLICIT_START_ABILITY_FAIL;
    }
   
#ifdef ANDROID_PLATFORM
    return AbilityRuntime::Platform::ApplicationContextAdapter::GetInstance()->StartAbility(want);
#else
    return AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->StartAbility("", want);
#endif
}

bool AbilityDelegator::DoAbilityForeground(const std::string &fullName)
{
    HILOG_INFO("DoAbilityForeground Enter");
    if (fullName == "") {
        return false;
    }

    auto ret = AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->DoAbilityForeground(fullName);
    if (ret) {
        HILOG_ERROR("Failed to call DelegatorDoAbilityForeground, reson : %{public}d", ret);
        return false;
    }

    return true;
}

bool AbilityDelegator::DoAbilityBackground(const std::string &fullName)
{
    if (fullName.empty()) {
        return false;
    }

    auto ret = AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->DoAbilityBackground(fullName);
    if (ret) {
        HILOG_ERROR("Failed to call DelegatorDoAbilityBackground, reson : %{public}d", ret);
        return false;
    }
    return true;
}

void AbilityDelegator::Print(const std::string &msg)
{
#ifdef ANDROID_PLATFORM
    AbilityRuntime::Platform::ApplicationContextAdapter::GetInstance()->Print(msg);
#else
    AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->print(msg);
#endif
    return;
}

void AbilityDelegator::PostPerformStart(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter AbilityDelegator::PostPerformStart");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability, true)) {
            monitor->OnAbilityStart(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformStageStart(const std::shared_ptr<DelegatorAbilityStageProperty> &abilityStage)
{
    HILOG_INFO("Enter");
    if (!abilityStage) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    std::unique_lock<std::mutex> lck(mutexStageMonitor_);
    if (abilityStageMonitors_.empty()) {
        HILOG_WARN("Empty abilityStageMonitors");
        return;
    }

    for (const auto &monitor : abilityStageMonitors_) {
        if (!monitor) {
            continue;
        }
        monitor->Match(abilityStage, true);
    }
}

void AbilityDelegator::PostPerformScenceCreated(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter PostPerformScenceCreated");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageCreate(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformScenceRestored(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
}

void AbilityDelegator::PostPerformScenceDestroyed(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter PostPerformScenceDestroyed");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnWindowStageDestroy(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformForeground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter PostPerformForeground");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityForeground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformBackground(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter PostPerformBackground");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityBackground(ability->object_);
        }
    }
}

void AbilityDelegator::PostPerformStop(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("Enter PostPerformStop");

    if (!ability) {
        HILOG_WARN("Invalid input parameter");
        return;
    }

    ProcessAbilityProperties(ability);

    std::unique_lock<std::mutex> lck(mutexMonitor_);
    if (abilityMonitors_.empty()) {
        HILOG_WARN("Empty abilityMonitors");
        return;
    }

    for (const auto &monitor : abilityMonitors_) {
        if (!monitor) {
            continue;
        }

        if (monitor->Match(ability)) {
            monitor->OnAbilityStop(ability->object_);
        }
    }

    RemoveAbilityProperty(ability);
    CallClearFunc(ability);
}

AbilityDelegator::AbilityState AbilityDelegator::ConvertAbilityState(
    const AbilityLifecycleExecutor::LifecycleState lifecycleState)
{
    HILOG_WARN("AbilityDelegator::ConvertAbilityState");
    AbilityDelegator::AbilityState abilityState {AbilityDelegator::AbilityState::UNINITIALIZED};
    switch (lifecycleState) {
        case AbilityLifecycleExecutor::LifecycleState::STARTED_NEW:
            abilityState = AbilityDelegator::AbilityState::STARTED;
            break;
        case AbilityLifecycleExecutor::LifecycleState::FOREGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::FOREGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::BACKGROUND_NEW:
            abilityState = AbilityDelegator::AbilityState::BACKGROUND;
            break;
        case AbilityLifecycleExecutor::LifecycleState::STOPED_NEW:
            abilityState = AbilityDelegator::AbilityState::STOPPED;
            break;
        default:
            HILOG_ERROR("Unknown lifecycleState");
            break;
    }

    return abilityState;
}

void AbilityDelegator::ProcessAbilityProperties(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_WARN("AbilityDelegator::ProcessAbilityProperties");

    if (!ability) {
        HILOG_WARN("Invalid ability property");
        return;
    }

    HILOG_WARN("ability property : name : %{public}s, state : %{public}d",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    auto existedProperty = FindPropertyByName(ability->fullName_);
    if (existedProperty) {
        // update
        existedProperty->lifecycleState_ = ability->lifecycleState_;
        return;
    }

    abilityProperties_.emplace_back(ability);
}

void AppExecFwk::AbilityDelegator::RemoveAbilityProperty(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("RemoveAbilityProperty Enter");

    if (!ability) {
        HILOG_WARN("Invalid ability property");
        return;
    }

    HILOG_INFO("ability property { name : %{public}s, state : %{public}d }",
        ability->name_.data(), ability->lifecycleState_);

    std::unique_lock<std::mutex> lck(mutexAbilityProperties_);
    abilityProperties_.remove_if([ability](const auto &properties) {
        return ability->fullName_ == properties->fullName_;
    });
}

std::shared_ptr<ADelegatorAbilityProperty> AbilityDelegator::FindPropertyByName(const std::string &name)
{
    if (name.empty()) {
        HILOG_WARN("Invalid input parameter");
        return {};
    }

    for (const auto &it : abilityProperties_) {
        if (!it) {
            HILOG_WARN("Invalid ability property");
            continue;
        }
        if (name == it->fullName_) {
            HILOG_INFO("Property exists");
            return it;
        }
    }
    return {};
}

void AppExecFwk::AbilityDelegator::FinishUserTest(const std::string &msg, const int64_t resultCode)
{
    HILOG_INFO("FinishUserTest start");
    auto realMsg(msg);
    if (realMsg.length() > INFORMATION_MAX_LENGTH) {
        HILOG_WARN("Too long message");
        realMsg.resize(INFORMATION_MAX_LENGTH);
        return;
    }

    HILOG_INFO("TestFinished-ResultMsg: %{public}s", msg.c_str());
    HILOG_INFO("TestFinished-ResultCode: %{public}d", resultCode);
#ifdef ANDROID_PLATFORM
    auto err = AbilityRuntime::Platform::ApplicationContextAdapter::GetInstance()->FinishUserTest();
#else
    auto err = AbilityRuntime::Platform::AbilityContextAdapter::GetInstance()->FinishUserTest();
#endif
    if (err) {
        HILOG_ERROR("Failed to call FinishUserTest : %{public}d", err);
        return;
    }
    HILOG_INFO("FinishUserTest end");
}

void AppExecFwk::AbilityDelegator::RegisterClearFunc(ClearFunc func)
{
    HILOG_INFO("RegisterClearFunc Enter");
    if (!func) {
        HILOG_ERROR("Invalid func");
        return;
    }

    clearFunc_ = func;
}

inline void AppExecFwk::AbilityDelegator::CallClearFunc(const std::shared_ptr<ADelegatorAbilityProperty> &ability)
{
    HILOG_INFO("CallClearFunc Enter");
    if (clearFunc_) {
        clearFunc_(ability);
    }
}

Ace::Platform::UIContent* AbilityDelegator::GetUIContent(int32_t instanceId)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    auto uicontent = contentMap_.find(instanceId);
    if (uicontent != contentMap_.end()) {
        LOGI("AbilityDelegator-GetUIContent instance is : %p", uicontent->second);
        return uicontent->second;
    } else {
        LOGI("AbilityDelegator-GetUIContent instance is null");
        return nullptr;
    }
}

void AbilityDelegator::AddUIContent(int32_t instanceId, Ace::Platform::UIContent* content)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    const auto result = contentMap_.try_emplace(instanceId, content);
    if (!result.second) {
        LOGW("AbilityDelegator::AddUIContent already have uicontent of this instance id: %{public}d", instanceId);
    } else {
        LOGI("AbilityDelegator::AddUIContent %{public}d   %p", instanceId, content);
    }
}

void AbilityDelegator::RemoveUIContent(int32_t instanceId)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    size_t num = contentMap_.erase(instanceId);
    if (num == 0) {
        LOGW("AbilityDelegator-uicontent not found with instance id: %{public}d", instanceId);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
