/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_DM_DISPLAY_MANAGER_H
#define FOUNDATION_DM_DISPLAY_MANAGER_H

#include <vector>
#include <mutex>

#include "display.h"
#include "dm_common.h"
#include "wm_single_instance.h"

namespace OHOS::Rosen {
class DisplayManager {
WM_DECLARE_SINGLE_INSTANCE_BASE(DisplayManager);
public:
    class DisplayListener : public virtual RefBase {
    public:
        /**
         * Notify when a new display is created.
         */
        virtual void OnCreate(DisplayId) = 0;

        /**
         * Notify when the display is destroyed.
         */
        virtual void OnDestroy(DisplayId) = 0;

        /**
         * Notify when the state of a display changes
         */
        virtual void OnChange(DisplayId) = 0;
    };

    class FoldStatusListener : public virtual RefBase {
    private:
        std::string instanceName;
    public:

        void setInstanceName(const std::string& newInstanceName)
        {
            instanceName = newInstanceName;
        }

        const std::string& getInstanceName() const
        {
            return instanceName;
        }

        /**
         * Notify listeners when screen fold status changed.
         *
         * @param foldStatus Screen foldStatus.
         */
        virtual void OnFoldStatusChanged([[maybe_unused]]FoldStatus foldStatus) {}
    };

    class DisplayModeListener : public virtual RefBase {
    public:
        /**
         * Notify listeners when display mode changed.
         *
         * @param displayMode DisplayMode.
         */
        virtual void OnDisplayModeChanged([[maybe_unused]]FoldDisplayMode displayMode) {}
    };

    /**
     * Register a display listener.
     *
     * @param listener DisplayListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayListener(sptr<DisplayListener> listener);

    /**
     * Unregister an existed display listener.
     *
     * @param listener DisplayListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayListener(sptr<DisplayListener> listener);

    /**
     * Register a listener for the event of screen fold status changed.
     *
     * @param listener FoldStatusListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterFoldStatusListener(sptr<FoldStatusListener> listener);

    /**
     * Unregister an existed listener for the event of screen fold status changed.
     *
     * @param listener FoldStatusListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterFoldStatusListener(sptr<FoldStatusListener> listener);

    /**
     * Register a listener for the event of display mode changed.
     *
     * @param listener DisplayModeListener.
     * @return DM_OK means register success, others means register failed.
     */
    DMError RegisterDisplayModeListener(sptr<DisplayModeListener> listener);

    /**
     * Unregister an existed listener for the event of display mode changed.
     *
     * @param listener DisplayModeListener.
     * @return DM_OK means unregister success, others means unregister failed.
     */
    DMError UnregisterDisplayModeListener(sptr<DisplayModeListener> listener);

    /**
     * Get the default display object by means of sync.
     *
     * @return Default display id.
     */
    sptr<Display> GetDefaultDisplaySync();

    /**
     * Check whether the device is foldable.
     *
     * @return true means the device is foldable.
     */
    bool IsFoldable();

    /**
     * Get the current fold status of the foldable device.
     *
     * @return fold status of device.
     */
    FoldStatus GetFoldStatus();

    /**
     * Get the display mode of the foldable device.
     *
     * @return display mode of the foldable device.
     */
    FoldDisplayMode GetFoldDisplayMode();

    /**
     * Cross platform callback method for display listener.
     *
     * @param notifyType display change type.
     * @param displayId display ID.
     */
    void NotifyDisplayListenerCallback(uint32_t notifyType, uint64_t displayId);

    /**
     *  Cross platform callback method for fold listener.
     *
     * @param instanceName the name of activity instance.
     * @param foldStatus the fold status of device.
     */
    void NotifyFoldStatusChangeCallback(const std::string& instanceName, uint32_t foldStatus);
    
private:
    DisplayManager();
    ~DisplayManager();
    // void OnRemoteDied();
    
    class Impl;
    std::recursive_mutex mutex_;
    bool destroyed_ = false;
    sptr<Impl> pImpl_;
};
} // namespace OHOS::Rosen

#endif // FOUNDATION_DM_DISPLAY_MANAGER_H