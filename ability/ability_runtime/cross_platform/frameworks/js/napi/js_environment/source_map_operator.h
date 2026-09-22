/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
#define OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H

#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bridge/common/utils/source_map.h"

namespace OHOS {
namespace JsEnv {

enum InitStatus { NOT_EXECUTED, EXECUTED_SUCCESSFULLY };

class SourceMapOperator : public std::enable_shared_from_this<SourceMapOperator> {
public:
    explicit SourceMapOperator(const std::string& bundleName)
        : bundleName_(bundleName), initStatus_(NOT_EXECUTED) {}
    ~SourceMapOperator() = default;

    bool AddModuleSourceMap(const std::string& moduleName, const std::string& sourceMapContent);
    bool UpdateModuleSourceMap(const std::string& moduleName, const std::string& sourceMapContent);
    bool IsModuleSourceMapLoaded(const std::string& moduleName) const;
    bool IsInitialized() const;
    void SetInitStatus(InitStatus value)
    {
        initStatus_ = value;
    }
    std::string TranslateStack(const std::string& rawStack);
    bool TranslatePosition(std::string& url, int& line, int& column, std::string& packageName);

private:
    using SourceMapConstIt = std::unordered_map<
        std::string, Ace::RefPtr<Ace::Framework::RevSourceMap>>::const_iterator;

    void MergeContent(const std::string& content);
    void EraseModuleEntries(const std::string& moduleName);
    void ParsePackageNames(const std::string& content);
    SourceMapConstIt FindSourceMapEntry(const std::string& url) const;
    bool ExtractStackFrameInfo(
        const std::string& frame, std::string& funcName, std::string& url, int& line, int& column);
    bool ParseUrlLineColumn(const std::string& str, std::string& url, int& line, int& column);
    std::string TranslateStackFrame(const std::string& frameLine);

    const std::string bundleName_;
    InitStatus initStatus_ = NOT_EXECUTED;
    bool isInitialized_ = false;
    std::unordered_map<std::string, Ace::RefPtr<Ace::Framework::RevSourceMap>> sourceMaps_;
    std::unordered_map<std::string, std::string> packageNames_;
    std::unordered_set<std::string> loadedModules_;
    mutable std::shared_mutex mutex_;
};

} // namespace JsEnv
} // namespace OHOS

#endif // OHOS_ABILITY_JS_ENVIRONMENT_SOURCE_MAP_OPERATOR_H
