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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_APPLICATION_CONFIGURATION_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_APPLICATION_CONFIGURATION_MANAGER_H

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
enum class SetLevel : uint8_t {
    // system level, only color mode have SA level
    System,
    SA,
    // application level
    Application,
    SetLevelCount,
};

class ApplicationConfigurationManager {
    ApplicationConfigurationManager() = default;
    ~ApplicationConfigurationManager() = default;
public:
    static ApplicationConfigurationManager& GetInstance();
    std::string SetColorModeSetLevel(SetLevel colorModeSetLevel, const std::string &value);
    SetLevel GetColorModeSetLevel() const;
    bool ColorModeHasSetByApplication() const;
    void SetFontSizeSetLevel(SetLevel fontSizeSetLevel);
    SetLevel GetFontSizeSetLevel() const;
    void SetLanguageSetLevel(SetLevel languageSetLevel);
    SetLevel GetLanguageSetLevel() const;

private:
    std::mutex mtx;
    SetLevel colorModeSetLevel_ = SetLevel::System;
    SetLevel fontSizeSetLevel_ = SetLevel::System;
    SetLevel languageSetLevel_ = SetLevel::System;
    std::vector<std::string> colorModeVal_ = std::vector<std::string>(static_cast<uint8_t>(SetLevel::SetLevelCount));
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_ABILITY_APPLICATION_CONFIGURATION_MANAGER_H