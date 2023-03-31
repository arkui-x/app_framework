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

#include "ability.h"

#include "ability_context_adapter.h"
#include "hilog.h"
#include "js_ability.h"
#include "runtime.h"
#include "window_view_adapter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
std::shared_ptr<Ability> Ability::Create(const std::unique_ptr<AbilityRuntime::Runtime>& runtime)
{
    HILOG_INFO("Ability create.");
    if (!runtime) {
        return std::make_shared<Ability>();
    }

    switch (runtime->GetLanguage()) {
        case AbilityRuntime::Runtime::Language::JS:
            return AbilityRuntime::Platform::JsAbility::Create(runtime);

        default:
            return std::make_shared<Ability>();
    }
}

void Ability::Init(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    HILOG_INFO("Init begin.");
}

void Ability::OnCreate(const Want& want)
{
    HILOG_INFO("OnCreate begin.");
    SetWant(want);
}

void Ability::OnNewWant(const Want& want)
{
    HILOG_INFO("OnNewWant begin.");
}

void Ability::OnForeground(const Want& want)
{
    HILOG_INFO("OnForeground begin.");
}

void Ability::OnBackground()
{
    HILOG_INFO("OnBackground begin.");
}

void Ability::OnDestory()
{
    HILOG_INFO("OnDestory begin.");
    AbilityContextAdapter::GetInstance()->RemoveStageActivity(instanceName_);
}

void Ability::OnWindowStageCreated()
{
    HILOG_INFO("OnWindowStageCreated begin.");
}

void Ability::OnWindowStageDestroy()
{
    HILOG_INFO("onWindowStageDestroy begin.");
    WindowViewAdapter::GetInstance()->RemoveWindowView(instanceName_);
}

void Ability::SetWant(const AAFwk::Want& want)
{
    want_ = std::make_shared<AAFwk::Want>(want);
}

std::shared_ptr<AAFwk::Want> Ability::GetWant()
{
    return want_;
}

void Ability::OnConfigurationUpdate(const Configuration& configuration)
{
    HILOG_INFO("OnConfigurationUpdate begin.");
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS