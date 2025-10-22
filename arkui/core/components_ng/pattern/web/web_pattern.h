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

#ifndef FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H
#define FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H

#include <optional>
#include <string>
#include <utility>

#include "base/thread/cancelable_callback.h"
#include "base/memory/referenced.h"
#include "base/utils/utils.h"
#include "base/geometry/axis.h"
#include "core/components/dialog/dialog_properties.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/gestures/recognizers/pan_recognizer.h"
#include "core/components_ng/manager/select_overlay/select_overlay_manager.h"
#include "core/components_ng/manager/select_overlay/select_overlay_proxy.h"
#include "core/components_ng/pattern/pattern.h"
#include "core/components_ng/pattern/web/web_accessibility_property.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/components_ng/pattern/web/web_layout_algorithm.h"
#include "core/components_ng/pattern/web/web_paint_property.h"
#include "core/components_ng/pattern/web/web_pattern_property.h"
#include "core/components_ng/pattern/web/web_paint_method.h"
#include "core/components_ng/pattern/web/web_delegate_interface.h"
#include "core/components_ng/property/property.h"
#include "core/components_ng/manager/select_overlay/selection_host.h"
#include "core/components_ng/render/render_surface.h"
#include "core/components_ng/pattern/scrollable/nestable_scroll_container.h"
#include "core/components_ng/pattern/scroll/scroll_pattern.h"
#include "core/components_ng/pattern/scrollable/scrollable_properties.h"
#include "core/components_ng/pattern/swiper/swiper_pattern.h"
#include "core/components_ng/pattern/web/web_delegate_interface.h"

namespace OHOS::Ace::NG {
namespace {
struct MouseClickInfo {
    double x = -1;
    double y = -1;
    TimeStamp start;
};

struct TouchInfo {
    double x = -1;
    double y = -1;
    int32_t id = -1;
};

struct TouchHandleState {
    int32_t id = -1;
    int32_t x = -1;
    int32_t y = -1;
    int32_t edge_height = 0;
};

enum WebOverlayType { INSERT_OVERLAY, SELECTION_OVERLAY, INVALID_OVERLAY };
} // namespace

enum class WebInfoType : int32_t {
    TYPE_MOBILE,
    TYPE_TABLET,
    TYPE_2IN1,
    TYPE_UNKNOWN
};

enum class ScrollDirection {
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UNKNOWN
};

class WebPattern : public NestableScrollContainer, public SelectionHost {
    DECLARE_ACE_TYPE(WebPattern, NestableScrollContainer, SelectionHost);

public:
    using SetWebIdCallback = std::function<void(int32_t)>;
    using SetWebDetachCallback = std::function<void(int32_t)>;
    using SetHapPathCallback = std::function<void(const std::string&)>;
    using JsProxyCallback = std::function<void()>;
    using OnControllerAttachedCallback = std::function<void()>;
    using PermissionClipboardCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using DefaultFileSelectorShowCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using OnOpenAppLinkCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using SetFaviconCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using OnWebNativeMessageConnectCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    using OnWebNativeMessageDisConnectCallback = std::function<void(const std::shared_ptr<BaseEventInfo>&)>;
    WebPattern();
    WebPattern(const std::string& webSrc, const RefPtr<WebController>& webController,
               RenderMode type = RenderMode::ASYNC_RENDER, bool incognitoMode = false,
			   const std::string& sharedRenderProcessToken = "");
    WebPattern(const std::string& webSrc, const SetWebIdCallback& setWebIdCallback,
               RenderMode type = RenderMode::ASYNC_RENDER, bool incognitoMode = false,
			   const std::string& sharedRenderProcessToken = "");

    ~WebPattern() override;

    enum class VkState {
        VK_NONE,
        VK_SHOW,
        VK_HIDE
    };

    std::optional<RenderContext::ContextParam> GetContextParam() const override
    {
        if (renderMode_ == RenderMode::SYNC_RENDER) {
            return RenderContext::ContextParam { RenderContext::ContextType::CANVAS };
        } else {
        return RenderContext::ContextParam { RenderContext::ContextType::SURFACE, "RosenWeb" };
        }
    }

    RefPtr<NodePaintMethod> CreateNodePaintMethod() override;

    bool IsAtomicNode() const override
    {
        return true;
    }

    RefPtr<EventHub> CreateEventHub() override
    {
        return MakeRefPtr<WebEventHub>();
    }

    RefPtr<AccessibilityProperty> CreateAccessibilityProperty() override
    {
        return MakeRefPtr<WebAccessibilityProperty>();
    }

    void OnModifyDone() override;

    Color GetDefaultBackgroundColor();

    void SetWebSrc(const std::string& webSrc)
    {
        if (webSrc_ != webSrc_) {
            OnWebSrcUpdate();
            webSrc_ = webSrc;
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webSrc);
        }
    }

    const std::optional<std::string>& GetWebSrc() const
    {
        return webSrc_;
    }

    void SetPopup(bool popup)
    {
        isPopup_ = popup;
    }

    void SetParentNWebId(int32_t parentNWebId)
    {
        parentNWebId_ = parentNWebId;
    }

    void SetWebData(const std::string& webData)
    {
        if (webData_ != webData) {
            webData_ = webData;
            OnWebDataUpdate();
        }
        if (webPaintProperty_) {
            webPaintProperty_->SetWebPaintData(webData);
        }
    }

    const std::optional<std::string>& GetWebData() const
    {
        return webData_;
    }

    void SetCustomScheme(const std::string& scheme)
    {
        customScheme_ = scheme;
    }

    const std::optional<std::string>& GetCustomScheme() const
    {
        return customScheme_;
    }

    void SetWebController(const RefPtr<WebController>& webController)
    {
        webController_ = webController;
    }

    RefPtr<WebController> GetWebController() const
    {
        return webController_;
    }

    void SetSetWebIdCallback(SetWebIdCallback&& SetIdCallback)
    {
        setWebIdCallback_ = std::move(SetIdCallback);
    }

    SetWebIdCallback GetSetWebIdCallback() const
    {
        return setWebIdCallback_;
    }

    void SetSetWebDetachCallback(SetWebDetachCallback&& SetDetachCallback)
    {
        setWebDetachCallback_ = std::move(SetDetachCallback);
    }

    SetWebDetachCallback GetSetWebDetachCallback() const
    {
        return setWebDetachCallback_;
    }

    void SetRenderMode(RenderMode renderMode)
    {
        renderMode_ = renderMode;
    }

    RenderMode GetRenderMode()
    {
        return renderMode_;
    }

    void SetIncognitoMode(bool incognitoMode)
    {
        incognitoMode_ = incognitoMode;
    }

    bool GetIncognitoMode() const
    {
        return incognitoMode_;
    }

    void SetSharedRenderProcessToken(const std::string& sharedRenderProcessToken)
    {
        sharedRenderProcessToken_ = sharedRenderProcessToken;
    }

    const std::optional<std::string>& GetSharedRenderProcessToken() const
    {
        return sharedRenderProcessToken_;
    }

    void SetOnControllerAttachedCallback(OnControllerAttachedCallback&& callback)
    {
        onControllerAttachedCallback_ = std::move(callback);
    }

    void SetPermissionClipboardCallback(PermissionClipboardCallback&& Callback)
    {
        permissionClipboardCallback_ = std::move(Callback);
    }

    PermissionClipboardCallback GetPermissionClipboardCallback() const
    {
        return permissionClipboardCallback_;
    }

    OnControllerAttachedCallback GetOnControllerAttachedCallback()
    {
        return onControllerAttachedCallback_;
    }

    void SetSetHapPathCallback(SetHapPathCallback&& callback)
    {
        setHapPathCallback_ = std::move(callback);
    }

    SetHapPathCallback GetSetHapPathCallback() const
    {
        return setHapPathCallback_;
    }

    void SetJsProxyCallback(JsProxyCallback&& jsProxyCallback)
    {
        jsProxyCallback_ = std::move(jsProxyCallback);
    }

    void CallJsProxyCallback()
    {
        if (jsProxyCallback_) {
            jsProxyCallback_();
        }
    }

    RefPtr<WebEventHub> GetWebEventHub()
    {
        return GetEventHub<WebEventHub>();
    }

    FocusPattern GetFocusPattern() const override
    {
        return { FocusType::NODE, true };
    }

    RefPtr<PaintProperty> CreatePaintProperty() override
    {
        if (!webPaintProperty_) {
            webPaintProperty_ = MakeRefPtr<WebPaintProperty>();
            if (!webPaintProperty_) {
            }
        }
        return webPaintProperty_;
    }

    RefPtr<LayoutAlgorithm> CreateLayoutAlgorithm() override
    {
        return MakeRefPtr<WebLayoutAlgorithm>();
    }

    bool BetweenSelectedPosition(const Offset& globalOffset) override
    {
        return false;
    }

    int32_t GetDragRecordSize() override
    {
        return 1;
    }

    void SetNestedScroll(const NestedScrollOptions& nestedOpt);

    void SetNestedScrollExt(const NestedScrollOptionsExt& nestedScroll);

    /**
     *  End of NestableScrollContainer implementations
     */

    ACE_DEFINE_PROPERTY_GROUP(WebProperty, WebPatternProperty);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, JsEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MediaPlayGestureAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OnLineImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DomStorageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ImageAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MixedMode, MixedModeContent);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ZoomAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ZoomControlAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, GeolocationAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, UserAgent, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CacheMode, WebCacheMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverviewModeAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, FileFromUrlAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DatabaseAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextZoomRatio, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebDebuggingAccessEnabledAndPort,
        WebPatternProperty::WebDebuggingConfigType);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BackgroundColor, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, InitialScale, float);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, PinchSmoothModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MultiWindowAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AllowWindowOpenMethod, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebCursiveFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFantasyFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebFixedFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSansSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebSerifFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebStandardFont, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFixedFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DefaultTextEncodingFormat, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MinLogicalFontSize, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BlockNetwork, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, DarkMode, WebDarkMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ForceDarkAccess, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioResumeInterval, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioExclusive, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, AudioSessionType, WebAudioSessionType);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, HorizontalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, VerticalScrollBarAccessEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ScrollBarColor, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverScrollMode, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BlurOnKeyboardHideMode, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, TextAutosizing, bool);
    using NativeVideoPlayerConfigType = std::tuple<bool, bool>;
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeVideoPlayerConfig, NativeVideoPlayerConfigType);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, SelectionMenuOptions, WebMenuOptionsParam);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, MetaViewport, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CopyOptionMode, int32_t);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedModeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, IntrinsicSizeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, CssDisplayChangeEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedRuleTag, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, NativeEmbedRuleType, std::string);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OverlayScrollbarEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, KeyboardAvoidMode, WebKeyboardAvoidMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, EnabledHapticFeedback, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, OptimizeParserBudgetEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, WebMediaAVSessionEnabled, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, EnableDataDetector, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BypassVsyncCondition, WebBypassVsyncCondition);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, EnableFollowSystemFontWeight, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BlankScreenDetectionConfig, BlankScreenDetectionConfig);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, GestureFocusMode, GestureFocusMode);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, RotateRenderEffect, WebRotateEffect);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, ForceEnableZoom, bool);
    ACE_DEFINE_PROPERTY_FUNC_WITH_GROUP(WebProperty, BackToTop, bool);
    void RequestFullScreen();
    void ExitFullScreen();
    bool IsFullScreen() const
    {
        return isFullScreen_;
    }
    void UpdateLocale();
    void SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height);
    void OnCompleteSwapWithNewSize();
    void OnResizeNotWork();
    bool OnBackPressed() const;
    bool OnBackPressedForFullScreen() const;
    void SetFullScreenExitHandler(const std::shared_ptr<FullScreenEnterEvent>& fullScreenExitHandler);
    bool NotifyStartDragTask(bool isDelayed = false);
    void OnContextMenuShow(const std::shared_ptr<BaseEventInfo>& info, bool isRichtext = true, bool result = false);
    void UpdateJavaScriptOnDocumentStart();
    void JavaScriptOnDocumentStart(const ScriptItems& scriptItems);
    void JavaScriptOnDocumentEnd(const ScriptItems& scriptItems);
    void JavaScriptOnDocumentStartByOrder(const ScriptItems& scriptItems,
        const ScriptItemsByOrder& scriptItemsByOrder);
    void JavaScriptOnDocumentEndByOrder(const ScriptItems& scriptItems,
        const ScriptItemsByOrder& scriptItemsByOrder);
    bool IsImageDrag();
    Offset GetDragOffset() const;
    void RemovePreviewMenuNode();
    void UpdateImagePreviewParam();
    void SetLayoutMode(WebLayoutMode mode)
    {
        layoutMode_ = mode;
    }
    WebLayoutMode GetLayoutMode() const
    {
        return layoutMode_;
    }
    void OnRootLayerChanged(int width, int height);
    int GetRootLayerWidth() const
    {
        return rootLayerWidth_;
    }
    int GetRootLayerHeight() const
    {
        return rootLayerHeight_;
    }
    void RichTextInit()
    {
        richTextInit_ = true;
    }
    bool GetRichTextInit() const
    {
        return richTextInit_;
    }
    Size GetDrawSize() const
    {
        return drawSize_;
    }
    bool IsVirtualKeyBoardShow() const
    {
        // cross platform is not support now;
        return false;
    }

    SizeF GetDragPixelMapSize() const;
    bool Backward();
    void OnBlankScreenDetectionConfigUpdate(const BlankScreenDetectionConfig &config);
    void OnSelectionMenuOptionsUpdate(const WebMenuOptionsParam& webMenuOption);
    void UpdateEditMenuOptions(const NG::OnCreateMenuCallback&& onCreateMenuCallback,
        const NG::OnMenuItemClickCallback&& onMenuItemClick, const NG::OnPrepareMenuCallback&& onPrepareMenuCallback);
    void UpdateDataDetectorConfig(const TextDetectConfig& config);
    void UpdateEnableSelectDataDetector(bool isEnabled);
    void UpdateSelectedDataDetectorConfig(const TextDetectConfig& config);
    WebInfoType GetWebInfoType();
    void SetUpdateInstanceIdCallback(std::function<void(int32_t)> &&callabck);

    void SetDefaultFileSelectorShowCallback(DefaultFileSelectorShowCallback&& Callback)
    {
        defaultFileSelectorShowCallback_ = std::move(Callback);
    }

    DefaultFileSelectorShowCallback GetDefaultFileSelectorShowCallback()
    {
        return defaultFileSelectorShowCallback_;
    }

    void SetOnOpenAppLinkCallback(OnOpenAppLinkCallback&& callback)
    {
        onOpenAppLinkCallback_ = std::move(callback);
    }

    OnOpenAppLinkCallback GetOnOpenAppLinkCallback() const
    {
        return onOpenAppLinkCallback_;
    }
    
    void SetWebNativeMessageConnectCallback(OnWebNativeMessageConnectCallback &&callback)
    {
        onWebNativeMessageConnectCallback_ = std::move(callback);
    }

    OnWebNativeMessageConnectCallback GetWebNativeMessageConnectCallback() const
    {
        return onWebNativeMessageConnectCallback_;
    }

    void SetWebNativeMessageDisConnectCallback(OnWebNativeMessageDisConnectCallback &&callback)
    {
        onWebNativeMessageDisConnectCallback_ = std::move(callback);
    }

    OnWebNativeMessageDisConnectCallback GetWebNativeMessageDisConnectCallback() const
    {
        return onWebNativeMessageDisConnectCallback_;
    }

    void SetFaviconFunction(OnOpenAppLinkCallback&& callback)
    {
        setFaviconCallback_ = std::move(callback);
    }

    SetFaviconCallback GetSetFaviconFunction() const
    {
        return setFaviconCallback_;
    }

    bool IsPreviewImageNodeExist() const
    {
        // cross platform is not support now;
        return false;
    }

    void SetNewDragStyle(bool isNewDragStyle) {}

    bool IsNewDragStyle() const
    {
        return false;
    }

    bool IsDragging() const
    {
        return false;
    }

    void SetPreviewSelectionMenu(const std::shared_ptr<WebPreviewSelectionMenuParam>& param);

    std::shared_ptr<WebPreviewSelectionMenuParam> GetPreviewSelectionMenuParams(
        const WebElementType& type, const ResponseType& responseType);

    bool IsPreviewMenuNotNeedShowPreview();

    bool RunJavascriptAsync(const std::string& jsCode, std::function<void(const std::string&)>&& callback);

    void JavaScriptOnHeadReadyByOrder(const ScriptItems& scriptItems,
        const ScriptItemsByOrder& scriptItemsByOrder);

    void OnWebMediaAVSessionEnabledUpdate(bool enable);
    void SetDefaultBackgroundColor();
    void OnBackToTopUpdate(bool isBackToTop);

    NestedScrollOptionsExt GetNestedScrollExt() const
    {
        return nestedScroll_;
    }
    Axis GetAxis() const override
    {
        return axis_;
    }
    void OnScroll(float currentX, float currentY, float cWidth, float cHeight, float fWidth, float fHeight);
    void OnScrollWillStart(float x, float y);
    void OnScrollStart(float x, float y);
    bool OnNestedScroll(float x, float y, float xVelocity, float yVelocity, bool isAvailable);
    bool IsRtl();
    ScrollResult HandleScroll(float offset, int32_t source, NestedState state, float velocity = 0.f) override;
    ScrollResult HandleScroll(RefPtr<NestableScrollContainer> parent, float offset, int32_t source, NestedState state);
    bool HandleScrollVelocity(float velocity, const RefPtr<NestableScrollContainer>& child = nullptr) override;
    bool HandleScrollVelocity(const RefPtr<NestableScrollContainer>& parent, float velocity);
    void OnScrollStartRecursive(WeakPtr<NestableScrollContainer> child, float position, float velocity = 0.f) override;
    void OnScrollStartRecursive(float position);
    void OnScrollEndRecursive(const std::optional<float>& velocity) override;
    void GetParentAxis();
    RefPtr<NestableScrollContainer> SearchParent() override;
    RefPtr<NestableScrollContainer> SearchParent(Axis scrollAxis);
    void OnParentScrollDragEndRecursive(RefPtr<NestableScrollContainer> parent);
private:
    void RegistVirtualKeyBoardListener();
    bool ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard);
    void UpdateWebLayoutSize(int32_t width, int32_t height);
    bool OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config) override;

    void OnAttachToFrameNode() override;
    void OnDetachFromFrameNode(FrameNode* frameNode) override;
    void OnWindowShow() override;
    void OnWindowHide() override;
    void OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) override;
    void OnInActive() override;
    void OnActive() override;
    void OnVisibleChange(bool isVisible) override;
    void OnAreaChangedInner() override;

    void OnWebSrcUpdate();
    void OnWebDataUpdate();
    void OnJsEnabledUpdate(bool value);
    void OnMediaPlayGestureAccessUpdate(bool value);
    void OnFileAccessEnabledUpdate(bool value);
    void OnOnLineImageAccessEnabledUpdate(bool value);
    void OnDomStorageAccessEnabledUpdate(bool value);
    void OnImageAccessEnabledUpdate(bool value);
    void OnMixedModeUpdate(MixedModeContent value);
    void OnZoomAccessEnabledUpdate(bool value);
    void OnZoomControlAccessUpdate(bool zoomControlAccess);
    void OnGeolocationAccessEnabledUpdate(bool value);
    void OnUserAgentUpdate(const std::string& value);
    void OnCacheModeUpdate(WebCacheMode value);
    void OnOverviewModeAccessEnabledUpdate(bool value);
    void OnFileFromUrlAccessEnabledUpdate(bool value);
    void OnDatabaseAccessEnabledUpdate(bool value);
    void OnTextZoomRatioUpdate(int32_t value);
    void OnWebDebuggingAccessEnabledAndPortUpdate(
        const WebPatternProperty::WebDebuggingConfigType& enabled_and_port);
    void OnPinchSmoothModeEnabledUpdate(bool value);
    void OnBackgroundColorUpdate(int32_t value);
    void OnInitialScaleUpdate(float value);
    void OnMultiWindowAccessEnabledUpdate(bool value);
    void OnAllowWindowOpenMethodUpdate(bool value);
    void OnWebCursiveFontUpdate(const std::string& value);
    void OnWebFantasyFontUpdate(const std::string& value);
    void OnWebFixedFontUpdate(const std::string& value);
    void OnWebSerifFontUpdate(const std::string& value);
    void OnWebSansSerifFontUpdate(const std::string& value);
    void OnWebStandardFontUpdate(const std::string& value);
    void OnDefaultFixedFontSizeUpdate(int32_t value);
    void OnDefaultFontSizeUpdate(int32_t value);
    void OnDefaultTextEncodingFormatUpdate(const std::string& value);
    void OnMinFontSizeUpdate(int32_t value);
    void OnMinLogicalFontSizeUpdate(int32_t value);
    void OnBlockNetworkUpdate(bool value);
    void OnDarkModeUpdate(WebDarkMode mode);
    void OnForceDarkAccessUpdate(bool access);
    void OnAudioResumeIntervalUpdate(int32_t resumeInterval);
    void OnAudioExclusiveUpdate(bool audioExclusive);
    void OnAudioSessionTypeUpdate(WebAudioSessionType value);
    void OnHorizontalScrollBarAccessEnabledUpdate(bool value);
    void OnVerticalScrollBarAccessEnabledUpdate(bool value);
    void OnScrollBarColorUpdate(const std::string& value);
    void OnOverScrollModeUpdate(const int32_t value);
    void OnBlurOnKeyboardHideModeUpdate(const int32_t mode);
    void OnTextAutosizingUpdate(bool isTextAutosizing);
    void OnNativeVideoPlayerConfigUpdate(const std::tuple<bool, bool>& config);
    void OnMetaViewportUpdate(bool value);
    void OnOverlayScrollbarEnabledUpdate(bool value);
    void OnKeyboardAvoidModeUpdate(const WebKeyboardAvoidMode& mode);
    void OnEnabledHapticFeedbackUpdate(bool enable);
    void OnOptimizeParserBudgetEnabledUpdate(bool value);
    void OnEnableDataDetectorUpdate(bool enable);
    void OnEnableFollowSystemFontWeightUpdate(bool value);
    void OnGestureFocusModeUpdate(GestureFocusMode mode);
    void OnRotateRenderEffectUpdate(WebRotateEffect effect);
    void WebRotateRenderEffect(WindowSizeChangeReason type);
    void OnForceEnableZoomUpdate(bool isEnabled);

    void InitEvent();
    void InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub);
    void InitMouseEvent(const RefPtr<InputEventHub>& inputHub);
    void InitHoverEvent(const RefPtr<InputEventHub>& inputHub);
    void InitPanEvent(const RefPtr<GestureEventHub>& gestureHub);
    void HandleMouseEvent(MouseInfo& info);
    void WebOnMouseEvent(const MouseInfo& info);
    bool HandleDoubleClickEvent(const MouseInfo& info);
    void SendDoubleClickEvent(const MouseClickInfo& info);
    void InitFocusEvent(const RefPtr<FocusHub>& focusHub);
    void HandleFocusEvent();
    void HandleBlurEvent(const BlurReason& blurReason);
    bool HandleKeyEvent(const KeyEvent& keyEvent);
    bool WebOnKeyEvent(const KeyEvent& keyEvent);
    void WebRequestFocus();
    void ResetDragAction();
    void UpdateRelativeOffset();
    void InitSlideUpdateListener();
    void CalculateHorizontalDrawRect();
    void CalculateVerticalDrawRect();
    void UpdateSlideOffset(bool isNeedReset = false);
    void OnNativeEmbedModeEnabledUpdate(bool value) {};
    void OnIntrinsicSizeEnabledUpdate(bool value);
    void OnCssDisplayChangeEnabledUpdate(bool value);
    void OnNativeEmbedRuleTagUpdate(const std::string& tag);
    void OnNativeEmbedRuleTypeUpdate(const std::string& type);
    void OnCopyOptionModeUpdate(const int32_t value);
    void OnBypassVsyncConditionUpdate(WebBypassVsyncCondition condition);
    int onDragMoveCnt = 0;
    std::chrono::time_point<std::chrono::system_clock> firstMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> preMoveInTime;
    std::chrono::time_point<std::chrono::system_clock> curMoveInTime;
    CancelableCallback<void()> timer_;
    int32_t duration_ = 100; // 100: 100ms
    void DoRepeat();
    void StartRepeatTimer();

    void HandleTouchDown(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchUp(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchMove(const TouchEventInfo& info, bool fromOverlay);

    void HandleTouchCancel(const TouchEventInfo& info);

    std::optional<OffsetF> GetCoordinatePoint();

    struct TouchInfo {
        float x = -1.0f;
        float y = -1.0f;
        int32_t id = -1;
    };
    static bool ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos);
    void InitEnhanceSurfaceFlag();
    void UpdateBackgroundColorRightNow(int32_t color);
    void UpdateContentOffset(const RefPtr<LayoutWrapper>& dirty);
    void PostTaskToUI(const std::function<void()>&& task) const;
    void StartVibraFeedback(const std::string& vibratorType);

    std::optional<std::string> webSrc_;
    std::optional<std::string> webData_;
    std::optional<std::string> customScheme_;
    RefPtr<WebController> webController_;
    SetWebIdCallback setWebIdCallback_ = nullptr;
    SetWebDetachCallback setWebDetachCallback_ = nullptr;
    RenderMode renderMode_;
    bool incognitoMode_ = false;
    SetHapPathCallback setHapPathCallback_ = nullptr;
    JsProxyCallback jsProxyCallback_ = nullptr;
    OnControllerAttachedCallback onControllerAttachedCallback_ = nullptr;
    PermissionClipboardCallback permissionClipboardCallback_ = nullptr;
    OnOpenAppLinkCallback onOpenAppLinkCallback_ = nullptr;
    SetFaviconCallback setFaviconCallback_ = nullptr;
    DefaultFileSelectorShowCallback defaultFileSelectorShowCallback_ = nullptr;
    OnWebNativeMessageConnectCallback onWebNativeMessageConnectCallback_ = nullptr;
    OnWebNativeMessageDisConnectCallback onWebNativeMessageDisConnectCallback_ = nullptr;
    RefPtr<TouchEventImpl> touchEvent_;
    RefPtr<InputEvent> mouseEvent_;
    RefPtr<InputEvent> hoverEvent_;
    RefPtr<PanEvent> panEvent_ = nullptr;
    RefPtr<WebPaintProperty> webPaintProperty_ = nullptr;
    RefPtr<DragEvent> dragEvent_;
    bool isUrlLoaded_ = false;
    std::queue<MouseClickInfo> doubleClickQueue_;
    bool isFullScreen_ = false;
    std::shared_ptr<FullScreenEnterEvent> fullScreenExitHandler_ = nullptr;
    bool needOnFocus_ = false;
    Size drawSize_;
    Size drawSizeCache_;
    bool needUpdateWeb_ = true;
    bool isFocus_ = false;
    VkState isVirtualKeyBoardShow_ { VkState::VK_NONE };
    bool isDragging_ = false;
    bool isW3cDragEvent_ = false;
    bool isWindowShow_ = true;
    bool isActive_ = true;
    bool isEnhanceSurface_ = false;
    bool isAllowWindowOpenMethod_ = false;
    OffsetF webOffset_;
    bool isPopup_ = false;
    int32_t parentNWebId_ = -1;
    bool isInWindowDrag_ = false;
    bool isWaiting_ = false;
    bool isDisableDrag_ = false;
    bool isMouseEvent_ = false;
    bool isVisible_ = true;
    bool isVisibleActiveEnable_ = true;
    bool isMemoryLevelEnable_ = true;
    bool isParentHasScroll_ = false;
    OffsetF relativeOffsetOfScroll_;
    std::function<void(int32_t)> updateInstanceIdCallback_;
    RefPtr<WebDelegateInterface> delegate_ = nullptr;
    std::optional<std::string> sharedRenderProcessToken_;

    bool selectPopupMenuShowing_ = false;
    WebLayoutMode layoutMode_ = WebLayoutMode::NONE;
    int32_t rootLayerWidth_ = 0;
    int32_t rootLayerHeight_ = 0;
    bool richTextInit_ = false;
    ACE_DISALLOW_COPY_AND_MOVE(WebPattern);
    bool needSetDefaultBackgroundColor_ = false;

    NestedScrollOptionsExt nestedScroll_ = {
        .scrollUp = NestedScrollMode::SELF_ONLY,
        .scrollDown = NestedScrollMode::SELF_ONLY,
        .scrollLeft = NestedScrollMode::SELF_ONLY,
        .scrollRight = NestedScrollMode::SELF_ONLY,
    };
    Axis axis_ = Axis::FREE;
    bool isScrollStarted_ = false;
    bool isParentReachEdge_ = false;
    bool isSelfReachEdge_ = false;
    bool isParentReverseReachEdge_ = false;
    bool isTouchpadSliding_ = false;
    bool isStartTouch_ = true;
    WeakPtr<NestableScrollContainer> dragEndRecursiveParent_;
    Axis expectedScrollAxis_ = Axis::FREE;
    std::unordered_map<Axis, WeakPtr<NestableScrollContainer>> parentsMap_;
    float prevX_ = 0.0f;
    float prevY_ = 0.0f;
    float averageDistanceX_ = 0.0f;
    float averageDistanceY_ = 0.0f;
    bool isDragEnd_ = false;
    TouchInfo prevOffset_;

    bool FilterScrollEventHandleOffset(float offset);
    bool FilterScrollEventHandleVelocity(float velocity);
    bool CheckParentScroll(float directValue, const NestedScrollMode &scrollMode);
    std::string GetStringToMode(const NestedScrollMode& mode);
    struct ScrollDirectionContext {
        void Set(bool isMinValueX, bool isMaxValueX, bool isMinValueY, bool isMaxValueY, float cX, float cY, float pX,
            float pY, float cWidth, float cHeight, float fWidth, float fHeight)
        {
            this->isMinX = isMinValueX;
            this->isMaxX = isMaxValueX;
            this->isMinY = isMinValueY;
            this->isMaxY = isMaxValueY;
            this->prevX = pX;
            this->prevY = pY;
            this->currentX = cX;
            this->currentY = cY;
            this->currentWidth = cWidth;
            this->currentHeight = cHeight;
            this->frameWidth = fWidth;
            this->frameHeight = fHeight;
        }
        bool isMinX { false };
        bool isMaxX { false };
        bool isMinY { false };
        bool isMaxY { false };
        float currentX { 0.0f };
        float currentY { 0.0f };
        float prevX { 0.0f };
        float prevY { 0.0f };
        float currentWidth { 0.0f };
        float currentHeight { 0.0f };
        float frameWidth { 0.0f };
        float frameHeight { 0.0f };
    };

    void InitPanGesture(const RefPtr<GestureEventHub>&);
    bool webPanActive_ = false;
    RefPtr<PanGesture> webPanGesture_;
    bool webPanEnable_ = true;

    ScrollDirection direction_;
    NestedScrollMode mode_ = NestedScrollMode::SELF_ONLY;
    ScrollDirectionContext directionCtx_;
    bool isFirstFlingScrollVelocity_ = false;

    ScrollDirection GetScrollDirection(float dx, float dy);
    bool IsScrollReachEdge(const ScrollDirection& direction, bool isMinX, bool isMaxX, bool isMinY, bool isMaxY);

    void CalcScrollParamsAndBorder(
        const TouchInfo& touchPoint, float& dx, float& dy, float& scrollOffset, bool& isAtBorder);

    void HandleParentFirstScroll(
        float dx, float dy, float scrollOffset, bool isAtBorder, const TouchInfo& touchPoint, bool fromOverlay);

    void HandleSelfFirstScroll(
        float scrollOffset, bool isAtBorder, const TouchInfo& touchPoint, bool fromOverlay, float dy);

    RefPtr<SheetPresentationPattern> SearchSheetParent(RefPtr<NestableScrollContainer> pattern,
        Axis scrollAxis = Axis::NONE);
    RefPtr<OHOS::Ace::NG::SwiperPattern> SearchSwiperParent(RefPtr<NestableScrollContainer> pattern,
        Axis scrollAxis = Axis::NONE);
    bool ParentHostTagIsRefresh(RefPtr<NestableScrollContainer> container);

    bool IsAtBorder();
    void SetNestedScrollOptionsExt(NestedScrollOptionsExt nestedOpt);
    void SetScrollLocked(bool value);
    void SetOnActionStartId();
    void SetOnActionUpdateId();
    void SetOnActionEndId();
    void SetOnActionCancelId();
    void SetDirectionMode();
    void HandleTouchMoveIOS(TouchInfo &touchPoint, bool fromOverlay);
    void HandleTouchMoveAndroid(TouchInfo &touchPoint, bool fromOverlay);
    bool GetScrollBoundary(float currentX, float currentY, float fWidth, float fHeight, float cWidth, float cHeight);
};
} // namespace OHOS::Ace::NG

#endif // FOUNDATION_ACE_FRAMEWORKS_CORE_COMPONENTS_NG_PATTERNS_WEB_CORS_WEB_PATTERN_H
