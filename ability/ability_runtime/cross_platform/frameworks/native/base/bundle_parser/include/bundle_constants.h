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

#ifndef FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_CONSTANTS_H
#define FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_CONSTANTS_H

#include <map>
#include <string>
#include <vector>

namespace OHOS {
namespace AppExecFwk {
namespace Constants {
const std::string EMPTY_STRING = "";
constexpr const char* BUNDLE_PROFILE_NAME = "config.json";
constexpr const char* MODULE_PROFILE_NAME = "module.json";
constexpr const char* PATCH_PROFILE_NAME = "patch.json";
constexpr const char* BUNDLE_PACKFILE_NAME = "pack.info";
constexpr const char* SYSCAP_NAME = "rpcid.sc";
constexpr const char* INSTALL_FILE_SUFFIX = ".hap";
constexpr const char* INSTALL_SHARED_FILE_SUFFIX = ".hsp";
constexpr const char* QUICK_FIX_FILE_SUFFIX = ".hqf";
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
const std::string PATH_SEPARATOR = "/";
constexpr const char* FILE_UNDERLINE = "_";
constexpr const char* ILLEGAL_PATH_FIELD = "../";
const char FILE_SEPARATOR_CHAR = '/';
constexpr const char* CURRENT_DEVICE_ID = "PHONE-001";
constexpr const char* BUNDLE_DATA_BASE_FILE = "/data/bundlemgr/bmsdb.json";
constexpr const char* SYSTEM_APP_SCAN_PATH = "/system/app";
constexpr const char* SYSTEM_RESOURCES_APP_PATH = "/system/app/ohos.global.systemres";
constexpr const char* SYSTEM_APP_INSTALL_PATH = "/data/app/el1/bundle";
constexpr const char* THIRD_SYSTEM_APP_SCAN_PATH = "/system/vendor";
constexpr const char* THIRD_SYSTEM_APP_INSTALL_PATH = "/data/app/el1/bundle";
constexpr const char* THIRD_PARTY_APP_INSTALL_PATH = "/data/app/el1/bundle";
constexpr const char* HAP_COPY_PATH = "/data/service/el1/public/bms/bundle_manager_service";
constexpr const char* CACHE_DIR = "cache";
constexpr const char* TMP_SUFFIX = "_tmp";
constexpr const char* ASSETS_DIR = "assets";
constexpr const char* RESOURCES_INDEX = "resources.index";
constexpr const char* BUNDLE_CODE_DIR = "/data/app/el1/bundle/public";
constexpr const char* BUNDLE_BASE_CODE_DIR = "/data/app/el1/bundle";
constexpr const char* BUNDLE_APP_DATA_BASE_DIR = "/data/app/";
constexpr const char* BASE = "/base/";
constexpr const char* DATABASE = "/database/";
constexpr const char* HAPS = "/haps/";
constexpr const char* BUNDLE_MANAGER_SERVICE_PATH = "/data/service/el1/public/bms/bundle_manager_service";
constexpr const char* BUNDLE_USER_INFO_PATH =
    "/data/service/el1/public/bms/bundle_manager_service/bundle_user_info.json";
constexpr const char* BUNDLE_BACKUP_HOME_PATH = "/data/service/el2/%/backup/bundles/";
constexpr const char* DISTRIBUTED_FILE = "/data/service/el2/%/hmdfs/account/data/";
constexpr const char* DISTRIBUTED_FILE_NON_ACCOUNT = "/data/service/el2/%/hmdfs/non_account/data/";
constexpr const char* DISTRIBUTED_FILE_PROPERTY = "const.distributed_file_property.enabled";
constexpr const char* SANDBOX_DATA_PATH = "/data/storage/el2/base";
constexpr const char* REAL_DATA_PATH = "/data/app/el2";
constexpr const char* STREAM_INSTALL_PATH = "stream_install";
constexpr const char* QUICK_FIX_PATH = "quick_fix";
constexpr const char* DEVICE_TYPE_OF_DEFAULT = "default";
constexpr const char* DEVICE_TYPE_OF_PHONE = "phone";
constexpr const char* BUNDLE_ASAN_LOG_DIR = "/data/local/app-logs";
const std::vector<std::string> BUNDLE_EL = { "el1", "el2" };
const std::vector<std::string> BUNDLE_DATA_DIR = { "/cache", "/files", "/temp", "/preferences", "/haps" };

constexpr const char* BMS_SERVICE_NAME = "BundleMgrService";
constexpr const char* QUICK_FIX_MGR = "QuickFixMgr";
constexpr const char* INSTALLD_SERVICE_NAME = "installd";
constexpr const char* SYSTEM_APP = "system";
constexpr const char* THIRD_PARTY_APP = "third-party";
constexpr const char* ACCESS_TOKEN_ID = "accessTokenId";
constexpr int START_USERID = 100;
constexpr int DEFAULT_USERID = 0;
constexpr int INVALID_USERID = -1;
constexpr int UNSPECIFIED_USERID = -2;
constexpr int ALL_USERID = -3;
constexpr int ANY_USERID = -4;
constexpr int NOT_EXIST_USERID = -5;
constexpr int PATH_MAX_SIZE = 256;
constexpr int SIGNATURE_MATCHED = 0;
constexpr int SIGNATURE_NOT_MATCHED = 1;
constexpr int SIGNATURE_UNKNOWN_BUNDLE = 2;
constexpr int PERMISSION_GRANTED = 0;
constexpr int PERMISSION_NOT_GRANTED = -1;
constexpr int DUMP_INDENT = 4;
constexpr unsigned int INSTALLD_UMASK = 0000;
constexpr int32_t INVALID_API_VERSION = -1;
constexpr int32_t API_VERSION_EIGHT = 8;
constexpr int32_t API_VERSION_NINE = 9;

// native so
constexpr const char* ABI_DEFAULT = "default";
constexpr const char* ABI_SEPARATOR = ",";
constexpr const char* ARM_EABI = "armeabi";
constexpr const char* ARM_EABI_V7A = "armeabi-v7a";
constexpr const char* ARM64_V8A = "arm64-v8a";
constexpr const char* X86 = "x86";
constexpr const char* X86_64 = "x86_64";
const std::string LIBS = "libs/";
const std::string AN = "an/";
const std::string AP = "ap/";
constexpr const char* LIBS_ARM_EABI_V7A = "libs/armeabi-v7a/";
constexpr const char* ARM = "arm";
const std::map<std::string, std::string> ABI_MAP = {
    { ARM_EABI, "arm" },
    { ARM_EABI_V7A, "arm" },
    { ARM64_V8A, "arm64" },
    { X86, "x86" },
    { X86_64, "x86_64" },
};
constexpr const char* SO_SUFFIX = ".so";
constexpr const char* AN_SUFFIX = ".an";
constexpr const char* AI_SUFFIX = ".ai";
constexpr const char* AP_SUFFIX = ".ap";
constexpr const char* SYSTEM_LIB64 = "/system/lib64";
constexpr const char* DIFF_SUFFIX = ".diff";
constexpr const char* PATCH_PATH = "patch_";
constexpr const char* HOT_RELOAD_PATH = "hotreload_";

// uid and gid
constexpr int32_t INVALID_UID = -1;
constexpr int32_t INVALID_GID = -1;
constexpr int32_t INVALID_BUNDLEID = -1;
constexpr int32_t INVALID_ACCESS_TOKEN_ID = -1;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t BMS_UID = 1000;
constexpr int32_t BACKU_HOME_GID = 1089;
constexpr int32_t ACCOUNT_UID = 3058;
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t BMS_GID = 1000;
constexpr int32_t BASE_SYS_UID = 2100;
constexpr int32_t MAX_SYS_UID = 2899;
constexpr int32_t BASE_SYS_VEN_UID = 5000;
constexpr int32_t MAX_SYS_VEN_UID = 5999;
constexpr int32_t BASE_APP_UID = 10000;
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t MAX_APP_UID = 65535;
constexpr int32_t U_VALUE = 100000;
constexpr int32_t DATABASE_DIR_GID = 3012;
constexpr int32_t DFS_GID = 1009;
constexpr const char* PROFILE_KEY_UID_SIZE = "size";
constexpr const char* PROFILE_KEY_UID_AND_GID = "uid_and_gid";
constexpr const char* FOUNDATION_PROCESS_NAME = "foundation";

// permissions
constexpr const char* PERMISSION_INSTALL_BUNDLE = "ohos.permission.INSTALL_BUNDLE";
constexpr const char* PERMISSION_GET_BUNDLE_INFO = "ohos.permission.GET_BUNDLE_INFO";
constexpr const char* PERMISSION_GET_BUNDLE_INFO_PRIVILEGED = "ohos.permission.GET_BUNDLE_INFO_PRIVILEGED";
constexpr const char* PERMISSION_CHANGE_ABILITY_ENABLED_STATE = "ohos.permission.CHANGE_ABILITY_ENABLED_STATE";
constexpr const char* PERMISSION_REMOVECACHEFILE = "ohos.permission.REMOVE_CACHE_FILES";
constexpr const char* PERMISSION_MANAGE_DISPOSED_APP_STATUS = "ohos.permission.MANAGE_DISPOSED_APP_STATUS";
constexpr const char* PERMISSION_GET_DEFAULT_APPLICATION = "ohos.permission.GET_DEFAULT_APPLICATION";
constexpr const char* PERMISSION_SET_DEFAULT_APPLICATION = "ohos.permission.SET_DEFAULT_APPLICATION";
// install list permissions file
constexpr const char* INSTALL_LIST_PERMISSIONS_FILE_PATH = "/system/etc/app/install_list_permissions.json";

enum class AppType {
    SYSTEM_APP = 0,
    THIRD_SYSTEM_APP,
    THIRD_PARTY_APP,
};

constexpr const char* ACTION_HOME = "action.system.home";
constexpr const char* WANT_ACTION_HOME = "ohos.want.action.home";
constexpr const char* ENTITY_HOME = "entity.system.home";
constexpr const char* FLAG_HOME_INTENT_FROM_SYSTEM = "flag.home.intent.from.system";

// the ability file folder name.
constexpr const char* LIB_FOLDER_NAME = "libs";
constexpr const char* RES_FOLDER_NAME = "resources";

constexpr uint8_t MAX_LABLE_LEN = 30;
constexpr uint8_t MAX_BUNDLE_NAME = 127;
constexpr uint8_t MIN_BUNDLE_NAME = 7;
constexpr uint8_t MAX_VENDOR = 255;
constexpr uint8_t EQUAL_ZERO = 0;
constexpr uint8_t MAX_MODULE_PACKAGE = 127;
constexpr uint8_t MAX_MODULE_NAME = 31;
constexpr uint8_t MAX_MODULE_ABILITIES_READPERMISSION = 255;
constexpr uint8_t MAX_MODULE_ABILITIES_WRITEPERMISSION = 255;
constexpr uint8_t MAX_MODULE_SHORTCUTID = 63;
constexpr uint8_t MAX_MODULE_LABEL = 63;
constexpr uint8_t MAX_JSON_ELEMENT_LENGTH = 255;
constexpr uint16_t MAX_JSON_ARRAY_LENGTH = 512;
// max number of haps under one direction
constexpr uint8_t MAX_HAP_NUMBER = 128;

// distributed database
constexpr const char* APP_ID = "bundle_manager_service";
constexpr const char* STORE_ID = "installed_bundle_datas";
constexpr const char* ABILITY_USAGE_STORE_ID = "ability_usage_datas";
constexpr const char* PRE_INSTALL_DATA_STORE_ID = "preinstall_usage_datas";
constexpr const char* DISTRIBUTE_DATA_STORE_ID = "distribute_bundle_datas";
constexpr const char* DEFAULT_APP_DATA_STORE_ID = "default_app_datas";

// single max hap size
constexpr int64_t ONE_GB = 1024 * 1024 * 1024;
constexpr int64_t MAX_HAP_SIZE = ONE_GB * 4; // 4GB

constexpr const char* UID = "uid";
constexpr const char* USER_ID = "userId";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* MODULE_NAME = "moduleName";
constexpr const char* ABILITY_NAME = "abilityName";
constexpr const char* HAP_PATH = "hapPath";
constexpr int32_t MAX_LIMIT_SIZE = 4;

constexpr const char* DATA_ABILITY_URI_PREFIX = "dataability://";
const char DATA_ABILITY_URI_SEPARATOR = '/';

constexpr const char* PARAM_URI_SEPARATOR = ":///";
constexpr uint32_t PARAM_URI_SEPARATOR_LEN = 4;
constexpr const char* URI_SEPARATOR = "://";
constexpr uint32_t URI_SEPARATOR_LEN = 3;
constexpr const char* SEPARATOR = "/";

constexpr int MAX_DIMENSION_SIZE = 10;
constexpr int MAX_DEVICETYPE_SIZE = 50;

// appFeature
constexpr const char* HOS_NORMAL_APP = "hos_normal_app";
constexpr const char* HOS_SYSTEM_APP = "hos_system_app";
constexpr const char* OHOS_SYSTEM_APP = "ohos_system_app";

// rdb
constexpr const char* BUNDLE_RDB_NAME = "/bmsdb.db";
constexpr const char* BUNDLE_RDB_TABLE_NAME = "installed_bundle";
constexpr const char* PRE_BUNDLE_RDB_TABLE_NAME = "preinstalled_bundle";
constexpr const char* DEFAULT_APP_RDB_TABLE_NAME = "default_app";
constexpr const char* QUICK_FIX_RDB_TABLE_NAME = "quick_fix";
constexpr const char* JOURNAL_MODE = "WAL";
constexpr const char* SYNC_MODE = "MODE_OFF";
const int32_t BUNDLE_RDB_VERSION = 1;

// ipc
constexpr int32_t CAPACITY_SIZE = 1 * 1024 * 1000;        // 1M
constexpr int32_t MAX_CAPACITY_BUNDLES = 5 * 1024 * 1000; // 5M

// file size
constexpr int32_t INVALID_FILE_SIZE = -1;

// permission
constexpr const char* LISTEN_BUNDLE_CHANGE = "ohos.permission.LISTEN_BUNDLE_CHANGE";

// hmdfs config
constexpr const char* HMDFS_CONFIG_PATH { "/config/hmdfs/" };
constexpr const char* BUNDLE_ID_FILE = { "bid" };

// pre bundle profile
constexpr const char* DEFAULT_PRE_BUNDLE_ROOT_DIR = "/system";
constexpr const char* PRODUCT_SUFFIX = "/etc/app";
constexpr const char* INSTALL_LIST_CONFIG = "/install_list.json";
constexpr const char* UNINSTALL_LIST_CONFIG = "/uninstall_list.json";
constexpr const char* INSTALL_LIST_CAPABILITY_CONFIG = "/install_list_capability.json";
constexpr const char* INSTALL_LIST_PERMISSIONS_CONFIG = "/install_list_permissions.json";

// sandbox application
constexpr const char* SANDBOX_APP_INDEX = "sandbox_app_index";
constexpr int32_t INITIAL_APP_INDEX = 0;
constexpr int32_t MAX_APP_INDEX = 100;

// disposed status
constexpr int32_t DEFAULT_DISPOSED_STATUS = 0;

// app-distribution-type
constexpr const char* APP_DISTRIBUTION_TYPE_NONE = "none";
constexpr const char* APP_DISTRIBUTION_TYPE_APP_GALLERY = "app_gallery";
constexpr const char* APP_DISTRIBUTION_TYPE_ENTERPRISE = "enterprise";
constexpr const char* APP_DISTRIBUTION_TYPE_OS_INTEGRATION = "os_integration";
constexpr const char* APP_DISTRIBUTION_TYPE_CROWDTESTING = "crowdtesting";
// app provision type
constexpr const char* APP_PROVISION_TYPE_DEBUG = "debug";
constexpr const char* APP_PROVISION_TYPE_RELEASE = "release";

// crowdtesting
constexpr int64_t INVALID_CROWDTEST_DEADLINE = -1;

// syscap
constexpr const char* SYSCAP_SERVICE_ID = "syscap";
constexpr const char* SYSCAP_SERVICE_TYPE = "syscap";
constexpr const char* CHARACTER_OS_SYSCAP = "ossyscap";
constexpr const char* CHARACTER_PRIVATE_SYSCAP = "privatesyscap";

// thread pool
constexpr int32_t MAX_TASK_NUMBER = 10;
constexpr const char* RELATIVE_PATH = "../";

// app detail ability
constexpr const char* APP_DETAIL_ABILITY = "AppDetailAbility";

// overlay installation
constexpr int32_t OVERLAY_MINIMUM_PRIORITY = 1;
constexpr int32_t OVERLAY_MAXIMUM_PRIORITY = 100;
constexpr const char* OVERLAY_REMOVED_ACTION = "usual.event.OVERLAY_PACKAGE_REMOVED";
constexpr const char* OVERLAY_CHANGED_ACTION = "usual.event.OVERLAY_PACKAGE_CHANGED";
} // namespace Constants
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_INTERFACES_INNERKITS_APPEXECFWK_BASE_INCLUDE_BUNDLE_CONSTANTS_H
