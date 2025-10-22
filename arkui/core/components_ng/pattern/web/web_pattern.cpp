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

#include "web_pattern.h"

#include <securec.h>

#include "base/geometry/ng/offset_t.h"
#include "base/mousestyle/mouse_style.h"
#include "base/utils/date_util.h"
#include "base/utils/linear_map.h"
#include "base/utils/utils.h"
#include "core/components/dialog/dialog_theme.h"
#include "core/components/picker/picker_data.h"
#include "core/components/text_overlay/text_overlay_theme.h"
#include "core/components/web/web_property.h"
#include "core/components_ng/base/view_stack_processor.h"
#include "core/components_ng/pattern/menu/menu_view.h"
#include "core/components_ng/pattern/overlay/overlay_manager.h"
#include "core/components_ng/pattern/web/web_event_hub.h"
#include "core/event/key_event.h"
#include "core/event/touch_event.h"
#include "core/pipeline_ng/pipeline_context.h"
#include "frameworks/base/utils/system_properties.h"
#include "core/components_ng/pattern/refresh/refresh_pattern.h"
#include "web_delegate_cross.h"

namespace OHOS::Ace::NG {

constexpr int32_t SINGLE_CLICK_NUM = 1;
constexpr int32_t DOUBLE_CLICK_NUM = 2;
constexpr double DEFAULT_DBCLICK_INTERVAL = 0.5;
constexpr double DEFAULT_DBCLICK_OFFSET = 2.0;
constexpr double DEFAULT_AXIS_RATIO = -0.06;
constexpr uint32_t DEFAULT_WEB_DRAW_HEIGHT = 4000;
const std::string PATTERN_TYPE_WEB = "WEBPATTERN";
constexpr int32_t ASYNC_SURFACE_QUEUE_SIZE = 3;
constexpr int32_t SYNC_SURFACE_QUEUE_SIZE = 8;
constexpr int32_t SIZE_GAP = 2;
// web feature params
const std::string VISIBLE_ACTIVE_ENABLE = "persist.web.visible_active_enable";
const std::string MEMORY_LEVEL_ENABEL = "persist.web.memory_level_enable";
const std::vector<int32_t> DEFAULT_HEIGHT_GEAR {7998, 7999, 8001, 8002, 8003};
const std::vector<int32_t> DEFAULT_ORIGN_GEAR {0, 2000, 4000, 6000, 8000};
const float MIN_SCROLL_OFFSET = 0.1f;
const std::string TAG_REFRESH = "Refresh";

#if defined(IOS_PLATFORM)
extern "C" void SetIsPointInsideWebForceResult(bool enable, bool result);
#endif

WebPattern::WebPattern() = default;

WebPattern::WebPattern(const std::string& webSrc, const RefPtr<WebController>& webController, RenderMode renderMode,
    bool incognitoMode, const std::string& sharedRenderProcessToken)
    : webSrc_(std::move(webSrc)), webController_(webController), renderMode_(renderMode), incognitoMode_(incognitoMode),
      sharedRenderProcessToken_(sharedRenderProcessToken)
{}

WebPattern::WebPattern(const std::string& webSrc, const SetWebIdCallback& setWebIdCallback, RenderMode renderMode,
    bool incognitoMode, const std::string& sharedRenderProcessToken)
    : webSrc_(std::move(webSrc)), setWebIdCallback_(setWebIdCallback), renderMode_(renderMode),
      incognitoMode_(incognitoMode), sharedRenderProcessToken_(sharedRenderProcessToken)
{}

WebPattern::~WebPattern()
{
    if (isActive_) {
        OnInActive();
    }
}

void WebPattern::OnAttachToFrameNode()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    host->GetRenderContext()->SetClipToFrame(true);
    host->GetRenderContext()->UpdateBackgroundColor(Color::WHITE);
    host->GetLayoutProperty()->UpdateMeasureType(MeasureType::MATCH_PARENT);
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    pipeline->AddNodesToNotifyMemoryLevel(host->GetId());
    auto OnAreaChangedCallBack = [weak = WeakClaim(this)](float x, float y, float w, float h) mutable {
        auto webPattern = weak.Upgrade();
        CHECK_NULL_VOID(webPattern);
        auto offset = Offset(webPattern->GetCoordinatePoint()->GetX(), webPattern->GetCoordinatePoint()->GetY());
        auto host = webPattern->GetHost();
        CHECK_NULL_VOID(host);
        auto renderContext = host->GetRenderContext();
        CHECK_NULL_VOID(renderContext);
        renderContext->SetContentRectToFrame(RectF(offset.GetX(), offset.GetY(),
            webPattern->drawSize_.Width() - SIZE_GAP, webPattern->drawSize_.Height() - SIZE_GAP));
    };
    host->GetRenderContext()->SetSurfaceChangedCallBack(OnAreaChangedCallBack);
}

void WebPattern::OnDetachFromFrameNode(FrameNode* frameNode)
{
#if defined(IOS_PLATFORM)
    SetIsPointInsideWebForceResult(false, false);
#endif
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    delegate_->OnBlur();

    auto id = frameNode->GetId();
    auto pipeline = AceType::DynamicCast<PipelineContext>(PipelineBase::GetCurrentContext());
    CHECK_NULL_VOID(pipeline);
    pipeline->RemoveWindowStateChangedCallback(id);
    pipeline->RemoveWindowSizeChangeCallback(id);
    pipeline->RemoveNodesToNotifyMemoryLevel(id);
}

void WebPattern::InitEvent()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);

    auto gestureHub = eventHub->GetOrCreateGestureEventHub();
    CHECK_NULL_VOID(gestureHub);

    InitTouchEvent(gestureHub);
#if defined(ANDROID_PLATFORM)
    InitPanGesture(gestureHub);
#endif

    auto inputHub = eventHub->GetOrCreateInputEventHub();
    CHECK_NULL_VOID(inputHub);
    InitMouseEvent(inputHub);
    InitHoverEvent(inputHub);

    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);
    InitFocusEvent(focusHub);

    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    auto langTask = [weak = AceType::WeakClaim(this)]() {
        auto WebPattern = weak.Upgrade();
        CHECK_NULL_VOID(WebPattern);
        WebPattern->UpdateLocale();
    };
    context->SetConfigChangedCallback(GetHost()->GetId(), std::move(langTask));
}

void WebPattern::InitTouchEvent(const RefPtr<GestureEventHub>& gestureHub)
{
    if (touchEvent_) {
        return;
    }

    auto touchTask = [weak = WeakClaim(this)](const TouchEventInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (info.GetChangedTouches().empty()) {
            return;
        }

        // only handle touch event
        if (info.GetSourceDevice() != SourceType::TOUCH) {
            return;
        }

        pattern->isMouseEvent_ = false;
        const auto& changedPoint = info.GetChangedTouches().front();
        if (changedPoint.GetTouchType() == TouchType::DOWN) {
            pattern->HandleTouchDown(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::MOVE) {
            pattern->HandleTouchMove(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::UP) {
            pattern->HandleTouchUp(info, false);
            return;
        }
        if (changedPoint.GetTouchType() == TouchType::CANCEL) {
            pattern->HandleTouchCancel(info);
            return;
        }
    };
    touchEvent_ = MakeRefPtr<TouchEventImpl>(std::move(touchTask));
    gestureHub->AddTouchEvent(touchEvent_);
}

void WebPattern::InitMouseEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (mouseEvent_) {
        return;
    }

    auto mouseTask = [weak = WeakClaim(this)](MouseInfo& info) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleMouseEvent(info);
    };

    mouseEvent_ = MakeRefPtr<InputEvent>(std::move(mouseTask));
    inputHub->AddOnMouseEvent(mouseEvent_);
}

void WebPattern::InitHoverEvent(const RefPtr<InputEventHub>& inputHub)
{
    if (hoverEvent_) {
        return;
    }

    auto hoverTask = [weak = WeakClaim(this)](bool isHover) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        MouseInfo info;
        info.SetAction(isHover ? MouseAction::HOVER : MouseAction::HOVER_EXIT);
        pattern->WebOnMouseEvent(info);
    };

    hoverEvent_ = MakeRefPtr<InputEvent>(std::move(hoverTask));
    inputHub->AddOnHoverEvent(hoverEvent_);
}

void WebPattern::InitPanGesture(const RefPtr<GestureEventHub>& gestureHub)
{
    if (!gestureHub || !webPanEnable_) {
        return;
    }
    if (webPanGesture_) {
        if (gestureHub->WillRecreateGesture()) {
            gestureHub->AddGesture(webPanGesture_);
        }
        return;
    }
    auto option = MakeRefPtr<PanGestureOption>();
    PanDirection panDirection;
    panDirection.type = PanDirection::ALL;
    option->SetDirection(panDirection);
    option->SetDistance(0.0);
    option->SetFingers(1);
    webPanGesture_ = MakeRefPtr<PanGesture>(option);
    CHECK_NULL_VOID(webPanGesture_);
    webPanGesture_->SetPriority(GesturePriority::High);

    SetOnActionStartId();
    SetOnActionUpdateId();
    SetOnActionEndId();
    SetOnActionCancelId();
    gestureHub->AddGesture(webPanGesture_);
}

void WebPattern::SetOnActionStartId()
{
    CHECK_NULL_VOID(webPanGesture_);
    webPanGesture_->SetOnActionStartId([weak = WeakClaim(this)](const GestureEvent& event) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->webPanActive_ = true;
        auto delta = event.GetDelta();
        pattern->expectedScrollAxis_ =
            (std::abs(delta.GetX()) > std::abs(delta.GetY())) ? Axis::HORIZONTAL : Axis::VERTICAL;
    });
}

void WebPattern::SetOnActionUpdateId()
{
    CHECK_NULL_VOID(webPanGesture_);
    webPanGesture_->SetOnActionUpdateId([weak = WeakClaim(this)](const GestureEvent& event) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        if (!pattern->webPanActive_) {
            return;
        }
        pattern->isFirstFlingScrollVelocity_ = true;
        pattern->GetParentAxis();
    });
}

void WebPattern::SetOnActionEndId()
{
    CHECK_NULL_VOID(webPanGesture_);
    webPanGesture_->SetOnActionEndId([weak = WeakClaim(this)](const GestureEvent& event) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        float directVelocity = (pattern->expectedScrollAxis_ == Axis::HORIZONTAL)
                                 ? event.GetVelocity().GetVelocityX()
                                 : event.GetVelocity().GetVelocityY();
        if (pattern->IsRtl() && pattern->expectedScrollAxis_ == Axis::HORIZONTAL) {
            directVelocity = -directVelocity;
        }
        auto it = pattern->parentsMap_.find(pattern->expectedScrollAxis_);
        CHECK_EQUAL_VOID(it, pattern->parentsMap_.end());
        auto parent = it->second;

        bool isAtBorder = pattern->IsAtBorder();

        if (pattern->mode_ == NestedScrollMode::SELF_ONLY) {
            return;
        }

        if ((!pattern->isParentReachEdge_ && pattern->mode_ == NestedScrollMode::PARENT_FIRST) ||
             (!pattern->isParentReachEdge_ && pattern->mode_ == NestedScrollMode::PARALLEL) ||
             (pattern->isFirstFlingScrollVelocity_ && isAtBorder)) {
            pattern->HandleScrollVelocity(parent.Upgrade(), directVelocity);
            pattern->isFirstFlingScrollVelocity_ = false;
        }
        pattern->webPanActive_ = false;
    });
}

void WebPattern::SetOnActionCancelId()
{
    CHECK_NULL_VOID(webPanGesture_);
    webPanGesture_->SetOnActionCancelId([weak = WeakClaim(this)](const GestureEvent&) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->webPanActive_ = false;
    });
}

bool WebPattern::IsAtBorder()
{
    float dx = directionCtx_.currentX - directionCtx_.prevX;
    float dy = directionCtx_.currentY - directionCtx_.prevY;
    float scrollDistance = 0.1f;
    float scrollOffset = 0;
    auto parent = SearchParent(expectedScrollAxis_);
    if (parent) {
        Axis scrollAxis = parent->GetAxis();
        if (scrollAxis == Axis::HORIZONTAL) {
            scrollOffset = dx;
        } else if (scrollAxis == Axis::VERTICAL) {
            scrollOffset = dy;
        } else {
            TAG_LOGW(AceLogTag::ACE_WEB, "Unknown scroll axis in IsAtBorder");
        }
    }
    directionCtx_.isMinX = LessOrEqual(directionCtx_.currentX, 0.0) && expectedScrollAxis_ == Axis::HORIZONTAL;
    directionCtx_.isMaxX = GreatOrEqual(directionCtx_.currentX + directionCtx_.frameWidth, directionCtx_.currentWidth)
        && (expectedScrollAxis_ == Axis::HORIZONTAL);
    directionCtx_.isMinY = LessOrEqual(directionCtx_.currentY, 0.0) && expectedScrollAxis_ == Axis::VERTICAL;
    directionCtx_.isMaxY = GreatOrEqual(directionCtx_.currentY + directionCtx_.frameHeight, directionCtx_.currentHeight)
                        && expectedScrollAxis_ == Axis::VERTICAL;
    bool isAtBorder = IsScrollReachEdge(
        direction_, directionCtx_.isMinX, directionCtx_.isMaxX, directionCtx_.isMinY, directionCtx_.isMaxY);
    return isAtBorder;
}

void WebPattern::HandleMouseEvent(MouseInfo& info)
{
    isMouseEvent_ = true;
    WebOnMouseEvent(info);

    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto mouseEventCallback = eventHub->GetOnMouseEvent();
    CHECK_NULL_VOID(mouseEventCallback);
    mouseEventCallback(info);
}

void WebPattern::WebOnMouseEvent(const MouseInfo& info)
{}

Offset WebPattern::GetDragOffset() const
{
    Offset webDragOffset;
    int x = 0;
    int y = 0;

    webDragOffset.SetX(x);
    webDragOffset.SetY(y);

    return webDragOffset;
}

SizeF WebPattern::GetDragPixelMapSize() const
{
    return SizeF(0, 0);
}

bool WebPattern::HandleDoubleClickEvent(const MouseInfo& info)
{
    if (info.GetButton() != MouseButton::LEFT_BUTTON || info.GetAction() != MouseAction::PRESS) {
        return false;
    }
    auto localLocation = info.GetLocalLocation();
    MouseClickInfo clickInfo;
    clickInfo.x = localLocation.GetX();
    clickInfo.y = localLocation.GetY();
    clickInfo.start = info.GetTimeStamp();
    if (doubleClickQueue_.empty()) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    std::chrono::duration<float> timeout_ = clickInfo.start - doubleClickQueue_.back().start;
    double offsetX = clickInfo.x - doubleClickQueue_.back().x;
    double offsetY = clickInfo.y - doubleClickQueue_.back().y;
    double offset = sqrt(offsetX * offsetX + offsetY * offsetY);
    if (timeout_.count() < DEFAULT_DBCLICK_INTERVAL && offset < DEFAULT_DBCLICK_OFFSET) {
        SendDoubleClickEvent(clickInfo);
        std::queue<MouseClickInfo> empty;
        swap(empty, doubleClickQueue_);
        return true;
    }
    if (doubleClickQueue_.size() == 1) {
        doubleClickQueue_.push(clickInfo);
        return false;
    }
    doubleClickQueue_.pop();
    doubleClickQueue_.push(clickInfo);
    return false;
}

void WebPattern::SendDoubleClickEvent(const MouseClickInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->OnMouseEvent(info.x, info.y, MouseButton::LEFT_BUTTON, MouseAction::PRESS, DOUBLE_CLICK_NUM);
}

bool WebPattern::IsImageDrag()
{
    return false;
}

void WebPattern::InitFocusEvent(const RefPtr<FocusHub>& focusHub)
{
    auto focusTask = [weak = WeakClaim(this)](FocusReason reason) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleFocusEvent();
    };
    focusHub->SetOnFocusInternal(focusTask);

    auto blurTask = [weak = WeakClaim(this)](const BlurReason& blurReason) {
        auto pattern = weak.Upgrade();
        CHECK_NULL_VOID(pattern);
        pattern->HandleBlurEvent(blurReason);
    };
    focusHub->SetOnBlurReasonInternal(blurTask);

    auto keyTask = [weak = WeakClaim(this)](const KeyEvent& keyEvent) -> bool {
        auto pattern = weak.Upgrade();
        CHECK_NULL_RETURN(pattern, false);
        return pattern->HandleKeyEvent(keyEvent);
    };
    focusHub->SetOnKeyEventInternal(keyTask);
}

void WebPattern::HandleFocusEvent()
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = true;
    if (needOnFocus_) {
        delegate_->OnFocus();
    } else {
        needOnFocus_ = true;
    }
}

void WebPattern::HandleBlurEvent(const BlurReason& blurReason)
{
    CHECK_NULL_VOID(delegate_);
    isFocus_ = false;
    if (!selectPopupMenuShowing_) {
        delegate_->OnBlur();
    }
}

bool WebPattern::HandleKeyEvent(const KeyEvent& keyEvent)
{
    bool ret = false;

    auto host = GetHost();
    CHECK_NULL_RETURN(host, ret);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_RETURN(eventHub, ret);

    KeyEventInfo info(keyEvent);
    auto keyEventCallback = eventHub->GetOnKeyEvent();
    if (keyEventCallback) {
        keyEventCallback(info);
    }

    auto preKeyEventCallback = eventHub->GetOnPreKeyEvent();
    if (preKeyEventCallback) {
        ret = preKeyEventCallback(info);
        if (ret) {
            return ret;
        }
    }

    ret = WebOnKeyEvent(keyEvent);
    return ret;
}

bool WebPattern::WebOnKeyEvent(const KeyEvent& keyEvent)
{
    CHECK_NULL_RETURN(delegate_, false);
    return delegate_->OnKeyEvent(static_cast<int32_t>(keyEvent.code), static_cast<int32_t>(keyEvent.action));
}

void WebPattern::WebRequestFocus()
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto eventHub = host->GetEventHub<WebEventHub>();
    CHECK_NULL_VOID(eventHub);
    auto focusHub = eventHub->GetOrCreateFocusHub();
    CHECK_NULL_VOID(focusHub);

    focusHub->RequestFocusImmediately();
}

WebInfoType WebPattern::GetWebInfoType()
{
    return WebInfoType::TYPE_MOBILE;
}

void WebPattern::UpdateContentOffset(const RefPtr<LayoutWrapper>& dirty)
{
    CHECK_NULL_VOID(dirty);
    auto geometryNode = dirty->GetGeometryNode();
    CHECK_NULL_VOID(geometryNode);
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    auto paddingOffset = geometryNode->GetPaddingOffset();
    auto webContentSize = geometryNode->GetContentSize();
    renderContext->SetBounds(
        paddingOffset.GetX(), paddingOffset.GetY(), webContentSize.Width(), webContentSize.Height());
}

bool WebPattern::OnDirtyLayoutWrapperSwap(const RefPtr<LayoutWrapper>& dirty, const DirtySwapConfig& config)
{
    if (!config.contentSizeChange || isInWindowDrag_) {
        return false;
    }
    CHECK_NULL_RETURN(delegate_, false);
    CHECK_NULL_RETURN(dirty, false);
    auto geometryNode = dirty->GetGeometryNode();
    auto drawSize = Size(geometryNode->GetContentSize().Width(), geometryNode->GetContentSize().Height());
    if (drawSize.IsInfinite() || drawSize.IsEmpty()) {
        return false;
    }

    drawSize_ = drawSize;
    drawSizeCache_ = drawSize_;
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    if (!isUrlLoaded_) {
        isUrlLoaded_ = true;
        if (webData_) {
            delegate_->LoadDataWithRichText();
        }
    }
    return false;
}

void WebPattern::OnAreaChangedInner()
{
    auto offset = OffsetF(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    if (webOffset_ == offset) {
        return;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    const double SCREEN_CENTER_POSITION = pipeline->GetRootWidth() / 2.f;
    if (offset.GetX() == webOffset_.GetX() + SCREEN_CENTER_POSITION) {
        auto pageOutOffset = Offset(webOffset_.GetX() + drawSize_.Width(), offset.GetY());
        TAG_LOGI(AceLogTag::ACE_WEB, "Set offset to pageOutOffset");
        delegate_->SetBoundsOrResize(drawSize_, pageOutOffset);
        return;
    }
    webOffset_ = offset;
    if (isInWindowDrag_)
        return;
    auto resizeOffset = Offset(offset.GetX(), offset.GetY());
    delegate_->SetBoundsOrResize(drawSize_, resizeOffset);
}

void WebPattern::OnWebSrcUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadUrl();
    }
}

void WebPattern::OnWebDataUpdate()
{
    if (delegate_ && isUrlLoaded_) {
        delegate_->LoadDataWithRichText();
    }
}

void WebPattern::OnJsEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateJavaScriptEnabled(value);
    }
}

void WebPattern::OnMediaPlayGestureAccessUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMediaPlayGestureAccess(value);
    }
}

void WebPattern::OnFileAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateAllowFileAccess(value);
    }
}

void WebPattern::OnOnLineImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetworkImage(!value);
    }
}

void WebPattern::OnDomStorageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDomStorageEnabled(value);
    }
}

void WebPattern::OnImageAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateLoadsImagesAutomatically(value);
    }
}

void WebPattern::OnMixedModeUpdate(MixedModeContent value)
{
    if (delegate_) {
        delegate_->UpdateMixedContentMode(value);
    }
}

void WebPattern::OnZoomAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateSupportZoom(value);
    }
}

void WebPattern::OnGeolocationAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateGeolocationEnabled(value);
    }
}

void WebPattern::OnUserAgentUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateUserAgent(value);
    }
}

void WebPattern::OnCacheModeUpdate(WebCacheMode value)
{
    if (delegate_) {
        delegate_->UpdateCacheMode(value);
    }
}

void WebPattern::OnDarkModeUpdate(WebDarkMode mode)
{
    if (delegate_) {
        delegate_->UpdateDarkMode(mode);
    }
}

void WebPattern::OnForceDarkAccessUpdate(bool access)
{
    if (delegate_) {
        delegate_->UpdateForceDarkAccess(access);
    }
}

void WebPattern::OnAudioResumeIntervalUpdate(int32_t resumeInterval)
{
    if (delegate_) {
        delegate_->UpdateAudioResumeInterval(resumeInterval);
    }
}

void WebPattern::OnAudioExclusiveUpdate(bool audioExclusive)
{
    if (delegate_) {
        delegate_->UpdateAudioExclusive(audioExclusive);
    }
}

void WebPattern::OnAudioSessionTypeUpdate(WebAudioSessionType value)
{
    if (delegate_) {
        delegate_->UpdateAudioSessionType(value);
    }
}

void WebPattern::OnOverviewModeAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateOverviewModeEnabled(value);
    }
}

void WebPattern::OnFileFromUrlAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateFileFromUrlEnabled(value);
    }
}

void WebPattern::OnDatabaseAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateDatabaseEnabled(value);
    }
}

void WebPattern::OnTextZoomRatioUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateTextZoomRatio(value);
    }
}

void WebPattern::OnWebDebuggingAccessEnabledAndPortUpdate(
    const WebPatternProperty::WebDebuggingConfigType& enabled_and_port)
{
    if (delegate_) {
        bool enabled = std::get<0>(enabled_and_port);
        delegate_->UpdateWebDebuggingAccess(enabled);
    }
}

void WebPattern::OnPinchSmoothModeEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdatePinchSmoothModeEnabled(value);
    }
}

void WebPattern::OnBackgroundColorUpdate(int32_t value)
{
    UpdateBackgroundColorRightNow(value);
    if (delegate_) {
        delegate_->UpdateBackgroundColor(value);
    }
}

void WebPattern::OnInitialScaleUpdate(float value)
{
    if (delegate_) {
        delegate_->UpdateInitialScale(value);
    }
}

void WebPattern::OnMultiWindowAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateMultiWindowAccess(value);
    }
}

void WebPattern::OnAllowWindowOpenMethodUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateAllowWindowOpenMethod(value);
    }
}

void WebPattern::OnWebCursiveFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebCursiveFont(value);
    }
}

void WebPattern::OnWebFantasyFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFantasyFont(value);
    }
}

void WebPattern::OnWebFixedFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebFixedFont(value);
    }
}

void WebPattern::OnWebSansSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSansSerifFont(value);
    }
}

void WebPattern::OnWebSerifFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebSerifFont(value);
    }
}

void WebPattern::OnWebStandardFontUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateWebStandardFont(value);
    }
}

void WebPattern::OnDefaultFixedFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFixedFontSize(value);
    }
}

void WebPattern::OnDefaultFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateDefaultFontSize(value);
    }
}

void WebPattern::OnMinFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinFontSize(value);
    }
}

void WebPattern::OnMinLogicalFontSizeUpdate(int32_t value)
{
    if (delegate_) {
        delegate_->UpdateMinLogicalFontSize(value);
    }
}

void WebPattern::OnBlockNetworkUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateBlockNetwork(value);
    }
}

void WebPattern::OnHorizontalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateHorizontalScrollBarAccess(value);
    }
}

void WebPattern::OnVerticalScrollBarAccessEnabledUpdate(bool value)
{
    if (delegate_) {
        delegate_->UpdateVerticalScrollBarAccess(value);
    }
}

void WebPattern::SetUpdateInstanceIdCallback(std::function<void(int32_t)>&& callback)
{
    updateInstanceIdCallback_ = callback;
}

void WebPattern::OnScrollBarColorUpdate(const std::string& value)
{
    if (delegate_) {
        delegate_->UpdateScrollBarColor(value);
    }
}

void WebPattern::RegistVirtualKeyBoardListener()
{
    if (!needUpdateWeb_) {
        return;
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->SetVirtualKeyBoardCallback(
        [weak = AceType::WeakClaim(this)](int32_t width, int32_t height, double keyboard, bool isCustomKeyboard) {
            auto webPattern = weak.Upgrade();
            CHECK_NULL_RETURN(webPattern, false);
            return webPattern->ProcessVirtualKeyBoard(width, height, keyboard);
        });
    needUpdateWeb_ = false;
}

void WebPattern::InitEnhanceSurfaceFlag()
{
    if (SystemProperties::GetExtSurfaceEnabled()) {
        isEnhanceSurface_ = true;
    } else {
        isEnhanceSurface_ = false;
    }
}

void WebPattern::OnModifyDone()
{
    Pattern::OnModifyDone();
    // called in each update function.
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);

#if !defined(IOS_PLATFORM) && !defined(ANDROID_PLATFORM)
    RegistVirtualKeyBoardListener();
#endif
    if (!delegate_) {
        // first create case,
#if defined(IOS_PLATFORM) || defined(ANDROID_PLATFORM)
        WeakPtr<PipelineContext> context = WeakPtr<PipelineContext>(PipelineContext::GetCurrentContext());
        delegate_ = AceType::MakeRefPtr<WebDelegateCross>(PipelineContext::GetCurrentContext(), nullptr, "web");
        CHECK_NULL_VOID(delegate_);
        delegate_->SetNGWebPattern(Claim(this));
        delegate_->CreatePlatformResource(Size(0, 0), Offset(0, 0), context);
        SetScrollLocked(false);
        auto nestedScrollExt = GetNestedScrollExt();
        SetNestedScrollOptionsExt(nestedScrollExt);
        if (setWebIdCallback_) {
            setWebIdCallback_(delegate_->GetWebId());
        }
        auto container = Container::Current();
        CHECK_NULL_VOID(container);
        if (setHapPathCallback_) {
            setHapPathCallback_(container->GetHapPath());
        }
        if (onControllerAttachedCallback_) {
            onControllerAttachedCallback_();
        }

#else
        delegate_ = AceType::MakeRefPtr<WebDelegate>(PipelineContext::GetCurrentContext(), nullptr, "");
        CHECK_NULL_VOID(delegate_);
        observer_ = AceType::MakeRefPtr<WebDelegateObserver>(delegate_, PipelineContext::GetCurrentContext());
        CHECK_NULL_VOID(observer_);
        delegate_->SetObserver(observer_);
        delegate_->SetRenderMode(renderMode_);
        InitEnhanceSurfaceFlag();
        delegate_->SetNGWebPattern(Claim(this));
        delegate_->SetEnhanceSurfaceFlag(isEnhanceSurface_);
        delegate_->SetPopup(isPopup_);
        delegate_->SetParentNWebId(parentNWebId_);
        delegate_->SetBackgroundColor(GetBackgroundColorValue(
            static_cast<int32_t>(renderContext->GetBackgroundColor().value_or(Color::WHITE).GetValue())));
        if (isEnhanceSurface_) {
            auto drawSize = Size(1, 1);
            delegate_->SetDrawSize(drawSize);
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext());
        } else {
            auto drawSize = Size(1, 1);
            delegate_->SetDrawSize(drawSize);
            renderSurface_->SetRenderContext(host->GetRenderContext());
            if (renderMode_ == RenderMode::SYNC_RENDER) {
                renderSurface_->SetIsTexture(true);
                renderSurface_->SetPatternType(PATTERN_TYPE_WEB);
                renderSurface_->SetSurfaceQueueSize(SYNC_SURFACE_QUEUE_SIZE);
            } else {
                renderSurface_->SetIsTexture(false);
                renderSurface_->SetSurfaceQueueSize(ASYNC_SURFACE_QUEUE_SIZE);
            }
            renderSurface_->InitSurface();
            renderSurface_->UpdateSurfaceConfig();
            delegate_->InitOHOSWeb(PipelineContext::GetCurrentContext(), renderSurface_);
        }
#endif
        delegate_->UpdateBackgroundColor(GetBackgroundColorValue(
            static_cast<int32_t>(renderContext->GetBackgroundColor().value_or(Color::WHITE).GetValue())));
        delegate_->UpdateJavaScriptEnabled(GetJsEnabledValue(true));
        delegate_->UpdateBlockNetworkImage(!GetOnLineImageAccessEnabledValue(true));
        delegate_->UpdateAllowFileAccess(GetFileAccessEnabledValue(false));
        delegate_->UpdateLoadsImagesAutomatically(GetImageAccessEnabledValue(true));
        delegate_->UpdateMixedContentMode(GetMixedModeValue(MixedModeContent::MIXED_CONTENT_NEVER_ALLOW));
        delegate_->UpdateSupportZoom(GetZoomAccessEnabledValue(true));
        delegate_->UpdateDomStorageEnabled(GetDomStorageAccessEnabledValue(false));
        delegate_->UpdateGeolocationEnabled(GetGeolocationAccessEnabledValue(true));
        delegate_->UpdateCacheMode(GetCacheModeValue(WebCacheMode::DEFAULT));
        delegate_->UpdateDarkMode(GetDarkModeValue(WebDarkMode::Off));
        delegate_->UpdateForceDarkAccess(GetForceDarkAccessValue(false));
        delegate_->UpdateAudioResumeInterval(GetAudioResumeIntervalValue(-1));
        delegate_->UpdateAudioExclusive(GetAudioExclusiveValue(true));
        delegate_->UpdateAudioSessionType(GetAudioSessionTypeValue(WebAudioSessionType::AUTO));
        delegate_->UpdateOverviewModeEnabled(GetOverviewModeAccessEnabledValue(true));
        delegate_->UpdateFileFromUrlEnabled(GetFileFromUrlAccessEnabledValue(false));
        delegate_->UpdateDatabaseEnabled(GetDatabaseAccessEnabledValue(false));
        delegate_->UpdateTextZoomRatio(GetTextZoomRatioValue(DEFAULT_TEXT_ZOOM_RATIO));
        auto webDebugingConfig = GetWebDebuggingAccessEnabledAndPort();
        if (webDebugingConfig) {
            bool enabled = std::get<0>(webDebugingConfig.value());
            delegate_->UpdateWebDebuggingAccess(enabled);
        }
        delegate_->UpdateMediaPlayGestureAccess(GetMediaPlayGestureAccessValue(true));
        delegate_->UpdatePinchSmoothModeEnabled(GetPinchSmoothModeEnabledValue(false));
        delegate_->UpdateMultiWindowAccess(GetMultiWindowAccessEnabledValue(false));
        delegate_->UpdateWebCursiveFont(GetWebCursiveFontValue(DEFAULT_CURSIVE_FONT_FAMILY));
        delegate_->UpdateWebFantasyFont(GetWebFantasyFontValue(DEFAULT_FANTASY_FONT_FAMILY));
        delegate_->UpdateWebFixedFont(GetWebFixedFontValue(DEFAULT_FIXED_fONT_FAMILY));
        delegate_->UpdateWebSansSerifFont(GetWebSansSerifFontValue(DEFAULT_SANS_SERIF_FONT_FAMILY));
        delegate_->UpdateWebSerifFont(GetWebSerifFontValue(DEFAULT_SERIF_FONT_FAMILY));
        delegate_->UpdateWebStandardFont(GetWebStandardFontValue(DEFAULT_STANDARD_FONT_FAMILY));
        delegate_->UpdateDefaultFixedFontSize(GetDefaultFixedFontSizeValue(DEFAULT_FIXED_FONT_SIZE));
        delegate_->UpdateDefaultFontSize(GetDefaultFontSizeValue(DEFAULT_FONT_SIZE));
        delegate_->UpdateMinFontSize(GetMinFontSizeValue(DEFAULT_MINIMUM_FONT_SIZE));
        delegate_->UpdateMinLogicalFontSize(GetMinLogicalFontSizeValue(DEFAULT_MINIMUM_LOGICAL_FONT_SIZE));
        delegate_->UpdateHorizontalScrollBarAccess(GetHorizontalScrollBarAccessEnabledValue(true));
        delegate_->UpdateVerticalScrollBarAccess(GetVerticalScrollBarAccessEnabledValue(true));
        delegate_->UpdateScrollBarColor(GetScrollBarColorValue(DEFAULT_SCROLLBAR_COLOR));
        if (GetBlockNetwork()) {
            delegate_->UpdateBlockNetwork(GetBlockNetwork().value());
        }
        if (GetUserAgent()) {
            delegate_->UpdateUserAgent(GetUserAgent().value());
        }
        if (GetInitialScale()) {
            delegate_->UpdateInitialScale(GetInitialScale().value());
        }
    }

    // Initialize events such as keyboard, focus, etc.
    InitEvent();

    // Initialize scrollupdate listener
    if (renderMode_ == RenderMode::SYNC_RENDER) {
        auto task = [this]() {
            InitSlideUpdateListener();
        };
        PostTaskToUI(std::move(task));
    }
    auto pipelineContext = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipelineContext);
    pipelineContext->AddOnAreaChangeNode(host->GetId());
}

bool WebPattern::ProcessVirtualKeyBoard(int32_t width, int32_t height, double keyboard)
{
    CHECK_NULL_RETURN(delegate_, false);
    if (!isFocus_) {
        if (isVirtualKeyBoardShow_ == VkState::VK_SHOW) {
            drawSize_.SetSize(drawSizeCache_);
            UpdateWebLayoutSize(width, height);
            isVirtualKeyBoardShow_ = VkState::VK_HIDE;
        }
        return false;
    }
    if (NearZero(keyboard)) {
        drawSize_.SetSize(drawSizeCache_);
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_HIDE;
    } else if (isVirtualKeyBoardShow_ != VkState::VK_SHOW) {
        drawSizeCache_.SetSize(drawSize_);
        if (drawSize_.Height() <= (height - keyboard - GetCoordinatePoint()->GetY())) {
            isVirtualKeyBoardShow_ = VkState::VK_SHOW;
            return true;
        }
        if (height - GetCoordinatePoint()->GetY() < keyboard) {
            return true;
        }
        drawSize_.SetHeight(height - keyboard - GetCoordinatePoint()->GetY());
        UpdateWebLayoutSize(width, height);
        isVirtualKeyBoardShow_ = VkState::VK_SHOW;
    }
    return true;
}

void WebPattern::UpdateWebLayoutSize(int32_t width, int32_t height)
{
    auto frameNode = GetHost();
    CHECK_NULL_VOID(frameNode);
    auto rect = frameNode->GetGeometryNode()->GetFrameRect();
    auto offset = Offset(GetCoordinatePoint()->GetX(), GetCoordinatePoint()->GetY());
    delegate_->SetBoundsOrResize(drawSize_, offset);
    rect.SetSize(SizeF(drawSize_.Width(), drawSize_.Height()));
    frameNode->GetRenderContext()->SyncGeometryProperties(rect);
    frameNode->MarkDirtyNode(PROPERTY_UPDATE_MEASURE_SELF);
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(context);
    context->SetRootRect(width, height, 0);
}

void WebPattern::HandleTouchDown(const TouchEventInfo& info, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    Offset touchOffset = Offset(0, 0);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        return;
    }
    for (auto& touchPoint : touchInfos) {
        prevOffset_ = touchPoint;
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchDown(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    }
    isStartTouch_ = false;
}

void WebPattern::HandleTouchUp(const TouchEventInfo& info, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
        delegate_->HandleTouchUp(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    }
    if (!touchInfos.empty()) {
        WebRequestFocus();
    }
}

void WebPattern::CalcScrollParamsAndBorder(
    const TouchInfo& touchPoint, float& dx, float& dy, float& scrollOffset, bool& isAtBorder)
{
    dx = touchPoint.x - prevOffset_.x;
    dy = touchPoint.y - prevOffset_.y;

    if (expectedScrollAxis_ == Axis::VERTICAL) {
        direction_ = (touchPoint.y < prevOffset_.y) ? ScrollDirection::DOWN : ScrollDirection::UP;
    } else {
        direction_ = (touchPoint.x < prevOffset_.x) ? ScrollDirection::RIGHT : ScrollDirection::LEFT;
    }
    scrollOffset = 0;
    auto parent = SearchParent(expectedScrollAxis_);
    if (parent) {
        Axis scrollAxis = parent->GetAxis();
        mode_ = NestedScrollMode::SELF_ONLY;
        if (scrollAxis == Axis::HORIZONTAL) {
            scrollOffset = dx;
            mode_ = (dx > 0) ? GetNestedScrollExt().scrollLeft : GetNestedScrollExt().scrollRight;
        } else if (scrollAxis == Axis::VERTICAL) {
            scrollOffset = dy;
            mode_ = (dy > 0) ? GetNestedScrollExt().scrollUp : GetNestedScrollExt().scrollDown;
        } else {
            TAG_LOGW(AceLogTag::ACE_WEB, "Unknown scroll axis in CalcScrollParamsAndBorder");
        }
    }

    directionCtx_.isMinX = LessOrEqual(directionCtx_.currentX, 0.0) && (expectedScrollAxis_ == Axis::HORIZONTAL);
    directionCtx_.isMaxX = GreatOrEqual(directionCtx_.currentX + directionCtx_.frameWidth, directionCtx_.currentWidth)
        && (expectedScrollAxis_ == Axis::HORIZONTAL);
    directionCtx_.isMinY = LessOrEqual(directionCtx_.currentY, 0.0) && (expectedScrollAxis_ == Axis::VERTICAL);
    directionCtx_.isMaxY = GreatOrEqual(directionCtx_.currentY + directionCtx_.frameHeight, directionCtx_.currentHeight)
        && (expectedScrollAxis_ == Axis::VERTICAL);
    isAtBorder = IsScrollReachEdge(
        direction_, directionCtx_.isMinX, directionCtx_.isMaxX, directionCtx_.isMinY, directionCtx_.isMaxY);
}

void WebPattern::HandleTouchMove(const TouchEventInfo& info, bool fromOverlay)
{
    if (isDragging_) {
        return;
    }
    auto pipeline = PipelineContext::GetCurrentContext();
    CHECK_NULL_VOID(pipeline);
    auto manager = pipeline->GetDragDropManager();
    CHECK_NULL_VOID(manager);
    if (manager->IsDragged()) {
        return;
    }
    CHECK_NULL_VOID(delegate_);
    std::list<TouchInfo> touchInfos;
    if (!ParseTouchInfo(info, touchInfos)) {
        return;
    }
    for (auto& touchPoint : touchInfos) {
        if (fromOverlay) {
            touchPoint.x -= webOffset_.GetX();
            touchPoint.y -= webOffset_.GetY();
        }
#if defined(IOS_PLATFORM)
        HandleTouchMoveIOS(touchPoint, fromOverlay);
#else
        HandleTouchMoveAndroid(touchPoint, fromOverlay);
#endif
    }
}

void WebPattern::HandleTouchMoveIOS(TouchInfo &touchPoint, bool fromOverlay)
{
    delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    float dx = touchPoint.x - prevOffset_.x;
    float dy = touchPoint.y - prevOffset_.y;
    if (!isStartTouch_) {
        OnScrollStart(dx, dy);
        isStartTouch_ = true;
    }
    direction_ = GetScrollDirection(dx, dy);
    auto parent = SearchParent(expectedScrollAxis_);
    if (parent) {
        SetDirectionMode();
        if (mode_ == NestedScrollMode::PARENT_FIRST) {
            float offset = (direction_ == ScrollDirection::LEFT || direction_ == ScrollDirection::UP) ?
                MIN_SCROLL_OFFSET : -MIN_SCROLL_OFFSET;
            auto result = parent->HandleScroll(offset, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
            isParentReachEdge_ = result.reachEdge;
            SetScrollLocked(!isParentReachEdge_);
        } else {
            SetScrollLocked(false);
        }
    } else {
        SetScrollLocked(false);
    }
    prevOffset_ = touchPoint;
}

void WebPattern::HandleTouchMoveAndroid(TouchInfo &touchPoint, bool fromOverlay)
{
    float dx = 0;
    float dy = 0;
    float scrollOffset = 0;
    bool isAtBorder = false;
    CalcScrollParamsAndBorder(touchPoint, dx, dy, scrollOffset, isAtBorder);
    switch (mode_) {
        case NestedScrollMode::PARENT_FIRST:
            HandleParentFirstScroll(dx, dy, scrollOffset, isAtBorder, touchPoint, fromOverlay);
            break;
        case NestedScrollMode::SELF_ONLY:
            delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
            break;
        case NestedScrollMode::SELF_FIRST:
            HandleSelfFirstScroll(scrollOffset, isAtBorder, touchPoint, fromOverlay, dy);
            break;
        case NestedScrollMode::PARALLEL:
            delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
            if (auto parent = SearchParent(expectedScrollAxis_)) {
                ScrollResult result = {0.f, true};
                auto sheetParent = SearchSheetParent(parent, expectedScrollAxis_);
                result = parent->HandleScroll(scrollOffset,
                SCROLL_FROM_UPDATE,
                (sheetParent || ParentHostTagIsRefresh(parent)) ?
                NestedState::CHILD_SCROLL : NestedState::CHILD_OVER_SCROLL);
            }
            break;
        default:
            break;
    }
    prevOffset_ = touchPoint;
}

void WebPattern::HandleParentFirstScroll(
    float dx, float dy, float scrollOffset, bool isAtBorder, const TouchInfo& touchPoint, bool fromOverlay)
{
    CHECK_NULL_VOID(delegate_);
    auto parent = SearchParent(expectedScrollAxis_);
    CHECK_NULL_VOID(parent);
    float offset = 0;
    Axis scrollAxis = parent->GetAxis();
    offset = (scrollAxis == Axis::HORIZONTAL) ?
            (dx > 0 ? 0.1f : (dx < 0 ? -0.1f : 0)) : (dy > 0 ? 0.1f : (dy < 0 ? -0.1f : 0));
    isParentReachEdge_ = parent->HandleScroll(offset, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL).reachEdge;
    auto swiperParent = SearchSwiperParent(parent, expectedScrollAxis_);
    if (swiperParent) {
        int32_t index = swiperParent->GetCurrentShownIndex();
        int32_t total = swiperParent->TotalCount();
        if (index != total - 1 && index != 0) {
            isParentReachEdge_ = false;
        }
    }
    if (isParentReachEdge_) {
        delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
        if (isAtBorder) {
            ScrollResult result = {0.f, true};
            auto sheetParent = SearchSheetParent(parent, expectedScrollAxis_);
            result = parent->HandleScroll(
                scrollOffset,
                SCROLL_FROM_UPDATE,
                sheetParent ? NestedState::CHILD_SCROLL : NestedState::CHILD_OVER_SCROLL);
            isParentReachEdge_ = result.reachEdge;
        }
    } else {
        auto result = parent->HandleScroll(scrollOffset, SCROLL_FROM_UPDATE, NestedState::CHILD_OVER_SCROLL);
        isParentReachEdge_ = result.reachEdge;
    }
}

void WebPattern::HandleSelfFirstScroll(
    float scrollOffset, bool isAtBorder, const TouchInfo& touchPoint, bool fromOverlay, float dy)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->HandleTouchMove(touchPoint.id, touchPoint.x, touchPoint.y, fromOverlay);
    if (isAtBorder) {
        auto parent = SearchParent(expectedScrollAxis_);
        if (parent) {
            ScrollResult result = {0.f, true};
            auto sheetParent = SearchSheetParent(parent, expectedScrollAxis_);
            result = parent->HandleScroll(scrollOffset,
            SCROLL_FROM_UPDATE,
            sheetParent ? NestedState::CHILD_SCROLL : NestedState::CHILD_OVER_SCROLL);
            isParentReachEdge_ = result.reachEdge;
        }
    }
}

RefPtr<SheetPresentationPattern> WebPattern::SearchSheetParent(RefPtr<NestableScrollContainer> pattern,
    Axis scrollAxis)
{
    if (pattern == nullptr) {
        return nullptr;
    }
    auto host = pattern->GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    for (auto parent = host->GetParent(); parent != nullptr; parent = parent->GetParent()) {
        RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
        if (!frameNode) {
            continue;
        }
        auto pattern = frameNode->GetPattern<SheetPresentationPattern>();
        if (!pattern ||
            (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE) &&
                InstanceOf<RefreshPattern>(pattern))) {
            continue;
        }
        if (scrollAxis == Axis::NONE || scrollAxis == pattern->GetAxis()) {
            return pattern;
        }
    }
    return nullptr;
}

RefPtr<SwiperPattern> WebPattern::SearchSwiperParent(RefPtr<NestableScrollContainer> pattern,
    Axis scrollAxis)
{
    if (pattern == nullptr) {
        return nullptr;
    }
    auto host = pattern->GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    for (auto parent = host->GetParent(); parent != nullptr; parent = parent->GetParent()) {
        RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
        if (!frameNode) {
            continue;
        }
        auto pattern = frameNode->GetPattern<OHOS::Ace::NG::SwiperPattern>();
        if (!pattern ||
            (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE) &&
                InstanceOf<RefreshPattern>(pattern))) {
            continue;
        }
        return pattern;
    }
    return nullptr;
}

bool WebPattern::ParentHostTagIsRefresh(RefPtr<NestableScrollContainer> container)
{
    CHECK_NULL_RETURN(container, false);
    auto host = container->GetHost();
    CHECK_NULL_RETURN(host, false);
    return host->GetHostTag() == TAG_REFRESH;
}

void WebPattern::HandleTouchCancel(const TouchEventInfo& info)
{
    CHECK_NULL_VOID(delegate_);
    delegate_->HandleTouchCancel();
}

bool WebPattern::ParseTouchInfo(const TouchEventInfo& info, std::list<TouchInfo>& touchInfos)
{
    auto context = PipelineContext::GetCurrentContext();
    CHECK_NULL_RETURN(context, false);
    auto viewScale = context->GetViewScale();
    if (info.GetChangedTouches().empty()) {
        return false;
    }
    for (const auto& point : info.GetChangedTouches()) {
        TouchInfo touchInfo;
        touchInfo.id = point.GetFingerId();
        const Offset& location = point.GetLocalLocation();
        touchInfo.x = static_cast<float>(location.GetX() * viewScale);
        touchInfo.y = static_cast<float>(location.GetY() * viewScale);
        touchInfos.emplace_back(touchInfo);
    }
    return true;
}

void WebPattern::RequestFullScreen()
{
    isFullScreen_ = true;
}

void WebPattern::ExitFullScreen()
{
    isFullScreen_ = false;
}

std::optional<OffsetF> WebPattern::GetCoordinatePoint()
{
    auto frameNode = GetHost();
    CHECK_NULL_RETURN(frameNode, OffsetF());
    return frameNode->GetTransformRelativeOffset();
}

void WebPattern::UpdateLocale()
{
    CHECK_NULL_VOID(delegate_);
    delegate_->UpdateLocale();
}

void WebPattern::OnWindowShow()
{
    if (isWindowShow_ || !isVisible_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->ShowWebView();
    isWindowShow_ = true;
}

void WebPattern::OnWindowHide()
{
    if (!isWindowShow_ || !isVisible_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->HideWebView();
    needOnFocus_ = false;
    isWindowShow_ = false;
}

void WebPattern::WebRotateRenderEffect(WindowSizeChangeReason type)
{
    // cross platform is not support now;
}

void WebPattern::OnWindowSizeChanged(int32_t width, int32_t height, WindowSizeChangeReason type) {}

void WebPattern::OnCompleteSwapWithNewSize()
{
    if (!isInWindowDrag_ || !isWaiting_)
        return;

    ACE_SCOPED_TRACE("WebPattern::OnCompleteSwapWithNewSize");
    isWaiting_ = false;
}

void WebPattern::OnResizeNotWork()
{
    if (!isInWindowDrag_ || !isWaiting_)
        return;

    ACE_SCOPED_TRACE("WebPattern::OnResizeNotWork");
    isWaiting_ = false;
}

bool WebPattern::OnBackPressed() const
{
    if (!isFullScreen_) {
        return false;
    }

    CHECK_NULL_RETURN(fullScreenExitHandler_, false);
    auto webFullScreenExitHandler = fullScreenExitHandler_->GetHandler();
    CHECK_NULL_RETURN(webFullScreenExitHandler, false);
    webFullScreenExitHandler->ExitFullScreen();
    return true;
}

bool WebPattern::OnBackPressedForFullScreen() const
{
    if (!isFullScreen_) {
        return false;
    }

    CHECK_NULL_RETURN(fullScreenExitHandler_, false);
    auto webFullScreenExitHandler = fullScreenExitHandler_->GetHandler();
    CHECK_NULL_RETURN(webFullScreenExitHandler, false);
    webFullScreenExitHandler->ExitFullScreen();
    return true;
}

void WebPattern::SetFullScreenExitHandler(const std::shared_ptr<FullScreenEnterEvent>& fullScreenExitHandler)
{
    fullScreenExitHandler_ = fullScreenExitHandler;
}

void WebPattern::OnInActive()
{
    if (!isActive_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->OnInactive();
    isActive_ = false;
}

void WebPattern::OnActive()
{
    if (isActive_) {
        return;
    }

    CHECK_NULL_VOID(delegate_);
    delegate_->OnActive();
    isActive_ = true;
}

void WebPattern::OnVisibleChange(bool isVisible)
{
    if (isVisible_ == isVisible) {
        return;
    }

    isVisible_ = isVisible;
    if (!isVisible_) {
        if (isVisibleActiveEnable_) {
            OnInActive();
        }
    } else {
        if (isVisibleActiveEnable_) {
            OnActive();
        }
    }
}

Color WebPattern::GetDefaultBackgroundColor()
{
    return Color::WHITE;
}

void WebPattern::UpdateBackgroundColorRightNow(int32_t color)
{
    auto host = GetHost();
    CHECK_NULL_VOID(host);
    auto renderContext = host->GetRenderContext();
    CHECK_NULL_VOID(renderContext);
    renderContext->UpdateBackgroundColor(Color(static_cast<uint32_t>(color)));
}

void WebPattern::OnRootLayerChanged(int width, int height)
{
    // cross platform is not support now;
}

void WebPattern::SetNestedScroll(const NestedScrollOptions& nestedOpt)
{
    // cross platform is not support now;
}

void WebPattern::SetNestedScrollExt(const NestedScrollOptionsExt& nestedScroll)
{
    SetScrollLocked(false);
    NestedScrollOptions nestedOpt = {
        .forward = NestedScrollMode::SELF_FIRST,
        .backward = NestedScrollMode::SELF_FIRST,
    };
    if (nestedScroll.scrollUp == nestedScroll.scrollLeft) {
        nestedOpt.backward = nestedScroll.scrollUp;
    }
    if (nestedScroll.scrollDown == nestedScroll.scrollRight) {
        nestedOpt.forward = nestedScroll.scrollDown;
    }
    auto instance = ViewStackProcessor::GetInstance();
    CHECK_NULL_VOID(instance);
    auto pattern = instance->GetMainFrameNodePattern<NestableScrollContainer>();
    if (pattern) {
        pattern->SetNestedScroll(nestedOpt);
    }

    SetNestedScrollOptionsExt(nestedScroll);
    nestedScroll_ = nestedScroll;
}

void WebPattern::OnScrollWillStart(float x, float y)
{
    prevX_ = x;
    prevY_ = y;
    averageDistanceX_ = 0;
    averageDistanceY_ = 0;
}

void WebPattern::OnScrollStart(float x, float y)
{
    GetParentAxis();
    expectedScrollAxis_ = (abs(x) > abs(y) ? Axis::HORIZONTAL : Axis::VERTICAL);
    OnScrollStartRecursive(0.0);
}

void WebPattern::OnScroll(float currentX, float currentY, float cWidth, float cHeight, float fWidth, float fHeight)
{
    float dx = ((LessOrEqual(currentX, 0.0) && currentX > prevX_) ||
    (GreatOrEqual(currentX + fWidth, cWidth) && currentX < prevX_)) ? currentX - prevX_ : prevX_ - currentX;
    float dy = ((LessOrEqual(currentY, 0.0) && currentY > prevY_) ||
    (GreatOrEqual(currentY + fHeight, cHeight) && currentY < prevY_)) ? currentY - prevY_ : prevY_ - currentY;
    bool minX = LessOrEqual(currentX, 0.0) && expectedScrollAxis_ == Axis::HORIZONTAL;
    bool maxX = GreatOrEqual(currentX + fWidth, cWidth) && expectedScrollAxis_ == Axis::HORIZONTAL;
    bool minY = LessOrEqual(currentY, 0.0) && expectedScrollAxis_ == Axis::VERTICAL;
    bool maxY = GreatOrEqual(currentY + fHeight, cHeight) && expectedScrollAxis_ == Axis::VERTICAL;
    directionCtx_.Set(minX, maxX, minY, maxY, currentX, currentY, prevX_, prevY_, cWidth, cHeight, fWidth, fHeight);
    direction_ = GetScrollDirection(dx, dy);
    SetDirectionMode();
    prevX_ = currentX;
    prevY_ = currentY;
#if defined(IOS_PLATFORM)
    bool isAtBorder = IsScrollReachEdge(direction_, minX, maxX, minY, maxY);
    if (mode_ == NestedScrollMode::PARENT_FIRST) {
        if (isAtBorder) {
            SetScrollLocked(true);
        } else {
            float offset = (direction_ == ScrollDirection::LEFT || direction_ == ScrollDirection::UP) ? 0.1 : -0.1;
            FilterScrollEventHandleOffset(offset);
            SetScrollLocked(!isParentReachEdge_);
        }
    } else if (mode_ == NestedScrollMode::SELF_ONLY) {
        SetScrollLocked(false);
    } else if (mode_ == NestedScrollMode::SELF_FIRST ||mode_ == NestedScrollMode::PARALLEL) {
        SetScrollLocked(false);
        float xVelocity = 0.0f;
        float yVelocity = 0.0f;
        bool isAvailable = true;
        if (GetScrollBoundary(currentX, currentY, fWidth, fHeight, cWidth, cHeight)) {
            if (expectedScrollAxis_ == Axis::VERTICAL) {
                averageDistanceY_ += dy;
            } else if (expectedScrollAxis_ == Axis::HORIZONTAL) {
                averageDistanceX_ += dx;
            }
            OnNestedScroll(averageDistanceX_, averageDistanceY_, xVelocity, yVelocity, isAvailable);
            return;
        }
        if (mode_ == NestedScrollMode::PARALLEL) {
            OnNestedScroll(dx, dy, xVelocity, yVelocity, isAvailable);
        }
    }
#endif
}

bool WebPattern::GetScrollBoundary(
    float currentX, float currentY, float fWidth, float fHeight, float cWidth, float cHeight)
{
    return currentX < 0 && expectedScrollAxis_ == Axis::HORIZONTAL ||
            currentX + fWidth > cWidth && expectedScrollAxis_ == Axis::HORIZONTAL ||
            currentY < 0 && expectedScrollAxis_ == Axis::VERTICAL ||
            currentY + fHeight > cHeight && expectedScrollAxis_ == Axis::VERTICAL;
}

bool WebPattern::OnNestedScroll(float x, float y, float xVelocity, float yVelocity, bool isAvailable)
{
    isAvailable = true;
    bool hasHorizontalParent = parentsMap_.find(Axis::HORIZONTAL) != parentsMap_.end();
    bool hasVerticalParent = parentsMap_.find(Axis::VERTICAL) != parentsMap_.end();
    if (!hasHorizontalParent && !hasVerticalParent) {
        return false;
    }
    float offset = y;
    float velocity = yVelocity;
    if (expectedScrollAxis_ == Axis::HORIZONTAL) {
        offset = x;
        velocity = xVelocity;
        if (isScrollStarted_) {
            y = 0.0f;
            yVelocity = 0.0f;
        }
    } else {
        if (isScrollStarted_) {
            x = 0.0f;
            xVelocity = 0.0f;
        }
    }
    bool isConsumed = !NearEqual(offset, 0.0f) ? FilterScrollEventHandleOffset(offset) :
        FilterScrollEventHandleVelocity(velocity);
    return isConsumed;
}

bool WebPattern::IsRtl()
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, false);
    auto layoutProperty = host->GetLayoutProperty();
    CHECK_NULL_RETURN(layoutProperty, false);
    auto dir = layoutProperty->GetLayoutDirection();
    if (TextDirection::RTL == dir) {
        return true;
    } else if (TextDirection::LTR == dir) {
        return false;
    } else {
        return AceApplicationInfo::GetInstance().IsRightToLeft();
    }
}

bool WebPattern::FilterScrollEventHandleOffset(float offset)
{
    float directOffset = offset;
    if (IsRtl() && expectedScrollAxis_ == Axis::HORIZONTAL) {
        directOffset = -offset;
    }
    isSelfReachEdge_ = false;
    auto it = parentsMap_.find(expectedScrollAxis_);
    CHECK_EQUAL_RETURN(it, parentsMap_.end(), false);
    auto parent = it->second;
    auto parentPtr = parent.Upgrade();
    if (parentPtr && !NearZero(directOffset)) {
        auto res =
            HandleScroll(parentPtr, directOffset, SCROLL_FROM_UPDATE, NestedState::CHILD_CHECK_OVER_SCROLL);
        if (NearZero(res.remain)) {
            isParentReverseReachEdge_ = true;
            return true;
        }
        isParentReverseReachEdge_ = false;
        directOffset = res.remain;
    }
    if (CheckParentScroll(offset, NestedScrollMode::PARENT_FIRST)) {
        auto result = HandleScroll(parentPtr, directOffset, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
        CHECK_EQUAL_RETURN(isParentReachEdge_ && result.reachEdge, true, false);
        isParentReachEdge_ = result.reachEdge;
        return true;
    } else if (CheckParentScroll(offset, NestedScrollMode::PARALLEL)) {
        HandleScroll(parentPtr, directOffset, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
    } else if (CheckParentScroll(offset, NestedScrollMode::SELF_FIRST)) {
        if (AceType::DynamicCast<ScrollPattern>(parentPtr)) {
            HandleScroll(parentPtr, offset, SCROLL_FROM_UPDATE, NestedState::CHILD_SCROLL);
        } else {
            HandleScroll(parentPtr, offset, SCROLL_FROM_UPDATE, NestedState::CHILD_OVER_SCROLL);
        }
        return true;
    }
    return false;
}

bool WebPattern::CheckParentScroll(float directValue, const NestedScrollMode &scrollMode)
{
    auto nestedScroll = GetNestedScrollExt();
    return (directValue > 0 && nestedScroll.scrollUp == scrollMode &&
            expectedScrollAxis_ != Axis::HORIZONTAL) ||
        (directValue > 0 && nestedScroll.scrollLeft == scrollMode &&
            expectedScrollAxis_ == Axis::HORIZONTAL) ||
        (directValue < 0 && nestedScroll.scrollDown == scrollMode &&
            expectedScrollAxis_ != Axis::HORIZONTAL) ||
        (directValue < 0 && nestedScroll.scrollRight == scrollMode &&
            expectedScrollAxis_ == Axis::HORIZONTAL);
}

bool WebPattern::FilterScrollEventHandleVelocity(float velocity)
{
    float directVelocity = velocity;
    if (IsRtl() && expectedScrollAxis_ == Axis::HORIZONTAL) {
        directVelocity = -velocity;
    }
    auto it = parentsMap_.find(expectedScrollAxis_);
    CHECK_EQUAL_RETURN(it, parentsMap_.end(), false);
    auto parent = it->second;
    auto parentPtr = parent.Upgrade();
    if (parentPtr && parentPtr->NestedScrollOutOfBoundary()) {
        if (isSelfReachEdge_ || isParentReverseReachEdge_) {
            return HandleScrollVelocity(parentPtr, directVelocity);
        }
        dragEndRecursiveParent_ = parentPtr;
        return false;
    }
    if (CheckParentScroll(velocity, NestedScrollMode::PARENT_FIRST)) {
        if (isParentReachEdge_) {
            return false;
        }
        return HandleScrollVelocity(parentPtr, directVelocity);
    } else if (CheckParentScroll(velocity, NestedScrollMode::PARALLEL)) {
        HandleScrollVelocity(parentPtr, directVelocity);
    }
    return false;
}

void WebPattern::GetParentAxis()
{
    auto parent = GetNestedScrollParent();
    if (parent) {
        axis_ = parent->GetAxis();
        parentsMap_ = { { axis_, parent } };
        auto oppositeParent = SearchParent(axis_ == Axis::HORIZONTAL ? Axis::VERTICAL : Axis::HORIZONTAL);
        if (oppositeParent) {
            parentsMap_.emplace(oppositeParent->GetAxis(), oppositeParent);
        }
    } else {
        auto verticalParent = SearchParent(Axis::VERTICAL);
        auto horizontalParent = SearchParent(Axis::HORIZONTAL);
        if (verticalParent) {
            parentsMap_.emplace(verticalParent->GetAxis(), verticalParent);
        }
        if (horizontalParent) {
            parentsMap_.emplace(horizontalParent->GetAxis(), horizontalParent);
        }
    }
}

RefPtr<NestableScrollContainer> WebPattern::SearchParent()
{
    return SearchParent(Axis::NONE);
}

RefPtr<NestableScrollContainer> WebPattern::SearchParent(Axis scrollAxis)
{
    auto host = GetHost();
    CHECK_NULL_RETURN(host, nullptr);
    for (auto parent = host->GetParent(); parent != nullptr; parent = parent->GetParent()) {
        RefPtr<FrameNode> frameNode = AceType::DynamicCast<FrameNode>(parent);
        if (!frameNode) {
            continue;
        }
        auto pattern = frameNode->GetPattern<NestableScrollContainer>();
        if (!pattern ||
            (!AceApplicationInfo::GetInstance().GreatOrEqualTargetAPIVersion(PlatformVersion::VERSION_TWELVE) &&
                InstanceOf<RefreshPattern>(pattern))) {
            continue;
        }
        if (scrollAxis == Axis::NONE || scrollAxis == pattern->GetAxis()) {
            return pattern;
        }
    }
    return nullptr;
}

ScrollResult WebPattern::HandleScroll(float offset, int32_t source, NestedState state, float velocity)
{
    return HandleScroll(GetNestedScrollParent(), offset, source, state);
}

ScrollResult WebPattern::HandleScroll(RefPtr<NestableScrollContainer> parent, float offset,
    int32_t source, NestedState state)
{
    TAG_LOGD(AceLogTag::ACE_WEB,
        "WebPattern::HandleScroll scroll direction: %{public}d, find parent component: %{public}d",
        expectedScrollAxis_,
        (parent != nullptr));
    if (parent) {
        return parent->HandleScroll(offset, source, state);
    }
    return { 0.0f, false };
}

bool WebPattern::HandleScrollVelocity(float velocity, const RefPtr<NestableScrollContainer>& child)
{
    return HandleScrollVelocity(GetNestedScrollParent(), velocity);
}

bool WebPattern::HandleScrollVelocity(const RefPtr<NestableScrollContainer>& parent, float velocity)
{
    CHECK_NULL_RETURN(parent, false);
    if (parent->HandleScrollVelocity(velocity)) {
        OnParentScrollDragEndRecursive(parent);
        return true;
    }
    return false;
}

void WebPattern::OnParentScrollDragEndRecursive(RefPtr<NestableScrollContainer> parent)
{
    if (isDragEnd_) {
        return;
    }
    if (parent) {
        parent->OnScrollDragEndRecursive();
    }
    isDragEnd_ = true;
}

void WebPattern::OnScrollStartRecursive(WeakPtr<NestableScrollContainer> child, float position, float velocity)
{
    OnScrollStartRecursive(position);
}

void WebPattern::OnScrollStartRecursive(float position)
{
    SetIsNestedInterrupt(false);
    isFirstFlingScrollVelocity_ = true;
    isScrollStarted_ = true;
    isDragEnd_ = false;
    auto it = parentsMap_.find(expectedScrollAxis_);
    CHECK_EQUAL_VOID(it, parentsMap_.end());
    auto parent = it->second.Upgrade();
    if (parent) {
        parent->OnScrollStartRecursive(WeakClaim(this), position);
    }
}

void WebPattern::OnScrollEndRecursive(const std::optional<float>& velocity)
{
    CHECK_EQUAL_VOID(isScrollStarted_, false);
    auto it = parentsMap_.find(expectedScrollAxis_);
    if (it != parentsMap_.end()) {
        auto parent = it->second.Upgrade();
        if (parent) {
            OnParentScrollDragEndRecursive(parent);
            parent->OnScrollEndRecursive(std::nullopt);
        }
    }
    isScrollStarted_ = false;
    SetIsNestedInterrupt(false);
    expectedScrollAxis_ = Axis::FREE;
}

void WebPattern::SetNestedScrollOptionsExt(NestedScrollOptionsExt nestedOpt)
{
    auto webDelegateCross = AceType::DynamicCast<WebDelegateCross>(delegate_);
    if (webDelegateCross) {
        webDelegateCross->SetNestedScrollOptionsExt(nestedOpt);
    }
}

void WebPattern::SetScrollLocked(bool value)
{
#if defined(IOS_PLATFORM)
    SetIsPointInsideWebForceResult(true, value);
    auto webDelegateCross = AceType::DynamicCast<WebDelegateCross>(delegate_);
    if (webDelegateCross) {
        webDelegateCross->SetScrollLocked(value);
    }
#endif
}

ScrollDirection WebPattern::GetScrollDirection(float dx, float dy)
{
    if (expectedScrollAxis_ == Axis::VERTICAL) {
        return dy < 0 ? ScrollDirection::DOWN : ScrollDirection::UP;
    } else if (expectedScrollAxis_ == Axis::HORIZONTAL) {
        return dx < 0 ? ScrollDirection::RIGHT : ScrollDirection::LEFT;
    }
    return ScrollDirection::UNKNOWN;
}

bool WebPattern::IsScrollReachEdge(
    const ScrollDirection& direction, bool isMinX, bool isMaxX, bool isMinY, bool isMaxY)
{
    switch (direction) {
        case ScrollDirection::LEFT:
            return isMinX;
        case ScrollDirection::RIGHT:
            return isMaxX;
        case ScrollDirection::UP:
            return isMinY;
        case ScrollDirection::DOWN:
            return isMaxY;
        default:
            return false;
    }
}

void WebPattern::SetDirectionMode()
{
    mode_ = NestedScrollMode::SELF_ONLY;
    const auto& nestedMode = GetNestedScrollExt();
    if (direction_ == ScrollDirection::UP) {
        mode_ = nestedMode.scrollUp;
    } else if (direction_ == ScrollDirection::DOWN) {
        mode_ = nestedMode.scrollDown;
    } else if (direction_ == ScrollDirection::LEFT) {
        mode_ = nestedMode.scrollLeft;
    } else if (direction_ == ScrollDirection::RIGHT) {
        mode_ = nestedMode.scrollRight;
    }
}

void WebPattern::JavaScriptOnDocumentStart(const ScriptItems& scriptItems)
{
    // cross platform is not support now;
}

void WebPattern::UpdateJavaScriptOnDocumentStart()
{
    // cross platform is not support now;
}

void WebPattern::JavaScriptOnDocumentEnd(const ScriptItems& scriptItems)
{
    // cross platform is not support now;
}

void WebPattern::JavaScriptOnDocumentStartByOrder(const ScriptItems& scriptItems,
    const ScriptItemsByOrder& scriptItemsByOrder)
{
    // cross platform is not support now;
}

void WebPattern::JavaScriptOnDocumentEndByOrder(const ScriptItems& scriptItems,
    const ScriptItemsByOrder& scriptItemsByOrder)
{
    // cross platform is not support now;
}

void WebPattern::JavaScriptOnHeadReadyByOrder(const ScriptItems& scriptItems,
    const ScriptItemsByOrder& scriptItemsByOrder)
{
    // cross platform is not support now;
}

void WebPattern::OnOverScrollModeUpdate(int mode)
{
   // cross platform is not support now;
}

void WebPattern::OnBlurOnKeyboardHideModeUpdate(int mode)
{
   // cross platform is not support now;
}

void WebPattern::OnCopyOptionModeUpdate(int32_t mode)
{
    // cross platform is not support now;
}

void WebPattern::OnTextAutosizingUpdate(bool isTextAutosizing)
{
    // cross platform is not support now;
}

void WebPattern::UpdateRelativeOffset()
{
    // cross platform is not support now;
}

void WebPattern::InitSlideUpdateListener()
{
    // cross platform is not support now;
}

void WebPattern::UpdateSlideOffset(bool isNeedReset)
{
    // cross platform is not support now;
}

bool WebPattern::Backward()
{
    return false;
}

void WebPattern::CalculateHorizontalDrawRect()
{
   // cross platform is not support now;
}

void WebPattern::CalculateVerticalDrawRect()
{
    // cross platform is not support now;
}

void WebPattern::PostTaskToUI(const std::function<void()>&& task) const
{
    // cross platform is not support now;
}

void WebPattern::SetDrawRect(int32_t x, int32_t y, int32_t width, int32_t height)
{
    // cross platform is not support now;
}

RefPtr<NodePaintMethod> WebPattern::CreateNodePaintMethod()
{
    // cross platform is not support now;
    return nullptr;
}

void WebPattern::OnDefaultTextEncodingFormatUpdate(const std::string& value)
{
    // cross platform is not support now;
}

void  WebPattern::OnSelectionMenuOptionsUpdate(const WebMenuOptionsParam& webMenuOption)
{
    // cross platform is not support now;
}

void WebPattern::OnBlankScreenDetectionConfigUpdate(const BlankScreenDetectionConfig &config)
{
    // cross platform is not support now;
}

void WebPattern::OnNativeVideoPlayerConfigUpdate(const std::tuple<bool, bool>& config)
{
    // cross platform is not support now;
}

void WebPattern::OnOverlayScrollbarEnabledUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnIntrinsicSizeEnabledUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnCssDisplayChangeEnabledUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnNativeEmbedRuleTagUpdate(const std::string& tag)
{
    // cross platform is not support now;
}

void WebPattern::OnNativeEmbedRuleTypeUpdate(const std::string& type)
{
    // cross platform is not support now;
}

void WebPattern::OnMetaViewportUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnKeyboardAvoidModeUpdate(const WebKeyboardAvoidMode& mode)
{
    // cross platform is not support now;
}


void WebPattern::UpdateEditMenuOptions(const NG::OnCreateMenuCallback&& onCreateMenuCallback,
    const NG::OnMenuItemClickCallback&& onMenuItemClick, const NG::OnPrepareMenuCallback&& onPrepareMenuCallback)
{
    // cross platform is not support now;
}

void WebPattern::UpdateDataDetectorConfig(const TextDetectConfig &config)
{
    // cross platform is not support now;
}

void WebPattern::UpdateEnableSelectDataDetector(bool isEnabled)
{
    // cross platform is not support now;
}

void WebPattern::UpdateSelectedDataDetectorConfig(const TextDetectConfig &config)
{
    // cross platform is not support now;
}

void WebPattern::OnEnabledHapticFeedbackUpdate(bool enable)
{
    // cross platform is not support now;
}

void WebPattern::OnBypassVsyncConditionUpdate(WebBypassVsyncCondition condition)
{
    // cross platform is not support now;
}

void WebPattern::StartVibraFeedback(const std::string& vibratorType)
{
    // cross platform is not support now;
}

void WebPattern::SetPreviewSelectionMenu(const std::shared_ptr<WebPreviewSelectionMenuParam>& param)
{
    // cross platform is not support now;
}

std::shared_ptr<WebPreviewSelectionMenuParam> WebPattern::GetPreviewSelectionMenuParams(
    const WebElementType& type, const ResponseType& responseType)
{
    // cross platform is not support now;
    return nullptr;
}

bool WebPattern::IsPreviewMenuNotNeedShowPreview()
{
    // cross platform is not support now;
    return false;
}

bool WebPattern::NotifyStartDragTask(bool isDelayed)
{
    // cross platform is not support now;
    return false;
}

void WebPattern::OnContextMenuShow(const std::shared_ptr<BaseEventInfo>& info, bool isRichtext, bool result)
{
    // cross platform is not support now;
}

void WebPattern::RemovePreviewMenuNode()
{
    // cross platform is not support now;
}

void WebPattern::UpdateImagePreviewParam()
{
    // cross platform is not support now;
}

bool WebPattern::RunJavascriptAsync(const std::string& jsCode, std::function<void(const std::string&)>&& callback)
{
    // cross platform is not support now;
    return false;
}

void WebPattern::OnOptimizeParserBudgetEnabledUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnWebMediaAVSessionEnabledUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::OnEnableDataDetectorUpdate(bool enable)
{
    // cross platform is not support now;
}

void WebPattern::OnEnableFollowSystemFontWeightUpdate(bool value)
{
    // cross platform is not support now;
}

void WebPattern::SetDefaultBackgroundColor()
{
    // cross platform is not support now;
}

void WebPattern::OnGestureFocusModeUpdate(GestureFocusMode mode)
{
    // cross platform is not support now;
}

void WebPattern::OnRotateRenderEffectUpdate(WebRotateEffect effect)
{
    // cross platform is not support now;
}

void WebPattern::OnForceEnableZoomUpdate(bool isEnabled)
{
    // cross platform is not support now;
}

void WebPattern::OnBackToTopUpdate(bool isBackToTop)
{
    // cross platform is not support now;
}

void WebPattern::OnZoomControlAccessUpdate(bool zoomControlAccess)
{
    // cross platform is not support now;
}
} // namespace OHOS::Ace::NG
