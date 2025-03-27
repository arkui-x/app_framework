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
#include "router_map_helper.h"

#include <fstream>

#include "bundle_parser.h"
#include "hilog.h"
#include "router_item_compare.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace AppExecFwk {
using namespace AbilityRuntime::Platform;
namespace {
const std::regex ALNUM_REGEX("^[a-zA-Z0-9]*$");
const std::regex NUM_REGEX("^[0-9]+$");
constexpr int8_t PROFILE_PREFIX_LENGTH = 9;
constexpr const char* PROFILE_PREFIX = "$profile:";
constexpr const char* PROFILE_PATH = "resources/base/profile/";
constexpr const char* JSON_SUFFIX = ".json";
} // namespace

void RouterMapHelper::MergeRouter(BundleInfo& info)
{
    if (info.hapModuleInfos.empty()) {
        HILOG_WARN("hapModuleInfos in bundleInfo is empty");
        return;
    }
    std::vector<RouterItem> routerArrayList;
    std::set<std::string> moduleNameSet;
    for (const auto& hapModuleInfo : info.hapModuleInfos) {
        if (hapModuleInfo.moduleType == ModuleType::ENTRY || hapModuleInfo.moduleType == ModuleType::FEATURE) {
            moduleNameSet.insert(hapModuleInfo.name);
        }
        for (const auto& routerItem : hapModuleInfo.routerArray) {
            routerArrayList.emplace_back(routerItem);
        }
    }
    if (routerArrayList.empty()) {
        return;
    }
    MergeRouter(routerArrayList, info.routerArray, moduleNameSet);
}

std::string RouterMapHelper::ExtractVersionFromOhmurl(const std::string& ohmurl)
{
    size_t lastAmpersandPos = ohmurl.rfind('&');
    std::string versionString;
    if (lastAmpersandPos == std::string::npos) {
        HILOG_INFO("No ampersand found in the input ohmurl");
        return versionString;
    }
    // "+1" for start intercepting after the "&" character
    versionString = ohmurl.substr(lastAmpersandPos + 1);
    return versionString;
}

void RouterMapHelper::MergeRouter(const std::vector<RouterItem>& routerArrayList, std::vector<RouterItem>& routerArray,
    const std::set<std::string>& moduleNameSet)
{
    std::map<RouterItem, std::string, RouterItemCompare> routerMap((RouterItemCompare(moduleNameSet)));

    for (const auto& item : routerArrayList) {
        routerMap.emplace(item, item.name);
    }

    std::vector<RouterItem> routerArraySorted;

    routerArraySorted.reserve(routerMap.size());
    for (const auto& pair : routerMap) {
        routerArraySorted.push_back(pair.first);
    }
    routerArray.clear();
    for (size_t i = 0; i < routerArraySorted.size(); i++) {
        if ((i == 0) || (routerArraySorted[i].name != routerArray[routerArray.size() - 1].name)) {
            routerArray.emplace_back(routerArraySorted[i]);
        }
    }
}

int32_t RouterMapHelper::CompareIdentifiers(const std::string& a, const std::string& b)
{
    if (!std::regex_match(a, ALNUM_REGEX) || !std::regex_match(b, ALNUM_REGEX)) {
        return 1;
    }
    bool anum = std::regex_match(a, NUM_REGEX);
    bool bnum = std::regex_match(b, NUM_REGEX);
    if (anum && bnum) {
        auto diff = atoi(a.c_str()) - atoi(b.c_str());
        if (diff) {
            return diff > 0 ? 1 : -1;
        }
        return 0;
    }

    if (anum && !bnum) {
        return -1;
    }
    if (bnum && !anum) {
        return 1;
    }
    if (a < b) {
        return -1;
    }
    if (a > b) {
        return 1;
    }
    return 0;
}

int32_t RouterMapHelper::CompareMain(const SemVer& semVer1, const SemVer& semVer2)
{
    auto res = CompareIdentifiers(semVer1.major, semVer2.major);
    if (res) {
        return res;
    }
    res = CompareIdentifiers(semVer1.minor, semVer2.minor);
    if (res) {
        return res;
    }
    return CompareIdentifiers(semVer1.patch, semVer2.patch);
}

int32_t RouterMapHelper::ComparePre(const SemVer& semVer1, const SemVer& semVer2)
{
    // NOT having a prerelease is > having one
    if (!semVer1.prerelease.empty() && semVer2.prerelease.empty()) {
        return -1;
    } else if (semVer1.prerelease.empty() && !semVer2.prerelease.empty()) {
        return 1;
    } else if (semVer1.prerelease.empty() && semVer2.prerelease.empty()) {
        return 0;
    }
    size_t i = 0;
    do {
        if ((i >= semVer1.prerelease.size()) && (i >= semVer2.prerelease.size())) {
            return 0;
        } else if (i >= semVer2.prerelease.size()) {
            return 1;
        } else if (i >= semVer1.prerelease.size()) {
            return -1;
        }
        std::string a = semVer1.prerelease[i];
        std::string b = semVer2.prerelease[i];
        if (a == b || !CompareIdentifiers(a, b)) {
            continue;
        }
        return CompareIdentifiers(a, b);
    } while (++i);
    return 0;
}

int32_t RouterMapHelper::Compare(const std::string& version1, const std::string& version2)
{
    SemVer semver1(version1);
    SemVer semver2(version2);
    return Compare(semver1, semver2);
}

int32_t RouterMapHelper::Compare(const SemVer& semVer1, const SemVer& semVer2)
{
    if (semVer1.raw == semVer2.raw) {
        return 0;
    }
    auto res = CompareMain(semVer1, semVer2);
    return res ? res : ComparePre(semVer1, semVer2);
}

void RouterMapHelper::ProcessBundleRouterMap(BundleInfo& bundleInfo, int32_t flag)
{
    HILOG_DEBUG("ProcessBundleRouterMap with flags: %{public}d", flag);
    if ((static_cast<uint32_t>(flag) & static_cast<uint32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE)) !=
        static_cast<uint32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE)) {
        return;
    }
    if ((static_cast<uint32_t>(flag) & static_cast<uint32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ROUTER_MAP)) !=
        static_cast<uint32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ROUTER_MAP)) {
        return;
    }
    for (auto& hapModuleInfo : bundleInfo.hapModuleInfos) {
        std::string routerPath = hapModuleInfo.routerMap;
        auto pos = routerPath.find(PROFILE_PREFIX);
        if (pos == std::string::npos) {
            HILOG_DEBUG("invalid router profile");
            continue;
        }
        std::string routerJsonName = routerPath.substr(pos + PROFILE_PREFIX_LENGTH) + JSON_SUFFIX;
        std::string routerJson = PROFILE_PATH + routerJsonName;
        std::string moduleName = hapModuleInfo.moduleName;
        std::string routerMapString = "";
#ifdef ANDROID_PLATFORM
        std::string appDataModuleDir = StageAssetManager::GetInstance()->GetAppDataModuleDir();
#else
        std::string appDataModuleDir = StageAssetManager::GetInstance()->GetBundleCodeDir();
#endif
        std::string routerJsonPath = appDataModuleDir + '/' + moduleName + '/' + routerJson;
        if (!ReadFromJson(routerJsonPath, routerMapString)) {
            HILOG_ERROR("get json string from %{public}s failed", routerJsonPath.c_str());
        }
        HILOG_INFO("routerMapString is %{public}s", routerMapString.c_str());
        BundleParser bundleParser;
        if (bundleParser.ParseRouterArray(routerMapString, hapModuleInfo.routerArray) != ERR_OK) {
            HILOG_ERROR("parse router failed");
            return;
        }
    }
    MergeRouter(bundleInfo);
}

bool RouterMapHelper::ReadFromJson(const std::string routerJsonPath, std::string& routerMapString)
{
    std::ifstream jsonFile(routerJsonPath);
    if (!jsonFile.is_open()) {
        HILOG_ERROR("open router json file failed");
        return false;
    }
    std::string line;
    while (getline(jsonFile, line)) {
        routerMapString += line;
        routerMapString += "\n";
    }
    jsonFile.close();
    return true;
}
} // namespace AppExecFwk
} // namespace OHOS