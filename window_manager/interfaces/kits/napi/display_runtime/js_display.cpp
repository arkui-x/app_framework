/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include "js_display.h"

#include <cinttypes>
#include <map>

#include "display.h"
#include "display_manager_agent.h"
#include "js_runtime_utils.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
namespace {
    constexpr size_t RECT_VALUES_COUNT = 4;
    constexpr size_t RECT_LEFT_OFFSET = 0;
    constexpr size_t RECT_TOP_OFFSET = 1;
    constexpr size_t RECT_WIDTH_OFFSET = 2;
    constexpr size_t RECT_HEIGHT_OFFSET = 3;
    const std::map<DisplayState,      DisplayStateMode> NATIVE_TO_JS_DISPLAY_STATE_MAP {
        { DisplayState::UNKNOWN,      DisplayStateMode::STATE_UNKNOWN      },
        { DisplayState::OFF,          DisplayStateMode::STATE_OFF          },
        { DisplayState::ON,           DisplayStateMode::STATE_ON           },
        { DisplayState::DOZE,         DisplayStateMode::STATE_DOZE         },
        { DisplayState::DOZE_SUSPEND, DisplayStateMode::STATE_DOZE_SUSPEND },
        { DisplayState::VR,           DisplayStateMode::STATE_VR           },
        { DisplayState::ON_SUSPEND,   DisplayStateMode::STATE_ON_SUSPEND   },
    };
}

static thread_local std::map<DisplayId, napi_ref> g_JsDisplayMap;
std::recursive_mutex g_jsDisplayMutex;

static napi_ref FindJsDisplayObject(DisplayId displayId)
{
    WLOGD("[NAPI]Try to find display %{public}" PRIu64" in g_JsDisplayMap", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
    if (g_JsDisplayMap.find(displayId) == g_JsDisplayMap.end()) {
        WLOGI("[NAPI]Can not find display %{public}" PRIu64"", displayId);
        return nullptr;
    }
    return g_JsDisplayMap[displayId];
}

JsDisplay::JsDisplay(const sptr<Display>& display) : display_(display)
{
}

JsDisplay::~JsDisplay()
{
    WLOGI("JsDisplay::~JsDisplay is called");
}

napi_value JsDisplay::GetLiveCreaseRegion(napi_env env, napi_callback_info info)
{
    JsDisplay* me = CheckParamsAndGetThis<JsDisplay>(env, info);
    return (me != nullptr) ? me->OnGetLiveCreaseRegion(env, info) : nullptr;
}

napi_value JsDisplay::OnGetLiveCreaseRegion(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_ONE;
    napi_value argv[ARGC_ONE] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    if (argc >= ARGC_ONE) {
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_INVALID_PARAM)));
        return CreateJsUndefined(env);
    }

    auto regionValues = DisplayManagerAgent::GetInstance()->GetLiveCreaseRegion();
    napi_value result = nullptr;
    napi_status status = napi_create_object(env, &result);
    if (status != napi_ok || result == nullptr) {
        WLOGE("OnGetLiveCreaseRegion: failed to create result object");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));
        return CreateJsUndefined(env);
    }

    const size_t rectCount = regionValues.size() / RECT_VALUES_COUNT;
    napi_value creaseRects = nullptr;
    status = napi_create_array_with_length(env, rectCount, &creaseRects);
    if (status != napi_ok) {
        WLOGE("OnGetLiveCreaseRegion: failed to create creaseRects array");
        napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));
        return CreateJsUndefined(env);
    }
    for (size_t index = 0; index < rectCount; index++) {
        const size_t baseIndex = index * RECT_VALUES_COUNT;
        napi_value rect = nullptr;
        status = napi_create_object(env, &rect);
        if (status != napi_ok) {
            WLOGE("OnGetLiveCreaseRegion: failed to create rect object at index %{public}zu", index);
            napi_throw(env, CreateJsError(env, static_cast<int32_t>(DmErrorCode::DM_ERROR_SYSTEM_INNORMAL)));
            return CreateJsUndefined(env);
        }
        napi_set_named_property(env, rect, "left", CreateJsValue(env, regionValues[baseIndex + RECT_LEFT_OFFSET]));
        napi_set_named_property(env, rect, "top", CreateJsValue(env, regionValues[baseIndex + RECT_TOP_OFFSET]));
        napi_set_named_property(env, rect, "width", CreateJsValue(env, regionValues[baseIndex + RECT_WIDTH_OFFSET]));
        napi_set_named_property(env, rect, "height", CreateJsValue(env, regionValues[baseIndex + RECT_HEIGHT_OFFSET]));
        napi_set_element(env, creaseRects, index, rect);
    }
    napi_set_named_property(env, result, "displayId", CreateJsValue(env, static_cast<int64_t>(display_->GetId())));
    napi_set_named_property(env, result, "creaseRects", creaseRects);
    return result;
}

void JsDisplay::Finalizer(napi_env env, void* data, void* hint)
{
    WLOGI("JsDisplay::Finalizer is called");
    auto jsDisplay = std::unique_ptr<JsDisplay>(static_cast<JsDisplay*>(data));
    if (jsDisplay == nullptr) {
        WLOGE("JsDisplay::Finalizer jsDisplay is null");
        return;
    }
    sptr<Display> display = jsDisplay->display_;
    if (display == nullptr) {
        WLOGE("JsDisplay::Finalizer display is null");
        return;
    }
    DisplayId displayId = display->GetId();
    WLOGI("JsDisplay::Finalizer displayId : %{public}" PRIu64"", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
    auto it = g_JsDisplayMap.find(displayId);
    if (it != g_JsDisplayMap.end()) {
        WLOGI("JsDisplay::Finalizer Display is destroyed: %{public}" PRIu64"", displayId);
        napi_delete_reference(env, it->second);
        g_JsDisplayMap.erase(it);
    }
}

napi_value CreateJsDisplayObject(napi_env env, sptr<Display>& display)
{
    napi_value objValue = nullptr;
    napi_ref jsDisplayRef = FindJsDisplayObject(display->GetId());
    if (jsDisplayRef != nullptr) {
        WLOGI("[NAPI]FindJsDisplayObject %{public}" PRIu64"", display->GetId());
        napi_status status = napi_get_reference_value(env, jsDisplayRef, &objValue);
        if (status != napi_ok) {
            return nullptr;
        }
    } else {
        WLOGI("CreateJsDisplay %{public}" PRIu64"", display->GetId());
        // create new obj
        napi_status status = napi_create_object(env, &objValue);
        if (status != napi_ok) {
            WLOGE("Failed to create object for %{public}" PRIu64"", display->GetId());
            return nullptr;
        }
        std::unique_ptr<JsDisplay> jsDisplay = std::make_unique<JsDisplay>(display);
        napi_wrap(env, objValue, jsDisplay.release(), JsDisplay::Finalizer, nullptr, nullptr);

        // save
        status = napi_create_reference(env, objValue, 1, &jsDisplayRef);
        if (status != napi_ok) {
            WLOGE("Failed to reference for %{public}" PRIu64"", display->GetId());
            return nullptr;
        }
        std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
        g_JsDisplayMap[display->GetId()] = jsDisplayRef;
    }

    sptr<DisplayInfo> info = display->GetDisplayInfo();
    if (info == nullptr) {
        WLOGE("Failed to GetDisplayInfo for %{public}" PRIu64"", display->GetId());
        return nullptr;
    }
    napi_set_named_property(env, objValue, "id", CreateJsValue(env, static_cast<uint32_t>(info->GetDisplayId())));
    napi_set_named_property(env, objValue, "width", CreateJsValue(env, info->GetWidth()));
    napi_set_named_property(env, objValue, "height", CreateJsValue(env, info->GetHeight()));
    napi_set_named_property(env, objValue, "orientation", CreateJsValue(env, info->GetDisplayOrientation()));
    napi_set_named_property(env, objValue, "rotation", CreateJsValue(env, info->GetRotation()));
    napi_set_named_property(env, objValue, "densityPixels", CreateJsValue(env, info->GetDensityPixels()));
    napi_set_named_property(env, objValue, "scaledDensity", CreateJsValue(env, info->GetScaledDensity()));
    napi_set_named_property(env, objValue, "densityDPI", CreateJsValue(env, info->GetDensityDpi()));
    napi_set_named_property(env, objValue, "xDPI", CreateJsValue(env, info->GetXDpi()));
    napi_set_named_property(env, objValue, "yDPI", CreateJsValue(env, info->GetYDpi()));
    BindNativeFunction(env, objValue, "getLiveCreaseRegion", "JsDisplay", JsDisplay::GetLiveCreaseRegion);
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
