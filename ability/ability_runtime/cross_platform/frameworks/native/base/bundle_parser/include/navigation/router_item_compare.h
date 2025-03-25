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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_ITEM_COMPARE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_ITEM_COMPARE_H

#include <set>
#include <string>

#include "hap_module_info.h"

namespace OHOS {
namespace AppExecFwk {
class RouterItemCompare {
public:
    explicit RouterItemCompare(const std::set<std::string>& moduleNameSet);
    bool operator()(const RouterItem& item1, const RouterItem& item2) const;

private:
    std::set<std::string> moduleNameSet;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_NAVIGATION_ROUTER_ITEM_COMPARE_H