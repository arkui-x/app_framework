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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_CONFIGURATION_CONFIGURATION_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_CONFIGURATION_CONFIGURATION_H

#include <string>
#include <unordered_map>

#include "res_common.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace ConfigurationInner {
constexpr const char* EMPTY_STRING = "";
constexpr const char* SYSTEM_COLORMODE = "ohos.system.colorMode";
constexpr const char* APPLICATION_DIRECTION = "ohos.application.direction";
constexpr const char* APPLICATION_DENSITYDPI = "ohos.application.densitydpi";
constexpr const char* APPLICATION_LANGUAGE = "ohos.system.language";
constexpr const char* DEVICE_TYPE = "const.build.characteristics";
constexpr const char* COLOR_MODE_LIGHT = "light";
constexpr const char* COLOR_MODE_DARK = "dark";
constexpr const char* DIRECTION_VERTICAL = "vertical";
constexpr const char* DIRECTION_HORIZONTAL = "horizontal";
constexpr const char* DEVICE_TYPE_PHONE = "Phone";
constexpr const char* DEVICE_TYPE_TABLET = "Tablet";
}; // namespace ConfigurationInner

class Configuration {
public:
    Configuration();
    void UpdateConfigurationInfo(const Configuration& other);
    std::string GetItem(const std::string& key) const;
    void ReadFromJsonConfiguration(const std::string& jsonConfiguration);
    Global::Resource::ColorMode ConvertColorMode(std::string colormode) const;
    Global::Resource::Direction ConvertDirection(std::string direction) const;

private:
    std::unordered_map<std::string, std::string> ConvertJsonStrToUnorderedMap(const std::string& json);
    std::unordered_map<std::string, std::string> configParameter_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_CONFIGURATION_CONFIGURATION_H
