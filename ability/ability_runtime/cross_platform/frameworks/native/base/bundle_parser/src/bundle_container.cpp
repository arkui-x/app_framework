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

#include "bundle_container.h"

#include <fstream>
#include <nlohmann/json.hpp>

#include "bundle_parser.h"
#include "hilog.h"
#include "json_serializer.h"
#include "module_profile.h"
#include "navigation/router_map_helper.h"

namespace OHOS {
namespace AppExecFwk {
BundleContainer::BundleContainer() {}

BundleContainer::~BundleContainer() {}

void BundleContainer::LoadBundleInfos(const std::list<std::vector<uint8_t>>& bufList)
{
    BundleParser parser;
    HILOG_INFO("BundleContainer LoadBundleInfos bufList size %{public}d", static_cast<int>(bufList.size()));
    for (auto it = bufList.begin(); it != bufList.end(); it++) {
        InnerBundleInfo bInfo;
        if (parser.Parse(*it, bInfo) != ERR_OK) {
            continue;
        }
        if (bundleInfo_ == nullptr) {
            bundleInfo_ = std::make_shared<InnerBundleInfo>();
            *bundleInfo_ = bInfo;
        } else {
            bool added = bundleInfo_->AddModuleInfo(bInfo);
            if (!added) {
                bundleInfo_->UpdateModuleInfo(bInfo);
            }
        }
    }
}

std::shared_ptr<ApplicationInfo> BundleContainer::GetApplicationInfo() const
{
    if (bundleInfo_ != nullptr) {
        auto appInfo = std::make_shared<ApplicationInfo>();
        bundleInfo_->GetApplicationInfo(0, Constants::UNSPECIFIED_USERID, *appInfo);
        return appInfo;
    }
    return nullptr;
}

std::shared_ptr<BundleInfo> BundleContainer::GetBundleInfo() const
{
    if (bundleInfo_ != nullptr) {
        auto bInfo = std::make_shared<BundleInfo>();
        auto flag = static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
        auto uid = Constants::UNSPECIFIED_USERID;
        if (bundleInfo_->GetBundleInfo(flag, *bInfo, uid)) {
            return bInfo;
        }
    }
    return nullptr;
}

std::shared_ptr<BundleInfo> BundleContainer::GetBundleInfoV9(int32_t flag) const
{
    HILOG_INFO("BundleContainer GetBundleInfoV9 flag %{public}d", flag);
    if (bundleInfo_ != nullptr) {
        auto bInfo = std::make_shared<BundleInfo>();
        if (bInfo == nullptr) {
            return nullptr;
        }
        auto uid = Constants::UNSPECIFIED_USERID;
        if (bundleInfo_->GetBundleInfoV9(flag, *bInfo, uid) == ERR_OK) {
            return bInfo;
        }
    }
    return nullptr;
}

std::shared_ptr<HapModuleInfo> BundleContainer::GetHapModuleInfo(const std::string& modulePackage) const
{
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        HILOG_INFO("BundleContainer GetHapModuleInfo by modulePackage %{public}s", modulePackage.c_str());
        std::optional<HapModuleInfo> hapMouduleInfo = bundleInfo_->FindHapModuleInfo(modulePackage, uid);
        if (hapMouduleInfo) {
            auto hapInfo = std::make_shared<HapModuleInfo>();
            *hapInfo = *hapMouduleInfo;
            return hapInfo;
        }
    }
    return nullptr;
}

void BundleContainer::RemoveModuleInfo(const std::string& modulePackage)
{
    if (bundleInfo_ != nullptr) {
        bundleInfo_->RemoveModuleInfo(modulePackage);
    }
}

std::shared_ptr<AbilityInfo> BundleContainer::GetAbilityInfo(
    const std::string& moduleName, const std::string& abilityName) const
{
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        std::optional<AbilityInfo> ablilityInfo = bundleInfo_->FindAbilityInfo(moduleName, abilityName, uid);
        if (ablilityInfo) {
            auto aInfo = std::make_shared<AbilityInfo>();
            *aInfo = *ablilityInfo;
            return aInfo;
        }
    }
    return nullptr;
}

std::vector<std::shared_ptr<AbilityInfo>> BundleContainer::GetAbilityInfos() const
{
    std::vector<std::shared_ptr<AbilityInfo>> array;
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        std::optional<std::vector<AbilityInfo>> ablilityInfos = bundleInfo_->FindAbilityInfos(uid);
        if (ablilityInfos) {
            for (auto it = (*ablilityInfos).begin(); it != (*ablilityInfos).end(); it++) {
                auto aInfo = std::make_shared<AbilityInfo>();
                *aInfo = *it;
                array.emplace_back(aInfo);
            }
        }
    }
    return array;
}

void BundleContainer::dump(const std::string& pathName)
{
    std::ofstream ofile(pathName.c_str());
    if (ofile.is_open()) {
        std::string str = bundleInfo_->ToString();
        ofile << str;
        ofile.close();
    }
}

void BundleContainer::SetAppCodePath(const std::string& codePath)
{
    if (bundleInfo_ == nullptr) {
        HILOG_ERROR("bundleInfo_ is nullptr");
        return;
    }
    bundleInfo_->SetAppCodePath(codePath);
}

void BundleContainer::SetPidAndUid(int32_t pid, int32_t uid)
{
    if (bundleInfo_ == nullptr) {
        HILOG_ERROR("bundleInfo_ is nullptr");
        return;
    }
    bundleInfo_->SetPid(pid);
    bundleInfo_->SetUid(uid);
}

std::string BundleContainer::GetBundleName() const
{
    if (bundleInfo_ == nullptr) {
        HILOG_ERROR("bundleInfo_ is nullptr");
        return "";
    }
    return bundleInfo_->GetBundleName();
}

ErrCode BundleContainer::GetBundleInfoForSelf(int32_t flag, BundleInfo& bundleInfo)
{
    HILOG_DEBUG("BundleContainer GetBundleInfoForSelf flag %{public}d", flag);
    if (bundleInfo_ != nullptr) {
        auto uid = Constants::UNSPECIFIED_USERID;
        if (bundleInfo_->GetBundleInfoV9(flag, bundleInfo, uid) == ERR_OK) {
            HILOG_DEBUG("GetBundleInfoV9 successfully, bundleName: %{public}s", bundleInfo_->GetBundleName().c_str());
            RouterMapHelper::ProcessBundleRouterMap(bundleInfo, flag);
        } else {
            HILOG_ERROR("GetBundleInfoV9 failed");
        }
        return ERR_OK;
    } else {
        HILOG_ERROR("bundleInfo_ is nullptr");
        return ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST;
    }
}
} // namespace AppExecFwk
} // namespace OHOS
