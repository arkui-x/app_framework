/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "configuration.h"

#include "hilog.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
Configuration::Configuration() {}

void Configuration::UpdateConfigurationInfo(const Configuration& config)
{
    configParameter_ = config.configParameter_;
}

std::string Configuration::GetItem(const std::string& key) const
{
    if (key.empty()) {
        return ConfigurationInner::EMPTY_STRING;
    }

    auto iter = configParameter_.find(key);
    if (iter != configParameter_.end()) {
        return iter->second;
    }

    return ConfigurationInner::EMPTY_STRING;
}

void Configuration::ReadFromJsonConfiguration(const std::string& jsonConfiguration)
{
    configParameter_ = ConvertJsonStrToUnorderedMap(jsonConfiguration);
}

std::unordered_map<std::string, std::string> Configuration::ConvertJsonStrToUnorderedMap(const std::string& json)
{
    return nlohmann::json::parse(json.c_str()).get<std::unordered_map<std::string, std::string>>();
}

Global::Resource::ColorMode Configuration::ConvertColorMode(std::string colormode) const
{
    auto resolution = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ColorMode>> resolutions = {
        { "dark", Global::Resource::ColorMode::DARK },
        { "light", Global::Resource::ColorMode::LIGHT },
    };

    for (const auto& [tempColorMode, value] : resolutions) {
        if (tempColorMode == colormode) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::Direction Configuration::ConvertDirection(std::string direction) const
{
    auto resolution = Global::Resource::Direction::DIRECTION_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::Direction>> resolutions = {
        { "vertical", Global::Resource::Direction::DIRECTION_VERTICAL },
        { "horizontal", Global::Resource::Direction::DIRECTION_HORIZONTAL },
    };

    for (const auto& [tempDirection, value] : resolutions) {
        if (tempDirection == direction) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

uint32_t Configuration::ConvertDensity(const std::string& density) const
{
    if (density.empty()) {
        return 0;
    } else {
        return static_cast<int32_t>(std::stoi(density));
    }
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
