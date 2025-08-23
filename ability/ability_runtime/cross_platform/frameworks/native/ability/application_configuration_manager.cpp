/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "application_configuration_manager.h"

#include <string>
#include <vector>

#include "configuration.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

bool operator<(SetLevel lhs, SetLevel rhs)
{
    return static_cast<uint8_t>(lhs) < static_cast<uint8_t>(rhs);
};

bool operator>(SetLevel lhs, SetLevel rhs)
{
    return static_cast<uint8_t>(lhs) > static_cast<uint8_t>(rhs);
};

ApplicationConfigurationManager& ApplicationConfigurationManager::GetInstance()
{
    static ApplicationConfigurationManager instance;
    return instance;
}

std::string ApplicationConfigurationManager::SetColorModeSetLevel(SetLevel colorModeSetLevel, const std::string& value)
{
    std::lock_guard<std::mutex> lock(mtx);
    colorModeVal_[static_cast<uint8_t>(colorModeSetLevel)] = value;
    for (int i = static_cast<int>(SetLevel::SetLevelCount) - 1; i >= 0; i--) {
        if (!colorModeVal_[i].empty() && colorModeVal_[i].compare(ConfigurationInner::COLOR_MODE_AUTO) != 0) {
            colorModeSetLevel_ = static_cast<SetLevel>(i);
            break;
        }
    }

    return colorModeVal_[static_cast<uint8_t>(colorModeSetLevel_)];
}

SetLevel ApplicationConfigurationManager::GetColorModeSetLevel() const
{
    return colorModeSetLevel_;
}

bool ApplicationConfigurationManager::ColorModeHasSetByApplication() const
{
    return !colorModeVal_[static_cast<uint8_t>(SetLevel::Application)].empty();
}

void ApplicationConfigurationManager::SetFontSizeSetLevel(SetLevel fontSizeSetLevel)
{
    fontSizeSetLevel_ = fontSizeSetLevel;
}

SetLevel ApplicationConfigurationManager::GetFontSizeSetLevel() const
{
    return fontSizeSetLevel_;
}

void ApplicationConfigurationManager::SetLanguageSetLevel(SetLevel languageSetLevel)
{
    languageSetLevel_ = languageSetLevel;
}

SetLevel ApplicationConfigurationManager::GetLanguageSetLevel() const
{
    return languageSetLevel_;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS