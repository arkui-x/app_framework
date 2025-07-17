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

#ifndef OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
#define OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H

#include <sstream>
#include <string>
#include <vector>



namespace OHOS {
namespace AbilityRuntime {
class JsModuleReader {
public:

    explicit JsModuleReader(const std::string& bundleName);
    ~JsModuleReader() = default;

    JsModuleReader(const JsModuleReader&) = default;
    JsModuleReader(JsModuleReader&&) = default;
    JsModuleReader& operator=(const JsModuleReader&) = default;
    JsModuleReader& operator=(JsModuleReader&&) = default;

    bool operator()(const std::string& inputPath, uint8_t** buff, size_t* buffSize, std::string& errorMsg);

private:
    std::string GetModuleName(const std::string& inputPath) const;

    std::string bundleName_;
    std::vector<uint8_t> moduleBuffer_;
};
} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_MODULE_READER_H
