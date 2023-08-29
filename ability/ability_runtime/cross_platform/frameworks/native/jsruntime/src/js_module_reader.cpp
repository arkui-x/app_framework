/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_module_reader.h"
#include "hilog.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace AbilityRuntime {

JsModuleReader::JsModuleReader(const std::string& bundleName) : bundleName_(bundleName) {}

bool JsModuleReader::operator()(const std::string& inputPath, uint8_t** buff, size_t* buffSize) const
{
    HILOG_INFO("JsModuleReader operator start: %{private}s", inputPath.c_str());
    if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
        HILOG_ERROR("Invalid param");
        return false;
    }

    std::string moduleName = GetModuleName(inputPath);
    std::string modulePath;
    auto moduleBuffer = Platform::StageAssetManager::GetInstance()->GetModuleBuffer(moduleName, modulePath, true);
    if (moduleBuffer.empty()) {
        HILOG_ERROR("GetModuleBuffer failed.");
        return false;
    }

    *buff = moduleBuffer.data();
    *buffSize = moduleBuffer.size();
    return true;
}

std::string JsModuleReader::GetModuleName(const std::string& inputPath) const
{
    return inputPath.substr(inputPath.find_last_of("/") + 1);
}

} // namespace AbilityRuntime
} // namespace OHOS
