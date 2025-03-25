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
#include "router_item_compare.h"

#include "router_map_helper.h"

namespace OHOS {
namespace AppExecFwk {
RouterItemCompare::RouterItemCompare(const std::set<std::string>& moduleNameSet) : moduleNameSet(moduleNameSet) {}

bool RouterItemCompare::operator()(const RouterItem& item1, const RouterItem& item2) const
{
    if (item1.name != item2.name) {
        return item1.name < item2.name;
    }

    std::string version1 = RouterMapHelper::ExtractVersionFromOhmurl(item1.ohmurl);
    std::string version2 = RouterMapHelper::ExtractVersionFromOhmurl(item2.ohmurl);
    int versionCompare = RouterMapHelper::Compare(version1, version2);
    if (versionCompare != 0) {
        return versionCompare > 0;
    }

    bool contains1 = moduleNameSet.find(item1.moduleName) != moduleNameSet.end();
    bool contains2 = moduleNameSet.find(item2.moduleName) != moduleNameSet.end();
    if (contains1 && contains2) {
        return item1.moduleName < item2.moduleName;
    } else if (!contains1 && contains2) {
        return false;
    } else if (contains1 && !contains2) {
        return true;
    }
    return item1.moduleName < item2.moduleName;
}
} // namespace AppExecFwk
} // namespace OHOS