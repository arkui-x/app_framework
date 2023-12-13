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

#ifndef IOS_PLATFORM
#include "jni.h"
#endif
#include <memory>
#include "ability_context.h"

namespace OHOS {
namespace AbilityRuntime::Platform {
class Configuration;
}
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

#ifdef IOS_PLATFORM
    /**
     * Init a WindowStage instance based on the parameters displayId, context, listener and option.
     *
     * @param context current ability context
     * @param view the object of the WindowStageView
     * @return void
     */
    void Init(const std::shared_ptr<AbilityRuntime::Platform::Context>& context, void* windowView);
#else
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
#endif

    /**
     * Get shared pointer of main window.
     *
     * @return the shared pointer of window
     */
    const std::shared_ptr<Window>& GetMainWindow() const;
    std::shared_ptr<Window> CreateSubWindow(const std::string& windowName);
    std::vector<std::shared_ptr<Window>> GetSubWindow();

    void UpdateConfigurationForAll(const std::shared_ptr<OHOS::AbilityRuntime::Platform::Configuration>& config);

private:
    std::shared_ptr<Window> mainWindow_;
    std::shared_ptr<AbilityRuntime::Platform::Context> context_ = nullptr;
};
} // namespace Rosen
} // namespace OHOS
#endif // INTERFACES_INNERKITS_WINDOW_STAGE_H
