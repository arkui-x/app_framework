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

#ifndef FOUNDATION_ACE_FRAMEWORKS_WEB_PATTERN_WEB_DELEGATE_CROSS_H
#define FOUNDATION_ACE_FRAMEWORKS_WEB_PATTERN_WEB_DELEGATE_CROSS_H

#include "web_object_event.h"

#include "base/log/log.h"
#include "core/common/container.h"
#include "core/common/recorder/event_recorder.h"
#include "web_pattern.h"
#include "web_resource.h"
#include "core/components_ng/pattern/web/web_delegate_interface.h"
#include "core/pipeline/pipeline_base.h"

namespace OHOS::Ace {
class WebResourceRequsetImpl : public AceType {
    DECLARE_ACE_TYPE(WebResourceRequsetImpl, AceType);
public:
    explicit WebResourceRequsetImpl(void* object) : object_(object) {}
    std::map<std::string, std::string> GetRequestHeader() const;
    std::string GetRequestUrl() const;
    std::string GetMethod() const;
    bool IsRequestGesture() const;
    bool IsMainFrame() const;
    bool IsRedirect() const;
private:
    void* object_ = nullptr;
};

class WebOffsetImpl : public AceType {
    DECLARE_ACE_TYPE(WebOffsetImpl, AceType);
public:
    explicit WebOffsetImpl(void* object) : object_(object) {}
    double GetX() const;
    double GetY() const;
    double GetContentWidth() const;
    double GetContentHeight() const;
    double GetFrameWidth() const;
    double GetFrameHeight() const;
private:
    void* object_ = nullptr;
};

class WebScaleChangeImpl : public AceType {
    DECLARE_ACE_TYPE(WebScaleChangeImpl, AceType);
public:
    explicit WebScaleChangeImpl(void* object) : object_(object) {}
    float GetNewScale() const;
    float GetOldScale() const;
private:
    void* object_ = nullptr;
};

class WebResourceResponseImpl : public AceType {
    DECLARE_ACE_TYPE(WebResourceResponseImpl, AceType);
public:
    explicit WebResourceResponseImpl(void* object) : object_(object) {}
    std::map<std::string, std::string> GetResponseHeader() const;
    std::string GetResponseData() const;
    std::string GetEncoding() const;
    std::string GetMimeType() const;
    std::string GetReason() const;
    int GetStatusCode() const;
private:
    void* object_ = nullptr;
};

class WebConsoleMessage : public AceType {
    DECLARE_ACE_TYPE(WebConsoleMessage, AceType);
public:
    explicit WebConsoleMessage(void* object) : object_(object) {}
    std::string GetMessage() const;
    int GetMessageLevel() const;
    std::string GetSourceId() const;
    int GetLineNumber() const;
private:
    void* object_ = nullptr;
};

class DialogResult : public Result {
    DECLARE_ACE_TYPE(DialogResult, Result);
public:
    DialogResult(void* object, DialogEventType dialogEventType) : object_(object), dialogEventType_(dialogEventType)
    {
        auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
        index_ = obj->AddObject(object);
    }
    ~DialogResult()
    {
        auto obj = WebObjectEventManager::GetInstance().GetCommonDialogObject();
        obj->DelObject(index_);
    }
    void Confirm(const std::string& promptResult) override;
    void Confirm() override;
    void Cancel() override;

private:
    void* object_ = nullptr;
    DialogEventType dialogEventType_;
    int index_;
};

class WebAuthResult : public AuthResult {
    DECLARE_ACE_TYPE(WebAuthResult, AuthResult);
public:
    explicit WebAuthResult(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
        index_ = obj->AddObject(object);
    }
    ~WebAuthResult()
    {
        auto obj = WebObjectEventManager::GetInstance().GetHttpAuthRequestObject();
        obj->DelObject(index_);
    }
    bool Confirm(std::string& userName, std::string& pwd) override;
    bool IsHttpAuthInfoSaved() override;
    void Cancel() override;

private:
    void* object_ = nullptr;
    int index_;
};

class WebCommonDialogImpl : public AceType {
    DECLARE_ACE_TYPE(WebCommonDialogImpl, AceType);
public:
    explicit WebCommonDialogImpl(void* object) : object_(object) {}
    std::string GetUrl() const;
    std::string GetMessage() const;
    std::string GetValue() const;

private:
    void* object_ = nullptr;
};

class PermissionRequestImpl : public WebPermissionRequest {
    DECLARE_ACE_TYPE(PermissionRequestImpl, WebPermissionRequest);
public:
    explicit PermissionRequestImpl(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
        index_ = obj->AddObject(object);
    }
    ~PermissionRequestImpl()
    {
        auto obj = WebObjectEventManager::GetInstance().GetPermissionRequestObject();
        obj->DelObject(index_);
    }

    void Deny() const override;
    std::string GetOrigin() const override;
    std::vector<std::string> GetResources() const override;
    void Grant(std::vector<std::string>& resources) const override;
    void SetOrigin();
    void SetResources();

private:
    void* object_ = nullptr;
    int index_;
    std::string origin_;
    std::vector<std::string> resources_;
};

class WebFileSelectorResult : public FileSelectorResult {
    DECLARE_ACE_TYPE(WebFileSelectorResult, FileSelectorResult);
public:
    explicit WebFileSelectorResult(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
        index_ = obj->AddObject(object);
    }
    ~WebFileSelectorResult()
    {
        auto obj = WebObjectEventManager::GetInstance().GetFileChooserObject();
        obj->DelObject(index_);
    }

    void HandleFileList(std::vector<std::string>& fileList) override;

private:
    void* object_;
    int index_;
};

class WebFileChooserImpl : public AceType {
    DECLARE_ACE_TYPE(WebFileChooserImpl, AceType);
public:
    explicit WebFileChooserImpl(void* object) : object_(object) {}
    std::string GetTitle() const;
    int GetMode() const;
    std::string GetDefaultFileName() const;
    std::vector<std::string> GetAcceptType() const;
    bool IsCapture() const;

private:
    void* object_ = nullptr;
};

class FileSelectorParam : public WebFileSelectorParam {
    DECLARE_ACE_TYPE(FileSelectorParam, AceType);
public:
    FileSelectorParam(
        std::string title, int mode, std::string defaultFileName, std::vector<std::string> acceptType, bool isCapture)
        : title_(title), mode_(mode), defaultFileName_(defaultFileName), acceptType_(acceptType), isCapture_(isCapture)
    {}
    ~FileSelectorParam() = default;

    std::string GetTitle() override
    {
        return title_;
    }
    int GetMode() override
    {
        return mode_;
    }
    std::string GetDefaultFileName() override
    {
        return defaultFileName_;
    }
    std::vector<std::string> GetAcceptType() override
    {
        return acceptType_;
    }
    bool IsCapture() override
    {
        return isCapture_;
    }
    std::vector<std::string> GetMimeType() override
    {
        return mimeType_;
    }

private:
    std::string title_;
    int mode_;
    std::string defaultFileName_;
    std::vector<std::string> acceptType_;
    bool isCapture_;
    std::vector<std::string> mimeType_;
};

class Geolocation : public WebGeolocation {
    DECLARE_ACE_TYPE(Geolocation, WebGeolocation);
public:
    explicit Geolocation(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetGeolocationObject();
        index_ = obj->AddObject(object);
    }
    ~Geolocation()
    {
        auto obj = WebObjectEventManager::GetInstance().GetGeolocationObject();
        obj->DelObject(index_);
    }
    void Invoke(const std::string& origin, const bool& allow, const bool& retain) override;

private:
    void* object_ = nullptr;
    int index_;
};

class WebAuthRequestImpl : public AceType {
    DECLARE_ACE_TYPE(WebAuthRequestImpl, AceType);
public:
    explicit WebAuthRequestImpl(void* object) : object_(object) {}
    std::string GetHost() const;
    std::string GetRealm() const;

private:
    void* object_ = nullptr;
};

class WebGeolocationImpl : public AceType {
    DECLARE_ACE_TYPE(WebGeolocationImpl, AceType);

public:
    explicit WebGeolocationImpl(void* object) : object_(object) {}
    std::string GetOrigin() const;
private:
    void* object_ = nullptr;
};

class WebDownloadResponseImpl : public AceType {
    DECLARE_ACE_TYPE(WebDownloadResponseImpl, AceType);
public:
    explicit WebDownloadResponseImpl(void* object) : object_(object) {}
    std::string GetUrl() const;
    std::string GetMimetype() const;
    long GetContentLength() const;
    std::string GetUserAgent() const;
    std::string GetContentDisposition() const;

private:
    void* object_ = nullptr;
};

class WebRefreshAccessedHistoryImpl : public AceType {
    DECLARE_ACE_TYPE(WebRefreshAccessedHistoryImpl, AceType);
public:
    explicit WebRefreshAccessedHistoryImpl(void* object) : object_(object) {}
    std::string GetUrl() const;
    bool GetIsRefreshed() const;

private:
    void* object_ = nullptr;
};

class WebFullScreenEnterImpl : public AceType {
    DECLARE_ACE_TYPE(WebFullScreenEnterImpl, AceType);
public:
    explicit WebFullScreenEnterImpl(void* object) : object_(object) {}
    int GetWidths() const;
    int GetHeights() const;
private:
    void* object_ = nullptr;
};

class WebFullScreenExitImpl : public AceType {
    DECLARE_ACE_TYPE(WebFullScreenExitImpl, AceType);
public:
    explicit WebFullScreenExitImpl(void* object) : object_(object) {}

private:
    void* object_ = nullptr;
};

class FullScreenExitHandlerImpl : public FullScreenExitHandler {
    DECLARE_ACE_TYPE(FullScreenExitHandlerImpl, FullScreenExitHandler);
public:
    explicit FullScreenExitHandlerImpl(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetFullScreenEnterObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetFullScreenEnterObject failed");
            return;
        }
        index_ = obj->AddObject(object);
    }
    ~FullScreenExitHandlerImpl()
    {
        auto obj = WebObjectEventManager::GetInstance().GetFullScreenEnterObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager get GetFullScreenEnterObject failed");
            return;
        }
        obj->DelObject(index_);
    }
    void ExitFullScreen() override;

private:
    void* object_ = nullptr;
    int index_;
};

class WebResourceErrorImpl : public AceType {
    DECLARE_ACE_TYPE(WebResourceErrorImpl, AceType);
public:
    explicit WebResourceErrorImpl(void* object) : object_(object) {}
    std::string GetErrorInfo() const;
    int GetErrorCode() const;
private:
    void* object_ = nullptr;
};

class SslErrorResultImpl : public SslErrorResult {
    DECLARE_ACE_TYPE(SslErrorResultImpl, SslErrorResult);
public:
    explicit SslErrorResultImpl(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager GetOnSslErrorEventReceiveEventObject failed");
            return;
        }
        index_ = obj->AddObject(object_);
    }

    ~SslErrorResultImpl()
    {
        auto obj = WebObjectEventManager::GetInstance().GetOnSslErrorEventReceiveEventObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager GetOnSslErrorEventReceiveEventObject failed");
            return;
        }
        obj->DelObject(index_);
    }

    void HandleConfirm() override;
    void HandleCancel(bool abortLoading) override;

private:
    void* object_ = nullptr;
    int index_;
};

class SslErrorEventImpl : public AceType {
    DECLARE_ACE_TYPE(SslErrorEventImpl, AceType);
public:
    explicit SslErrorEventImpl(void* object) : object_(object) {}

    int GetError() const;
    std::vector<std::string> GetCertChainData() const;

private:
    void* object_ = nullptr;
};

class SslSelectCertResultImpl : public SslSelectCertResult {
    DECLARE_ACE_TYPE(SslSelectCertResultImpl, SslSelectCertResult);
public:
    explicit SslSelectCertResultImpl(void* object) : object_(object) {}

    void HandleConfirm(const std::string& privateKeyFile, const std::string& certChainFile) override;
    void HandleCancel() override;
    void HandleIgnore() override;

private:
    void* object_ = nullptr;
};

class SslSelectCertEventImpl : public AceType {
    DECLARE_ACE_TYPE(SslSelectCertEventImpl, AceType);
public:
    explicit SslSelectCertEventImpl(void* object) : object_(object) {}

    std::string GetHost();
    int GetPort();
    std::vector<std::string> GetKeyTypes();
    std::vector<std::string> GetIssuers();

private:
    void* object_ = nullptr;
};

class AllSslErrorResultImpl : public AllSslErrorResult {
    DECLARE_ACE_TYPE(AllSslErrorResultImpl, AllSslErrorResult);
public:
    explicit AllSslErrorResultImpl(void* object) : object_(object)
    {
        auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager GetSslErrorEventObject failed");
            return;
        }
        index_ = obj->AddObject(object_);
    }
    ~AllSslErrorResultImpl()
    {
        auto obj = WebObjectEventManager::GetInstance().GetSslErrorEventObject();
        if (!obj) {
            TAG_LOGE(AceLogTag::ACE_WEB, "WebObjectEventManager GetSslErrorEventObject failed");
            return;
        }
        obj->DelObject(index_);
    }

    void HandleConfirm() override;
    void HandleCancel(bool abortLoading) override;

private:
    void* object_ = nullptr;
    int index_;
};

class AllSslErrorEventImpl : public AceType {
    DECLARE_ACE_TYPE(AllSslErrorEventImpl, AceType);
public:
    explicit AllSslErrorEventImpl(void* object) : object_(object) {}

    int GetError() const;
    std::string GetUrl() const;
    std::string GetOriginalUrl() const;
    std::string GetReferrer() const;
    bool IsFatalError() const;
    bool IsMainFrame() const;
    std::vector<std::string> GetCertificateChain() const;

private:
    void* object_ = nullptr;
};
class WebDelegateCross : public WebDelegateInterface,
                         public WebResource {
    DECLARE_ACE_TYPE(WebDelegateCross, WebResource);
public:
    using CreatedCallback = std::function<void()>;
    using ReleasedCallback = std::function<void(bool)>;
    using EventCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    enum class State : char {
        WAITINGFORSIZE,
        CREATING,
        CREATED,
        CREATEFAILED,
        RELEASED,
    };

    WebDelegateCross() = delete;
    ~WebDelegateCross() override;
    WebDelegateCross(const WeakPtr<PipelineBase>& context, ErrorCallback&& onError, const std::string& type)
        : WebResource(type, context, std::move(onError)), context_(context), instanceId_(Container::CurrentId())
    {}

    void CreatePlatformResource(const Size& size, const Offset& position,
        const WeakPtr<NG::PipelineContext>& context) override;
    void SetNGWebPattern(const RefPtr<NG::WebPattern>& webPattern) override
    {
        webPattern_ = webPattern;
    }
    int GetWebId() override;
    void HandleTouchDown(const int32_t& id, const double& x, const double& y, bool from_overlay) override;
    void HandleTouchUp(const int32_t& id, const double& x, const double& y, bool from_overlay) override;
    void HandleTouchMove(const int32_t& id, const double& x, const double& y, bool from_overlay) override;
    void HandleTouchCancel() override;
    void HandleAxisEvent(const double& x, const double& y, const double& deltaX, const double& deltaY);
    bool OnKeyEvent(int32_t keyCode, int32_t keyAction) override;
    void OnMouseEvent(int32_t x, int32_t y, const MouseButton button, const MouseAction action, int count) override;
    void OnFocus() override;
    void OnBlur() override;
    void UpdateLocale() override;
    void SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height) override;
    void OnInactive() override;
    void OnActive() override;
    void ShowWebView() override;
    void HideWebView() override;
    void OnFullScreenEnter(void* object) ;
    void OnFullScreenExit(void* object);
    void OnRefreshAccessedHistory(void* object) ;
    void OnPageStarted(const std::string& param) override;
    void OnPageFinished(const std::string& param) override;
    void OnPageError(const std::string& param) override;
    void OnProgressChanged(const std::string& param) override;
    void OnReceivedTitle(const std::string& param) override;
    void OnPageVisible(const std::string& param) override;
    void OnGeolocationPermissionsHidePrompt() override;

    void UpdateUserAgent(const std::string& userAgent) override;
    void UpdateBackgroundColor(const int backgroundColor) override;
    void UpdateInitialScale(float scale) override;
    void UpdateJavaScriptEnabled(const bool& isJsEnabled) override;
    void UpdateAllowFileAccess(const bool& isFileAccessEnabled) override;
    void UpdateBlockNetworkImage(const bool& onLineImageAccessEnabled) override;
    void UpdateLoadsImagesAutomatically(const bool& isImageAccessEnabled) override;
    void UpdateMixedContentMode(const MixedModeContent& mixedMode) override;
    void UpdateSupportZoom(const bool& isZoomAccessEnabled) override;
    void UpdateDomStorageEnabled(const bool& isDomStorageAccessEnabled) override;
    void UpdateGeolocationEnabled(const bool& isGeolocationAccessEnabled) override;
    void UpdateCacheMode(const WebCacheMode& mode) override;
    void UpdateDarkMode(const WebDarkMode& mode) override;
    void UpdateForceDarkAccess(const bool& access) override;
    void UpdateAudioResumeInterval(const int32_t& resumeInterval) override;
    void UpdateAudioExclusive(const bool& audioExclusive) override;
    void UpdateAudioSessionType(const WebAudioSessionType& audioSessionType) override;
    void UpdateOverviewModeEnabled(const bool& isOverviewModeAccessEnabled) override;
    void UpdateFileFromUrlEnabled(const bool& isFileFromUrlAccessEnabled) override;
    void UpdateDatabaseEnabled(const bool& isDatabaseAccessEnabled) override;
    void UpdateTextZoomRatio(const int32_t& textZoomRatioNum) override;
    void UpdateWebDebuggingAccess(bool isWebDebuggingAccessEnabled) override;
    void UpdatePinchSmoothModeEnabled(bool isPinchSmoothModeEnabled) override;
    void UpdateMediaPlayGestureAccess(bool isNeedGestureAccess) override;
    void UpdateMultiWindowAccess(bool isMultiWindowAccessEnabled) override;
    void UpdateAllowWindowOpenMethod(bool isAllowWindowOpenMethod) override;
    void UpdateWebCursiveFont(const std::string& cursiveFontFamily) override;
    void UpdateWebFantasyFont(const std::string& fantasyFontFamily) override;
    void UpdateWebFixedFont(const std::string& fixedFontFamily) override;
    void UpdateWebSansSerifFont(const std::string& sansSerifFontFamily) override;
    void UpdateWebSerifFont(const std::string& serifFontFamily) override;
    void UpdateWebStandardFont(const std::string& standardFontFamily) override;
    void UpdateDefaultFixedFontSize(int32_t size) override;
    void UpdateDefaultFontSize(int32_t defaultFontSize) override;
    void UpdateMinFontSize(int32_t minFontSize) override;
    void UpdateMinLogicalFontSize(int32_t minLogicalFontSize) override;
    void UpdateBlockNetwork(bool isNetworkBlocked) override;
    void UpdateHorizontalScrollBarAccess(bool isHorizontalScrollBarAccessEnabled) override;
    void UpdateVerticalScrollBarAccess(bool isVerticalScrollBarAccessEnabled) override;
    void UpdateScrollBarColor(const std::string& colorValue) override;
    void LoadUrl() override;
    void SetBackgroundColor(int32_t backgroundColor) override;

    bool LoadDataWithRichText() override;

    void SetBoundsOrResize(const Size& drawSize, const Offset& offset) override;
    void UpdateOptimizeParserBudgetEnabled(const bool enable);
    void MaximizeResize() override;
    void SetScrollLocked(const bool value);
    void SetNestedScrollOptionsExt(NestedScrollOptionsExt nestedOpt);
private:
    void ReleasePlatformResource();
    void CreatePluginResource(const Size& size, const Offset& position, const WeakPtr<NG::PipelineContext>& context);
    void InitWebEvent();
    void RegisterWebEvent();
    void RegisterWebObjectEvent();
    void OnErrorReceive(void* object);
    void OnScroll(void* object);
    void OnScrollWillStart(void* object);
    void OnScrollStart(void* object);
    void OnScrollEnd(void* object);
    void OnScaleChange(void* object);
    void OnHttpErrorReceive(void* object);
    bool OnConsoleMessage(void* object);
    bool OnLoadIntercept(void* object);
    bool OnCommonDialog(void* object, DialogEventType dialogEventType);
    bool OnPermissionRequest(void* object);
    bool OnHttpAuthRequest(void* object);
    void OnDownloadStart(void* object);
    bool OnShowFileChooser(void* object);
    void OnGeolocationPermissionsShowPrompt(void* object);
    void RecordWebEvent(Recorder::EventType eventType, const std::string& param) const;
    void RunJsProxyCallback();
    void RegisterWebInerceptAndOverrideEvent();
    void UnRegisterWebObjectEvent();
    void InitWebStatus();
    void HandleCreateError();
    bool OnOverrideUrlLoading(void* object);
    RefPtr<WebResponse> OnInterceptRequest(void* object);
    bool OnSslErrorEventReceive(void* object);
    bool OnSslErrorEvent(void* object);
    bool OnClientAuthenticationRequest(void* object);
    void EncodeCertificateChainToDer(std::vector<std::string>& certificateChain);

    WeakPtr<NG::WebPattern> webPattern_;
    WeakPtr<PipelineBase> context_;

    State state_ { State::WAITINGFORSIZE };

    Method reloadMethod_;
    Method updateUrlMethod_;
    Method routerBackMethod_;
    Method changePageUrlMethod_;
    Method isPagePathInvalidMethod_;
    Method backwardMethod_;
    Method forwardMethod_;
    Method clearHistoryMethod_;
    Method getHitTestMethod_;
    Method onActiveMethod_;
    Method onInactiveMethod_;
    Method refreshMethod_;
    Method stopLoadingMethod_;
    Method requestFocusMethod_;
    Method accessBackwardMethod_;
    Method accessForwardMethod_;
    Method accessStepMethod_;
    Method setWebViewJavaScriptResultCallBackMethod_;
    Method registerJavaScriptProxyMethod_;
    Method runTypeScriptMethod_;
    Method deleteJavaScriptInterfaceMethod_;
    Method loadUrlMethod_;
    Method loadDataMethod_;
    Method updateAttributeMethod_;
    Method saveCookieSyncMethod_;
    Method setCookieMethod_;
    Method touchDownMethod_;
    Method touchUpMethod_;
    Method touchMoveMethod_;
    Method touchCancelMethod_;
    Method updateLayoutMethod_;
    Method zoomMethod_;
    Method onBlockNetworkUpdateMethod_;
    Method onUpdateMixedContentModeMethod_;
    Method updateBlockNetworkImageMethod_;
    Method UpdateGeolocationEnabledMethod_;
    Method updateDomStorageEnabledMethod_;
    Method updateCacheModeMethod_;
    Method updateLoadsImagesAutomaticallyMethod_;
    Method updateFileAccessMethod_;

    Method updateZoomAccess_;
    Method updateJavaScriptEnabled_;
    Method updateMinFontSize_;
    Method updateHorizontalScrollBarAccess_;
    Method updateVerticalScrollBarAccess_;
    Method updateBackgroundColor_;
    Method updateMediaPlayGestureAccess_;
    Method updateTextZoomRatioMethod_;
    Method setNestedScrollExtMethod_;

    EventCallbackV2 onPageFinishedV2_;
    EventCallbackV2 onPageStartedV2_;
    EventCallbackV2 onProgressChangeV2_;
    EventCallbackV2 onRefreshAccessedHistoryV2_;
    EventCallbackV2 onFullScreenEnterV2_;
    EventCallbackV2 onFullScreenExitV2_;

    int instanceId_ = -1;
};
} // namespace OHOS::Ace

#endif
