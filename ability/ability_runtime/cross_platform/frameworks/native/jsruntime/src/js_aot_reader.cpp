/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_aot_reader.h"

#include "hilog.h"
#include "stage_asset_manager.h"
namespace OHOS {
namespace AbilityRuntime {
bool JsAotReader::operator()(const std::string &fileName, uint8_t **buff, size_t *buffSize)
{
    HILOG_INFO("Called, fileName: %{public}s.", fileName.c_str());
    if (fileName.empty() || buff == nullptr || buffSize == nullptr) {
        HILOG_ERROR("Invalid param.");
        return false;
    }
    aotBuffer_.clear();
    aotBuffer_ = Platform::StageAssetManager::GetInstance()->GetAotBuffer(fileName);
    if (aotBuffer_.empty()) {
        HILOG_ERROR("Get aot buffer failed.");
        return false;
    }

    *buff = aotBuffer_.data();
    *buffSize = aotBuffer_.size();
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
