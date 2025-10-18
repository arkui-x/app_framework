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

#include "web_delegate_cross.h"

#include "bridge/js_frontend/frontend_delegate_impl.h"
#include <atomic>

#if defined(IOS_PLATFORM)
#include "adapter/ios/capability/web/AceWebPatternOCBridge.h"
#endif

namespace OHOS::Ace {
namespace {
constexpr char WEB_METHOD_RELOAD[] = "reload";
constexpr char WEB_METHOD_ROUTER_BACK[] = "routerBack";
constexpr char WEB_METHOD_UPDATEURL[] = "updateUrl";
constexpr char WEB_METHOD_CHANGE_PAGE_URL[] = "changePageUrl";
constexpr char WEB_METHOD_PAGE_PATH_INVALID[] = "pagePathInvalid";

// The function name in Java and C++ must be same.
constexpr char WEB_METHOD_BACKWARD[] = "backward";
constexpr char WEB_METHOD_FORWARD[] = "forward";
constexpr char WEB_METHOD_ACCESSBACKWARD[] = "accessBackward";
constexpr char WEB_METHOD_ACCESSFORWARD[] = "accessForward";
constexpr char WEB_METHOD_ACCESS_STEP[] = "accessStep";
constexpr char WEB_METHOD_CLEAR_HISTORY[] = "clearHistory";
constexpr char WEB_METHOD_GET_HIT_TEST[] = "getHitTest";
constexpr char WEB_METHOD_ON_ACTIVE[] = "onActive";
constexpr char WEB_METHOD_ON_INACTIVE[] = "onInactive";
constexpr char WEB_METHOD_REQUEST_FOCUS[] = "requestFocus";
constexpr char WEB_METHOD_STOP[] = "stop";
constexpr char WEB_METHOD_REFRESH[] = "refresh";
constexpr char WEB_METHOD_LOAD_URL[] = "loadUrl";
constexpr char WEB_METHOD_LOAD_DATA[] = "loadData";
constexpr char WEB_METHOD_ADD_JAVASCRIPT[] = "registerJavaScriptProxy";
constexpr char WEB_METHOD_RUN_JAVASCRIPT[] = "runJavaScript";
constexpr char WEB_METHOD_REMOVE_JAVASCRIPT[] = "deleteJavaScriptRegister";
constexpr char WEB_METHOD_UPDATE_ATTRIBUTE[] = "updateAttribute";
constexpr char WEB_METHOD_SETCOOKIE[] = "setcookie";
constexpr char WEB_METHOD_SAVECOOKIESYNC[] = "savecookiesync";
constexpr char WEB_METHOD_TOUCH_DOWN[] = "touchDown";
constexpr char WEB_METHOD_TOUCH_UP[] = "touchUp";
constexpr char WEB_METHOD_TOUCH_MOVE[] = "touchMove";
constexpr char WEB_METHOD_TOUCH_CANCEL[] = "touchCancel";
constexpr char WEB_METHOD_UPDATE_LAYOUT[] = "updateLayout";
constexpr char WEB_METHOD_ZOOM[] = "zoom";
constexpr char WEB_METHOD_UPDATE_CONTENT[] = "updateWebContent";
constexpr char WEB_METHOD_SET_NESTED_SCROLL_EXT[] = "setNestedScrollExt";

// The parameters in Java and C++ must be same.
constexpr char NTC_PARAM_ACCESS_STEP[] = "accessStep";
constexpr char NTC_PARAM_RUN_JS_CODE[] = "runJavaScriptCode";
constexpr char NTC_PARAM_COOKIE_URL[] = "cookie_url";
constexpr char NTC_PARAM_COOKIE_VALUE[] = "cookie_value";
constexpr char NTC_PARAM_REGISTER_JS_NAME[] = "jsInterfaceName";
constexpr char NTC_PARAM_REGISTER_JS_METHODLIST[] = "jsInterfaceMethodList";
constexpr char NTC_PARAM_DEL_INTERFACE[] = "javaScriptInterfaceName";
constexpr char NTC_PARAM_LOAD_URL[] = "load_url";
constexpr char NTC_PARAM_LOADDATA_BASEURL[] = "load_data_base_url";
constexpr char NTC_PARAM_LOADDATA_DATA[] = "load_data_data";
constexpr char NTC_PARAM_LOADDATA_MIMETYPE[] = "load_data_mimetype";
constexpr char NTC_PARAM_LOADDATA_ENCODING[] = "load_data_encoding";
constexpr char NTC_PARAM_LOADDATA_HISTORY[] = "load_data_history_url";
constexpr char NTC_PARAM_ZOOM_FACTOR[] = "zoom";
constexpr char NTC_PARAM_SET_NESTED_SCROLL_EXT_UP[] = "scrollUp";
constexpr char NTC_PARAM_SET_NESTED_SCROLL_EXT_DOWN[] = "scrollDown";
constexpr char NTC_PARAM_SET_NESTED_SCROLL_EXT_LEFT[] = "scrollLeft";
constexpr char NTC_PARAM_SET_NESTED_SCROLL_EXT_RIGHT[] = "scrollRight";
constexpr char WEB_EVENT_PAGESTART[] = "onPageStarted";
constexpr char WEB_EVENT_PAGEFINISH[] = "onPageFinished";
constexpr char WEB_EVENT_DOWNLOADSTART[] = "onDownloadStart";
constexpr char WEB_EVENT_LOADINTERCEPT[] = "onLoadIntercept";
constexpr char WEB_EVENT_ONINTERCEPTREQUEST[] = "onInterceptRequest";
constexpr char WEB_EVENT_RUNJSCODE_RECVVALUE[] = "onRunJSRecvValue";
constexpr char WEB_EVENT_ONWILLSCROLLSTART[] = "onWillScrollStart";
constexpr char WEB_EVENT_ONSCROLLSTART[] = "onScrollStart";
constexpr char WEB_EVENT_ONSCROLLEND[] = "onScrollEnd";
constexpr char WEB_EVENT_SCROLL[] = "onScroll";
constexpr char WEB_EVENT_SCALECHANGE[] = "onScaleChange";
constexpr char WEB_EVENT_JS_INVOKE_METHOD[] = "onJSInvokeMethod";
constexpr char WEB_EVENT_ON_BEFORE_UNLOAD[] = "onBeforeUnload";
constexpr char WEB_EVENT_REFRESH_HISTORY[] = "onRefreshAccessedHistory";
constexpr char WEB_EVENT_RENDER_EXITED[] = "onRenderExited";
constexpr char WEB_EVENT_FULLSCREEN_ENTER[] = "onFullScreenEnter";
constexpr char WEB_EVENT_FULLSCREEN_EXIT[] = "onFullScreenExit";
constexpr char WEB_EVENT_URL_LOAD_INTERCEPT[] = "onUrlLoadIntercept";
constexpr char WEB_EVENT_PAGECHANGED[] = "onProgressChanged";
constexpr char WEB_EVENT_RECVTITLE[] = "onReceivedTitle";
constexpr char WEB_EVENT_PAGEERROR[] = "onPageError";
constexpr char WEB_EVENT_ONMESSAGE[] = "onMessage";
constexpr char WEB_EVENT_ROUTERPUSH[] = "routerPush";
constexpr char WEB_EVENT_GEOHIDEPERMISSION[] = "onGeoHidePermission";
constexpr char WEB_EVENT_GEOPERMISSION[] = "onGeoPermission";
constexpr char WEB_EVENT_COMMONDIALOG[] = "onCommonDialog";
constexpr char WEB_EVENT_ONALERT[] = "onAlert";
constexpr char WEB_EVENT_ONCONFIRM[] = "onConfirm";
constexpr char WEB_EVENT_ONPROMPT[] = "onPrompt";
constexpr char WEB_EVENT_CONSOLEMESSAGE[] = "onConsoleMessage";
constexpr char WEB_EVENT_ERRORRECEIVE[] = "onErrorReceive";
constexpr char WEB_EVENT_ONSHOWFILECHOOSER[] = "onShowFileChooser";
constexpr char WEB_EVENT_ONHTTPERRORRECEIVE[] = "onHttpErrorReceive";
constexpr char WEB_EVENT_ONPAGEVISIBLE[] = "onPageVisible";
constexpr char WEB_EVENT_ONHTTPAUTHREQUEST[] = "onHttpAuthRequest";
constexpr char WEB_EVENT_ONPERMISSIONREQUEST[] = "onPermissionRequest";
constexpr char WEB_EVENT_ONOVERRIDEURLLOADING[] = "onOverrideUrlLoading";
constexpr char WEB_EVENT_ONSSLERROREVENTRECEIVE[] = "onSslErrorEventReceive";
constexpr char WEB_EVENT_ONSSLERROREVENT[] = "onSslErrorEvent";

constexpr char WEB_CREATE[] = "web";
constexpr char NTC_PARAM_WEB[] = "web";
constexpr char NTC_PARAM_WIDTH[] = "width";
constexpr char NTC_PARAM_HEIGHT[] = "height";
constexpr char NTC_PARAM_LEFT[] = "left";
constexpr char NTC_PARAM_TOP[] = "top";
constexpr char NTC_ERROR[] = "create error";
constexpr char NTC_PARAM_SRC[] = "src";
constexpr char NTC_PARAM_ERROR_CODE[] = "errorCode";
constexpr char NTC_PARAM_URL[] = "url";
constexpr char NTC_PARAM_PAGE_URL[] = "pageUrl";
constexpr char NTC_PARAM_PAGE_INVALID[] = "pageInvalid";
constexpr char NTC_PARAM_DESCRIPTION[] = "description";
constexpr char NTC_PARAM_RICH_TEXT_INIT[] = "richTextInit";
constexpr char WEB_ERROR_CODE_CREATEFAIL[] = "error-web-delegate-000001";
constexpr char WEB_ERROR_MSG_CREATEFAIL[] = "create web_delegate failed.";

constexpr char NTC_PARAM_X[] = "x";
constexpr char NTC_PARAM_Y[] = "y";

constexpr char WEB_ATTRIBUTE_ZOOM_ACCESS[] = "zoomAccess";
constexpr char NTC_ZOOM_ACCESS[] = "zoomAccess";
constexpr char NTC_INCOGNITO_MODE[] = "incognitoMode";
constexpr char WEB_ATTRIBUTE_JAVASCRIPT_ACCESS[] = "javascriptAccess";
constexpr char NTC_JAVASCRIPT_ACCESS[] = "javascriptAccess";
constexpr char WEB_ATTRIBUTE_MIN_FONT_SIZE[] = "minFontSize";
constexpr char NTC_MIN_FONT_SIZE[] = "minFontSize";
constexpr char WEB_ATTRIBUTE_HORIZONTAL_SCROLLBAR_ACCESS[] = "horizontalScrollBarAccess";
constexpr char NTC_HORIZONTAL_SCROLLBAR_ACCESS[] = "horizontalScrollBarAccess";
constexpr char WEB_ATTRIBUTE_VERTICAL_SCROLLBAR_ACCESS[] = "verticalScrollBarAccess";
constexpr char NTC_VERTICAL_SCROLLBAR_ACCESS[] = "verticalScrollBarAccess";
constexpr char WEB_ATTRIBUTE_BACKGROUND_COLOR[] = "backgroundColor";
constexpr char NTC_BACKGROUND_COLOR[] = "backgroundColor";
constexpr char WEB_ATTRIBUTE_MEDIA_PLAY_GESTURE_ACCESS[] = "mediaPlayGestureAccess";
constexpr char NTC_MEDIA_PLAY_GESTURE_ACCESS[] = "mediaPlayGestureAccess";
constexpr char WEB_ATTRIBUTE_BLOCK_NETWORK[] = "blockNetwork";
constexpr char NTC_BLOCK_NETWORK[] = "blockNetwork";
constexpr char WEB_ATTRIBUTE_MIXED_MODE[] = "mixedMode";
constexpr char NTC_MIXED_MODE[] = "mixedMode";
constexpr char WEB_ATTRIBUTE_ONLINE_IMAGE_ACCESS[] = "onlineImageAccess";
constexpr char NTC_ONLINE_IMAGE_ACCESS[] = "onlineImageAccess";
constexpr char WEB_ATTRIBUTE_GEOLOCATION_ACCESS[] = "geolocationAccess";
constexpr char NTC_GEOLOCATION_ACCESS[] = "geolocationAccess";
constexpr char WEB_ATTRIBUTE_DOM_STORAGE_ACCESS[] = "domStorageAccess";
constexpr char NTC_DOM_STORAGE_ACCESS[] = "domStorageAccess";
constexpr char WEB_CACHE_MODE[] = "cacheMode";
constexpr char NTC_CACHE_MODE[] = "cacheMode";
constexpr char WEB_IMAGE_ACCESS[] = "imageAccess";
constexpr char NTC_IMAGE_ACCESS[] = "imageAccess";
constexpr char WEB_TEXT_ZOOM_RATIO[] = "textZoomRatio";
constexpr char NTC_TEXT_ZOOM_RATIO[] = "textZoomRatio";
constexpr char WEB_FILE_ACCESS[] = "fileAccess";
constexpr char NTC_FILE_ACCESS[] = "fileAccess";

const char WEB_PARAM_NONE[] = "";
const char WEB_PARAM_AND[] = "#HWJS-&-#";
const char WEB_PARAM_VALUE[] = "value";
const char WEB_PARAM_EQUALS[] = "#HWJS-=-#";
const char WEB_PARAM_BEGIN[] = "#HWJS-?-#";
const char WEB_METHOD[] = "method";
const char WEB_EVENT[] = "event";
const char WEB_RESULT_FAIL[] = "fail";
constexpr char MIMETYPE[] = "text/html";
constexpr char ENCODING[] = "UTF-8";

constexpr int FONT_MIN_SIZE = 1;
constexpr int FONT_MAX_SIZE = 72;
constexpr int RESOURCESID_ONE = 1;
constexpr int RESOURCESID_TWO = 2;
constexpr int RESOURCESID_THREE = 3;

constexpr int TIMEOUT_DURATION_MS = 15000;
constexpr int POLLING_INTERVAL_MS = 50;
constexpr int HTTP_STATUS_GATEWAY_TIMEOUT = 504;
constexpr int TIMEOUT_SEMAPHORE_S = 20;
constexpr int TWO_BYTES_LENGTH = 16;
constexpr int TWO_BYTES = 2;

const std::string RESOURCE_VIDEO_CAPTURE = "TYPE_VIDEO_CAPTURE";
const std::string RESOURCE_AUDIO_CAPTURE = "TYPE_AUDIO_CAPTURE";
}
#if defined(IOS_PLATFORM)
static std::atomic<int64_t> g_atomicId{0};
#else
static std::atomic<int64_t> g_atomicId{-1};
#endif

std::map<std::string, std::string> WebResourceRequsetImpl::GetRequestHeader() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceRequsetImpl GetRequestHeader failed");
        return std::map<std::string, std::string>();
    }
    return obj->GetRequestHeader(object_);
}

std::string WebResourceRequsetImpl::GetRequestUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        return std::string();
    }
    return obj->GetRequestUrl(object_);
}

std::string WebResourceRequsetImpl::GetMethod() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceRequsetImpl GetMethod failed");
        return std::string();
    }
    return obj->GetMethod(object_);
}

bool WebResourceRequsetImpl::IsRequestGesture() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceRequsetImpl IsRequestGesture failed");
        return false;
    }
    return obj->IsRequestGesture(object_);
}

bool WebResourceRequsetImpl::IsMainFrame() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceRequsetImpl IsMainFrame failed");
        return true;
    }
    return obj->IsMainFrame(object_);
}

bool WebResourceRequsetImpl::IsRedirect() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceRequsetImpl IsRedirect failed");
        return false;
    }
    return obj->IsRedirect(object_);
}

std::map<std::string, std::string> WebResourceResponseImpl::GetResponseHeader() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl ResponseHeader failed");
        return std::map<std::string, std::string>();
    }
    return obj->GetResponseHeader(object_);
}

std::string WebResourceResponseImpl::GetResponseData() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl ResponseData failed");
        return std::string();
    }
    return obj->GetResponseData(object_);
}

std::string WebResourceResponseImpl::GetEncoding() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl encoding failed");
        return std::string();
    }
    return obj->GetEncoding(object_);
}

std::string WebResourceResponseImpl::GetMimeType() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl mimeType failed");
        return std::string();
    }
    return obj->GetMimeType(object_);
}

std::string WebResourceResponseImpl::GetReason() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl Reason failed");
        return std::string();
    }
    return obj->GetReason(object_);
}

int WebResourceResponseImpl::GetStatusCode() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebResourceResponseImpl statusCode failed");
        return 0;
    }
    return obj->GetStatusCode(object_);
}

std::string WebResourceErrorImpl::GetErrorInfo() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceErrorObject();
    if (!obj) {
        return std::string();
    }
    return obj->GetErrorInfo(object_);
}

int WebResourceErrorImpl::GetErrorCode() const
{
    auto obj = WebObjectEventManager::GetInstance().GetResourceErrorObject();
    if (!obj) {
        return 0;
    }
    return obj->GetErrorCode(object_);
}

double WebOffsetImpl::GetX() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl X failed");
        return 0.0;
    }
    return obj->GetX(object_);
}

double WebOffsetImpl::GetY() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl Y failed");
        return 0.0;
    }
    return obj->GetY(object_);
}

double WebOffsetImpl::GetContentWidth() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl ContentWidth failed");
        return 0.0;
    }
    return obj->GetContentWidth(object_);
}

double WebOffsetImpl::GetContentHeight() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl ContentHeight failed");
        return 0.0;
    }
    return obj->GetContentHeight(object_);
}

double WebOffsetImpl::GetFrameWidth() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl FrameWidth failed");
        return 0.0;
    }
    return obj->GetFrameWidth(object_);
}

double WebOffsetImpl::GetFrameHeight() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScrollObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebOffsetImpl FrameHeight failed");
        return 0.0;
    }
    return obj->GetFrameHeight(object_);
}

std::string WebConsoleMessage::GetMessage() const
{
    auto obj = WebObjectEventManager::GetInstance().GetConsoleMessageObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebConsoleMessage Message failed");
        return std::string();
    }
    return obj->GetMessage(object_);
}

int WebConsoleMessage::GetMessageLevel() const
{
    auto obj = WebObjectEventManager::GetInstance().GetConsoleMessageObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebConsoleMessage MessageLevel failed");
        return 0;
    }
    return obj->GetMessageLevel(object_);
}

std::string WebConsoleMessage::GetSourceId() const
{
    auto obj = WebObjectEventManager::GetInstance().GetConsoleMessageObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebConsoleMessage GetSourceId failed");
        return std::string();
    }
    return obj->GetSourceId(object_);
}

int WebConsoleMessage::GetLineNumber() const
{
    auto obj = WebObjectEventManager::GetInstance().GetConsoleMessageObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebConsoleMessage GetLineNumber failed");
        return 0;
    }
    return obj->GetLineNumber(object_);
}

float WebScaleChangeImpl::GetNewScale() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScaleChangeObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebScaleChangeImpl newScale failed");
        return 0.0f;
    }
    return obj->GetNewScale(object_);
}

float WebScaleChangeImpl::GetOldScale() const
{
    auto obj = WebObjectEventManager::GetInstance().GetScaleChangeObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebScaleChangeImpl oldScale failed");
        return 0.0f;
    }
    return obj->GetOldScale(object_);
}

std::string WebCommonDialogImpl::GetUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebCommonDialogImpl url failed");
        return std::string();
    }
    return obj->GetUrl(object_);
}

std::string WebCommonDialogImpl::GetMessage() const
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebCommonDialogImpl message failed");
        return std::string();
    }
    return obj->GetMessage(object_);
}

std::string WebCommonDialogImpl::GetValue() const
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebCommonDialogImpl value failed");
        return std::string();
    }
    return obj->GetValue(object_);
}

void PermissionRequestImpl::SetOrigin()
{
    auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get PermissionRequestImpl origin failed");
        return;
    }
    origin_ = obj->GetOrigin(object_);
}

void PermissionRequestImpl::SetResources()
{
    auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get PermissionRequestImpl resources failed");
        return;
    }
    int resourcesId = obj->GetResourcesId(object_);
    if (resourcesId == RESOURCESID_ONE) {
        resources_.push_back(RESOURCE_VIDEO_CAPTURE);
    } else if (resourcesId == RESOURCESID_TWO) {
        resources_.push_back(RESOURCE_AUDIO_CAPTURE);
    } else if (resourcesId == RESOURCESID_THREE) {
        resources_.push_back(RESOURCE_VIDEO_CAPTURE);
        resources_.push_back(RESOURCE_AUDIO_CAPTURE);
    } else {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get error type");
    }
}

std::string PermissionRequestImpl::GetOrigin() const
{
    return origin_;
}

std::vector<std::string> PermissionRequestImpl::GetResources() const
{
    return resources_;
}

void PermissionRequestImpl::Grant(std::vector<std::string>& resources) const
{
    auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager PermissionRequestObject grant failed");
        return;
    }
    int resourcesId = 0;
    for (auto res : resources) {
        if (res == RESOURCE_VIDEO_CAPTURE) {
            resourcesId |= RESOURCESID_ONE;
        } else if (res == RESOURCE_AUDIO_CAPTURE) {
            resourcesId |= RESOURCESID_TWO;
        }
    }
    if (resourcesId != 0) {
        obj->Grant(object_, resourcesId, index_);
    }
}

void PermissionRequestImpl::Deny() const
{
    auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager PermissionRequestObject Deny failed");
        return;
    }
    obj->Deny(object_, index_);
}

std::string WebAuthRequestImpl::GetHost() const
{
    auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebAuthRequestImpl host failed");
        return std::string();
    }
    return obj->GetHost(object_);
}

std::string WebAuthRequestImpl::GetRealm() const
{
    auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebAuthRequestImpl realm failed");
        return std::string();
    }
    return obj->GetRealm(object_);
}

std::string WebRefreshAccessedHistoryImpl::GetUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetRefreshAccessedHistoryObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get RefreshAccessedHistory Url failed");
        return std::string();
    }
    return obj->GetUrl(object_);
}

bool WebRefreshAccessedHistoryImpl::GetIsRefreshed() const
{
    auto obj = WebObjectEventManager::GetInstance().GetRefreshAccessedHistoryObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebRefreshAccessedHistoryImpl GetIsRefreshed failed");
        return false;
    }
    return obj->GetIsRefreshed(object_);
}

int WebFullScreenEnterImpl::GetHeights() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFullScreenEnterObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager WebFullScreenEnterImpl GetHeights failed");
        return 0;
    }
    return obj->GetHeights(object_);
}
int WebFullScreenEnterImpl::GetWidths() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFullScreenEnterObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager WebFullScreenEnterImpl GetWidths failed");
        return 0;
    }
    return obj->GetWidths(object_);
}

void FullScreenExitHandlerImpl::ExitFullScreen()
{
    auto obj = WebObjectEventManager::GetInstance().GetFullScreenEnterObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager FullScreenExitHandlerImpl ExitFullScreen failed");
        return;
    }
    obj->ExitFullScreen(object_, index_);
}

std::string WebDownloadResponseImpl::GetUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetDownloadResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebDownloadResponseImpl Url failed");
        return std::string();
    }
    return obj->GetUrl(object_);
}

std::string WebDownloadResponseImpl::GetMimetype() const
{
    auto obj = WebObjectEventManager::GetInstance().GetDownloadResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebDownloadResponseImpl mimetype failed");
        return std::string();
    }
    return obj->GetMimetype(object_);
}

long WebDownloadResponseImpl::GetContentLength() const
{
    auto obj = WebObjectEventManager::GetInstance().GetDownloadResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebDownloadResponseImpl contentLength failed");
        return 0;
    }
    return obj->GetContentLength(object_);
}

std::string WebDownloadResponseImpl::GetUserAgent() const
{
    auto obj = WebObjectEventManager::GetInstance().GetDownloadResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebDownloadResponseImpl userAgent failed");
        return std::string();
    }
    return obj->GetUserAgent(object_);
}

std::string WebDownloadResponseImpl::GetContentDisposition() const
{
    auto obj = WebObjectEventManager::GetInstance().GetDownloadResponseObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebDownloadResponseImpl GetContentDisposition failed");
        return std::string();
    }
    return obj->GetContentDisposition(object_);
}

void DialogResult::Confirm(const std::string& promptResult)
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager DialogResult prompt Confirm failed");
        return;
    }
    if (dialogEventType_ == DialogEventType::DIALOG_EVENT_PROMPT) {
        obj->Confirm(object_, promptResult, index_);
    }
}

void DialogResult::Confirm()
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager DialogResult Confirm failed");
        return;
    }
    obj->Confirm(object_, index_);
}

void DialogResult::Cancel()
{
    auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager DialogResult Cancel failed");
        return;
    }
    obj->Cancel(object_, index_);
}

bool WebAuthResult::Confirm(std::string& userName, std::string& pwd)
{
    auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager WebAuthResult confirm failed");
        return false;
    }
    return obj->Confirm(object_, userName, pwd, index_);
}

bool WebAuthResult::IsHttpAuthInfoSaved()
{
    auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager WebAuthResult isHttpAuthInfoSaved failed");
        return false;
    }
    return obj->IsHttpAuthInfoSaved(object_, index_);
}

void WebAuthResult::Cancel()
{
    auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager WebAuthResult cancel failed");
        return;
    }
    obj->Cancel(object_, index_);
}

std::string WebFileChooserImpl::GetTitle() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebFileChooserImpl title failed");
        return std::string();
    }
    return obj->GetTitle(object_);
}

int WebFileChooserImpl::GetMode() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebFileChooserImpl title failed");
        return 0;
    }
    return obj->GetMode(object_);
}

std::string WebFileChooserImpl::GetDefaultFileName() const
{
    return "DefaultFileName";
}

std::vector<std::string> WebFileChooserImpl::GetAcceptType() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebFileChooserImpl acceptType failed");
        return std::vector<std::string>();
    }
    return obj->GetAcceptType(object_);
}

bool WebFileChooserImpl::IsCapture() const
{
    auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebFileChooserImpl title failed");
        return false;
    }
    return obj->IsCapture(object_);
}

void WebFileSelectorResult::HandleFileList(std::vector<std::string>& fileList)
{
    auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
    if (!obj) {
        TAG_LOGE(
            AceLogTag::ACE_WEB, "WebObjectEventManager get WebFileSelectorResult FileSelectorResultCallback failed");
        return;
    }
    obj->HandleFileList(object_, fileList, index_);
}

std::string WebGeolocationImpl::GetOrigin() const
{
    auto obj = WebObjectEventManager::GetInstance().GetGeolocationObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get WebGeolocationImpl FileSelectorResultCallback failed");
        return nullptr;
    }
    return obj->GetOrigin(object_);
}

void Geolocation::Invoke(const std::string& origin, const bool& allow, const bool& retain)
{
    auto obj = WebObjectEventManager::GetInstance().GetGeolocationObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get Geolocation FileSelectorResultCallback failed");
        return;
    }
    obj->Invoke(index_, origin, allow, retain);
}

void SslErrorResultImpl::HandleConfirm()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnSslErrorEventReceiveEventObject ConfirmCallback failed");
        return;
    }
    obj->Confirm(object_, index_);
}

void SslErrorResultImpl::HandleCancel(bool abortLoading)
{
    auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnSslErrorEventReceiveEventObject CancelCallback failed");
        return;
    }
    obj->Cancel(object_, index_);
}

int SslErrorEventImpl::GetError() const
{
    auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnSslErrorEventReceiveEventObject Error failed");
        return -1;
    }
    return obj->GetError(object_);
}

std::vector<std::string> SslErrorEventImpl::GetCertChainData() const
{
    auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnSslErrorEventReceiveEventObject CertChainData failed");
        return std::vector<std::string>();
    }
    return obj->GetCertChainData(object_);
}

void AllSslErrorResultImpl::HandleConfirm()
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject ConfirmCallBack failed");
        return;
    }
    obj->Confirm(object_, index_);
}

void AllSslErrorResultImpl::HandleCancel(bool abortLoading)
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject CancelCallBack failed");
        return;
    }
    obj->Cancel(object_, abortLoading, index_);
}

int AllSslErrorEventImpl::GetError() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject error failed");
        return -1;
    }
    return obj->GetError(object_);
}

std::string AllSslErrorEventImpl::GetUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject url failed");
        return std::string();
    }
    return obj->GetUrl(object_);
}

std::string AllSslErrorEventImpl::GetOriginalUrl() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject originalUrl failed");
        return std::string();
    }
    return obj->GetOriginalUrl(object_);
}

std::string AllSslErrorEventImpl::GetReferrer() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject referrer failed");
        return std::string();
    }
    return obj->GetReferrer(object_);
}

bool AllSslErrorEventImpl::IsFatalError() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject isFatalError failed");
        return false;
    }
    return obj->IsFatalError(object_);
}

std::vector<std::string> AllSslErrorEventImpl::GetCertificateChain() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject isMainFrame failed");
        return std::vector<std::string>();
    }
    return obj->GetCertificateChain(object_);
}

bool AllSslErrorEventImpl::IsMainFrame() const
{
    auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetSslErrorEventObject isMainFrame failed");
        return false;
    }
    return obj->IsMainFrame(object_);
}

void SslSelectCertResultImpl::HandleConfirm(const std::string& privateKeyFile, const std::string& certChainFile)
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject ConfirmCallback failed");
        return;
    }
    obj->Confirm(object_, privateKeyFile, certChainFile);
}

void SslSelectCertResultImpl::HandleCancel()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject CancelCallback failed");
        return;
    }
    obj->Cancel(object_);
}

void SslSelectCertResultImpl::HandleIgnore()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject IgnoreCallback failed");
        return;
    }
    obj->Ignore(object_);
}

std::string SslSelectCertEventImpl::GetHost()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject host failed");
        return std::string();
    }
    return obj->GetHost(object_);
}

int SslSelectCertEventImpl::GetPort()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject port failed");
        return -1;
    }
    return obj->GetPort(object_);
}

std::vector<std::string> SslSelectCertEventImpl::GetKeyTypes()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject keyTypes failed");
        return std::vector<std::string>();
    }
    return obj->GetKeyTypes(object_);
}

std::vector<std::string> SslSelectCertEventImpl::GetIssuers()
{
    auto obj = WebObjectEventManager::GetInstance().GetOnClientAuthenticationEventObject();
    if (!obj) {
        TAG_LOGE(AceLogTag::ACE_WEB,
            "WebObjectEventManager get GetOnClientAuthenticationEventObject issuers failed");
        return std::vector<std::string>();
    }
    return obj->GetIssuers(object_);
}

WebDelegateCross::~WebDelegateCross()
{
    ReleasePlatformResource();
}

void WebDelegateCross::CreatePlatformResource(
    const Size& size, const Offset& position, const WeakPtr<NG::PipelineContext>& context)
{
    ReleasePlatformResource();
    context_ = context;
    CreatePluginResource(size, position, context);
}

void WebDelegateCross::ReleasePlatformResource()
{
    Release();
}

void WebDelegateCross::CreatePluginResource(
    const Size& size, const Offset& position, const WeakPtr<NG::PipelineContext>& context)
{
    state_ = State::CREATING;
    auto pipelineContext = context.Upgrade();
    if (!pipelineContext) {
        state_ = State::CREATEFAILED;
        return;
    }
    context_ = context;
    auto platformTaskExecutor =
        SingleTaskExecutor::Make(pipelineContext->GetTaskExecutor(), TaskExecutor::TaskType::PLATFORM);
    auto resRegister = pipelineContext->GetPlatformResRegister();
    auto weakRes = AceType::WeakClaim(AceType::RawPtr(resRegister));
    platformTaskExecutor.PostSyncTask([weakWeb = AceType::WeakClaim(this), weakRes, size, position] {
        auto webDelegate = weakWeb.Upgrade();
        if (webDelegate == nullptr) {
            return;
        }
        auto webPattern = webDelegate->webPattern_.Upgrade();
        if (!webPattern) {
            webDelegate->OnError(WEB_ERROR_CODE_CREATEFAIL, WEB_ERROR_MSG_CREATEFAIL);
            return;
        }
        auto resRegister = weakRes.Upgrade();
        if (!resRegister) {
            webDelegate->OnError(WEB_ERROR_CODE_CREATEFAIL, WEB_ERROR_MSG_CREATEFAIL);
            return;
        }
        auto context = webDelegate->context_.Upgrade();
        if (!context) {
            return;
        }

        std::string pageUrl;
        int32_t pageId;
        OHOS::Ace::Framework::DelegateClient::GetInstance().GetWebPageUrl(pageUrl, pageId);
        webDelegate->id_ = g_atomicId.fetch_add(1) + 1;
        webDelegate->InitWebStatus();
        webDelegate->RegisterWebInerceptAndOverrideEvent();
        std::stringstream paramStream;
        paramStream << NTC_PARAM_WEB << WEB_PARAM_EQUALS << webDelegate->id_ << WEB_PARAM_AND << NTC_PARAM_WIDTH
                    << WEB_PARAM_EQUALS << size.Width() * context->GetViewScale() << WEB_PARAM_AND << NTC_PARAM_HEIGHT
                    << WEB_PARAM_EQUALS << size.Height() * context->GetViewScale() << WEB_PARAM_AND << NTC_PARAM_LEFT
                    << WEB_PARAM_EQUALS << position.GetX() * context->GetViewScale() << WEB_PARAM_AND << NTC_PARAM_TOP
                    << WEB_PARAM_EQUALS << position.GetY() * context->GetViewScale() << WEB_PARAM_AND << NTC_PARAM_SRC
                    << WEB_PARAM_EQUALS << webPattern->GetWebSrc().value_or("") << WEB_PARAM_AND << NTC_PARAM_PAGE_URL
                    << WEB_PARAM_EQUALS << pageUrl << WEB_PARAM_AND << NTC_PARAM_RICH_TEXT_INIT
                    << WEB_PARAM_EQUALS << webPattern->GetRichTextInit() << WEB_PARAM_AND << NTC_INCOGNITO_MODE
                    << WEB_PARAM_EQUALS << webPattern->GetIncognitoMode();
        std::string param = paramStream.str();
        webDelegate->id_ = resRegister->CreateResource(WEB_CREATE, param);
        if (webDelegate->id_ == INVALID_ID) {
            webDelegate->HandleCreateError();
            return;
        }
        webDelegate->InitWebStatus();
        webDelegate->RegisterWebEvent();
        webDelegate->RegisterWebObjectEvent();
        }, "ArkUIWebCreatePluginResource");
}

void WebDelegateCross::HandleCreateError()
{
    OnError(WEB_ERROR_CODE_CREATEFAIL, WEB_ERROR_MSG_CREATEFAIL);
    UnRegisterWebObjectEvent();
    state_ = State::CREATEFAILED;
}

void WebDelegateCross::InitWebStatus()
{
    state_ = State::CREATED;
    hash_ = MakeResourceHash();
}

int WebDelegateCross::GetWebId()
{
    return id_;
}

void WebDelegateCross::RegisterWebEvent()
{
    auto context = DynamicCast<NG::PipelineContext>(context_.Upgrade());
    CHECK_NULL_VOID(context);
    auto resRegister = context->GetPlatformResRegister();
    if (resRegister == nullptr) {
        return;
    }
    resRegister->RegisterEvent(MakeEventHash(WEB_EVENT_PAGESTART), [weak = WeakClaim(this)](const std::string& param) {
        auto delegate = weak.Upgrade();
        if (delegate) {
            delegate->OnPageStarted(param);
            delegate->RunJsProxyCallback();
        }
    });
    resRegister->RegisterEvent(MakeEventHash(WEB_EVENT_PAGEFINISH), [weak = WeakClaim(this)](const std::string& param) {
        auto delegate = weak.Upgrade();
        if (delegate) {
            delegate->OnPageFinished(param);
        }
    });
    resRegister->RegisterEvent(
        MakeEventHash(WEB_EVENT_PAGECHANGED), [weak = WeakClaim(this)](const std::string& param) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnProgressChanged(param);
            }
        });
    resRegister->RegisterEvent(MakeEventHash(WEB_EVENT_RECVTITLE), [weak = WeakClaim(this)](const std::string& param) {
        auto delegate = weak.Upgrade();
        if (delegate) {
            delegate->OnReceivedTitle(param);
        }
    });
    resRegister->RegisterEvent(
        MakeEventHash(WEB_EVENT_ONPAGEVISIBLE), [weak = WeakClaim(this)](const std::string& param) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnPageVisible(param);
            }
        });
    resRegister->RegisterEvent(
        MakeEventHash(WEB_EVENT_GEOHIDEPERMISSION), [weak = WeakClaim(this)](const std::string& param) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnGeolocationPermissionsHidePrompt();
            }
        });
}

void WebDelegateCross::RegisterWebObjectEvent()
{
    WebObjectEventManager::GetInstance().RegisterObjectEvent(MakeEventHash(WEB_EVENT_ERRORRECEIVE),
        [weak = WeakClaim(this)](const std::string& param, void* object) {
        auto delegate = weak.Upgrade();
        if (delegate) {
            delegate->OnErrorReceive(object);
        }
    });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_SCROLL), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnScroll(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_ONWILLSCROLLSTART), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnScrollWillStart(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_ONSCROLLSTART), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnScrollStart(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_ONSCROLLEND), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnScrollEnd(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_SCALECHANGE), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnScaleChange(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_ONHTTPERRORRECEIVE), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnHttpErrorReceive(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_CONSOLEMESSAGE),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnConsoleMessage(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_LOADINTERCEPT),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnLoadIntercept(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_REFRESH_HISTORY),
        [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnRefreshAccessedHistory(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_FULLSCREEN_ENTER), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnFullScreenEnter(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_FULLSCREEN_EXIT),
        [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnFullScreenExit(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_DOWNLOADSTART), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnDownloadStart(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_ONSHOWFILECHOOSER),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnShowFileChooser(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEvent(
        MakeEventHash(WEB_EVENT_GEOPERMISSION), [weak = WeakClaim(this)](const std::string& param, void* object) {
            auto delegate = weak.Upgrade();
            if (delegate) {
                delegate->OnGeolocationPermissionsShowPrompt(object);
            }
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONALERT), [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnCommonDialog(object, DialogEventType::DIALOG_EVENT_ALERT);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONCONFIRM), [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnCommonDialog(object, DialogEventType::DIALOG_EVENT_CONFIRM);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONPROMPT), [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnCommonDialog(object, DialogEventType::DIALOG_EVENT_PROMPT);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_ONPERMISSIONREQUEST),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnPermissionRequest(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_ON_BEFORE_UNLOAD),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnCommonDialog(object, DialogEventType::DIALOG_EVENT_BEFORE_UNLOAD);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_ONHTTPAUTHREQUEST),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnHttpAuthRequest(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONSSLERROREVENTRECEIVE),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnSslErrorEventReceive(object);
            }
            return false;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(MakeEventHash(WEB_EVENT_ONSSLERROREVENT),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnSslErrorEvent(object);
            }
            return false;
        });
}

void WebDelegateCross::RegisterWebInerceptAndOverrideEvent()
{
    WebObjectEventManager::GetInstance().RegisterObjectEventWithResponseReturn(
        MakeEventHash(WEB_EVENT_ONINTERCEPTREQUEST),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> RefPtr<WebResponse> {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnInterceptRequest(object);
            }
            return nullptr;
        });
    WebObjectEventManager::GetInstance().RegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONOVERRIDEURLLOADING),
        [weak = WeakClaim(this)](const std::string& param, void* object) -> bool {
            auto delegate = weak.Upgrade();
            if (delegate) {
                return delegate->OnOverrideUrlLoading(object);
            }
            return false;
        });
}

void WebDelegateCross::UnRegisterWebObjectEvent()
{
    WebObjectEventManager::GetInstance().UnRegisterObjectEventWithResponseReturn(
        MakeEventHash(WEB_EVENT_ONINTERCEPTREQUEST));
    WebObjectEventManager::GetInstance().UnRegisterObjectEventWithBoolReturn(
        MakeEventHash(WEB_EVENT_ONOVERRIDEURLLOADING));
}

void WebDelegateCross::HandleTouchDown(
    const int32_t& id, const double& x, const double& y, bool from_overlay)
{
    hash_ = MakeResourceHash();
    touchDownMethod_ = MakeMethodHash(WEB_METHOD_TOUCH_DOWN);
    CallResRegisterMethod(touchDownMethod_, "", nullptr);
}

void WebDelegateCross::HandleTouchUp(
    const int32_t& id, const double& x, const double& y, bool from_overlay)
{
    hash_ = MakeResourceHash();
    touchUpMethod_ = MakeMethodHash(WEB_METHOD_TOUCH_UP);
    CallResRegisterMethod(touchUpMethod_, "", nullptr);
}

void WebDelegateCross::HandleTouchMove(
    const int32_t& id, const double& x, const double& y, bool from_overlay)
{
    hash_ = MakeResourceHash();
    touchMoveMethod_ = MakeMethodHash(WEB_METHOD_TOUCH_MOVE);
    CallResRegisterMethod(touchMoveMethod_, "", nullptr);
}

void WebDelegateCross::HandleTouchCancel()
{
    hash_ = MakeResourceHash();
    touchCancelMethod_ = MakeMethodHash(WEB_METHOD_TOUCH_CANCEL);
    CallResRegisterMethod(touchCancelMethod_, "", nullptr);
}

void WebDelegateCross::HandleAxisEvent(const double& x, const double& y, const double& deltaX, const double& deltaY)
{
    // cross platform is not support now;
}

bool WebDelegateCross::OnKeyEvent(int32_t keyCode, int32_t keyAction)
{
    return true;
}

void WebDelegateCross::OnMouseEvent(
    int32_t x, int32_t y, const MouseButton button, const MouseAction action, int count)
{}

void WebDelegateCross::OnFocus()
{}

void WebDelegateCross::OnBlur()
{}

void WebDelegateCross::UpdateLocale()
{}

void WebDelegateCross::OnInactive()
{}

void WebDelegateCross::OnActive()
{}

void WebDelegateCross::ShowWebView()
{}

void WebDelegateCross::HideWebView()
{}

void WebDelegateCross::RecordWebEvent(Recorder::EventType eventType, const std::string& param) const
{
    if (!Recorder::EventRecorder::Get().IsComponentRecordEnable()) {
        return;
    }
    auto pattern = webPattern_.Upgrade();
    CHECK_NULL_VOID(pattern);
    auto host = pattern->GetHost();
    CHECK_NULL_VOID(host);
    Recorder::EventParamsBuilder builder;
    builder.SetId(host->GetInspectorIdValue(""))
        .SetType(host->GetHostTag())
        .SetEventType(eventType)
        .SetText(param)
        .SetHost(host)
        .SetDescription(host->GetAutoEventParamValue(""));
    Recorder::EventRecorder::Get().OnEvent(std::move(builder));
}

void WebDelegateCross::OnPageStarted(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), param]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            delegate->RecordWebEvent(Recorder::EventType::WEB_PAGE_BEGIN, param);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnPageStarted = webEventHub->GetOnPageStartedEvent();
                CHECK_NULL_VOID(propOnPageStarted);
                auto eventParam = std::make_shared<LoadWebPageStartEvent>(param);
                propOnPageStarted(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebPageStartEvent");
}

void WebDelegateCross::OnPageFinished(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), param]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            delegate->RecordWebEvent(Recorder::EventType::WEB_PAGE_END, param);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnPageFinished = webEventHub->GetOnPageFinishedEvent();
                CHECK_NULL_VOID(propOnPageFinished);
                auto eventParam = std::make_shared<LoadWebPageFinishEvent>(param);
                propOnPageFinished(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebPageFinishEvent");
}

void WebDelegateCross::OnPageError(const std::string& param)
{}

void WebDelegateCross::OnProgressChanged(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), param]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnProgressChange = webEventHub->GetOnProgressChangeEvent();
                CHECK_NULL_VOID(propOnProgressChange);
                auto eventParam = std::make_shared<LoadWebProgressChangeEvent>(atoi(param.c_str()));
                propOnProgressChange(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebProgressChangeEvent");
}

void WebDelegateCross::OnReceivedTitle(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), param]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnTitleReceive = webEventHub->GetOnTitleReceiveEvent();
                CHECK_NULL_VOID(propOnTitleReceive);
                auto eventParam = std::make_shared<LoadWebTitleReceiveEvent>(param);
                propOnTitleReceive(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebTitleReceiveEvent");
}

void WebDelegateCross::OnErrorReceive(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webResourceRequest = AceType::MakeRefPtr<WebResourceRequsetImpl>(object);
    CHECK_NULL_VOID(webResourceRequest);
    auto webResourceError = AceType::MakeRefPtr<WebResourceErrorImpl>(object);
    CHECK_NULL_VOID(webResourceError);
    auto requestHeader = webResourceRequest->GetRequestHeader();
    auto method = webResourceRequest->GetMethod();
    auto url = webResourceRequest->GetRequestUrl();
    auto hasGesture = webResourceRequest->IsRequestGesture();
    auto isMainFrame = webResourceRequest->IsMainFrame();
    auto isRedirect = webResourceRequest->IsRedirect();
    auto request = AceType::MakeRefPtr<WebRequest>(requestHeader, method, url, hasGesture, isMainFrame, isRedirect);
    auto errorInfo = webResourceError->GetErrorInfo();
    auto errorCode = webResourceError->GetErrorCode();
    auto error = AceType::MakeRefPtr<WebError>(errorInfo, errorCode);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), request, error]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnErrorReceive = webEventHub->GetOnErrorReceiveEvent();
                CHECK_NULL_VOID(propOnErrorReceive);
                auto eventParam = std::make_shared<ReceivedErrorEvent>(request, error);
                propOnErrorReceive(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebReceivedErrorEvent");
}

void WebDelegateCross::OnScroll(void* object)
{
    CHECK_NULL_VOID(object);
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webScrollOffset = AceType::MakeRefPtr<WebOffsetImpl>(object);
    CHECK_NULL_VOID(webScrollOffset);
    auto offsetX = webScrollOffset->GetX();
    auto offsetY = webScrollOffset->GetY();
    auto contentWidth = webScrollOffset->GetContentWidth();
    auto contentHeight = webScrollOffset->GetContentHeight();
    auto frameWidth = webScrollOffset->GetFrameWidth();
    auto frameHeight = webScrollOffset->GetFrameHeight();
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), offsetX, offsetY, contentWidth, contentHeight, frameWidth, frameHeight]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            webPattern->OnScroll(offsetX, offsetY, contentWidth, contentHeight, frameWidth, frameHeight);
            auto webEventHub = webPattern->GetWebEventHub();
            CHECK_NULL_VOID(webEventHub);
            auto propOnScroll = webEventHub->GetOnScrollEvent();
            CHECK_NULL_VOID(propOnScroll);
            auto eventParam = std::make_shared<WebOnScrollEvent>(offsetX, offsetY);
            propOnScroll(eventParam);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebScrollEvent");
}

void WebDelegateCross::OnScrollWillStart(void* object)
{
    CHECK_NULL_VOID(object);
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webScrollOffset = AceType::MakeRefPtr<WebOffsetImpl>(object);
    CHECK_NULL_VOID(webScrollOffset);
    auto offsetX = webScrollOffset->GetX();
    auto offsetY = webScrollOffset->GetY();

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), offsetX, offsetY]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            webPattern->OnScrollWillStart(offsetX, offsetY);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebScrollEvent");
}

void WebDelegateCross::OnScrollStart(void* object)
{
    CHECK_NULL_VOID(object);
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webScrollOffset = AceType::MakeRefPtr<WebOffsetImpl>(object);
    CHECK_NULL_VOID(webScrollOffset);
    auto offsetX = webScrollOffset->GetX();
    auto offsetY = webScrollOffset->GetY();

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), offsetX, offsetY]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            webPattern->OnScrollStart(offsetX, offsetY);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebScrollEvent");
}

void WebDelegateCross::OnScrollEnd(void* object)
{
    CHECK_NULL_VOID(object);
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this)]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            webPattern->OnScrollEndRecursive(std::nullopt);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebScrollEvent");
}

void WebDelegateCross::OnScaleChange(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webScaleChange = AceType::MakeRefPtr<WebScaleChangeImpl>(object);
    CHECK_NULL_VOID(webScaleChange);
    auto newScale = webScaleChange->GetNewScale();
    auto oldScale = webScaleChange->GetOldScale();
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), newScale, oldScale]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnScaleChange = webEventHub->GetOnScaleChangeEvent();
                CHECK_NULL_VOID(propOnScaleChange);
                auto eventParam = std::make_shared<ScaleChangeEvent>(oldScale, newScale);
                propOnScaleChange(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebScaleChangeEvent");
}

void WebDelegateCross::OnHttpErrorReceive(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webResourceRequest = AceType::MakeRefPtr<WebResourceRequsetImpl>(object);
    CHECK_NULL_VOID(webResourceRequest);
    auto webResourceResponse = AceType::MakeRefPtr<WebResourceResponseImpl>(object);
    CHECK_NULL_VOID(webResourceResponse);
    auto requestHeader = webResourceRequest->GetRequestHeader();
    auto method = webResourceRequest->GetMethod();
    auto url = webResourceRequest->GetRequestUrl();
    auto hasGesture = webResourceRequest->IsRequestGesture();
    auto isMainFrame = webResourceRequest->IsMainFrame();
    auto isRedirect = webResourceRequest->IsRedirect();
    auto request = AceType::MakeRefPtr<WebRequest>(requestHeader, method, url, hasGesture, isMainFrame, isRedirect);
    auto responseHeader = webResourceResponse->GetResponseHeader();
    auto responseDate = webResourceResponse->GetResponseData();
    auto encoding = webResourceResponse->GetEncoding();
    auto mimeType = webResourceResponse->GetMimeType();
    auto reason = webResourceResponse->GetReason();
    auto statusCode = webResourceResponse->GetStatusCode();
    auto response =
        AceType::MakeRefPtr<WebResponse>(responseHeader, responseDate, encoding, mimeType, reason, statusCode);

    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), request, response]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnHttpErrorReceive = webEventHub->GetOnHttpErrorReceiveEvent();
                CHECK_NULL_VOID(propOnHttpErrorReceive);
                auto eventParam = std::make_shared<ReceivedHttpErrorEvent>(request, response);
                propOnHttpErrorReceive(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebHttpErrorReceiveEvent");
}

bool WebDelegateCross::OnConsoleMessage(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool result = false;
    auto webConsoleMessage = AceType::MakeRefPtr<WebConsoleMessage>(object);
    CHECK_NULL_RETURN(webConsoleMessage, false);
    auto message = webConsoleMessage->GetMessage();
    auto sourceId = webConsoleMessage->GetSourceId();
    auto lineNumber = webConsoleMessage->GetLineNumber();
    auto messageLevel = webConsoleMessage->GetMessageLevel();
    auto consoleMessageParam = AceType::MakeRefPtr<WebConsoleMessageParam>(message, sourceId, lineNumber, messageLevel);

    context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), consoleMessageParam, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnConsole = webEventHub->GetOnConsoleEvent();
                CHECK_NULL_VOID(propOnConsole);
                auto eventParam = std::make_shared<LoadWebConsoleLogEvent>(consoleMessageParam);
                result = propOnConsole(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebConsoleMessageEvent");
    return result;
}

bool WebDelegateCross::OnLoadIntercept(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool result = false;
    auto webResourceRequest = AceType::MakeRefPtr<WebResourceRequsetImpl>(object);
    CHECK_NULL_RETURN(webResourceRequest, false);
    auto requestHeader = webResourceRequest->GetRequestHeader();
    auto method = webResourceRequest->GetMethod();
    auto url = webResourceRequest->GetRequestUrl();
    auto hasGesture = webResourceRequest->IsRequestGesture();
    auto isMainFrame = webResourceRequest->IsMainFrame();
    auto isRedirect = webResourceRequest->IsRedirect();
    auto request = AceType::MakeRefPtr<WebRequest>(requestHeader, method, url, hasGesture, isMainFrame, isRedirect);

    context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), request, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnLoadIntercept = webEventHub->GetOnLoadInterceptEvent();
                CHECK_NULL_VOID(propOnLoadIntercept);
                auto eventParam = std::make_shared<LoadInterceptEvent>(request);
                result = propOnLoadIntercept(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebLoadInterceptEvent");
    return result;
}

bool WebDelegateCross::OnOverrideUrlLoading(void* object)
{
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, false);
    ContainerScope scope(instanceId_);
    bool result = false;
    auto webResourceRequest = AceType::MakeRefPtr<WebResourceRequsetImpl>(object);
    CHECK_NULL_RETURN(webResourceRequest, false);
    auto requestHeader = webResourceRequest->GetRequestHeader();
    auto method = webResourceRequest->GetMethod();
    auto url = webResourceRequest->GetRequestUrl();
    auto hasGesture = webResourceRequest->IsRequestGesture();
    auto isMainFrame = webResourceRequest->IsMainFrame();
    auto isRedirect = webResourceRequest->IsRedirect();
    auto request = AceType::MakeRefPtr<WebRequest>(requestHeader, method, url, hasGesture, isMainFrame, isRedirect);

    taskExecutor->PostSyncTask(
        [weak = WeakClaim(this), request, &result]() {
            auto delegate = weak.Upgrade();
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnOverrideUrlLoading = webEventHub->GetOnOverrideUrlLoadingEvent();
                CHECK_NULL_VOID(propOnOverrideUrlLoading);
                auto eventParam = std::make_shared<LoadOverrideEvent>(request);
                CHECK_NULL_VOID(eventParam);
                result = propOnOverrideUrlLoading(eventParam);
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebOverrideUrlLoadingEvent");
    return result;
}

auto WaitForReady(std::function<bool()> checkFunc, int timeoutMs) -> bool
{
    const auto start = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::milliseconds(timeoutMs);

    while (!checkFunc()) {
        if (std::chrono::steady_clock::now() - start >= timeout) {
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(POLLING_INTERVAL_MS));
    }
    return true;
}

RefPtr<WebResponse> TimeoutResponse()
{
    TAG_LOGE(AceLogTag::ACE_WEB, "OnInterceptRequest request has timed out.");
    auto timeoutResponse = AceType::MakeRefPtr<WebResponse>();
    std::string errorHtml = "<html><body><h1>Response Timeout</h1><p>The request has timed out.</p></body></html>";
    std::string mimeType = "text/html";
    std::string encoding = "utf-8";
    std::string reason = "Response timed out";
    timeoutResponse->SetData(errorHtml);
    timeoutResponse->SetMimeType(mimeType);
    timeoutResponse->SetEncoding(encoding);
    timeoutResponse->SetStatusCode(HTTP_STATUS_GATEWAY_TIMEOUT);
    timeoutResponse->SetReason(reason);
    CHECK_NULL_RETURN(timeoutResponse, nullptr);
    return timeoutResponse;
}

RefPtr<WebResponse> WebDelegateCross::OnInterceptRequest(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, nullptr);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, nullptr);
    RefPtr<WebResponse> result = nullptr;
    auto webResourceRequest = AceType::MakeRefPtr<WebResourceRequsetImpl>(object);
    CHECK_NULL_RETURN(webResourceRequest, nullptr);
    auto requestHeader = webResourceRequest->GetRequestHeader();
    auto method = webResourceRequest->GetMethod();
    auto url = webResourceRequest->GetRequestUrl();
    auto hasGesture = webResourceRequest->IsRequestGesture();
    auto isMainFrame = webResourceRequest->IsMainFrame();
    auto isRedirect = webResourceRequest->IsRedirect();
    auto request = AceType::MakeRefPtr<WebRequest>(requestHeader, method, url, hasGesture, isMainFrame, isRedirect);

    auto jsTaskExecutor = SingleTaskExecutor::Make(context->GetTaskExecutor(), TaskExecutor::TaskType::JS);
    jsTaskExecutor.PostSyncTask(
        [weak = WeakClaim(this), request, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnInterceptRequestEvent = webEventHub->GetOnInterceptRequestEvent();
                CHECK_NULL_VOID(propOnInterceptRequestEvent);
                auto param = std::make_shared<OnInterceptRequestEvent>(request);
                result = propOnInterceptRequestEvent(param);
            }
        },
        "ArkUIWebInterceptRequest");
#if defined ANDROID_PLATFORM
    if (!result) {
        return nullptr;
    }
    auto isReady = result->GetResponseStatus();
    if (!isReady) {
        isReady = WaitForReady([&] { return result->GetResponseStatus(); }, TIMEOUT_DURATION_MS);
        if (!isReady) {
            result = TimeoutResponse();
        }
    }
#endif
    return result;
}

void WebDelegateCross::OnPageVisible(const std::string& param)
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), param]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnPageVisible = webEventHub->GetOnPageVisibleEvent();
                CHECK_NULL_VOID(propOnPageVisible);
                auto eventParam = std::make_shared<PageVisibleEvent>(param);
                propOnPageVisible(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebPageVisibleEvent");
}

void WebDelegateCross::OnDownloadStart(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webDownloadResponse = AceType::MakeRefPtr<WebDownloadResponseImpl>(object);
    CHECK_NULL_VOID(webDownloadResponse);
    auto url = webDownloadResponse->GetUrl();
    auto userAgent = webDownloadResponse->GetUserAgent();
    auto contentDisposition = webDownloadResponse->GetContentDisposition();
    auto mimetype = webDownloadResponse->GetMimetype();
    auto contentLength = webDownloadResponse->GetContentLength();
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), url, userAgent, contentDisposition, mimetype, contentLength]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnDownloadStart = webEventHub->GetOnDownloadStartEvent();
                CHECK_NULL_VOID(propOnDownloadStart);
                auto eventParam =
                    std::make_shared<DownloadStartEvent>(url, userAgent, contentDisposition, mimetype, contentLength);
                propOnDownloadStart(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebDownloadStartEvent");
}

void WebDelegateCross::OnRefreshAccessedHistory(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto WebRefreshAccessedHistory = AceType::MakeRefPtr<WebRefreshAccessedHistoryImpl>(object);
    CHECK_NULL_VOID(WebRefreshAccessedHistory);
    auto url = WebRefreshAccessedHistory->GetUrl();
    auto isRefreshed = WebRefreshAccessedHistory->GetIsRefreshed();
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), url, isRefreshed]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnRefreshAccessedHistory = webEventHub->GetOnRefreshAccessedHistoryEvent();
                CHECK_NULL_VOID(propOnRefreshAccessedHistory);
                auto eventParam = std::make_shared<RefreshAccessedHistoryEvent>(url, isRefreshed);
                propOnRefreshAccessedHistory(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebRefreshAccessedHistoryEvent");
}

void WebDelegateCross::OnFullScreenEnter(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webFullScreenEnter = AceType::MakeRefPtr<WebFullScreenEnterImpl>(object);
    CHECK_NULL_VOID(webFullScreenEnter);
    int heights = webFullScreenEnter->GetHeights();
    int widths = webFullScreenEnter->GetWidths();
    auto fullScreenExitHandlerResponse = AceType::MakeRefPtr<FullScreenExitHandlerImpl>(object);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), widths, heights, fullScreenExitHandlerResponse]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnFullScreenEnter = webEventHub->GetOnFullScreenEnterEvent();
                CHECK_NULL_VOID(propOnFullScreenEnter);
                auto eventParam = std::make_shared<FullScreenEnterEvent>(fullScreenExitHandlerResponse,
                    widths, heights);
                propOnFullScreenEnter(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebFullScreenEnterEvent");
}

void WebDelegateCross::OnFullScreenExit(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webFullScreenExit = AceType::MakeRefPtr<WebFullScreenExitImpl>(object);
    CHECK_NULL_VOID(webFullScreenExit);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this)]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnFullScreenExit = webEventHub->GetOnFullScreenExitEvent();
                CHECK_NULL_VOID(propOnFullScreenExit);
                auto eventParam = std::make_shared<FullScreenExitEvent>();
                propOnFullScreenExit(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebFullScreenExitEvent");
}

bool WebDelegateCross::OnCommonDialog(void* object, DialogEventType dialogEventType)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool result = false;
    auto webCommonDialog = AceType::MakeRefPtr<WebCommonDialogImpl>(object);
    CHECK_NULL_RETURN(webCommonDialog, false);
    auto url = webCommonDialog->GetUrl();
    auto message = webCommonDialog->GetMessage();
    auto value = dialogEventType == DialogEventType::DIALOG_EVENT_PROMPT ? webCommonDialog->GetValue() : "";
    auto dialogResult = AceType::MakeRefPtr<DialogResult>(object, dialogEventType);
    context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), url, message, value, dialogResult, dialogEventType, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto eventParam = std::make_shared<WebDialogEvent>(url, message, value, dialogEventType, dialogResult);
                result = webEventHub->FireOnCommonDialogEvent(eventParam, dialogEventType);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebFireOnCommonDialogEvent");
    return result;
}

bool WebDelegateCross::OnPermissionRequest(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool result = false;
    auto webPermissionRequest = AceType::MakeRefPtr<PermissionRequestImpl>(object);
    webPermissionRequest->SetOrigin();
    webPermissionRequest->SetResources();
    CHECK_NULL_RETURN(webPermissionRequest, false);
    context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), webPermissionRequest, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnPermissionRequest = webEventHub->GetOnPermissionRequestEvent();
                CHECK_NULL_VOID(propOnPermissionRequest);
                auto eventParam = std::make_shared<WebPermissionRequestEvent>(webPermissionRequest);
                propOnPermissionRequest(eventParam);
                result = true;
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebPermissionRequestEvent");
    return result;
}

bool WebDelegateCross::OnHttpAuthRequest(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool result = false;
    auto webHttpAuthRequest = AceType::MakeRefPtr<WebAuthRequestImpl>(object);
    CHECK_NULL_RETURN(webHttpAuthRequest, false);
    auto host = webHttpAuthRequest->GetHost();
    auto realm = webHttpAuthRequest->GetRealm();
    auto authResult = AceType::MakeRefPtr<WebAuthResult>(object);
    context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), host, realm, authResult, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnHttpAuthRequest = webEventHub->GetOnHttpAuthRequestEvent();
                CHECK_NULL_VOID(propOnHttpAuthRequest);
                auto eventParam = std::make_shared<WebHttpAuthEvent>(authResult, host, realm);
                result = propOnHttpAuthRequest(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebHttpAuthRequestEvent");
    return result;
}

bool WebDelegateCross::OnShowFileChooser(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    bool isAllowed = false;
    auto webFileChooser = AceType::MakeRefPtr<WebFileChooserImpl>(object);
    CHECK_NULL_RETURN(webFileChooser, false);
    auto title = webFileChooser->GetTitle();
    auto mode = webFileChooser->GetMode();
    auto defaultFileName = webFileChooser->GetDefaultFileName();
    auto acceptType = webFileChooser->GetAcceptType();
    auto isCapture = webFileChooser->IsCapture();
    auto param = AceType::MakeRefPtr<FileSelectorParam>(title, mode, defaultFileName, acceptType, isCapture);
    auto result = AceType::MakeRefPtr<WebFileSelectorResult>(object);
    bool postTask = context->GetTaskExecutor()->PostSyncTask(
        [weak = WeakClaim(this), param, result, &isAllowed]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnFileSelectorShow = webEventHub->GetOnFileSelectorShowEvent();
                CHECK_NULL_VOID(propOnFileSelectorShow);
                auto eventParam = std::make_shared<FileSelectorEvent>(param, result);
                CHECK_NULL_VOID(eventParam);
                isAllowed = propOnFileSelectorShow(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebFileSelectorShowEvent");
    return isAllowed;
}

bool WebDelegateCross::OnSslErrorEventReceive(void* object)
{
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, false);
    auto sslErrorEvent = AceType::MakeRefPtr<SslErrorEventImpl>(object);
    CHECK_NULL_RETURN(sslErrorEvent, false);
    bool result = false;
    auto sslErrorResult = AceType::MakeRefPtr<SslErrorResultImpl>(object);
    int32_t error = sslErrorEvent->GetError();
    std::vector<std::string> certChainData = sslErrorEvent->GetCertChainData();
#if defined ANDROID_PLATFORM
    EncodeCertificateChainToDer(certChainData);
#endif
    taskExecutor->PostSyncTask(
        [weak = WeakClaim(this), &sslErrorResult, error, &certChainData, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            auto webEventHub = webPattern->GetWebEventHub();
            CHECK_NULL_VOID(webEventHub);
            auto propOnSslErrorRequest = webEventHub->GetOnSslErrorRequestEvent();
            CHECK_NULL_VOID(propOnSslErrorRequest);
            auto eventParam = std::make_shared<WebSslErrorEvent>(sslErrorResult, error, certChainData);
            result = propOnSslErrorRequest(eventParam);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebSslErrorEventReceive");
    return result;
}

bool WebDelegateCross::OnSslErrorEvent(void* object)
{
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, false);
    auto allSslErrorEvent = AceType::MakeRefPtr<AllSslErrorEventImpl>(object);
    CHECK_NULL_RETURN(allSslErrorEvent, false);
    bool result = false;
    auto allSslErrorResult = AceType::MakeRefPtr<AllSslErrorResultImpl>(object);
    int32_t error = allSslErrorEvent->GetError();
    std::string url = allSslErrorEvent->GetUrl();
    std::string originalUrl = allSslErrorEvent->GetOriginalUrl();
    std::string referrer = allSslErrorEvent->GetReferrer();
    bool isFatalError = allSslErrorEvent->IsFatalError();
    bool isMainFrame = allSslErrorEvent->IsMainFrame();
    std::vector<std::string> certificateChain = allSslErrorEvent->GetCertificateChain();
#if defined ANDROID_PLATFORM
    EncodeCertificateChainToDer(certificateChain);
#endif
    taskExecutor->PostSyncTask([weak = WeakClaim(this), &allSslErrorResult, error, url, originalUrl, referrer,
        isFatalError, isMainFrame, &certificateChain, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            auto webEventHub = webPattern->GetWebEventHub();
            CHECK_NULL_VOID(webEventHub);
            auto propOnAllSslErrorRequest = webEventHub->GetOnAllSslErrorRequestEvent();
            auto propOnSslErrorRequest = webEventHub->GetOnSslErrorRequestEvent();
            if (propOnAllSslErrorRequest == nullptr) {
                if (propOnSslErrorRequest == nullptr || !isMainFrame) {
                    CHECK_NULL_VOID(allSslErrorResult);
                    allSslErrorResult->HandleCancel(false);
                }
                return;
            }
            auto eventParam = std::make_shared<WebAllSslErrorEvent>(allSslErrorResult, error, url, originalUrl,
                referrer, isFatalError, isMainFrame, certificateChain);
            result = propOnAllSslErrorRequest(eventParam);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebSslErrorEvent");
    return result;
}

bool WebDelegateCross::OnClientAuthenticationRequest(void* object)
{
    CHECK_NULL_RETURN(object, false);
    auto context = context_.Upgrade();
    CHECK_NULL_RETURN(context, false);
    auto taskExecutor = context->GetTaskExecutor();
    CHECK_NULL_RETURN(taskExecutor, false);
    auto sslSelectCertEvent = AceType::MakeRefPtr<SslSelectCertEventImpl>(object);
    CHECK_NULL_RETURN(sslSelectCertEvent, false);
    bool result = false;
    auto sslSelectCertResult = AceType::MakeRefPtr<SslSelectCertResultImpl>(object);
    std::string host = sslSelectCertEvent->GetHost();
    int32_t port = sslSelectCertEvent->GetPort();
    std::vector<std::string> keyTypes = sslSelectCertEvent->GetKeyTypes();
    std::vector<std::string> issuers = sslSelectCertEvent->GetIssuers();
    taskExecutor->PostSyncTask(
        [weak = WeakClaim(this), &sslSelectCertResult, host, port, &keyTypes, &issuers, &result]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            auto webPattern = delegate->webPattern_.Upgrade();
            CHECK_NULL_VOID(webPattern);
            auto webEventHub = webPattern->GetWebEventHub();
            CHECK_NULL_VOID(webEventHub);
            auto propOnSslSelectCertRequest = webEventHub->GetOnSslSelectCertRequestEvent();
            CHECK_NULL_VOID(propOnSslSelectCertRequest);
            auto eventParam = std::make_shared<WebSslSelectCertEvent>(
                sslSelectCertResult, host, port, keyTypes, issuers);
            result = propOnSslSelectCertRequest(eventParam);
        },
        TaskExecutor::TaskType::JS, "ArkUIWebClientAuthenticationRequest");
    return result;
}

void WebDelegateCross::OnGeolocationPermissionsShowPrompt(void* object)
{
    ContainerScope scope(instanceId_);
    CHECK_NULL_VOID(object);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    auto webGeolocationImpl = AceType::MakeRefPtr<WebGeolocationImpl>(object);
    CHECK_NULL_VOID(webGeolocationImpl);
    auto origin = webGeolocationImpl->GetOrigin();
    auto callback = AceType::MakeRefPtr<Geolocation>(object);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this), origin, callback]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnGeolocationShow = webEventHub->GetOnGeolocationShowEvent();
                CHECK_NULL_VOID(propOnGeolocationShow);
                auto eventParam = std::make_shared<LoadWebGeolocationShowEvent>(origin, callback);
                propOnGeolocationShow(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebGeolocationShowEvent");
}

void WebDelegateCross::OnGeolocationPermissionsHidePrompt()
{
    ContainerScope scope(instanceId_);
    auto context = context_.Upgrade();
    CHECK_NULL_VOID(context);
    context->GetTaskExecutor()->PostTask(
        [weak = WeakClaim(this)]() {
            auto delegate = weak.Upgrade();
            CHECK_NULL_VOID(delegate);
            if (Container::IsCurrentUseNewPipeline()) {
                auto webPattern = delegate->webPattern_.Upgrade();
                CHECK_NULL_VOID(webPattern);
                auto webEventHub = webPattern->GetWebEventHub();
                CHECK_NULL_VOID(webEventHub);
                auto propOnGeolocationHide = webEventHub->GetOnGeolocationHideEvent();
                CHECK_NULL_VOID(propOnGeolocationHide);
                auto eventParam = std::make_shared<LoadWebGeolocationHideEvent>("");
                propOnGeolocationHide(eventParam);
                return;
            }
        },
        TaskExecutor::TaskType::JS, "ArkUIWebGeolocationHideEvent");
}

void WebDelegateCross::EncodeCertificateChainToDer(std::vector<std::string>& certificateChain)
{
    std::vector<std::string> finalCertChainData;
    std::vector<uint8_t> bytes;
    for (auto &data : certificateChain) {
        for (size_t i = 0; i < data.length(); i += TWO_BYTES) {
            std::string byteString = data.substr(i, TWO_BYTES);
            bytes.push_back(static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, TWO_BYTES_LENGTH)));
        }
        finalCertChainData.push_back(std::string(bytes.begin(), bytes.end()));
        bytes.clear();
    }
    certificateChain = finalCertChainData;
}

void WebDelegateCross::UpdateUserAgent(const std::string& userAgent) {}

void WebDelegateCross::UpdateBackgroundColor(const int backgroundColor)
{
    hash_ = MakeResourceHash();
    updateBackgroundColor_ = MakeMethodHash(WEB_ATTRIBUTE_BACKGROUND_COLOR);
    std::stringstream paramStream;
    paramStream << NTC_BACKGROUND_COLOR << WEB_PARAM_EQUALS << backgroundColor;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateBackgroundColor_, param, nullptr);
}

void WebDelegateCross::UpdateInitialScale(float scale) {}
void WebDelegateCross::UpdateJavaScriptEnabled(const bool& isJsEnabled)
{
    hash_ = MakeResourceHash();
    updateJavaScriptEnabled_ = MakeMethodHash(WEB_ATTRIBUTE_JAVASCRIPT_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_JAVASCRIPT_ACCESS << WEB_PARAM_EQUALS << isJsEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateJavaScriptEnabled_, param, nullptr);
}

void WebDelegateCross::UpdateAllowFileAccess(const bool& isFileAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateFileAccessMethod_ = MakeMethodHash(WEB_FILE_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_FILE_ACCESS << WEB_PARAM_EQUALS << isFileAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateFileAccessMethod_, param, nullptr);
}

void WebDelegateCross::UpdateBlockNetworkImage(const bool& onLineImageAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateBlockNetworkImageMethod_ = MakeMethodHash(WEB_ATTRIBUTE_ONLINE_IMAGE_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_ONLINE_IMAGE_ACCESS << WEB_PARAM_EQUALS << onLineImageAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateBlockNetworkImageMethod_, param, nullptr);
}

void WebDelegateCross::UpdateLoadsImagesAutomatically(const bool& isImageAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateLoadsImagesAutomaticallyMethod_ = MakeMethodHash(WEB_IMAGE_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_IMAGE_ACCESS << WEB_PARAM_EQUALS << isImageAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateLoadsImagesAutomaticallyMethod_, param, nullptr);
}

void WebDelegateCross::UpdateMixedContentMode(const MixedModeContent& mixedMode)
{
    hash_ = MakeResourceHash();
    onUpdateMixedContentModeMethod_ = MakeMethodHash(WEB_ATTRIBUTE_MIXED_MODE);
    std::stringstream paramStream;
    paramStream << NTC_MIXED_MODE << WEB_PARAM_EQUALS << mixedMode;
    std::string param = paramStream.str();
    CallResRegisterMethod(onUpdateMixedContentModeMethod_, param, nullptr);
}

void WebDelegateCross::UpdateSupportZoom(const bool& isZoomAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateZoomAccess_ = MakeMethodHash(WEB_ATTRIBUTE_ZOOM_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_ZOOM_ACCESS << WEB_PARAM_EQUALS << isZoomAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateZoomAccess_, param, nullptr);
}

void WebDelegateCross::UpdateDomStorageEnabled(const bool& isDomStorageAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateDomStorageEnabledMethod_ = MakeMethodHash(WEB_ATTRIBUTE_DOM_STORAGE_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_DOM_STORAGE_ACCESS << WEB_PARAM_EQUALS << isDomStorageAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateDomStorageEnabledMethod_, param, nullptr);
}

void WebDelegateCross::UpdateGeolocationEnabled(const bool& isGeolocationAccessEnabled)
{
    hash_ = MakeResourceHash();
    UpdateGeolocationEnabledMethod_ = MakeMethodHash(WEB_ATTRIBUTE_GEOLOCATION_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_GEOLOCATION_ACCESS << WEB_PARAM_EQUALS << isGeolocationAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(UpdateGeolocationEnabledMethod_, param, nullptr);
}

void WebDelegateCross::UpdateCacheMode(const WebCacheMode& mode)
{
    hash_ = MakeResourceHash();
    updateCacheModeMethod_ = MakeMethodHash(WEB_CACHE_MODE);
    std::stringstream paramStream;
    paramStream << NTC_CACHE_MODE << WEB_PARAM_EQUALS << mode;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateCacheModeMethod_, param, nullptr);
}

void WebDelegateCross::UpdateDarkMode(const WebDarkMode& mode)
{}

void WebDelegateCross::UpdateForceDarkAccess(const bool& access)
{}

void WebDelegateCross::UpdateAudioResumeInterval(const int32_t& resumeInterval)
{}

void WebDelegateCross::UpdateAudioExclusive(const bool& audioExclusive)
{}

void WebDelegateCross::UpdateAudioSessionType(const WebAudioSessionType& audioSessionType)
{}

void WebDelegateCross::UpdateOverviewModeEnabled(const bool& isOverviewModeAccessEnabled)
{}

void WebDelegateCross::UpdateFileFromUrlEnabled(const bool& isFileFromUrlAccessEnabled)
{}

void WebDelegateCross::UpdateDatabaseEnabled(const bool& isDatabaseAccessEnabled)
{}

void WebDelegateCross::UpdateTextZoomRatio(const int32_t& textZoomRatioNum)
{
    if (textZoomRatioNum <= 0 || textZoomRatioNum > INT32_MAX) {
        TAG_LOGE(AceLogTag::ACE_WEB, "textZoomRatioNum is out of range");
        return;
    }
    hash_ = MakeResourceHash();
    updateTextZoomRatioMethod_ = MakeMethodHash(WEB_TEXT_ZOOM_RATIO);
    std::stringstream paramStream;
    paramStream << NTC_TEXT_ZOOM_RATIO << WEB_PARAM_EQUALS << textZoomRatioNum;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateTextZoomRatioMethod_, param, nullptr);
}

void WebDelegateCross::UpdateWebDebuggingAccess(bool isWebDebuggingAccessEnabled)
{}

void WebDelegateCross::UpdatePinchSmoothModeEnabled(bool isPinchSmoothModeEnabled)
{}

void WebDelegateCross::UpdateMediaPlayGestureAccess(bool isNeedGestureAccess)
{
    hash_ = MakeResourceHash();
    updateMediaPlayGestureAccess_ = MakeMethodHash(WEB_ATTRIBUTE_MEDIA_PLAY_GESTURE_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_MEDIA_PLAY_GESTURE_ACCESS << WEB_PARAM_EQUALS << isNeedGestureAccess;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateMediaPlayGestureAccess_, param, nullptr);
}

void WebDelegateCross::UpdateMultiWindowAccess(bool isMultiWindowAccessEnabled)
{}

void WebDelegateCross::UpdateAllowWindowOpenMethod(bool isAllowWindowOpenMethod)
{}

void WebDelegateCross::UpdateWebCursiveFont(const std::string& cursiveFontFamily)
{}

void WebDelegateCross::UpdateWebFantasyFont(const std::string& fantasyFontFamily)
{}

void WebDelegateCross::UpdateWebFixedFont(const std::string& fixedFontFamily)
{}

void WebDelegateCross::UpdateWebSansSerifFont(const std::string& sansSerifFontFamily)
{}

void WebDelegateCross::UpdateWebSerifFont(const std::string& serifFontFamily)
{}

void WebDelegateCross::UpdateWebStandardFont(const std::string& standardFontFamily)
{}

void WebDelegateCross::UpdateDefaultFixedFontSize(int32_t size)
{}

void WebDelegateCross::UpdateDefaultFontSize(int32_t defaultFontSize)
{}

void WebDelegateCross::UpdateMinFontSize(int32_t minFontSize)
{
    hash_ = MakeResourceHash();
    updateMinFontSize_ = MakeMethodHash(WEB_ATTRIBUTE_MIN_FONT_SIZE);
    std::stringstream paramStream;
    paramStream << NTC_MIN_FONT_SIZE << WEB_PARAM_EQUALS << std::clamp(minFontSize, FONT_MIN_SIZE, FONT_MAX_SIZE);
    std::string param = paramStream.str();
    CallResRegisterMethod(updateMinFontSize_, param, nullptr);
}

void WebDelegateCross::UpdateMinLogicalFontSize(int32_t minLogicalFontSize)
{}

void WebDelegateCross::UpdateBlockNetwork(bool isNetworkBlocked)
{
    hash_ = MakeResourceHash();
    onBlockNetworkUpdateMethod_ = MakeMethodHash(WEB_ATTRIBUTE_BLOCK_NETWORK);
    std::stringstream paramStream;
    paramStream << NTC_BLOCK_NETWORK << WEB_PARAM_EQUALS << isNetworkBlocked;
    std::string param = paramStream.str();
    CallResRegisterMethod(onBlockNetworkUpdateMethod_, param, nullptr);
}

void WebDelegateCross::UpdateHorizontalScrollBarAccess(bool isHorizontalScrollBarAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateHorizontalScrollBarAccess_ = MakeMethodHash(WEB_ATTRIBUTE_HORIZONTAL_SCROLLBAR_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_HORIZONTAL_SCROLLBAR_ACCESS << WEB_PARAM_EQUALS << isHorizontalScrollBarAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateHorizontalScrollBarAccess_, param, nullptr);
}

void WebDelegateCross::UpdateVerticalScrollBarAccess(bool isVerticalScrollBarAccessEnabled)
{
    hash_ = MakeResourceHash();
    updateVerticalScrollBarAccess_ = MakeMethodHash(WEB_ATTRIBUTE_VERTICAL_SCROLLBAR_ACCESS);
    std::stringstream paramStream;
    paramStream << NTC_VERTICAL_SCROLLBAR_ACCESS << WEB_PARAM_EQUALS << isVerticalScrollBarAccessEnabled;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateVerticalScrollBarAccess_, param, nullptr);
}

void WebDelegateCross::UpdateScrollBarColor(const std::string& colorValue)
{}

void WebDelegateCross::LoadUrl()
{}

bool WebDelegateCross::LoadDataWithRichText()
{
    auto webPattern = webPattern_.Upgrade();
    CHECK_NULL_RETURN(webPattern, false);
    auto webData = webPattern->GetWebData();
    CHECK_NULL_RETURN(webData, false);
    const std::string& data = webData.value();
    if (data.empty()) {
        return false;
    }
    hash_ = MakeResourceHash();
    loadDataMethod_ = MakeMethodHash(WEB_METHOD_LOAD_DATA);
    std::stringstream paramStream;
    paramStream << NTC_PARAM_LOADDATA_DATA << WEB_PARAM_EQUALS
                << data.c_str() << WEB_PARAM_AND
                << NTC_PARAM_LOADDATA_MIMETYPE << WEB_PARAM_EQUALS
                << MIMETYPE << WEB_PARAM_AND
                << NTC_PARAM_LOADDATA_ENCODING << WEB_PARAM_EQUALS
                << ENCODING;
    std::string param = paramStream.str();
    CallResRegisterMethod(loadDataMethod_, param, nullptr);
    return true;
}

void WebDelegateCross::SetBackgroundColor(int32_t backgroundColor)
{}

void WebDelegateCross::SetBoundsOrResize(const Size& drawSize, const Offset& offset)
{
    hash_ = MakeResourceHash();
    updateLayoutMethod_ = MakeMethodHash(WEB_METHOD_UPDATE_LAYOUT);
    auto context = context_.Upgrade();
    if (!context) {
        return;
    }
    std::stringstream paramStream;
    float viewScale = context->GetViewScale();
    paramStream << NTC_PARAM_WIDTH << WEB_PARAM_EQUALS
                << drawSize.Width() * viewScale << WEB_PARAM_AND
                << NTC_PARAM_HEIGHT << WEB_PARAM_EQUALS
                << drawSize.Height() * viewScale << WEB_PARAM_AND
                << NTC_PARAM_LEFT << WEB_PARAM_EQUALS
                << offset.GetX() * viewScale << WEB_PARAM_AND
                << NTC_PARAM_TOP << WEB_PARAM_EQUALS
                << offset.GetY() * viewScale;
    std::string param = paramStream.str();
    CallResRegisterMethod(updateLayoutMethod_, param, nullptr);
}

void WebDelegateCross::SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    // cross platform is not support now;
}

void WebDelegateCross::UpdateOptimizeParserBudgetEnabled(const bool enable)
{
    // cross platform is not support now;
}

void WebDelegateCross::MaximizeResize()
{
    // cross platform is not support now;
}

void WebDelegateCross::RunJsProxyCallback()
{
    auto pattern = webPattern_.Upgrade();
    CHECK_NULL_VOID(pattern);
    pattern->CallJsProxyCallback();
}

void WebDelegateCross::SetNestedScrollOptionsExt(NestedScrollOptionsExt nestedOpt)
{
#if defined(IOS_PLATFORM)
    auto webId = GetWebId();
    SetNestedScrollOptionsExtOC(webId, static_cast<void*>(&nestedOpt));
#else
    hash_ = MakeResourceHash();
    setNestedScrollExtMethod_ = MakeMethodHash(WEB_METHOD_SET_NESTED_SCROLL_EXT);
    std::stringstream paramStream;
    paramStream << NTC_PARAM_SET_NESTED_SCROLL_EXT_UP << WEB_PARAM_EQUALS
                << (int)nestedOpt.scrollUp << WEB_PARAM_AND
                << NTC_PARAM_SET_NESTED_SCROLL_EXT_DOWN << WEB_PARAM_EQUALS
                << (int)nestedOpt.scrollDown << WEB_PARAM_AND
                << NTC_PARAM_SET_NESTED_SCROLL_EXT_LEFT << WEB_PARAM_EQUALS
                << (int)nestedOpt.scrollLeft << WEB_PARAM_AND
                << NTC_PARAM_SET_NESTED_SCROLL_EXT_RIGHT << WEB_PARAM_EQUALS
                << (int)nestedOpt.scrollRight;
    std::string param = paramStream.str();
    CallResRegisterMethod(setNestedScrollExtMethod_, param, nullptr);
#endif
}

void WebDelegateCross::SetScrollLocked(const bool value)
{
#if defined(IOS_PLATFORM)
    auto webId = GetWebId();
    SetScrollLockedRegisterOC(webId, value);
#endif
}
}
