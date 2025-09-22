/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_CONTAINER_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_CONTAINER_H

#include <functional>
#include <list>
#include <set>
#include <string>

#include "application_info.h"
#include "bundle_constants.h"
#include "bundle_info.h"
#include "inner_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
class BundleContainer {
public:
    BundleContainer();
    virtual ~BundleContainer();

public:
    void LoadBundleInfos(const std::list<std::vector<uint8_t>>& bufList);
    std::shared_ptr<ApplicationInfo> GetApplicationInfo() const;
    std::shared_ptr<BundleInfo> GetBundleInfo() const;
    std::shared_ptr<BundleInfo> GetBundleInfoV9(int32_t flag) const;
    std::shared_ptr<HapModuleInfo> GetHapModuleInfo(const std::string& modulePackage) const;
    void RemoveModuleInfo(const std::string& modulePackage);
    std::shared_ptr<AbilityInfo> GetAbilityInfo(const std::string& moduleName, const std::string& abilityName) const;
    std::vector<std::shared_ptr<AbilityInfo>> GetAbilityInfos() const;
    void dump(const std::string& pathName);
    void SetAppCodePath(const std::string& codePath);
    void SetBundleName(const std::string& bundleName);
    void SetPidAndUid(int32_t pid, int32_t uid);
    std::string GetBundleName() const;
    ErrCode GetBundleInfoForSelf(int32_t flags, BundleInfo& bundleInfo);

private:
    std::shared_ptr<InnerBundleInfo> bundleInfo_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_BUNDLE_CONTAINER_H
