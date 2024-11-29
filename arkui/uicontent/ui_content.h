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

#ifndef FOUNDATION_APPFRAMEWORK_UICONTENT_ACE_UI_CONTENT_H
#define FOUNDATION_APPFRAMEWORK_UICONTENT_ACE_UI_CONTENT_H

#include <memory>
#include <string>
#include <vector>

#include "interfaces/inner_api/ace/viewport_config.h"
#include "core/accessibility/accessibility_node.h"
#include "foundation/appframework/arkui/uicontent/component_info.h"

namespace OHOS::Ace {
struct TouchEvent;
}

namespace OHOS {
namespace AbilityRuntime::Platform {
class Context;
class Configuration;
class Ability;
}

namespace Rosen {
class Window;
enum class WindowSizeChangeReason : uint32_t;
enum class WindowMode : uint32_t;
} // namespace Rosen

namespace AAFwk {
class Want;
}

namespace MMI {
class PointerEvent;
class KeyEvent;
class AxisEvent;
}

namespace AppExecFwk{
struct AbilityInfo;
}
} // namespace OHOS

class NativeEngine;
typedef struct napi_value__* napi_value;

namespace OHOS::Ace::Platform {
class UIContent {
public:
    static std::unique_ptr<UIContent> Create(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime);
    static void ShowDumpHelp(std::vector<std::string>& info);

    static void AddUIContent(int32_t instanceId, UIContent* content);
    static void RemoveUIContent(int32_t instanceId);

    virtual ~UIContent() = default;

    // UI content life-cycles
    virtual void Initialize(OHOS::Rosen::Window* window, const std::string& url, napi_value storage) = 0;
    virtual void InitializeByName(OHOS::Rosen::Window* window, const std::string& name, napi_value storage) = 0;
    virtual void Foreground() = 0;
    virtual void Background() = 0;
    virtual void Focus() = 0;
    virtual void UnFocus() = 0;
    virtual void Destroy() = 0;
    virtual void OnNewWant(const OHOS::AAFwk::Want& want) = 0;
    virtual void Finish() = 0;

    // UI content event process
    virtual bool ProcessBackPressed() = 0;
    virtual bool ProcessBasicEvent(const std::vector<TouchEvent>& touchEvents) = 0;
    virtual bool ProcessPointerEvent(const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent) = 0;
    virtual bool ProcessMouseEvent(const std::vector<uint8_t>& data) = 0;
    virtual bool ProcessKeyEvent(int32_t keyCode, int32_t keyAction, int32_t repeatTime, int64_t timeStamp = 0,
        int64_t timeStampStart = 0, int32_t metaKey = 0, int32_t sourceDevice = 0, int32_t deviceId = 0,
        std::string msg = "") = 0;

    // surface and resregister
    virtual void NotifySurfaceCreated() = 0;
    virtual void NotifySurfaceDestroyed() = 0;
    virtual void UpdateViewportConfig(const ViewportConfig& config, OHOS::Rosen::WindowSizeChangeReason reason) = 0;

    // config
    virtual void UpdateConfiguration(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config) = 0;
    
    // Window color
    virtual uint32_t GetBackgroundColor() = 0;
    virtual void SetBackgroundColor(uint32_t color) = 0;

    // Control filtering
    virtual bool GetAllComponents(NodeId nodeID, OHOS::Ace::Platform::ComponentInfo& components) = 0;

    virtual void DumpInfo(const std::vector<std::string>& params, std::vector<std::string>& info) = 0;

    // Set UIContent callback for custom window animation
    virtual void SetNextFrameLayoutCallback(std::function<void()>&& callback) = 0;

    // Receive memory level notification
    virtual void NotifyMemoryLevel(int32_t level) = 0;

    virtual napi_value GetUINapiContext()
    {
        napi_value result = nullptr;
        return result;
    }

    virtual bool ProcessPointerEventWithCallback(
        const std::shared_ptr<OHOS::MMI::PointerEvent>& pointerEvent, const std::function<void()>& callback)
    {
        return true;
    };
#if defined(SUPPORT_TOUCH_TARGET_TEST)
    virtual bool ProcessPointerEventTargetHitTest(const std::shared_ptr<OHOS::MMI::PointerEvent> &pointerEvent,
        const std::string &target) = 0;
#endif
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_APPFRAMEWORK_UICONTENT_ACE_UI_CONTENT_H