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

#include "window_stage.h"

#include "foundation/appframework/arkui/uicontent/ui_content.h"
#include "virtual_rs_window.h"
#include "window_hilog.h"
#include "window_option.h"

namespace OHOS {
namespace Rosen {
WindowStage::~WindowStage() {}

#ifdef IOS_PLATFORM
void WindowStage::Init(const std::shared_ptr<AbilityRuntime::Platform::Context>& context, void* windowView)
{
    mainWindow_ = Window::Create(context, windowView);
    context_ = context;
    return;
}
#else
void WindowStage::Init(
    const std::shared_ptr<AbilityRuntime::Platform::Context>& context, jobject windowStageView, JNIEnv* env)
{
    mainWindow_ = Window::Create(context, env, windowStageView);
    context_ = context;
    return;
}
#endif

const std::shared_ptr<Window>& WindowStage::GetMainWindow() const
{
    return mainWindow_;
}

std::shared_ptr<Window> WindowStage::CreateSubWindow(const std::string& windowName)
{
    if (windowName.empty() || mainWindow_ == nullptr) {
        return nullptr;
    }
    std::shared_ptr<WindowOption> option = std::make_shared<WindowOption>();
    option->SetParentId(mainWindow_->GetWindowId());
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_APP_SUB_WINDOW);
    option->SetWindowMode(Rosen::WindowMode::WINDOW_MODE_FLOATING);
    option->SetWindowName(windowName);
    std::shared_ptr<Window> subWindow = Window::CreateSubWindow(context_, option);
    return subWindow;
}

std::vector<std::shared_ptr<Window>> WindowStage::GetSubWindow()
{
    if (mainWindow_ == nullptr) {
        LOGE("Get sub window failed, because main window is null");
        return std::vector<std::shared_ptr<Window>>();
    }
    return Window::GetSubWindow(mainWindow_->GetWindowId());
}

void WindowStage::UpdateConfigurationForAll(
    const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config)
{
    if (mainWindow_ != nullptr) {
        mainWindow_->UpdateConfiguration(config);
    }
    auto subWindows = GetSubWindow();
    if (subWindows.empty()) {
        return;
    }
    for (const auto& subWindow : subWindows) {
        if (subWindow != nullptr) {
            subWindow->UpdateConfiguration(config);
        }
    }
}
} // namespace Rosen
} // namespace OHOS
