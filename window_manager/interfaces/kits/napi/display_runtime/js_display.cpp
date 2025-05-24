/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "js_runtime_utils.h"
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
constexpr size_t ARGC_ONE = 1;
namespace {
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
    napi_set_named_property(env, objValue, "densityPixels", CreateJsValue(env, info->GetDensityPixels()));
    napi_set_named_property(env, objValue, "scaledDensity", CreateJsValue(env, info->GetScaledDensity()));
    napi_set_named_property(env, objValue, "densityDPI", CreateJsValue(env, info->GetDensityDpi()));
    napi_set_named_property(env, objValue, "xDPI", CreateJsValue(env, info->GetXDpi()));
    napi_set_named_property(env, objValue, "yDPI", CreateJsValue(env, info->GetYDpi()));
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
