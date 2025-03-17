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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_COMMON_PROFILE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_COMMON_PROFILE_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
namespace ProfileReader {
// common tag
constexpr const char* PROFILE_KEY_NAME = "name";
constexpr const char* PROFILE_KEY_ORIGINAL_NAME = "originalName";
constexpr const char* PROFILE_KEY_LABEL = "label";
constexpr const char* PROFILE_KEY_ICON_ID = "iconId";
constexpr const char* PROFILE_KEY_LABEL_ID = "labelId";
constexpr const char* PROFILE_KEY_DESCRIPTION = "description";
constexpr const char* PROFILE_KEY_DESCRIPTION_ID = "descriptionId";
constexpr const char* PROFILE_KEY_TYPE = "type";
constexpr const char* PROFILE_KEY_SRCPATH = "srcPath";
constexpr const char* PROFILE_KEY_SRCLANGUAGE = "srcLanguage";
constexpr const char* PRIORITY = "priority";

// bundle profile tag
constexpr const char* BUNDLE_PROFILE_KEY_APP = "app";
constexpr const char* BUNDLE_PROFILE_KEY_DEVICE_CONFIG = "deviceConfig";
constexpr const char* BUNDLE_PROFILE_KEY_MODULE = "module";
// sub  BUNDLE_PROFILE_KEY_APP
constexpr const char* BUNDLE_APP_PROFILE_KEY_BUNDLE_NAME = "bundleName";
constexpr const char* BUNDLE_APP_PROFILE_KEY_VENDOR = "vendor";
constexpr const char* BUNDLE_APP_PROFILE_KEY_VERSION = "version";
constexpr const char* BUNDLE_APP_PROFILE_KEY_API_VERSION = "apiVersion";
constexpr const char* BUNDLE_APP_PROFILE_KEY_SINGLETON = "singleton";
constexpr const char* BUNDLE_APP_PROFILE_KEY_REMOVABLE = "removable";
constexpr const char* BUNDLE_APP_PROFILE_KEY_USER_DATA_CLEARABLE = "userDataClearable";
constexpr const char* BUNDLE_APP_PROFILE_KEY_TARGETET_BUNDLE_LIST = "targetBundleList";
constexpr const char* BUNDLE_APP_PROFILE_KEY_ASAN_ENABLED = "asanEnabled";
// sub BUNDLE_APP_PROFILE_KEY_VERSION
constexpr const char* BUNDLE_APP_PROFILE_KEY_CODE = "code";
constexpr const char* BUNDLE_APP_PROFILE_KEY_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
// sub BUNDLE_APP_PROFILE_KEY_API_VERSION
constexpr const char* BUNDLE_APP_PROFILE_KEY_COMPATIBLE = "compatible";
constexpr const char* BUNDLE_APP_PROFILE_KEY_TARGET = "target";
constexpr const char* BUNDLE_APP_PROFILE_KEY_RELEASE_TYPE = "releaseType";
constexpr const char* APP_RELEASE_TYPE_VALUE_RELEASE = "Release";
// sub  BUNDLE_PROFILE_KEY_DEVICE_CONFIG
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DEFAULT = "default";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_PHONE = "phone";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_TABLET = "tablet";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_TV = "tv";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_CAR = "car";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_WEARABLE = "wearable";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_LITE_WEARABLE = "liteWearable";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_SMART_VISION = "smartVision";
// sub BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DEFAULT
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_JOINT_USER_ID = "jointUserId";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_PROCESS = "process";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_KEEP_ALIVE = "keepAlive";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_ARK = "ark";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DIRECT_LAUNCH = "directLaunch";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_SUPPORT_BACKUP = "supportBackup";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DEBUG = "debug";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_COMPRESS_NATIVE_LIBS = "compressNativeLibs";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_NETWORK = "network";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_REQ_VERSION = "reqVersion";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_FLAG = "flag";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_COMPATIBLE = "compatible";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_TARGET = "target";
// sub BUNDLE_DEVICE_CONFIG_PROFILE_KEY_NETWORK
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_USES_CLEAR_TEXT = "usesCleartext";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_SECURITY_CONFIG = "securityConfig";
// sub BUNDLE_DEVICE_CONFIG_PROFILE_KEY_SECURITY_CONFIG
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DOMAIN_SETTINGS = "domainSettings";
// sub BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DOMAIN_SETTINGS
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_CLEAR_TEXT_PERMITTED = "cleartextPermitted";
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DOMAINS = "domains";
// sub BUNDLE_DEVICE_CONFIG_PROFILE_KEY_DOMAINS
constexpr const char* BUNDLE_DEVICE_CONFIG_PROFILE_KEY_SUB_DOMAINS = "subDomains";
// sub BUNDLE_PROFILE_KEY_MODULE
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PACKAGE = "package";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SUPPORTED_MODES = "supportedModes";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_CAPABILITIES = "reqCapabilities";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SUPPORTED_REQ_CAPABILITIES = "reqCapabilities";
constexpr const char* BUNDLE_MODULE_DEPENDENCIES = "dependencies";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_IS_LIB_ISOLATED = "libIsolation";
constexpr const char* MODULE_SUPPORTED_MODES_VALUE_DRIVE = "drive";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DEVICE_TYPE = "deviceType";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_COLOR_MODE = "colorMode";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DISTRO = "distro";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_META_DATA = "metaData";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ABILITIES = "abilities";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_JS = "js";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_COMMON_EVENTS = "commonEvents";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SHORTCUTS = "shortcuts";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DEFINE_PERMISSIONS = "definePermissions";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS = "reqPermissions";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQUEST_PERMISSIONS = "requestPermissions";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_NAME = "name";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_REASON = "reason";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_USEDSCENE = "usedScene";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_ABILITY = "ability";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_WHEN = "when";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_WHEN_INUSE = "inuse";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REQ_PERMISSIONS_WHEN_ALWAYS = "always";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_CUSTOMIZE_DATA = "customizeData";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MAIN_ABILITY = "mainAbility";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SRC_PATH = "srcPath";
// sub BUNDLE_MODULE_PROFILE_KEY_DISTRO
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DELIVERY_WITH_INSTALL = "deliveryWithInstall";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MODULE_NAME = "moduleName";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MODULE_TYPE = "moduleType";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MODULE_INSTALLATION_FREE = "installationFree";
// sub BUNDLE_MODULE_PROFILE_KEY_SKILLS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ACTIONS = "actions";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ENTITIES = "entities";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_URIS = "uris";
// sub BUNDLE_MODULE_PROFILE_KEY_URIS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SCHEME = "scheme";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_HOST = "host";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PORT = "port";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PATH = "path";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PATHSTARTWITH = "pathStartWith";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PATHREGX = "pathRegx";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PATHREGEX = "pathRegex";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_TYPE = "type";
// sub BUNDLE_MODULE_PROFILE_KEY_META_DATA
constexpr const char* BUNDLE_MODULE_META_KEY_CUSTOMIZE_DATA = "customizeData";
constexpr const char* BUNDLE_MODULE_META_KEY_NAME = "name";
constexpr const char* BUNDLE_MODULE_META_KEY_VALUE = "value";
constexpr const char* BUNDLE_MODULE_META_KEY_EXTRA = "extra";
// sub BUNDLE_MODULE_PROFILE_KEY_DISTRO_TYPE
constexpr const char* MODULE_DISTRO_MODULE_TYPE_VALUE_ENTRY = "entry";
constexpr const char* MODULE_DISTRO_MODULE_TYPE_VALUE_FEATURE = "feature";
// sub BUNDLE_MODULE_PROFILE_KEY_ABILITIES
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ICON = "icon";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ICON_ID = "iconId";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_URI = "uri";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_LAUNCH_TYPE = "launchType";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_LAUNCH_THEME = "theme";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_VISIBLE = "visible";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_CONTINUABLE = "continuable";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PERMISSIONS = "permissions";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SKILLS = "skills";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PROCESS = "process";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DEVICE_CAP_ABILITY = "deviceCapability";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_FORM_ENABLED = "formEnabled";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_FORM = "form";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_ORIENTATION = "orientation";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_BACKGROUND_MODES = "backgroundModes";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_GRANT_PERMISSION = "grantPermission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_URI_PERMISSION = "uriPermission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_READ_PERMISSION = "readPermission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_WRITE_PERMISSION = "writePermission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DIRECT_LAUNCH = "directLaunch";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_CONFIG_CHANGES = "configChanges";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MISSION = "mission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_TARGET_ABILITY = "targetAbility";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MULTIUSER_SHARED = "multiUserShared";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SUPPORT_PIP_MODE = "supportPipMode";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_FORMS_ENABLED = "formsEnabled";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_FORMS = "forms";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_START_WINDOW_ICON = "startWindowIcon";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_START_WINDOW_ICON_ID = "startWindowIconId";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_START_WINDOW_BACKGROUND = "startWindowBackground";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_START_WINDOW_BACKGROUND_ID = "startWindowBackgroundId";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_REMOVE_MISSION_AFTER_TERMINATE = "removeMissionAfterTerminate";
// sub BUNDLE_MODULE_PROFILE_KEY_FORM
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_MODE = "mode";
// sub BUNDLE_MODULE_PROFILE_KEY_FORM
constexpr const char* BUNDLE_MODULE_PROFILE_FORM_ENTITY = "formEntity";
constexpr const char* BUNDLE_MODULE_PROFILE_FORM_MIN_HEIGHT = "minHeight";
constexpr const char* BUNDLE_MODULE_PROFILE_FORM_DEFAULT_HEIGHT = "defaultHeight";
constexpr const char* BUNDLE_MODULE_PROFILE_FORM_MIN_WIDTH = "minWidth";
constexpr const char* BUNDLE_MODULE_PROFILE_FORM_DEFAULT_WIDTH = "defaultWidth";
// sub BUNDLE_MODULE_PROFILE_KEY_FORMS
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_IS_DEFAULT = "isDefault";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_COLOR_MODE = "colorMode";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_SUPPORT_DIMENSIONS = "supportDimensions";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_DEFAULT_DIMENSION = "defaultDimension";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_LANDSCAPE_LAYOUTS = "landscapeLayouts";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_PORTRAIT_LAYOUTS = "portraitLayouts";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_UPDATEENABLED = "updateEnabled";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_SCHEDULED_UPDATE_TIME = "scheduledUpdateTime";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_UPDATE_DURATION = "updateDuration";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_DEEP_LINK = "deepLink";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_JS_COMPONENT_NAME = "jsComponentName";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_VALUE = "value";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_FORM_CONFIG_ABILITY = "formConfigAbility";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_FORM_VISIBLE_NOTIFY = "formVisibleNotify";
constexpr const char* BUNDLE_MODULE_PROFILE_FORMS_SRC = "src";
// sub BUNDLE_MODULE_PROFILE_KEY_JS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PAGES = "pages";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_WINDOW = "window";
// sub BUNDLE_MODULE_PROFILE_KEY_COMMON_EVENTS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_PERMISSION = "permission";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DATA = "data";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_EVENTS = "events";
constexpr const char* MODULE_ABILITY_JS_TYPE_VALUE_NORMAL = "normal";
constexpr const char* MODULE_ABILITY_JS_TYPE_VALUE_FORM = "form";
// sub BUNDLE_MODULE_PROFILE_KEY_WINDOW
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_DESIGN_WIDTH = "designWidth";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_AUTO_DESIGN_WIDTH = "autoDesignWidth";
// sub BUNDLE_MODULE_PROFILE_KEY_SHORTCUTS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SHORTCUT_ID = "shortcutId";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_SHORTCUT_WANTS = "intents";
// sub BUNDLE_MODULE_PROFILE_KEY_SHORTCUT_WANTS
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_TARGET_CLASS = "targetClass";
constexpr const char* BUNDLE_MODULE_PROFILE_KEY_TARGET_BUNDLE = "targetBundle";
// sub BUNDLE_INSTALL_MARK
constexpr const char* BUNDLE_INSTALL_MARK_BUNDLE = "installMarkBundle";
constexpr const char* BUNDLE_INSTALL_MARK_PACKAGE = "installMarkPackage";
constexpr const char* BUNDLE_INSTALL_MARK_STATUS = "installMarkStatus";
// sub BUNDLE_SANDBOX_PERSISTENT_INFO
constexpr const char* BUNDLE_SANDBOX_PERSISTENT_ACCESS_TOKEN_ID = "accessTokenId";
constexpr const char* BUNDLE_SANDBOX_PERSISTENT_APP_INDEX = "appIndex";
constexpr const char* BUNDLE_SANDBOX_PERSISTENT_USER_ID = "userId";

const uint32_t VALUE_HOME_SCREEN = 1 << 0;
// 000010 represents supporting search box
const uint32_t VALUE_SEARCHBOX = 1 << 1;

constexpr const char* KEY_HOME_SCREEN = "homeScreen";
constexpr const char* KEY_SEARCHBOX = "searchbox";

constexpr const char* BUNDLE_MODULE_PROFILE_KEY_JS_TYPE_ETS = "ets";

static std::map<std::string, uint32_t> formEntityMap;

extern thread_local int32_t parseResult;

// background modes
// different bits in the binary represent different services
// 0000001 represents data transmission services
constexpr uint32_t VALUE_DATA_TRANSFER = 1 << 0;

// 0000 0010 represents audio output service
constexpr uint32_t VALUE_AUDIO_PLAYBACK = 1 << 1;

// 0000 0100 represents audio input service
constexpr uint32_t VALUE_AUDIO_RECORDING = 1 << 2;

// 0000 1000 represents positioning navigation service
constexpr uint32_t VALUE_LOCATION = 1 << 3;

// 0001 0000 represents bluetooth scanning, connection, transmission service (wearing)
constexpr uint32_t VALUE_BLUETOOTH_INTERACTION = 1 << 4;

// 0010 0000 represents multi device connection services
constexpr uint32_t VALUE_MULTI_DEVICE_CONNECTION = 1 << 5;

// 0100 0000 represents WiFi scanning, connection, transmission services (multi-screen)
constexpr uint32_t VALUE_WIFI_INTERACTION = 1 << 6;

// 1000 0000 represents audio call,VOIP service
constexpr uint32_t VALUE_VOIP = 1 << 7;

// 1 0000 0000 represents task Keeping service
constexpr uint32_t VALUE_TASK_KEEPING = 1 << 8;

// 10 0000 0000 represents picture in picture service
constexpr uint32_t VALUE_PICTURE_IN_PICTURE = 1 << 9;

// 100 0000 0000 represents screen fetch service
constexpr uint32_t VALUE_SCREEN_FETCH = 1 << 10;

constexpr const char* KEY_DATA_TRANSFER = "dataTransfer";
constexpr const char* KEY_AUDIO_PLAYBACK = "audioPlayback";
constexpr const char* KEY_AUDIO_RECORDING = "audioRecording";
constexpr const char* KEY_LOCATION = "location";
constexpr const char* KEY_BLUETOOTH_INTERACTION = "bluetoothInteraction";
constexpr const char* KEY_MULTI_DEVICE_CONNECTION = "multiDeviceConnection";
constexpr const char* KEY_WIFI_INTERACTION = "wifiInteraction";
constexpr const char* KEY_VOIP = "voip";
constexpr const char* KEY_TASK_KEEPING = "taskKeeping";
constexpr const char* KEY_PICTURE_IN_PICTURE = "pictureInPicture";
constexpr const char* KEY_SCREEN_FETCH = "screenFetch";
} // namespace ProfileReader

namespace Profile {
// common
constexpr const char* ICON = "icon";
constexpr const char* ICON_ID = "iconId";
constexpr const char* LABEL = "label";
constexpr const char* LABEL_ID = "labelId";
constexpr const char* DESCRIPTION = "description";
constexpr const char* DESCRIPTION_ID = "descriptionId";
constexpr const char* META_DATA = "metadata";
constexpr const char* SKILLS = "skills";
constexpr const char* SRC_ENTRANCE = "srcEntrance";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* PERMISSIONS = "permissions";
constexpr const char* VISIBLE = "visible";
constexpr const char* EXPORTED = "exported";
constexpr const char* SRC_LANGUAGE = "srcLanguage";
constexpr const char* PRIORITY = "priority";
// module.json
constexpr const char* APP = "app";
constexpr const char* MODULE = "module";
// app
constexpr const char* APP_BUNDLE_NAME = "bundleName";
constexpr const char* APP_DEBUG = "debug";
constexpr const char* APP_VENDOR = "vendor";
constexpr const char* APP_VERSION_CODE = "versionCode";
constexpr const char* APP_VERSION_NAME = "versionName";
constexpr const char* APP_MIN_COMPATIBLE_VERSION_CODE = "minCompatibleVersionCode";
constexpr const char* APP_MIN_API_VERSION = "minAPIVersion";
constexpr const char* APP_TARGET_API_VERSION = "targetAPIVersion";
constexpr const char* APP_API_RELEASETYPE = "apiReleaseType";
constexpr const char* APP_API_RELEASETYPE_DEFAULT_VALUE = "Release";
constexpr const char* APP_ENTITY_TYPE_DEFAULT_VALUE = "unspecified";
constexpr const char* APP_KEEP_ALIVE = "keepAlive";
constexpr const char* APP_REMOVABLE = "removable";
constexpr const char* APP_SINGLETON = "singleton";
constexpr const char* APP_USER_DATA_CLEARABLE = "userDataClearable";
constexpr const char* APP_PHONE = "phone";
constexpr const char* APP_TABLET = "tablet";
constexpr const char* APP_TV = "tv";
constexpr const char* APP_WEARABLE = "wearable";
constexpr const char* APP_LITE_WEARABLE = "liteWearable";
constexpr const char* APP_CAR = "car";
constexpr const char* APP_SMART_VISION = "smartVision";
constexpr const char* APP_ROUTER = "router";
constexpr const char* APP_ACCESSIBLE = "accessible";
constexpr const char* APP_TARGETBUNDLELIST = "targetBundleList";
constexpr const char* APP_MULTI_PROJECTS = "multiProjects";
constexpr const char* APP_ASAN_ENABLED = "asanEnabled";
constexpr const char* APP_ATOMIC_SERVICE = "atomicService";
// module
constexpr const char* MODULE_NAME = "name";
constexpr const char* MODULE_TYPE = "type";
constexpr const char* MODULE_PROCESS = "process";
constexpr const char* MODULE_MAIN_ELEMENT = "mainElement";
constexpr const char* MODULE_DEVICE_TYPES = "deviceTypes";
constexpr const char* MODULE_DELIVERY_WITH_INSTALL = "deliveryWithInstall";
constexpr const char* MODULE_INSTALLATION_FREE = "installationFree";
constexpr const char* MODULE_VIRTUAL_MACHINE = "virtualMachine";
constexpr const char* MODULE_VIRTUAL_MACHINE_DEFAULT_VALUE = "default";
constexpr const char* MODULE_UI_SYNTAX = "uiSyntax";
constexpr const char* MODULE_UI_SYNTAX_DEFAULT_VALUE = "hml";
constexpr const char* MODULE_PAGES = "pages";
constexpr const char* MODULE_ABILITIES = "abilities";
constexpr const char* MODULE_EXTENSION_ABILITIES = "extensionAbilities";
constexpr const char* MODULE_REQUEST_PERMISSIONS = "requestPermissions";
constexpr const char* MODULE_DEFINE_PERMISSIONS = "definePermissions";
constexpr const char* MODULE_DEPENDENCIES = "dependencies";
constexpr const char* MODULE_COMPILE_MODE = "compileMode";
constexpr const char* MODULE_IS_LIB_ISOLATED = "libIsolation";
constexpr const char* MODULE_ATOMIC_SERVICE = "atomicService";
// module type
constexpr const char* MODULE_TYPE_ENTRY = "entry";
constexpr const char* MODULE_TYPE_FEATURE = "feature";
constexpr const char* MODULE_TYPE_SHARED = "shared";
// deviceConfig
constexpr const char* MIN_API_VERSION = "minAPIVersion";
constexpr const char* DEVICE_CONFIG_KEEP_ALIVE = "keepAlive";
constexpr const char* DEVICE_CONFIG_REMOVABLE = "removable";
constexpr const char* DEVICE_CONFIG_SINGLETON = "singleton";
constexpr const char* DEVICE_CONFIG_USER_DATA_CLEARABLE = "userDataClearable";
constexpr const char* DEVICE_CONFIG_ACCESSIBLE = "accessible";
// metadata
constexpr const char* META_DATA_NAME = "name";
constexpr const char* META_DATA_VALUE = "value";
constexpr const char* META_DATA_RESOURCE = "resource";
// metadata reserved
constexpr const char* META_DATA_FORM = "ohos.extension.form";
constexpr const char* META_DATA_SHORTCUTS = "ohos.ability.shortcuts";
constexpr const char* META_DATA_COMMON_EVENTS = "ohos.extension.staticSubscriber";
// ability
constexpr const char* ABILITY_NAME = "name";
constexpr const char* ABILITY_LAUNCH_TYPE = "launchType";
constexpr const char* ABILITY_LAUNCH_TYPE_DEFAULT_VALUE = "singleton";
constexpr const char* ABILITY_BACKGROUNDMODES = "backgroundModes";
constexpr const char* ABILITY_CONTINUABLE = "continuable";
constexpr const char* ABILITY_START_WINDOW_ICON = "startWindowIcon";
constexpr const char* ABILITY_START_WINDOW_ICON_ID = "startWindowIconId";
constexpr const char* ABILITY_START_WINDOW_BACKGROUND = "startWindowBackground";
constexpr const char* ABILITY_START_WINDOW_BACKGROUND_ID = "startWindowBackgroundId";
constexpr const char* ABILITY_REMOVE_MISSION_AFTER_TERMINATE = "removeMissionAfterTerminate";
constexpr const char* ABILITY_ORIENTATION = "orientation";
constexpr const char* ABILITY_SUPPORT_WINDOW_MODE = "supportWindowMode";
constexpr const char* ABILITY_MAX_WINDOW_RATIO = "maxWindowRatio";
constexpr const char* ABILITY_MIN_WINDOW_RATIO = "minWindowRatio";
constexpr const char* ABILITY_MAX_WINDOW_WIDTH = "maxWindowWidth";
constexpr const char* ABILITY_MIN_WINDOW_WIDTH = "minWindowWidth";
constexpr const char* ABILITY_MAX_WINDOW_HEIGHT = "maxWindowHeight";
constexpr const char* ABILITY_MIN_WINDOW_HEIGHT = "minWindowHeight";
constexpr const char* ABILITY_EXCLUDE_FROM_MISSIONS = "excludeFromMissions";
constexpr const char* ABILITY_RECOVERABLE = "recoverable";
// extension ability
constexpr const char* EXTENSION_ABILITY_NAME = "name";
constexpr const char* EXTENSION_ABILITY_TYPE = "type";
constexpr const char* EXTENSION_URI = "uri";
constexpr const char* EXTENSION_ABILITY_READ_PERMISSION = "readPermission";
constexpr const char* EXTENSION_ABILITY_WRITE_PERMISSION = "writePermission";
// requestPermission
constexpr const char* REQUESTPERMISSION_NAME = "name";
constexpr const char* REQUESTPERMISSION_REASON = "reason";
constexpr const char* REQUESTPERMISSION_REASON_ID = "reasonId";
constexpr const char* REQUESTPERMISSION_USEDSCENE = "usedScene";
constexpr const char* REQUESTPERMISSION_ABILITIES = "abilities";
constexpr const char* REQUESTPERMISSION_WHEN = "when";
// definePermission
constexpr const char* DEFINEPERMISSION_NAME = "name";
constexpr const char* DEFINEPERMISSION_GRANT_MODE = "grantMode";
constexpr const char* DEFINEPERMISSION_AVAILABLE_LEVEL = "availableLevel";
constexpr const char* DEFINEPERMISSION_PROVISION_ENABLE = "provisionEnable";
constexpr const char* DEFINEPERMISSION_DISTRIBUTED_SCENE_ENABLE = "distributedSceneEnable";
constexpr const char* DEFINEPERMISSION_GRANT_MODE_SYSTEM_GRANT = "system_grant";
constexpr const char* DEFINEPERMISSION_AVAILABLE_LEVEL_DEFAULT_VALUE = "normal";
// apl
constexpr const char* AVAILABLELEVEL_NORMAL = "normal";
constexpr const char* AVAILABLELEVEL_SYSTEM_BASIC = "system_basic";
constexpr const char* AVAILABLELEVEL_SYSTEM_CORE = "system_core";
// compile mode
constexpr const char* COMPILE_MODE_JS_BUNDLE = "jsbundle";
constexpr const char* COMPILE_MODE_ES_MODULE = "esmodule";
// dependencies
constexpr const char* DEPENDENCIES_MODULE_NAME = "moduleName";
constexpr const char* DEPENDENCIES_BUNDLE_NAME = "bundleName";

// app detail ability library path
constexpr const char* APP_DETAIL_ABILITY_LIBRARY_PATH = "/system/lib/appdetailability";
constexpr const char* APP_DETAIL_ABILITY_LIBRARY_PATH_64 = "/system/lib64/appdetailability";

// overlay installation
constexpr const char* APP_TARGET_BUNDLE_NAME = "targetBundleName";
constexpr const char* APP_TARGET_PRIORITY = "targetPriority";

constexpr const char* MODULE_TARGET_MODULE_NAME = "targetModuleName";
constexpr const char* MODULE_TARGET_PRIORITY = "targetPriority";

constexpr const char* SYSTEM_RESOURCES_APP = "ohos.global.systemres";

// app atomicService
constexpr const char* APP_ATOMIC_SERVICE_SPLIT = "split";
constexpr const char* APP_ATOMIC_SERVICE_MAIN = "main";
// module atomicService
constexpr const char* MODULE_ATOMIC_SERVICE_PRELOADS = "preloads";
// module atomicService preloads
constexpr const char* PRELOADS_MODULE_NAME = "moduleName";
extern thread_local int32_t parseResult;

constexpr const char* MODULE_PACKAGE_NAME  = "packageName";
} // namespace Profile
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_COMMON_PROFILE_H
