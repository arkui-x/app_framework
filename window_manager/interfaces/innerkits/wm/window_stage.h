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

#ifndef INTERFACES_INNERKITS_WINDOW_STAGE_H
#define INTERFACES_INNERKITS_WINDOW_STAGE_H

#include "jni.h"

#include <memory>
#include "ability_context.h"

namespace OHOS {
namespace Rosen {
class Window;
class WindowStage {
public:
    /**
     * Default constructor used to create an empty WindowStage instance.
     */
    WindowStage() = default;

    /**
     * Default deconstructor used to deconstruct.
     *
     */
    ~WindowStage();

    /**
     * Init a WindowStage instance based on the parameters displayId, context, listener and option.
     *
     * @param context current ability context
     * @param listener the life cycle listener of the window
     * @param windowStageView the java object of the WindowStageView
     * @return void
     */
    void Init(const std::shared_ptr<AbilityRuntime::Platform::Context>& context,
        jobject windowStageView, JNIEnv* env);

    /**
     * Get shared pointer of main window.
     *
     * @return the shared pointer of window
     */
    const std::shared_ptr<Window>& GetMainWindow() const;

private:
    std::shared_ptr<Window> mainWindow_;
    std::shared_ptr<AbilityRuntime::Platform::Context> context_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // INTERFACES_INNERKITS_WINDOW_STAGE_H
