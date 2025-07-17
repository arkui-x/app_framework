/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include <chrono>

#include "js_module_reader.h"

#include "app_main.h"
#include "hilog.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
uint64_t GetNowTime()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
}
}
JsModuleReader::JsModuleReader(const std::string& bundleName) : bundleName_(bundleName) {}

bool JsModuleReader::operator()(const std::string& inputPath, uint8_t** buff, size_t* buffSize, std::string& errorMsg)
{
    HILOG_INFO("Begin: %{private}s, time is: %{public}ld.", inputPath.c_str(), GetNowTime());
    if (inputPath.empty() || buff == nullptr || buffSize == nullptr) {
        HILOG_ERROR("Invalid param");
        return false;
    }
    moduleBuffer_.clear();
    std::string moduleName = GetModuleName(inputPath);
    std::string modulePath;
    Platform::StageAssetManager::GetInstance()->isDynamicModule(moduleName, true);
    bool isDynamicModule = Platform::StageAssetManager::GetInstance()->IsDynamicUpdateModule(moduleName);
    if (isDynamicModule) {
#ifdef ANDROID_PLATFORM
        Platform::StageAssetManager::GetInstance()->RemoveModuleFilePath(moduleName);
#endif
    }
    moduleBuffer_ = Platform::StageAssetManager::GetInstance()->GetModuleBuffer(moduleName, modulePath, true);
    if (moduleBuffer_.empty()) {
        errorMsg = "modulePath:" + modulePath;
        HILOG_ERROR("GetModuleBuffer failed.");
        return false;
    }
    Platform::AppMain::GetInstance()->ParseHspModuleJson(moduleName);

    *buff = moduleBuffer_.data();
    *buffSize = moduleBuffer_.size();
    HILOG_INFO("End, time is: %{public}ld.", GetNowTime());
    return true;
}

std::string JsModuleReader::GetModuleName(const std::string& inputPath) const
{
    return inputPath.substr(inputPath.find_last_of("/") + 1);
}

} // namespace AbilityRuntime
} // namespace OHOS
