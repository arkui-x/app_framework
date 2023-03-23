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

#include "foundation/appframework/arkui/uicontent/ui_content.h"

#include <vector>

#ifdef ANDROID_PLATFORM
#include "adapter/android/stage/uicontent/ui_content_impl.h"
#else
#include "adapter/ios/stage/uicontent/ui_content_impl.h"
#endif

namespace OHOS::Ace::Platform {
UIContent* CreateUIContent(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime)
{
    return new UIContentImpl(context, runtime);
}

std::unique_ptr<UIContent> UIContent::Create(OHOS::AbilityRuntime::Platform::Context* context, NativeEngine* runtime)
{
    std::unique_ptr<UIContent> content;
    content.reset(CreateUIContent(context, runtime));
    return content;
}

void UIContent::ShowDumpHelp(std::vector<std::string>& info)
{
    info.emplace_back(" -element                       |show element tree");
    info.emplace_back(" -render                        |show render tree");
    info.emplace_back(" -inspector                     |show inspector tree");
    info.emplace_back(" -frontend                      |show path and components count of current page");
}
} // namespace OHOS::Ace::Platform 