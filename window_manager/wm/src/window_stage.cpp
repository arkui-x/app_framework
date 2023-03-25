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
#include "virtual_rs_window.h"
#include "foundation/appframework/arkui/uicontent/ui_content.h"

namespace OHOS {
namespace Rosen {
WindowStage::~WindowStage()
{
}

void WindowStage::Init(const std::shared_ptr<AbilityRuntime::Platform::Context>& context,
    jobject windowStageView, JNIEnv* env)
{
    mainWindow_ = Window::Create(context, env, windowStageView);
    context_ = context;
    return;
}

const std::shared_ptr<Window>& WindowStage::GetMainWindow() const
{
    return mainWindow_;
}
} // namespace Rosen
} // namespace OHOS
