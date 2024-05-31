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

#include "ability_delegator/ability_delegator_registry.h"
#include "base/log/log.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"

namespace OHOS::Ace::Platform {
void UIContent::ShowDumpHelp(std::vector<std::string>& info)
{
    info.emplace_back(" -element                       |show element tree");
    info.emplace_back(" -render                        |show render tree");
    info.emplace_back(" -inspector                     |show inspector tree");
    info.emplace_back(" -frontend                      |show path and components count of current page");
}

void UIContent::AddUIContent(int32_t instanceId, UIContent* content)
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator != nullptr) {
        delegator->AddUIContent(instanceId, content);
    } else {
        LOGE("UIContent::AddUIContent Delegator is null, delegator is not initialized");
    }
}

void UIContent::RemoveUIContent(int32_t instanceId)
{
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator != nullptr) {
        delegator->RemoveUIContent(instanceId);
    } else {
        LOGE("UIContent::RemoveUIContent Delegator is null, delegator is not initialized");
    }
}
} // namespace OHOS::Ace::Platform 