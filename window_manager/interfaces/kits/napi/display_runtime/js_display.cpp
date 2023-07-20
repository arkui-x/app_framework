/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <map>
#include "js_display.h"
#include "display.h"
#include "hilog.h"

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

static thread_local std::map<DisplayId, std::shared_ptr<NativeReference>> g_JsDisplayMap;
std::recursive_mutex g_jsDisplayMutex;

JsDisplay::JsDisplay(const sptr<Display>& display) : display_(display)
{
}

JsDisplay::~JsDisplay()
{
    HILOG_INFO("JsDisplay::~JsDisplay is called");
}

void JsDisplay::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsDisplay::Finalizer is called");
    auto jsDisplay = std::unique_ptr<JsDisplay>(static_cast<JsDisplay*>(data));
    if (jsDisplay == nullptr) {
        HILOG_ERROR("JsDisplay::Finalizer jsDisplay is null");
        return;
    }
    sptr<Display> display = jsDisplay->display_;
    if (display == nullptr) {
        HILOG_ERROR("JsDisplay::Finalizer display is null");
        return;
    }
    DisplayId displayId = display->GetId();
    HILOG_INFO("JsDisplay::Finalizer displayId : %{public}" PRIu64"", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
    if (g_JsDisplayMap.find(displayId) != g_JsDisplayMap.end()) {
        HILOG_INFO("JsDisplay::Finalizer Display is destroyed: %{public}" PRIu64"", displayId);
        g_JsDisplayMap.erase(displayId);
    }
}

std::shared_ptr<NativeReference> FindJsDisplayObject(DisplayId displayId)
{
    HILOG_INFO("[NAPI]Try to find display %{public}" PRIu64" in g_JsDisplayMap", displayId);
    std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
    if (g_JsDisplayMap.find(displayId) == g_JsDisplayMap.end()) {
        HILOG_INFO("[NAPI]Can not find display %{public}" PRIu64" in g_JsDisplayMap", displayId);
        return nullptr;
    }
    return g_JsDisplayMap[displayId];
}

NativeValue* CreateJsDisplayObject(NativeEngine& engine, sptr<Display>& display)
{
    HILOG_INFO("CreateJsDisplay is called");
    NativeValue* objValue = nullptr;
    std::shared_ptr<NativeReference> jsDisplayObj = FindJsDisplayObject(display->GetId());
    if (jsDisplayObj != nullptr && jsDisplayObj->Get() != nullptr) {
        HILOG_INFO("[NAPI]FindJsDisplayObject %{public}" PRIu64"", display->GetId());
        objValue = jsDisplayObj->Get();
    }
    if (objValue == nullptr) {
        objValue = engine.CreateObject();
    }
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to convert prop to jsObject");
        return engine.CreateUndefined();
    }
    sptr<DisplayInfo> info = display->GetDisplayInfo();
    if (info == nullptr) {
        HILOG_ERROR("Failed to GetDisplayInfo");
        return engine.CreateUndefined();
    }
    object->SetProperty("id", CreateJsValue(engine, static_cast<uint32_t>(info->GetDisplayId())));
    object->SetProperty("width", CreateJsValue(engine, info->GetWidth()));
    object->SetProperty("height", CreateJsValue(engine, info->GetHeight()));
    object->SetProperty("orientation", CreateJsValue(engine, info->GetDisplayOrientation()));
    if (jsDisplayObj == nullptr || jsDisplayObj->Get() == nullptr) {
        std::unique_ptr<JsDisplay> jsDisplay = std::make_unique<JsDisplay>(display);
        object->SetNativePointer(jsDisplay.release(), JsDisplay::Finalizer, nullptr);
        std::shared_ptr<NativeReference> jsDisplayRef;
        jsDisplayRef.reset(engine.CreateReference(objValue, 1));
        DisplayId displayId = display->GetId();
        std::lock_guard<std::recursive_mutex> lock(g_jsDisplayMutex);
        g_JsDisplayMap[displayId] = jsDisplayRef;
    }
    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
