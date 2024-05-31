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

#include "ability_monitor.h"

#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "napi/native_common.h"

namespace OHOS {
namespace AbilityDelegatorJs {
using namespace OHOS::AbilityRuntime;
AbilityMonitor::AbilityMonitor(const std::string &name, const std::shared_ptr<JSAbilityMonitor> &jsAbilityMonitor)
    : IAbilityMonitor(name), jsMonitor_(jsAbilityMonitor)
{}

void AbilityMonitor::OnAbilityStart(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnAbilityCreate(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnAbilityForeground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnAbilityForeground(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnAbilityBackground(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnAbilityBackground(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnAbilityStop(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnAbilityDestroy(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnWindowStageCreate(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnWindowStageCreate(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnWindowStageRestore(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnWindowStageRestore(abilityObj);

    HILOG_INFO("exit");
}

void AbilityMonitor::OnWindowStageDestroy(const std::weak_ptr<NativeReference> &abilityObj)
{
    HILOG_INFO("enter");

    if (jsMonitor_ == nullptr) {
        return;
    }

    jsMonitor_->OnWindowStageDestroy(abilityObj);

    HILOG_INFO("exit");
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
