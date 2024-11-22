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

#ifndef OHOS_ABILITY_BASE_JSON_WANT_H
#define OHOS_ABILITY_BASE_JSON_WANT_H

#include <string>
#include <vector>

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
struct WantParamsJson {
    std::string key;
    std::string value;
    int32_t type;
};

struct JsonWant {
    std::vector<WantParamsJson> params;
};

void to_json(nlohmann::json& jsonObject, const JsonWant& jsonWant);
void from_json(const nlohmann::json& jsonObject, JsonWant& jsonWant);

void to_json(nlohmann::json& jsonObject, const WantParamsJson& wantParams);
void from_json(const nlohmann::json& jsonObject, WantParamsJson& wantParams);
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_BASE_WANT_H