/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_OPTION_H
#define OHOS_ROSEN_WINDOW_OPTION_H
#include <refbase.h>
#include <string>
#include <unordered_map>

#include "wm_common.h"

namespace OHOS {
namespace Rosen {
/**
 * @class WindowOption
 * WindowOption is used to create a window.
 */
class WindowOption : public RefBase {
public:
    /**
     * @brief Default construct of WindowOption.
     */
    WindowOption();

    /**
     * @brief Deconstruct of WindowOption.
     */
    virtual ~WindowOption() = default;

    /**
     * @brief Set window rect.
     *
     * @param rect The rect of window to set window position and size.
     */
    void SetWindowRect(const struct Rect& rect);

     /**
     * @brief Set window tag.
     *
     * @param windowTag The tag of window.
     */
    void SetWindowTag(WindowTag windowTag);

    /**
     * @brief Set parent id.
     *
     * @param parentId The parent window id.
     */
    void SetParentId(uint32_t parentId);

    /**
     * @brief Set window name.
     *
     * @param windowName The window name.
     */
    void SetWindowName(const std::string& windowName);

    /**
     * @brief Set window type.
     *
     * @param type The type of window.
     */
    void SetWindowType(WindowType type);

     /**
     * @brief Set window mode.
     *
     * @param mode The mode of window.
     */
    void SetWindowMode(WindowMode mode);

    /**
     * @brief Set whether the window is modal.
     *
     * @param isModal Whether the window should block parent window interaction.
     */
    void SetWindowModal(bool isModal);

    /**
     * @brief Set window modality type.
     *
     * @param modalityType The modality type of window.
     */
    void SetWindowModalityType(ModalityType modalityType);

    /**
     * @brief Set window z level.
     *
     * @param zLevel The z level of window.
     */
    void SetWindowZLevel(int32_t zLevel);

    /**
     * @brief Set whether sub window supports maximize/fullscreen.
     *
     * @param maximizeSupported Whether maximize/fullscreen is supported.
     */
    void SetWindowMaximizeSupported(bool maximizeSupported);

    /**
     * @brief Set whether sub window outline is enabled.
     *
     * @param outlineEnabled Whether outline is enabled.
     */
    void SetWindowOutlineEnabled(bool outlineEnabled);

    /**
     * @brief Set sub window title.
     *
     * @param title The title of sub window.
     */
    void SetWindowTitle(const std::string& title);

    /**
     * @brief Set whether sub window uses decor view.
     *
     * @param decorEnabled Whether decor view is enabled.
     */
    void SetDecorEnabled(bool decorEnabled);

    /**
     * @brief Get window tag.
     *
     * @return Return window tag.
     */
    WindowTag GetWindowTag() const;

    /**
     * @brief Get parent id.
     *
     * @return Return parent window id.
     */
    uint32_t GetParentId() const;
    
    /**
     * @brief Get window name.
     *
     * @return Return the window name.
     */
    const std::string& GetWindowName() const;

    /**
     * @brief Get window rect.
     *
     * @return The rect of window.
     */
    Rect GetWindowRect() const;

     /**
     * @brief Get window type.
     *
     * @return The type of window.
     */
    WindowType GetWindowType() const;

    /**
     * @brief Get window mode.
     *
     * @return The mode of window.
     */
    WindowMode GetWindowMode() const;

    /**
     * @brief Get whether the window is modal.
     *
     * @return Return true if the window is modal.
     */
    bool IsWindowModal() const;

    /**
     * @brief Get window modality type.
     *
     * @return Return the modality type of window.
     */
    ModalityType GetWindowModalityType() const;

    /**
     * @brief Get window z level.
     *
     * @return Return the z level of window.
     */
    int32_t GetWindowZLevel() const;

    /**
     * @brief Get whether sub window supports maximize/fullscreen.
     *
     * @return Return true if maximize/fullscreen is supported.
     */
    bool IsWindowMaximizeSupported() const;

    /**
     * @brief Get whether sub window outline is enabled.
     *
     * @return Return true if outline is enabled.
     */
    bool IsWindowOutlineEnabled() const;

    /**
     * @brief Get sub window title.
     *
     * @return Return sub window title.
     */
    const std::string& GetWindowTitle() const;

    /**
     * @brief Get whether sub window uses decor view.
     *
     * @return Return true if decor view is enabled.
     */
    bool IsDecorEnabled() const;

private:
    Rect windowRect_ { 0, 0, 0, 0 };
    WindowType type_ { WindowType::WINDOW_TYPE_APP_MAIN_WINDOW };
    WindowMode mode_ { WindowMode::WINDOW_MODE_UNDEFINED };
    bool isModal_ = false;
    ModalityType modalityType_ = ModalityType::WINDOW_MODALITY;
    int32_t zLevel_ = 0;
    bool maximizeSupported_ = false;
    bool outlineEnabled_ = false;
    std::string title_;
    bool decorEnabled_ = false;
    WindowTag windowTag_;
    uint32_t parentId_ = INVALID_WINDOW_ID;
    std::string windowName_ { "" };
};
} // namespace Rosen
} // namespace OHOS
#endif // OHOS_ROSEN_WINDOW_OPTION_H
