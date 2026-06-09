/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "source_map_operator.h"

#include <charconv>
#include <sstream>

#include "hilog.h"

namespace OHOS {
namespace JsEnv {

namespace {
constexpr char TAG[] = "SourceMapOperator";

// JSON structural characters
constexpr char BLOCK_START = '{';
constexpr char BLOCK_END = '}';
constexpr char QUOTE = '"';
constexpr char COLON = ':';
constexpr char COMMA = ',';

// Whitespace characters
constexpr char SPACE = ' ';
constexpr char TAB = '\t';
constexpr char NEWLINE = '\n';
constexpr char CARRIAGE_RETURN = '\r';

// Stack frame parsing characters
constexpr char OPEN_PAREN = '(';
constexpr char CLOSE_PAREN = ')';
constexpr char AT_SIGN = '@';
constexpr char PIPE = '|';
constexpr char SLASH = '/';
constexpr char BACKSLASH = '\\';

// JSON keys in sourceMaps.map
constexpr std::string_view KEY_PACKAGE_INFO = "\"package-info\"";
constexpr std::string_view KEY_ENTRY_PACKAGE_INFO = "\"entry-package-info\"";

// Stack frame keywords
constexpr std::string_view AT_KEYWORD = "at ";
constexpr std::string_view AT_ANONYMOUS_PREFIX = "    at anonymous ";
constexpr std::string_view AT_PREFIX = "    at ";

// Block boundary offset: excludes both the opening and closing braces
constexpr size_t BRACE_PAIR_COUNT = 2;

bool IsWhitespace(char c)
{
    return c == SPACE || c == TAB || c == NEWLINE || c == CARRIAGE_RETURN;
}

bool IsJsonKeyPrefix(char c)
{
    return IsWhitespace(c) || c == COMMA || c == BLOCK_START;
}

bool ParsePositiveInt(std::string_view value, int& result)
{
    if (value.empty()) {
        return false;
    }
    int parsed = 0;
    auto parseResult = std::from_chars(value.data(), value.data() + value.size(), parsed);
    if (parseResult.ec != std::errc() || parseResult.ptr != value.data() + value.size()) {
        return false;
    }
    result = parsed;
    return true;
}

size_t SkipWhitespace(std::string_view data, size_t pos)
{
    while (pos < data.size() && IsWhitespace(data[pos])) {
        ++pos;
    }
    return pos;
}

size_t SkipWhitespaceAndComma(std::string_view data, size_t pos)
{
    pos = SkipWhitespace(data, pos);
    if (pos < data.size() && data[pos] == COMMA) {
        ++pos;
    }
    return SkipWhitespace(data, pos);
}

size_t FindNextChar(std::string_view data, size_t pos, char target)
{
    while (pos < data.size() && data[pos] != target) {
        ++pos;
    }
    return pos;
}

std::string ExtractJsonStringValue(std::string_view data, size_t keyPos, std::string_view key)
{
    size_t pos = keyPos + key.size();
    pos = SkipWhitespace(data, pos);
    if (pos >= data.size() || data[pos] != COLON) {
        return {};
    }
    ++pos;
    pos = SkipWhitespace(data, pos);
    if (pos >= data.size() || data[pos] != QUOTE) {
        return {};
    }
    ++pos;
    size_t valEnd = FindNextChar(data, pos, QUOTE);
    if (valEnd >= data.size()) {
        return {};
    }
    return std::string(data.substr(pos, valEnd - pos));
}

std::string TryExtractPackageField(std::string_view block, std::string_view key)
{
    size_t pos = 0;
    while (pos < block.size()) {
        pos = block.find(key, pos);
        if (pos == std::string_view::npos) {
            return {};
        }
        if (pos > 0 && !IsJsonKeyPrefix(block[pos - 1])) {
            pos += key.size();
            continue;
        }
        return ExtractJsonStringValue(block, pos, key);
    }
    return {};
}

std::string ExtractPackageName(std::string_view block)
{
    std::string name = TryExtractPackageField(block, KEY_PACKAGE_INFO);
    if (name.empty()) {
        name = TryExtractPackageField(block, KEY_ENTRY_PACKAGE_INFO);
    }
    if (!name.empty()) {
        auto last = name.rfind(PIPE);
        if (last != std::string::npos) {
            name = name.substr(0, last);
        }
    }
    return name;
}

bool ParseQuotedUrl(std::string_view data, size_t& pos, std::string& url)
{
    pos = SkipWhitespaceAndComma(data, pos);
    if (pos >= data.size() || data[pos] != QUOTE) {
        return false;
    }
    size_t urlStart = pos + 1;
    size_t urlEnd = FindNextChar(data, urlStart, QUOTE);
    if (urlEnd >= data.size()) {
        return false;
    }
    url = std::string(data.substr(urlStart, urlEnd - urlStart));
    pos = urlEnd + 1;
    return true;
}

bool FindMatchingBrace(std::string_view data, size_t blockStart, size_t& blockEnd)
{
    size_t pos = blockStart + 1;
    int braceDepth = 1;
    while (pos < data.size() && braceDepth > 0) {
        if (data[pos] == BLOCK_START) {
            ++braceDepth;
        } else if (data[pos] == BLOCK_END) {
            --braceDepth;
        }
        ++pos;
    }
    if (braceDepth != 0) {
        return false;
    }
    blockEnd = pos;
    return true;
}

bool FindNextBlock(std::string_view data, size_t& pos, std::string& url, std::string_view& block)
{
    if (!ParseQuotedUrl(data, pos, url)) {
        return false;
    }
    size_t colonPos = SkipWhitespace(data, pos);
    if (colonPos >= data.size() || data[colonPos] != COLON) {
        return false;
    }
    size_t blockStart = SkipWhitespace(data, colonPos + 1);
    if (blockStart >= data.size() || data[blockStart] != BLOCK_START) {
        return false;
    }
    size_t blockEnd = 0;
    if (!FindMatchingBrace(data, blockStart, blockEnd)) {
        return false;
    }
    block = data.substr(blockStart + 1, blockEnd - blockStart - BRACE_PAIR_COUNT);
    pos = blockEnd;
    return true;
}

void ResolvePackageNameFromKey(const std::string& key, std::string& packageName)
{
    auto sep = key.find(PIPE);
    if (sep != std::string::npos) {
        packageName = key.substr(0, sep);
        return;
    }
    sep = key.find(SLASH);
    if (sep != std::string::npos) {
        packageName = key.substr(0, sep);
    }
}
} // namespace

// Requires: caller must hold unique_lock on mutex_ (write access)
void SourceMapOperator::MergeContent(const std::string& content)
{
    std::unordered_map<std::string, Ace::RefPtr<Ace::Framework::RevSourceMap>> newMaps;
    Ace::Framework::RevSourceMap::StageModeSourceMapSplit(content, newMaps);
    for (auto& [key, val] : newMaps) {
        sourceMaps_[key] = std::move(val);
    }
    if (!newMaps.empty()) {
        isInitialized_ = true;
        HILOG_INFO("[%{public}s] SourceMapOperator initialized, totalMaps=%{public}zu",
            TAG, sourceMaps_.size());
    }
    ParsePackageNames(content);
}

void SourceMapOperator::ParsePackageNames(const std::string& content)
{
    std::string_view data(content);
    size_t pos = 0;

    pos = SkipWhitespace(data, pos);
    if (pos < data.size() && data[pos] == BLOCK_START) {
        ++pos;
    }

    while (pos < data.size()) {
        std::string url;
        std::string_view block;
        if (!FindNextBlock(data, pos, url, block)) {
            break;
        }
        if (url.empty()) {
            continue;
        }
        std::string pkgName = ExtractPackageName(block);
        if (!pkgName.empty()) {
            packageNames_[url] = std::move(pkgName);
        }
    }
}

void SourceMapOperator::EraseModuleEntries(const std::string& moduleName)
{
    std::string prefix = moduleName + SLASH;
    for (auto it = sourceMaps_.begin(); it != sourceMaps_.end();) {
        const std::string& key = it->first;
        if (key == moduleName ||
            (key.size() > prefix.size() && key.compare(0, prefix.size(), prefix) == 0)) {
            packageNames_.erase(key);
            it = sourceMaps_.erase(it);
        } else {
            ++it;
        }
    }
}

bool SourceMapOperator::AddModuleSourceMap(
    const std::string& moduleName, const std::string& sourceMapContent)
{
    if (sourceMapContent.empty()) {
        HILOG_WARN("[%{public}s] AddModuleSourceMap: empty content for %{public}s",
            TAG, moduleName.c_str());
        return false;
    }
    {
        std::shared_lock<std::shared_mutex> rlock(mutex_);
        if (loadedModules_.count(moduleName)) {
            return true;
        }
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (loadedModules_.count(moduleName)) {
        return true;
    }
    MergeContent(sourceMapContent);
    loadedModules_.insert(moduleName);
    if (isInitialized_) {
        initStatus_ = EXECUTED_SUCCESSFULLY;
    }
    return true;
}

bool SourceMapOperator::UpdateModuleSourceMap(
    const std::string& moduleName, const std::string& sourceMapContent)
{
    if (sourceMapContent.empty()) {
        HILOG_WARN("[%{public}s] UpdateModuleSourceMap: empty content for %{public}s",
            TAG, moduleName.c_str());
        return false;
    }
    std::unique_lock<std::shared_mutex> lock(mutex_);
    EraseModuleEntries(moduleName);
    loadedModules_.erase(moduleName);
    MergeContent(sourceMapContent);
    loadedModules_.insert(moduleName);
    return true;
}

bool SourceMapOperator::IsModuleSourceMapLoaded(const std::string& moduleName) const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return loadedModules_.count(moduleName) > 0;
}

bool SourceMapOperator::IsInitialized() const
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return isInitialized_;
}

SourceMapOperator::SourceMapConstIt SourceMapOperator::FindSourceMapEntry(const std::string& url) const
{
    auto it = sourceMaps_.find(url);
    if (it != sourceMaps_.end()) {
        return it;
    }
    for (auto it2 = sourceMaps_.begin(); it2 != sourceMaps_.end(); ++it2) {
        const auto& key = it2->first;
        if (url.size() >= key.size() &&
            url.compare(url.size() - key.size(), key.size(), key) == 0) {
            return it2;
        }
        if (key.size() >= url.size() &&
            key.compare(key.size() - url.size(), url.size(), url) == 0) {
            return it2;
        }
    }
    return sourceMaps_.end();
}

bool SourceMapOperator::TranslatePosition(
    std::string& url, int& line, int& column, std::string& packageName)
{
    std::shared_lock<std::shared_mutex> lock(mutex_);
    if (!isInitialized_ || sourceMaps_.empty()) {
        return false;
    }

    auto it = FindSourceMapEntry(url);
    if (it == sourceMaps_.end() || !it->second) {
        return false;
    }

    auto pkgIt = packageNames_.find(it->first);
    if (pkgIt != packageNames_.end()) {
        packageName = pkgIt->second;
    } else {
        ResolvePackageNameFromKey(it->first, packageName);
    }

    auto info = it->second->Find(static_cast<int32_t>(line), static_cast<int32_t>(column));
    if (info.row == 0 && info.col == 0) {
        return false;
    }
    if (!info.sources.empty()) {
        url = info.sources;
    }
    line = static_cast<int>(info.row);
    column = static_cast<int>(info.col);
    return true;
}

bool SourceMapOperator::ParseUrlLineColumn(
    const std::string& str, std::string& url, int& line, int& column)
{
    auto lastColon = str.rfind(COLON);
    if (lastColon == std::string::npos) {
        return false;
    }
    auto secondColon = str.rfind(COLON, lastColon - 1);
    if (secondColon == std::string::npos) {
        return false;
    }

    url = str.substr(0, secondColon);
    auto lineStr = std::string_view(str).substr(secondColon + 1, lastColon - secondColon - 1);
    auto colStr = std::string_view(str).substr(lastColon + 1);
    if (!ParsePositiveInt(lineStr, line) || !ParsePositiveInt(colStr, column)) {
        return false;
    }
    return true;
}

bool SourceMapOperator::ExtractStackFrameInfo(
    const std::string& frame, std::string& funcName, std::string& url, int& line, int& column)
{
    auto atPos = frame.find(AT_KEYWORD);
    auto atSignPos = frame.find(AT_SIGN);
    if (atSignPos != std::string::npos &&
        (atPos == std::string::npos || atSignPos < atPos)) {
        funcName = frame.substr(0, atSignPos);
        bool parsed = ParseUrlLineColumn(frame.substr(atSignPos + 1), url, line, column);
        return parsed;
    }
    if (atPos == std::string::npos) {
        return false;
    }
    atPos += AT_KEYWORD.size();
    auto openParen = frame.find(OPEN_PAREN, atPos);
    if (openParen != std::string::npos) {
        funcName = frame.substr(atPos, openParen - atPos);
        while (!funcName.empty() && funcName.back() == SPACE) {
            funcName.pop_back();
        }
        auto closeParen = frame.find(CLOSE_PAREN, openParen);
        if (closeParen == std::string::npos) {
            return false;
        }
        auto inner = frame.substr(openParen + 1, closeParen - openParen - 1);
        bool parsed = ParseUrlLineColumn(inner, url, line, column);
        return parsed;
    }
    funcName = "";
    bool parsed = ParseUrlLineColumn(frame.substr(atPos), url, line, column);
    return parsed;
}

std::string SourceMapOperator::TranslateStackFrame(const std::string& frameLine)
{
    if (frameLine.find(AT_KEYWORD) == std::string::npos &&
        frameLine.find(AT_SIGN) == std::string::npos) {
        return frameLine;
    }
    std::string funcName;
    std::string url;
    std::string packageName;
    int lineNum = 0;
    int colNum = 0;
    if (!ExtractStackFrameInfo(frameLine, funcName, url, lineNum, colNum)) {
        return frameLine;
    }
    std::string translatedUrl = url;
    int translatedLine = lineNum;
    int translatedCol = colNum;
    if (!TranslatePosition(translatedUrl, translatedLine, translatedCol, packageName)) {
        return frameLine;
    }

    std::string sourceInfo = std::string(1, OPEN_PAREN) + translatedUrl +
        std::string(1, COLON) + std::to_string(translatedLine) +
        std::string(1, COLON) + std::to_string(translatedCol) +
        std::string(1, CLOSE_PAREN);
    if (!packageName.empty()) {
        sourceInfo = packageName + std::string(1, SPACE) + sourceInfo;
    }

    std::string result = frameLine;
    auto openP = result.rfind(OPEN_PAREN);
    auto closeP = result.rfind(CLOSE_PAREN);
    if (openP != std::string::npos && closeP != std::string::npos && closeP > openP) {
        result.replace(openP, closeP - openP + 1, sourceInfo);
    } else {
        result = funcName.empty()
            ? std::string(AT_ANONYMOUS_PREFIX) + sourceInfo
            : std::string(AT_PREFIX) + funcName + std::string(1, SPACE) + sourceInfo;
    }
    std::replace(result.begin(), result.end(), BACKSLASH, SLASH);
    return result;
}

std::string SourceMapOperator::TranslateStack(const std::string& rawStack)
{
    if (rawStack.empty() || !IsInitialized()) {
        return rawStack;
    }
    std::istringstream stream(rawStack);
    std::string line;
    std::string result;
    bool first = true;
    while (std::getline(stream, line)) {
        if (!first) {
            result += NEWLINE;
        }
        first = false;
        result += TranslateStackFrame(line);
    }
    return result;
}

} // namespace JsEnv
} // namespace OHOS
