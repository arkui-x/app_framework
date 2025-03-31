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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_CONFIG_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_CONFIG_H

#include <string>

namespace OHOS {
namespace HiviewDFX {
class HiAppEventConfig {
public:
    static HiAppEventConfig& GetInstance();
    bool SetConfigurationItem(std::string name, std::string value);
    bool GetDisable();
    std::string GetRunningId();

private:
    HiAppEventConfig() {}
    ~HiAppEventConfig() {}
    HiAppEventConfig(const HiAppEventConfig&);
    HiAppEventConfig& operator=(const HiAppEventConfig&);
    bool SetDisableItem(const std::string& value);
    void SetDisable(bool disable);

private:
    bool disable = false;
    std::string runningId = "";
};
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_CONFIG_H
