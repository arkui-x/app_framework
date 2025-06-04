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

#ifndef OHOS_ROSEN_WINDOW_INTERFACE_H
#define OHOS_ROSEN_WINDOW_INTERFACE_H

#include <refbase.h>
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
/**
 * @class IWindowLifeCycle
 *
 * @brief IWindowLifeCycle is a listener used to notify caller that lifecycle of window.
 */
class IWindowLifeCycle : virtual public RefBase {
public:
    /**
     * @brief Notify caller that window is on the forground.
     */
    virtual void AfterForeground() {}
    /**
     * @brief Notify caller that window is on the background.
     */
    virtual void AfterBackground() {}
    /**
     * @brief Notify caller that window is focused.
     */
    virtual void AfterFocused() {}
    /**
     * @brief Notify caller that window is unfocused.
     */
    virtual void AfterUnfocused() {}
    /**
     * @brief Notify caller the error code when window go forground failed.
     *
     * @param ret Error code when window go forground failed.
     */
    virtual void ForegroundFailed(int32_t ret) {}
    /**
     * @brief Notify caller that window is active.
     */
    virtual void AfterActive() {}
    /**
     * @brief Notify caller that window is inactive.
     */
    virtual void AfterInactive() {}
};

/**
 * @class IWindowChangeListener
 *
 * @brief IWindowChangeListener is used to observe the window size or window mode when window changed.
 */
class IWindowChangeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window size changed.
     *
     * @param Rect Rect of the current window.
     */
    virtual void OnSizeChange(Rect rect) {}
};

/**
 * @class ITouchOutsideListener
 *
 * @brief ITouchOutsideListener is used to observe touch outside event.
 */
class ITouchOutsideListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller that window is on the forground.
     */
    virtual void OnTouchOutside() {}
};

/**
 * @class IWindowSurfaceNodeListener
 *
 * @brief IWindowSurfaceNodeListener is used to observe the window SurfaceNode event.
 */
class IWindowSurfaceNodeListener : virtual public RefBase {
public:
    /**
     * @brief Notify caller when window size changed.
     */
    virtual void OnSurfaceNodeCreated() {}
    /**
     * @brief Notify caller when window size changed.
     */
    virtual void OnSurfaceNodeChanged(int32_t width, int32_t height, float density) {}
    /**
     * @brief Notify caller when window size changed.
     */
    virtual void OnSurfaceNodeDestroyed() {}
};

class IWindowStatusChangeListener : virtual public RefBase {
    public:
        /**
         * @brief Notify caller when window status changed.
         *
         * @param status Mode of the current window.
         */
        virtual void OnWindowStatusChange(WindowStatus status) {}
    };

class IAvoidAreaChangedListener : virtual public RefBase {
    public:
        /**
         * @brief Notify caller when avoid area size changed.
         *
         * @param avoidArea Area needed to be avoided.
         * @param type Type of avoid area.
         */
        virtual void OnAvoidAreaChanged(const AvoidArea avoidArea, AvoidAreaType type) {}
    };
}
}
#endif // OHOS_ROSEN_WINDOW_INTERFACE_H
