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

#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace Rosen {
WindowOption::WindowOption(): windowTag_(WindowTag::SYSTEM_WINDOW)
{
}

void WindowOption::SetWindowRect(const struct Rect& rect)
{
    windowRect_ = rect;
}

void WindowOption::SetWindowType(WindowType type)
{
    type_ = type;
}

void WindowOption::SetWindowMode(WindowMode mode)
{
    mode_ = mode;
}

void WindowOption::SetWindowModal(bool isModal)
{
    isModal_ = isModal;
}

void WindowOption::SetWindowModalityType(ModalityType modalityType)
{
    modalityType_ = modalityType;
}

void WindowOption::SetWindowZLevel(int32_t zLevel)
{
    zLevel_ = zLevel;
}

void WindowOption::SetWindowMaximizeSupported(bool maximizeSupported)
{
    maximizeSupported_ = maximizeSupported;
}

void WindowOption::SetWindowOutlineEnabled(bool outlineEnabled)
{
    outlineEnabled_ = outlineEnabled;
}

void WindowOption::SetWindowTitle(const std::string& title)
{
    title_ = title;
}

void WindowOption::SetDecorEnabled(bool decorEnabled)
{
    decorEnabled_ = decorEnabled;
}

void WindowOption::SetParentId(uint32_t parentId)
{
    parentId_ = parentId;
}

void WindowOption::SetWindowName(const std::string& windowName)
{
    windowName_ = windowName;
}

Rect WindowOption::GetWindowRect() const
{
    return windowRect_;
}

WindowType WindowOption::GetWindowType() const
{
    return type_;
}

WindowMode WindowOption::GetWindowMode() const
{
    return mode_;
}

bool WindowOption::IsWindowModal() const
{
    return isModal_;
}

ModalityType WindowOption::GetWindowModalityType() const
{
    return modalityType_;
}

int32_t WindowOption::GetWindowZLevel() const
{
    return zLevel_;
}

bool WindowOption::IsWindowMaximizeSupported() const
{
    return maximizeSupported_;
}

bool WindowOption::IsWindowOutlineEnabled() const
{
    return outlineEnabled_;
}

const std::string& WindowOption::GetWindowTitle() const
{
    return title_;
}

bool WindowOption::IsDecorEnabled() const
{
    return decorEnabled_;
}

uint32_t WindowOption::GetParentId() const
{
    return parentId_;
}

const std::string& WindowOption::GetWindowName() const
{
    return windowName_;
}

void WindowOption::SetWindowTag(WindowTag windowTag)
{
    windowTag_ = windowTag;
}

WindowTag WindowOption::GetWindowTag() const
{
    return windowTag_;
}
} // namespace Rosen
} // namespace OHOS

