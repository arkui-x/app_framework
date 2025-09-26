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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INNER_BUNDLE_INFO_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INNER_BUNDLE_INFO_H

#include "ability_info.h"
#include "bundle_constants.h"
#include "bundle_info.h"
#include "common_profile.h"
#include "hap_module_info.h"
#include "inner_bundle_user_info.h"
#include "json_util.h"
// #include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
struct Distro {
    bool deliveryWithInstall = false;
    std::string moduleName;
    std::string moduleType;
    bool installationFree = false;
};

struct DefinePermission {
    std::string name;
    std::string grantMode = Profile::DEFINEPERMISSION_GRANT_MODE_SYSTEM_GRANT;
    std::string availableLevel = Profile::DEFINEPERMISSION_AVAILABLE_LEVEL_DEFAULT_VALUE;
    bool provisionEnable = true;
    bool distributedSceneEnable = false;
    std::string label;
    int32_t labelId = 0;
    std::string description;
    int32_t descriptionId = 0;
};

struct Dependency {
    std::string moduleName;
    std::string bundleName;
};

struct InnerModuleInfo {
    std::string name;
    std::string modulePackage;
    std::string moduleName;
    std::string modulePath;
    std::string moduleDataDir;
    std::string moduleResPath;
    std::string label;
    std::string hapPath;
    int32_t labelId = 0;
    std::string description;
    int32_t descriptionId = 0;
    std::string icon;
    int32_t iconId = 0;
    std::string mainAbility;     // config.json : mainAbility; module.json : mainElement
    std::string entryAbilityKey; // skills contains "action.system.home" and "entity.system.home"
    std::string srcPath;
    std::string hashValue;
    bool isEntry = false;
    bool installationFree = false;
    // all user's value of isRemovable
    // key:userId
    // value:isRemovable true or flase
    std::map<std::string, bool> isRemovable;
    MetaData metaData;
    ModuleColorMode colorMode = ModuleColorMode::AUTO;
    Distro distro;
    std::vector<std::string> reqCapabilities;
    std::vector<std::string> abilityKeys;
    std::vector<std::string> skillKeys;
    // new version fields
    std::string pages;
    std::string process;
    std::string srcEntrance;
    std::string uiSyntax;
    std::string virtualMachine;
    bool isModuleJson = false;
    bool isStageBasedModel = false;
    std::vector<DefinePermission> definePermissions;
    std::vector<RequestPermission> requestPermissions;
    std::vector<std::string> deviceTypes;
    std::vector<std::string> extensionKeys;
    std::vector<std::string> extensionSkillKeys;
    std::vector<Metadata> metadata;
    int32_t upgradeFlag = 0;
    std::vector<Dependency> dependencies;
    std::string compileMode;
    bool isLibIsolated = false;
    std::string nativeLibraryPath;
    std::string cpuAbi;
    std::string targetModuleName;
    int32_t targetPriority;
    std::vector<std::string> preloads;
    std::string packageName;
    std::string routerMap;
};

struct SkillUri {
    std::string scheme;
    std::string host;
    std::string port;
    std::string path;
    std::string pathStartWith;
    std::string pathRegex;
    std::string type;
};

struct Skill {
public:
    std::vector<std::string> actions;
    std::vector<std::string> entities;
    std::vector<SkillUri> uris;
    bool MatchType(const std::string& type, const std::string& skillUriType) const;

private:
    bool MatchAction(const std::string& action) const;
    bool MatchEntities(const std::vector<std::string>& paramEntities) const;
    bool MatchUriAndType(const std::string& uriString, const std::string& type) const;
    bool MatchUri(const std::string& uriString, const SkillUri& skillUri) const;
    bool StartsWith(const std::string& sourceString, const std::string& targetPrefix) const;
};

enum InstallExceptionStatus : int32_t {
    INSTALL_START = 1,
    INSTALL_FINISH,
    UPDATING_EXISTED_START,
    UPDATING_NEW_START,
    UPDATING_FINISH,
    UNINSTALL_BUNDLE_START,
    UNINSTALL_PACKAGE_START,
    UNKNOWN_STATUS,
};

struct InstallMark {
    std::string bundleName;
    std::string packageName;
    int32_t status = InstallExceptionStatus::UNKNOWN_STATUS;
};

struct SandboxAppPersistentInfo {
    uint32_t accessTokenId = 0;
    int32_t appIndex = 0;
    int32_t userId = Constants::INVALID_USERID;
};

class InnerBundleInfo {
public:
    enum class BundleStatus {
        ENABLED = 1,
        DISABLED,
    };

    InnerBundleInfo();
    InnerBundleInfo& operator=(const InnerBundleInfo& info);
    ~InnerBundleInfo();
    /**
     * @brief Transform the InnerBundleInfo object to json.
     * @param jsonObject Indicates the obtained json object.
     * @return
     */
    void ToJson(nlohmann::json& jsonObject) const;
    /**
     * @brief Transform the json object to InnerBundleInfo object.
     * @param jsonObject Indicates the obtained json object.
     * @return Returns 0 if the json object parsed successfully; returns error code otherwise.
     */
    int32_t FromJson(const nlohmann::json& jsonObject);
    /**
     * @brief Add module info to old InnerBundleInfo object.
     * @param newInfo Indicates the new InnerBundleInfo object.
     * @return Returns true if the module successfully added; returns false otherwise.
     */
    bool AddModuleInfo(const InnerBundleInfo& newInfo);
    /**
     * @brief Update module info to old InnerBundleInfo object.
     * @param newInfo Indicates the new InnerBundleInfo object.
     * @return
     */
    void UpdateModuleInfo(const InnerBundleInfo& newInfo);
    /**
     * @brief Remove module info from InnerBundleInfo object.
     * @param modulePackage Indicates the module package to be remove.
     * @return
     */
    void RemoveModuleInfo(const std::string& modulePackage);
    /**
     * @brief Find hap module info by module package.
     * @param modulePackage Indicates the module package.
     * @param userId Indicates the user ID.
     * @return Returns the HapModuleInfo object if find it; returns null otherwise.
     */
    std::optional<HapModuleInfo> FindHapModuleInfo(
        const std::string& modulePackage, int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Get module hashValue.
     * @param modulePackage Indicates the module package.
     * @param hapModuleInfo Indicates the hapModuleInfo.
     * @return
     */
    void GetModuleWithHashValue(int32_t flags, const std::string& modulePackage, HapModuleInfo& hapModuleInfo) const;
    /**
     * @brief Find abilityInfo by bundle name and ability name.
     * @param moduleName Indicates the module name
     * @param abilityName Indicates the ability name.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo object if find it; returns null otherwise.
     */
    std::optional<AbilityInfo> FindAbilityInfo(const std::string& moduleName, const std::string& abilityName,
        int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Find abilityInfo by bundle name and ability name.
     * @param moduleName Indicates the module name
     * @param abilityName Indicates the ability name.
     * @return Returns the AbilityInfo object if find it; returns null otherwise.
     */
    std::optional<AbilityInfo> FindAbilityInfoV9(const std::string& moduleName, const std::string& abilityName) const;
    /**
     * @brief Find abilityInfo by bundle name module name and ability name.
     * @param moduleName Indicates the module name
     * @param abilityName Indicates the ability name.
     * @return Returns ERR_OK if abilityInfo find successfully obtained; returns other ErrCode otherwise.
     */
    ErrCode FindAbilityInfo(const std::string& moduleName, const std::string& abilityName, AbilityInfo& info) const;
    /**
     * @brief Find abilityInfo of list by bundle name.
     * @param bundleName Indicates the bundle name.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo of list if find it; returns null otherwise.
     */
    std::optional<std::vector<AbilityInfo>> FindAbilityInfos(int32_t userId = Constants::UNSPECIFIED_USERID) const;

    /**
     * @brief Transform the InnerBundleInfo object to string.
     * @return Returns the string object
     */
    std::string ToString() const;
    /**
     * @brief Add ability infos to old InnerBundleInfo object.
     * @param abilityInfos Indicates the AbilityInfo object to be add.
     * @return
     */
    void AddModuleAbilityInfo(const std::map<std::string, AbilityInfo>& abilityInfos)
    {
        for (const auto& ability : abilityInfos) {
            baseAbilityInfos_.try_emplace(ability.first, ability.second);
        }
    }

    /**
     * @brief Add skill infos to old InnerBundleInfo object.
     * @param skillInfos Indicates the Skill object to be add.
     * @return
     */
    void AddModuleSkillInfo(const std::map<std::string, std::vector<Skill>>& skillInfos)
    {
        for (const auto& skills : skillInfos) {
            skillInfos_.try_emplace(skills.first, skills.second);
        }
    }
    void AddModuleExtensionSkillInfos(const std::map<std::string, std::vector<Skill>>& extensionSkillInfos)
    {
        for (const auto& skills : extensionSkillInfos) {
            extensionSkillInfos_.try_emplace(skills.first, skills.second);
        }
    }

    /**
     * @brief Add innerModuleInfos to old InnerBundleInfo object.
     * @param innerModuleInfos Indicates the InnerModuleInfo object to be add.
     * @return
     */
    void AddInnerModuleInfo(const std::map<std::string, InnerModuleInfo>& innerModuleInfos)
    {
        for (const auto& info : innerModuleInfos) {
            innerModuleInfos_.try_emplace(info.first, info.second);
        }
    }
    /**
     * @brief Get application name.
     * @return Return application name
     */
    std::string GetApplicationName() const
    {
        return baseApplicationInfo_->name;
    }
    /**
     * @brief Set bundle status.
     * @param status Indicates the BundleStatus object to set.
     * @return
     */
    void SetBundleStatus(const BundleStatus& status)
    {
        bundleStatus_ = status;
    }
    /**
     * @brief Get bundle status.
     * @return Return the BundleStatus object
     */
    BundleStatus GetBundleStatus() const
    {
        return bundleStatus_;
    }
    /**
     * @brief Set bundle install time.
     * @param time Indicates the install time to set.
     * @param userId Indicates the user ID.
     * @return
     */
    void SetBundleInstallTime(const int64_t time, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get bundle install time.
     * @param userId Indicates the user ID.
     * @return Return the bundle install time.
     */
    int64_t GetBundleInstallTime(int32_t userId = Constants::UNSPECIFIED_USERID) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            HILOG_ERROR("can not find userId %{public}d when GetBundleInstallTime", userId);
            return -1;
        }
        return innerBundleUserInfo.installTime;
    }
    /**
     * @brief Set bundle update time.
     * @param time Indicates the update time to set.
     * @param userId Indicates the user ID.
     * @return
     */
    void SetBundleUpdateTime(const int64_t time, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get bundle update time.
     * @param userId Indicates the user ID.
     * @return Return the bundle update time.
     */
    int64_t GetBundleUpdateTime(int32_t userId = Constants::UNSPECIFIED_USERID) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            HILOG_ERROR("can not find userId %{public}d when GetBundleUpdateTime", userId);
            return -1;
        }
        return innerBundleUserInfo.updateTime;
    }
    /**
     * @brief Get bundle name.
     * @return Return bundle name
     */
    const std::string GetBundleName() const
    {
        return baseApplicationInfo_->bundleName;
    }
    /**
     * @brief Get baseBundleInfo.
     * @return Return the BundleInfo object.
     */
    BundleInfo GetBaseBundleInfo() const
    {
        return *baseBundleInfo_;
    }
    /**
     * @brief Set baseBundleInfo.
     * @param bundleInfo Indicates the BundleInfo object.
     */
    void SetBaseBundleInfo(const BundleInfo& bundleInfo)
    {
        *baseBundleInfo_ = bundleInfo;
    }
    /**
     * @brief Update baseBundleInfo.
     * @param bundleInfo Indicates the new BundleInfo object.
     * @return
     */
    void UpdateBaseBundleInfo(const BundleInfo& bundleInfo, bool isEntry);
    /**
     * @brief Get baseApplicationInfo.
     * @return Return the ApplicationInfo object.
     */
    ApplicationInfo GetBaseApplicationInfo() const
    {
        return *baseApplicationInfo_;
    }
    /**
     * @brief Set baseApplicationInfo.
     * @param applicationInfo Indicates the ApplicationInfo object.
     */
    void SetBaseApplicationInfo(const ApplicationInfo& applicationInfo)
    {
        *baseApplicationInfo_ = applicationInfo;
    }
    /**
     * @brief Update baseApplicationInfo.
     * @param applicationInfo Indicates the ApplicationInfo object.
     */
    void UpdateBaseApplicationInfo(const ApplicationInfo& applicationInfo);
    /**
     * @brief Get application enabled.
     * @param userId Indicates the user ID.
     * @return Return whether the application is enabled.
     */
    bool GetApplicationEnabled(int32_t userId = Constants::UNSPECIFIED_USERID) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            HILOG_INFO("can not find userId %{public}d when GetApplicationEnabled", userId);
            return false;
        }

        return innerBundleUserInfo.bundleUserInfo.enabled;
    }
    ErrCode GetApplicationEnabledV9(int32_t userId, bool& isEnabled) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            HILOG_INFO("can not find bundleUserInfo in userId: %{public}d when GetApplicationEnabled", userId);
            return ERR_BUNDLE_MANAGER_BUNDLE_NOT_EXIST;
        }
        isEnabled = innerBundleUserInfo.bundleUserInfo.enabled;
        return ERR_OK;
    }
    /**
     * @brief Set application enabled.
     * @param userId Indicates the user ID.
     * @return Returns ERR_OK if the SetApplicationEnabled is successfully; returns error code otherwise.
     */
    ErrCode SetApplicationEnabled(bool enabled, int32_t userId = Constants::UNSPECIFIED_USERID);
    /**
     * @brief Get application code path.
     * @return Return the string object.
     */
    const std::string GetAppCodePath() const
    {
        return baseApplicationInfo_->codePath;
    }
    /**
     * @brief Set application code path.
     * @param codePath Indicates the code path to be set.
     */
    void SetAppCodePath(const std::string codePath)
    {
        baseApplicationInfo_->codePath = codePath;
    }
    void SetPid(int32_t pid)
    {
        if (baseApplicationInfo_) {
            baseApplicationInfo_->pid = pid;
        }
    }
    void SetUid(int32_t uid)
    {
        if (baseApplicationInfo_) {
            baseApplicationInfo_->uid = uid;
        }
    }

    void SetBundleName(const std::string& bundleName)
    {
        if (baseApplicationInfo_) {
            baseApplicationInfo_->bundleName = bundleName;
        }
    }

    /**
     * @brief Insert innerModuleInfos.
     * @param modulePackage Indicates the modulePackage object as key.
     * @param innerModuleInfo Indicates the InnerModuleInfo object as value.
     */
    void InsertInnerModuleInfo(const std::string& modulePackage, const InnerModuleInfo& innerModuleInfo)
    {
        innerModuleInfos_.try_emplace(modulePackage, innerModuleInfo);
    }
    /**
     * @brief Insert AbilityInfo.
     * @param key bundleName.moduleName.abilityName
     * @param abilityInfo value.
     */
    void InsertAbilitiesInfo(const std::string& key, const AbilityInfo& abilityInfo)
    {
        baseAbilityInfos_.emplace(key, abilityInfo);
    }

    /**
     * @brief Insert ability skillInfos.
     * @param key bundleName.moduleName.abilityName
     * @param skills ability skills.
     */
    void InsertSkillInfo(const std::string& key, const std::vector<Skill>& skills)
    {
        skillInfos_.emplace(key, skills);
    }
    /**
     * @brief Insert extension skillInfos.
     * @param key bundleName.moduleName.extensionName
     * @param skills extension skills.
     */
    void InsertExtensionSkillInfo(const std::string& key, const std::vector<Skill>& skills)
    {
        extensionSkillInfos_.emplace(key, skills);
    }
    /**
     * @brief Find AbilityInfo object by Uri.
     * @param abilityUri Indicates the ability uri.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo object if find it; returns null otherwise.
     */
    std::optional<AbilityInfo> FindAbilityInfoByUri(const std::string& abilityUri) const
    {
        HILOG_INFO("Uri is %{public}s", abilityUri.c_str());
        for (const auto& ability : baseAbilityInfos_) {
            auto abilityInfo = ability.second;
            if (abilityInfo.uri.size() < strlen(Constants::DATA_ABILITY_URI_PREFIX)) {
                continue;
            }

            auto configUri = abilityInfo.uri.substr(strlen(Constants::DATA_ABILITY_URI_PREFIX));
            HILOG_INFO("configUri is %{public}s", configUri.c_str());
            if (configUri == abilityUri) {
                return abilityInfo;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Find AbilityInfo object by Uri.
     * @param abilityUri Indicates the ability uri.
     * @param userId Indicates the user ID.
     * @return Returns the AbilityInfo object if find it; returns null otherwise.
     */
    void FindAbilityInfosByUri(const std::string& abilityUri, std::vector<AbilityInfo>& abilityInfos,
        int32_t userId = Constants::UNSPECIFIED_USERID)
    {
        HILOG_INFO("Uri is %{public}s", abilityUri.c_str());
        for (auto& ability : baseAbilityInfos_) {
            auto abilityInfo = ability.second;
            if (abilityInfo.uri.size() < strlen(Constants::DATA_ABILITY_URI_PREFIX)) {
                continue;
            }

            auto configUri = abilityInfo.uri.substr(strlen(Constants::DATA_ABILITY_URI_PREFIX));
            HILOG_INFO("configUri is %{public}s", configUri.c_str());
            if (configUri == abilityUri) {
                GetApplicationInfo(
                    ApplicationFlag::GET_APPLICATION_INFO_WITH_PERMISSION, userId, abilityInfo.applicationInfo);
                abilityInfos.emplace_back(abilityInfo);
            }
        }
        return;
    }
    /**
     * @brief Get all ability names in application.
     * @return Returns ability names.
     */
    auto GetAbilityNames() const
    {
        std::vector<std::string> abilityNames;
        for (auto& ability : baseAbilityInfos_) {
            abilityNames.emplace_back(ability.second.name);
        }
        return abilityNames;
    }
    /**
     * @brief Get version code in application.
     * @return Returns version code.
     */
    uint32_t GetVersionCode() const
    {
        return baseBundleInfo_->versionCode;
    }
    /**
     * @brief Get version name in application.
     * @return Returns version name.
     */
    std::string GetVersionName() const
    {
        return baseBundleInfo_->versionName;
    }
    /**
     * @brief Get vendor in application.
     * @return Returns vendor.
     */
    std::string GetVendor() const
    {
        return baseBundleInfo_->vendor;
    }
    /**
     * @brief Get comparible version in application.
     * @return Returns comparible version.
     */
    uint32_t GetCompatibleVersion() const
    {
        return baseBundleInfo_->compatibleVersion;
    }
    /**
     * @brief Get target version in application.
     * @return Returns target version.
     */
    uint32_t GetTargetVersion() const
    {
        return baseBundleInfo_->targetVersion;
    }
    /**
     * @brief Get release type in application.
     * @return Returns release type.
     */
    std::string GetReleaseType() const
    {
        return baseBundleInfo_->releaseType;
    }
    /**
     * @brief Get minCompatibleVersionCode in base bundleInfo.
     * @return Returns release type.
     */
    uint32_t GetMinCompatibleVersionCode() const
    {
        return baseBundleInfo_->minCompatibleVersionCode;
    }
    /**
     * @brief Get install mark in application.
     * @return Returns install mark.
     */
    void SetInstallMark(
        const std::string& bundleName, const std::string& packageName, const InstallExceptionStatus& status)
    {
        mark_.bundleName = bundleName;
        mark_.packageName = packageName;
        mark_.status = status;
    }
    /**
     * @brief Get install mark in application.
     * @return Returns install mark.
     */
    InstallMark GetInstallMark() const
    {
        return mark_;
    }
    /**
     * @brief Set application base data dir.
     * @param baseDataDir Indicates the dir to be set.
     */
    void SetBaseDataDir(const std::string& baseDataDir)
    {
        baseDataDir_ = baseDataDir;
    }
    /**
     * @brief Get application base data dir.
     * @return Return the string object.
     */
    std::string GetBaseDataDir() const
    {
        return baseDataDir_;
    }
    /**
     * @brief Get application data dir.
     * @return Return the string object.
     */
    std::string GetAppDataDir() const
    {
        return baseApplicationInfo_->dataDir;
    }
    /**
     * @brief Set application data dir.
     * @param dataDir Indicates the data Dir to be set.
     */
    void SetAppDataDir(std::string dataDir)
    {
        baseApplicationInfo_->dataDir = dataDir;
    }
    /**
     * @brief Set application data base dir.
     * @param dataBaseDir Indicates the data base Dir to be set.
     */
    void SetAppDataBaseDir(std::string dataBaseDir)
    {
        baseApplicationInfo_->dataBaseDir = dataBaseDir;
    }
    /**
     * @brief Set application cache dir.
     * @param cacheDir Indicates the cache Dir to be set.
     */
    void SetAppCacheDir(std::string cacheDir)
    {
        baseApplicationInfo_->cacheDir = cacheDir;
    }
    /**
     * @brief Get application uid.
     * @param userId Indicates the user ID.
     * @return Returns the uid.
     */
    int GetUid(int32_t userId = Constants::UNSPECIFIED_USERID) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            return Constants::INVALID_UID;
        }

        return innerBundleUserInfo.uid;
    }
    /**
     * @brief Get application gid.
     * @param userId Indicates the user ID.
     * @return Returns the gid.
     */
    int GetGid(int32_t userId = Constants::UNSPECIFIED_USERID) const
    {
        InnerBundleUserInfo innerBundleUserInfo;
        if (!GetInnerBundleUserInfo(userId, innerBundleUserInfo)) {
            return Constants::INVALID_GID;
        }

        if (innerBundleUserInfo.gids.empty()) {
            return Constants::INVALID_GID;
        }

        return innerBundleUserInfo.gids[0];
    }
    /**
     * @brief Set application gid.
     * @param gid Indicates the gid to be set.
     */
    void SetGid(int gid) {}
    /**
     * @brief Get application AppType.
     * @return Returns the AppType.
     */
    Constants::AppType GetAppType() const
    {
        return appType_;
    }
    /**
     * @brief Set application AppType.
     * @param gid Indicates the AppType to be set.
     */
    void SetAppType(Constants::AppType appType)
    {
        appType_ = appType;
        if (appType_ == Constants::AppType::SYSTEM_APP) {
            baseApplicationInfo_->isSystemApp = true;
        } else {
            baseApplicationInfo_->isSystemApp = false;
        }
    }
    /**
     * @brief Get application user id.
     * @return Returns the user id.
     */
    int GetUserId() const
    {
        return userId_;
    }
    /**
     * @brief Set application user id.
     * @param gid Indicates the user id to be set.
     */
    void SetUserId(int userId)
    {
        userId_ = userId;
    }

    // only used in install progress with newInfo
    std::string GetCurrentModulePackage() const
    {
        return currentPackage_;
    }
    void SetCurrentModulePackage(const std::string& modulePackage)
    {
        currentPackage_ = modulePackage;
    }
    void AddModuleSrcDir(const std::string& moduleSrcDir)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).modulePath = moduleSrcDir;
        }
    }
    void AddModuleDataDir(const std::string& moduleDataDir)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).moduleDataDir = moduleDataDir;
        }
    }

    void AddModuleResPath(const std::string& moduleSrcDir)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            std::string moduleResPath;
            if (isNewVersion_) {
                moduleResPath = moduleSrcDir + Constants::PATH_SEPARATOR + Constants::RESOURCES_INDEX;
            } else {
                moduleResPath = moduleSrcDir + Constants::PATH_SEPARATOR + Constants::ASSETS_DIR +
                                Constants::PATH_SEPARATOR + innerModuleInfos_.at(currentPackage_).distro.moduleName +
                                Constants::PATH_SEPARATOR + Constants::RESOURCES_INDEX;
            }

            innerModuleInfos_.at(currentPackage_).moduleResPath = moduleResPath;
            for (auto& abilityInfo : baseAbilityInfos_) {
                abilityInfo.second.resourcePath = moduleResPath;
            }
        }
    }

    void SetModuleHapPath(const std::string& hapPath)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).hapPath = hapPath;
            for (auto& abilityInfo : baseAbilityInfos_) {
                abilityInfo.second.hapPath = hapPath;
            }
        }
    }

    const std::string& GetModuleHapPath(const std::string& modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).hapPath;
        }

        return Constants::EMPTY_STRING;
    }

    const std::string& GetModuleName(const std::string& modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).moduleName;
        }

        return Constants::EMPTY_STRING;
    }

    const std::string& GetCurModuleName() const;

    std::vector<DefinePermission> GetDefinePermissions() const
    {
        std::vector<DefinePermission> definePermissions;
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            definePermissions = innerModuleInfos_.at(currentPackage_).definePermissions;
        }
        return definePermissions;
    }

    std::vector<RequestPermission> GetRequestPermissions() const
    {
        std::vector<RequestPermission> requestPermissions;
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            requestPermissions = innerModuleInfos_.at(currentPackage_).requestPermissions;
        }
        return requestPermissions;
    }

    std::vector<DefinePermission> GetAllDefinePermissions() const;

    std::vector<RequestPermission> GetAllRequestPermissions() const;

    bool FindModule(std::string modulePackage) const
    {
        return (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end());
    }

    bool IsEntryModule(std::string modulePackage) const
    {
        if (FindModule(modulePackage)) {
            return innerModuleInfos_.at(modulePackage).isEntry;
        }
        return false;
    }

    bool GetIsKeepAlive() const
    {
        return baseBundleInfo_->isKeepAlive;
    }

    void SetIsFreeInstallApp(bool isFreeInstall)
    {
        baseApplicationInfo_->isFreeInstallApp = isFreeInstall;
    }

    bool GetIsFreeInstallApp() const
    {
        return baseApplicationInfo_->isFreeInstallApp;
    }

    std::string GetMainAbility() const;

    void GetMainAbilityInfo(AbilityInfo& abilityInfo) const;

    std::string GetModuleDir(std::string modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).modulePath;
        }
        return Constants::EMPTY_STRING;
    }

    std::string GetModuleDataDir(std::string modulePackage) const
    {
        if (innerModuleInfos_.find(modulePackage) != innerModuleInfos_.end()) {
            return innerModuleInfos_.at(modulePackage).moduleDataDir;
        }
        return Constants::EMPTY_STRING;
    }

    bool IsDisabled() const
    {
        return (bundleStatus_ == BundleStatus::DISABLED);
    }

    bool IsEnabled() const
    {
        return (bundleStatus_ == BundleStatus::ENABLED);
    }

    bool IsOnlyModule(const std::string& modulePackage)
    {
        if ((innerModuleInfos_.size() == 1) && (innerModuleInfos_.count(modulePackage) == 1)) {
            return true;
        }
        return false;
    }

    void SetProvisionId(const std::string& provisionId)
    {
        baseBundleInfo_->appId = baseBundleInfo_->name + Constants::FILE_UNDERLINE + provisionId;
    }

    std::string GetProvisionId() const
    {
        if (!baseBundleInfo_->appId.empty()) {
            return baseBundleInfo_->appId.substr(baseBundleInfo_->name.size() + 1);
        }
        return "";
    }

    std::string GetAppId() const
    {
        return baseBundleInfo_->appId;
    }

    void SetAppFeature(const std::string& appFeature)
    {
        appFeature_ = appFeature;
    }

    std::string GetAppFeature() const
    {
        return appFeature_;
    }

    void SetAppPrivilegeLevel(const std::string& appPrivilegeLevel)
    {
        if (appPrivilegeLevel.empty()) {
            return;
        }
        baseApplicationInfo_->appPrivilegeLevel = appPrivilegeLevel;
    }

    std::string GetAppPrivilegeLevel() const
    {
        return baseApplicationInfo_->appPrivilegeLevel;
    }

    bool HasEntry() const;

    // use for new Info in updating progress
    void RestoreFromOldInfo(const InnerBundleInfo& oldInfo)
    {
        SetAppCodePath(oldInfo.GetAppCodePath());
        SetBaseDataDir(oldInfo.GetBaseDataDir());
    }
    void RestoreModuleInfo(const InnerBundleInfo& oldInfo)
    {
        if (oldInfo.FindModule(currentPackage_)) {
            innerModuleInfos_.at(currentPackage_).moduleDataDir = oldInfo.GetModuleDataDir(currentPackage_);
        }
    }

    void SetModuleHashValue(const std::string& hashValue)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).hashValue = hashValue;
        }
    }

    void SetModuleCpuAbi(const std::string& cpuAbi)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).cpuAbi = cpuAbi;
        }
    }

    void SetModuleNativeLibraryPath(const std::string& nativeLibraryPath)
    {
        if (innerModuleInfos_.count(currentPackage_) == 1) {
            innerModuleInfos_.at(currentPackage_).nativeLibraryPath = nativeLibraryPath;
        }
    }
    /**
     * @brief Set ability enabled.
     * @param moduleName Indicates the moduleName.
     * @param abilityName Indicates the abilityName.
     * @param isEnabled Indicates the ability enabled.
     * @param userId Indicates the user id.
     * @return Returns ERR_OK if the setAbilityEnabled is successfully; returns error code otherwise.
     */
    ErrCode SetAbilityEnabled(
        const std::string& moduleName, const std::string& abilityName, bool isEnabled, int32_t userId);
    /**
     * @brief Set the Application Need Recover object
     * @param moduleName Indicates the module name of the application.
     * @param upgradeFlag Indicates the module is need update or not.
     * @return Return ERR_OK if set data successfully.
     */
    ErrCode SetModuleUpgradeFlag(std::string moduleName, int32_t upgradeFlag);

    /**
     * @brief Get the Application Need Recover object
     * @param moduleName Indicates the module name of the application.
     * @return upgradeFlag type,NOT_UPGRADE means not need to be upgraded,SINGLE_UPGRADE means
     *         single module need to be upgraded,RELATION_UPGRADE means relation module need to be upgraded.
     */
    // int32_t GetModuleUpgradeFlag(std::string moduleName) const;

    /**
     * @brief Obtains configuration information about an application.
     * @param flags Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     */
    void GetApplicationInfo(int32_t flags, int32_t userId, ApplicationInfo& appInfo) const;
    /**
     * @brief Obtains configuration information about an application.
     * @param flags Indicates the flag used to specify information contained
     *             in the ApplicationInfo object that will be returned.
     * @param userId Indicates the user ID.
     * @param appInfo Indicates the obtained ApplicationInfo object.
     * @return return ERR_OK if getApplicationInfo successfully, return error code otherwise.
     */
    ErrCode GetApplicationInfoV9(int32_t flags, int32_t userId, ApplicationInfo& appInfo) const;
    /**
     * @brief Obtains configuration information about an bundle.
     * @param flags Indicates the flag used to specify information contained in the BundleInfo that will be returned.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @param userId Indicates the user ID.
     */
    bool GetBundleInfo(int32_t flags, BundleInfo& bundleInfo, int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Obtains configuration information about an bundle.
     * @param flags Indicates the flag used to specify information contained in the BundleInfo that will be returned.
     * @param bundleInfos Indicates all of the obtained BundleInfo objects.
     * @param userId Indicates the user ID.
     * @return Returns ERR_OK if the BundleInfo is successfully obtained; returns error code otherwise.
     */
    ErrCode GetBundleInfoV9(
        int32_t flags, BundleInfo& bundleInfo, int32_t userId = Constants::UNSPECIFIED_USERID) const;
    /**
     * @brief Check if special metadata is in the application.
     * @param metaData Indicates the special metaData.
     * @param bundleInfos Returns true if the metadata in application; returns false otherwise.
     */
    bool CheckSpecialMetaData(const std::string& metaData) const;

    std::optional<InnerModuleInfo> GetInnerModuleInfoByModuleName(const std::string& moduleName) const;

    void GetModuleNames(std::vector<std::string>& moduleNames) const;

    const std::map<std::string, InnerModuleInfo>& GetInnerModuleInfos() const
    {
        return innerModuleInfos_;
    }
    /**
     * @brief Fetch all innerModuleInfos, can be modify.
     */
    std::map<std::string, InnerModuleInfo>& FetchInnerModuleInfos()
    {
        return innerModuleInfos_;
    }
    /**
     * @brief Fetch all abilityInfos, can be modify.
     */
    std::map<std::string, AbilityInfo>& FetchAbilityInfos()
    {
        return baseAbilityInfos_;
    }
    /**
     * @brief Obtains all abilityInfos.
     */
    const std::map<std::string, AbilityInfo>& GetInnerAbilityInfos() const
    {
        return baseAbilityInfos_;
    }
    /**
     * @brief Obtains all skillInfos.
     */
    const std::map<std::string, std::vector<Skill>>& GetInnerSkillInfos() const
    {
        return skillInfos_;
    }

    /**
     * @brief Obtains all extensionSkillInfos.
     */
    const std::map<std::string, std::vector<Skill>>& GetExtensionSkillInfos() const
    {
        return extensionSkillInfos_;
    }
    /**
     * @brief Get the bundle is whether removable.
     * @return Return whether the bundle is removable.
     */
    bool IsRemovable() const
    {
        return baseApplicationInfo_->removable;
    }
    void SetIsPreInstallApp(bool isPreInstallApp)
    {
        baseBundleInfo_->isPreInstallApp = isPreInstallApp;
    }
    bool IsPreInstallApp() const
    {
        return baseBundleInfo_->isPreInstallApp;
    }
    /**
     * @brief Get whether the bundle is a system app.
     * @return Return whether the bundle is a system app.
     */
    bool IsSystemApp() const
    {
        return baseApplicationInfo_->isSystemApp;
    }
    /**
     * @brief Get all InnerBundleUserInfo.
     * @return Return about all userinfo under the app.
     */
    const std::map<std::string, InnerBundleUserInfo>& GetInnerBundleUserInfos() const
    {
        return innerBundleUserInfos_;
    }
    /**
     * @brief Reset bundle state.
     * @param userId Indicates the userId to set.
     */
    void ResetBundleState(int32_t userId);
    /**
     * @brief Set userId to remove userinfo.
     * @param userId Indicates the userId to set.
     */
    void RemoveInnerBundleUserInfo(int32_t userId);
    /**
     * @brief Set userId to add userinfo.
     * @param userId Indicates the userInfo to set.
     */
    void AddInnerBundleUserInfo(const InnerBundleUserInfo& userInfo);
    /**
     * @brief Set userId to add userinfo.
     * @param userId Indicates the userInfo to set.
     * @param userInfo Indicates the userInfo to get.
     * @return Return whether the user information is obtained successfully.
     */
    bool GetInnerBundleUserInfo(int32_t userId, InnerBundleUserInfo& userInfo) const;
    /**
     * @brief  Check whether the user exists.
     * @param userId Indicates the userInfo to set.
     * @return Return whether the user exists..
     */
    bool HasInnerBundleUserInfo(int32_t userId) const;
    /**
     * @brief  Check whether onlyCreateBundleUser.
     * @return Return onlyCreateBundleUser.
     */
    bool IsOnlyCreateBundleUser() const
    {
        return onlyCreateBundleUser_;
    }
    /**
     * @brief Set onlyCreateBundleUser.
     * @param onlyCreateBundleUser Indicates the onlyCreateBundleUser.
     */
    void SetOnlyCreateBundleUser(bool onlyCreateBundleUser)
    {
        onlyCreateBundleUser_ = onlyCreateBundleUser;
    }
    /**
     * @brief Check whether isSingleton.
     * @return Return isSingleton.
     */
    bool IsSingleton() const
    {
        return baseApplicationInfo_->singleton;
    }
    /**
     * @brief Get response userId.
     * @param userId Indicates the request userId..
     * @return Return response userId.
     */
    int32_t GetResponseUserId(int32_t requestUserId) const;

    std::vector<std::string> GetModuleNameVec() const
    {
        std::vector<std::string> moduleVec;
        for (const auto& it : innerModuleInfos_) {
            moduleVec.emplace_back(it.first);
        }
        return moduleVec;
    }

    uint32_t GetAccessTokenId(const int32_t userId) const
    {
        InnerBundleUserInfo userInfo;
        if (GetInnerBundleUserInfo(userId, userInfo)) {
            return userInfo.accessTokenId;
        }
        return 0;
    }

    void SetAccessTokenId(uint32_t accessToken, const int32_t userId);

    uint64_t GetAccessTokenIdEx(const int32_t userId) const
    {
        InnerBundleUserInfo userInfo;
        if (GetInnerBundleUserInfo(userId, userInfo)) {
            return userInfo.accessTokenIdEx;
        }
        return 0;
    }

    void SetIsNewVersion(bool flag)
    {
        isNewVersion_ = flag;
    }

    bool GetIsNewVersion() const
    {
        return isNewVersion_;
    }

    void SetAllowedAcls(const std::vector<std::string>& allowedAcls)
    {
        allowedAcls_.clear();
        for (const auto& acl : allowedAcls) {
            allowedAcls_.emplace_back(acl);
        }
    }

    std::vector<std::string> GetAllowedAcls() const
    {
        return allowedAcls_;
    }
    /**
     * @brief ability is enabled.
     * @param abilityInfo Indicates the abilityInfo.
     * @param userId Indicates the user Id.
     * @return Return set ability enabled result.
     */
    bool IsAbilityEnabled(const AbilityInfo& abilityInfo, int32_t userId) const;

    ErrCode IsAbilityEnabledV9(const AbilityInfo& abilityInfo, int32_t userId, bool& isEnable) const;

    bool IsAccessible() const
    {
        return baseApplicationInfo_->accessible;
    }

    bool GetDependentModuleNames(const std::string& moduleName, std::vector<std::string>& dependentModuleNames) const;

    bool GetAllDependentModuleNames(
        const std::string& moduleName, std::vector<std::string>& dependentModuleNames) const;

    bool IsBundleRemovable() const;
    /**
     * @brief Which modules can be removed.
     * @param moduleToDelete Indicates the modules.
     * @return Return get module isRemoved result
     */
    bool GetRemovableModules(std::vector<std::string>& moduleToDelete) const;
    /**
     * @brief Get freeInstall module.
     * @param freeInstallModule Indicates the modules.
     * @return Return get freeInstall module result
     */
    bool GetFreeInstallModules(std::vector<std::string>& freeInstallModule) const;
    /**
     * @brief Whether module of userId is exist.
     * @param moduleName Indicates the moduleName.
     * @param userId Indicates the userId.
     * @return Return get module exist result.
     */
    bool IsUserExistModule(const std::string& moduleName, int32_t userId) const;
    /**
     * @brief whether userId's module should be removed.
     * @param moduleName Indicates the moduleName.
     * @param userId Indicates the userId.
     * @param isRemovable Indicates the module whether is removable.
     * @return Return get module isRemoved result.
     */
    ErrCode IsModuleRemovable(const std::string& moduleName, int32_t userId, bool& isRemovable) const;
    /**
     * @brief Add module removable info
     * @param info Indicates the innerModuleInfo of module.
     * @param stringUserId Indicates the string userId add to isRemovable map.
     * @param isEnable Indicates the value of enable module is removed.
     * @return Return add module isRemovable info result.
     */
    bool AddModuleRemovableInfo(InnerModuleInfo& info, const std::string& stringUserId, bool isEnable) const;
    /**
     * @brief Set userId's module value of isRemoved.
     * @param moduleName Indicates the moduleName.
     * @param isEnable Indicates the module isRemovable is enable.
     * @param userId Indicates the userId.
     * @return Return set module isRemoved result.
     */
    bool SetModuleRemovable(const std::string& moduleName, bool isEnable, int32_t userId);
    /**
     * @brief Delete userId isRemoved info from module.
     * @param moduleName Indicates the moduleName.
     * @param userId Indicates the userId.
     * @return
     */
    void DeleteModuleRemovable(const std::string& moduleName, int32_t userId);
    /**
     * @brief Delete removable info.
     * @param info Indicates the innerModuleInfo of module.
     * @param stringUserId Indicates the string userId of isRemovable map.
     * @return
     */
    void DeleteModuleRemovableInfo(InnerModuleInfo& info, const std::string& stringUserId);

    void SetEntryInstallationFree(bool installationFree)
    {
        baseBundleInfo_->entryInstallationFree = installationFree;
        if (installationFree) {
            baseApplicationInfo_->needAppDetail = false;
            baseApplicationInfo_->appDetailAbilityLibraryPath = Constants::EMPTY_STRING;
        }
    }

    bool GetEntryInstallationFree() const
    {
        return baseBundleInfo_->entryInstallationFree;
    }

    void SetAppIndex(int32_t appIndex)
    {
        appIndex_ = appIndex;
    }

    int32_t GetAppIndex() const
    {
        return appIndex_;
    }

    void SetIsSandbox(bool isSandbox)
    {
        isSandboxApp_ = isSandbox;
    }

    bool GetIsSandbox() const
    {
        return isSandboxApp_;
    }

    void CleanInnerBundleUserInfos()
    {
        innerBundleUserInfos_.clear();
    }

    std::vector<SandboxAppPersistentInfo> GetSandboxPersistentInfo() const
    {
        return sandboxPersistentInfo_;
    }

    void AddSandboxPersistentInfo(const SandboxAppPersistentInfo& info)
    {
        auto it = std::find_if(sandboxPersistentInfo_.begin(), sandboxPersistentInfo_.end(),
            [&info](const auto& sandboxInfo) { return sandboxInfo.appIndex == info.appIndex; });

        if (it != sandboxPersistentInfo_.end()) {
            sandboxPersistentInfo_.erase(it);
        }
        sandboxPersistentInfo_.emplace_back(info);
    }

    void RemoveSandboxPersistentInfo(const SandboxAppPersistentInfo& info)
    {
        auto it = std::find_if(sandboxPersistentInfo_.begin(), sandboxPersistentInfo_.end(),
            [&info](const auto& sandboxInfo) { return sandboxInfo.appIndex == info.appIndex; });

        if (it == sandboxPersistentInfo_.end()) {
            return;
        }
        sandboxPersistentInfo_.erase(it);
    }

    void ClearSandboxPersistentInfo()
    {
        sandboxPersistentInfo_.clear();
    }

    std::string GetCertificateFingerprint() const
    {
        return baseApplicationInfo_->fingerprint;
    }

    void SetCertificateFingerprint(const std::string& fingerprint)
    {
        baseApplicationInfo_->fingerprint = fingerprint;
    }

    const std::string& GetNativeLibraryPath() const
    {
        return baseApplicationInfo_->nativeLibraryPath;
    }

    void SetNativeLibraryPath(const std::string& nativeLibraryPath)
    {
        baseApplicationInfo_->nativeLibraryPath = nativeLibraryPath;
    }

    const std::string& GetArkNativeFileAbi() const
    {
        return baseApplicationInfo_->arkNativeFileAbi;
    }

    void SetArkNativeFileAbi(const std::string& arkNativeFileAbi)
    {
        baseApplicationInfo_->arkNativeFileAbi = arkNativeFileAbi;
    }

    const std::string& GetArkNativeFilePath() const
    {
        return baseApplicationInfo_->arkNativeFilePath;
    }

    void SetArkNativeFilePath(const std::string& arkNativeFilePath)
    {
        baseApplicationInfo_->arkNativeFilePath = arkNativeFilePath;
    }

    const std::string& GetCpuAbi() const
    {
        return baseApplicationInfo_->cpuAbi;
    }

    void SetCpuAbi(const std::string& cpuAbi)
    {
        baseApplicationInfo_->cpuAbi = cpuAbi;
    }

    void SetRemovable(bool removable)
    {
        baseApplicationInfo_->removable = removable;
    }

    void SetKeepAlive(bool keepAlive)
    {
        baseApplicationInfo_->keepAlive = keepAlive;
        baseBundleInfo_->isKeepAlive = keepAlive;
    }

    void SetSingleton(bool singleton)
    {
        baseApplicationInfo_->singleton = singleton;
        baseBundleInfo_->singleton = singleton;
    }

    void SetRunningResourcesApply(bool runningResourcesApply)
    {
        baseApplicationInfo_->runningResourcesApply = runningResourcesApply;
    }

    void SetAssociatedWakeUp(bool associatedWakeUp)
    {
        baseApplicationInfo_->associatedWakeUp = associatedWakeUp;
    }

    void SetUserDataClearable(bool userDataClearable)
    {
        baseApplicationInfo_->userDataClearable = userDataClearable;
    }

    void SetHideDesktopIcon(bool hideDesktopIcon)
    {
        baseApplicationInfo_->hideDesktopIcon = hideDesktopIcon;
        if (hideDesktopIcon) {
            baseApplicationInfo_->needAppDetail = false;
            baseApplicationInfo_->appDetailAbilityLibraryPath = Constants::EMPTY_STRING;
        }
    }

    void SetFormVisibleNotify(bool formVisibleNotify)
    {
        baseApplicationInfo_->formVisibleNotify = formVisibleNotify;
    }

    void SetAllowCommonEvent(const std::vector<std::string>& allowCommonEvent)
    {
        baseApplicationInfo_->allowCommonEvent.clear();
        for (const auto& event : allowCommonEvent) {
            baseApplicationInfo_->allowCommonEvent.emplace_back(event);
        }
    }

    bool isOverlayModule(const std::string& moduleName) const
    {
        if (innerModuleInfos_.find(moduleName) == innerModuleInfos_.end()) {
            return true;
        }
        return !innerModuleInfos_.at(moduleName).targetModuleName.empty();
    }

    bool isExistedOverlayModule() const
    {
        for (const auto& innerModuleInfo : innerModuleInfos_) {
            if (!innerModuleInfo.second.targetModuleName.empty()) {
                return true;
            }
        }
        return false;
    }

    bool SetInnerModuleAtomicPreload(const std::string& moduleName, const std::vector<std::string>& preloads)
    {
        if (innerModuleInfos_.find(moduleName) == innerModuleInfos_.end()) {
            HILOG_ERROR("innerBundleInfo does not contain the module.");
            return false;
        }
        innerModuleInfos_.at(moduleName).preloads = preloads;
        return true;
    }

    bool GetHasAtomicServiceConfig() const
    {
        return hasAtomicServiceConfig_;
    }

    void SetHasAtomicServiceConfig(bool hasConfig)
    {
        hasAtomicServiceConfig_ = hasConfig;
    }

    std::string GetAtomicMainModuleName() const
    {
        return mainAtomicModuleName_;
    }

    void SetAtomicMainModuleName(std::string main)
    {
        mainAtomicModuleName_ = main;
    }

    void SetDisposedStatus(int32_t status);

    int32_t GetDisposedStatus() const;

    void SetAppDistributionType(const std::string& appDistributionType);

    std::string GetAppDistributionType() const;

    void SetAppProvisionType(const std::string& appProvisionType);

    std::string GetAppProvisionType() const;

    void SetAppCrowdtestDeadline(int64_t crowdtestDeadline);

    int64_t GetAppCrowdtestDeadline() const;

    std::vector<std::string> GetDistroModuleName() const;

    std::string GetModuleNameByPackage(const std::string& packageName) const;

    std::string GetModuleTypeByPackage(const std::string& packageName) const;

    void UpdatePrivilegeCapability(const ApplicationInfo& applicationInfo);
    void UpdateRemovable(bool isPreInstall, bool removable);
    bool FetchNativeSoAttrs(
        const std::string& requestPackage, std::string& cpuAbi, std::string& nativeLibraryPath) const;
    void UpdateNativeLibAttrs(const ApplicationInfo& applicationInfo);
    void UpdateArkNativeAttrs(const ApplicationInfo& applicationInfo);
    bool IsLibIsolated(const std::string& moduleName) const;
    std::vector<std::string> GetDeviceType(const std::string& packageName) const;
    int64_t GetLastInstallationTime() const;
    void UpdateAppDetailAbilityAttrs();
    void AddApplyQuickFixFrequency();
    int32_t GetApplyQuickFixFrequency() const;
    void ResetApplyQuickFixFrequency();

private:
    void RemoveDuplicateName(std::vector<std::string>& name) const;
    void GetBundleWithReqPermissionsV9(int32_t flags, uint32_t userId, BundleInfo& bundleInfo) const;
    void ProcessBundleFlags(int32_t flags, int32_t userId, BundleInfo& bundleInfo) const;
    void ProcessBundleWithHapModuleInfoFlag(int32_t flags, BundleInfo& bundleInfo, int32_t userId) const;
    // using for get
    Constants::AppType appType_ = Constants::AppType::THIRD_PARTY_APP;
    int uid_ = Constants::INVALID_UID;
    int gid_ = Constants::INVALID_GID;
    int userId_ = Constants::DEFAULT_USERID;
    std::string baseDataDir_;
    BundleStatus bundleStatus_ = BundleStatus::ENABLED;
    std::shared_ptr<ApplicationInfo> baseApplicationInfo_;
    std::shared_ptr<BundleInfo> baseBundleInfo_; // applicationInfo and abilityInfo empty
    std::string appFeature_;
    std::vector<std::string> allowedAcls_;
    InstallMark mark_;
    int32_t appIndex_ = Constants::INITIAL_APP_INDEX;
    int32_t disposedStatus_ = Constants::DEFAULT_DISPOSED_STATUS;
    bool isSandboxApp_ = false;
    std::string currentPackage_;
    // Auxiliary property, which is used when the application
    // has been installed when the user is created.
    bool onlyCreateBundleUser_ = false;

    std::map<std::string, InnerModuleInfo> innerModuleInfos_;

    std::map<std::string, AbilityInfo> baseAbilityInfos_;
    std::map<std::string, std::vector<Skill>> skillInfos_;

    std::map<std::string, InnerBundleUserInfo> innerBundleUserInfos_;

    bool isNewVersion_ = false;
    std::map<std::string, std::vector<Skill>> extensionSkillInfos_;

    std::vector<SandboxAppPersistentInfo> sandboxPersistentInfo_;
    int32_t applyQuickFixFrequency_ = 0;
    bool hasAtomicServiceConfig_ = false;
    std::string mainAtomicModuleName_;
};

void from_json(const nlohmann::json& jsonObject, InnerModuleInfo& info);
void from_json(const nlohmann::json& jsonObject, SkillUri& uri);
void from_json(const nlohmann::json& jsonObject, Skill& skill);
void from_json(const nlohmann::json& jsonObject, Distro& distro);
void from_json(const nlohmann::json& jsonObject, InstallMark& installMark);
void from_json(const nlohmann::json& jsonObject, DefinePermission& definePermission);
void from_json(const nlohmann::json& jsonObject, SandboxAppPersistentInfo& sandboxPersistentInfo);
void from_json(const nlohmann::json& jsonObject, Dependency& dependency);
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_INNER_BUNDLE_INFO_H
