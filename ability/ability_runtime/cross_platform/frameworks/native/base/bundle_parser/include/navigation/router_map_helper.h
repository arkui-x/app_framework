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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_MAP_HELPER_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_MAP_HELPER_H

#include <set>

#include "appexecfwk_errors.h"
#include "bundle_info.h"
#include "sem_ver.h"

namespace OHOS {
namespace AppExecFwk {
class RouterMapHelper {
public:
    static void MergeRouter(BundleInfo& info);
    static void MergeRouter(const std::vector<RouterItem>& routerArrayList, std::vector<RouterItem>& routerArray,
        const std::set<std::string>& moduleNameSet);
    static int32_t Compare(const std::string& version1, const std::string& version2);
    static int32_t Compare(const SemVer& semVer1, const SemVer& semVer2);
    static int32_t CompareIdentifiers(const std::string& a, const std::string& b);
    static int32_t CompareMain(const SemVer& semVer1, const SemVer& semVer2);
    static int32_t ComparePre(const SemVer& semVer1, const SemVer& semVer2);
    static std::string ExtractVersionFromOhmurl(const std::string& ohmurl);
    static void ProcessBundleRouterMap(BundleInfo& bundleInfo, int32_t flag);

private:
    static bool ReadFromJson(const std::string routerJsonPath, std::string& routerMapString);
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_MAP_HELPER_H