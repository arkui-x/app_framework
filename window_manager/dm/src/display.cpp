/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "display.h"

#include <cstdint>
#include <new>
#include <refbase.h>

#include "class_var_definition.h"
#include "singleton_container.h"
#include "window_hilog.h"

namespace OHOS::Rosen {

class Display::Impl : public RefBase {
public:
    Impl(const std::string& name, sptr<DisplayInfo> info)
    {
        name_= name;
        displayInfo_ = info;
    }
    ~Impl() = default;
    DEFINE_VAR_FUNC_GET_SET(std::string, Name, name);
    DEFINE_VAR_FUNC_GET_SET_WITH_LOCK(sptr<DisplayInfo>, DisplayInfo, displayInfo);
};

Display::Display(const std::string& name, sptr<DisplayInfo> info)
    : pImpl_(new Impl(name, info))
{
}

Display::~Display()
{
}

DisplayId Display::GetId() const
{
    return pImpl_->GetDisplayInfo()->GetDisplayId();
}

void Display::UpdateDisplayInfo(sptr<DisplayInfo> displayInfo) const
{
    if (displayInfo == nullptr) {
        WLOGE("displayInfo is invalid");
        return;
    }
    pImpl_->SetDisplayInfo(displayInfo);
}

void Display::UpdateDisplayInfo() const
{
}

sptr<DisplayInfo> Display::GetDisplayInfo() const
{
    UpdateDisplayInfo();
    return pImpl_->GetDisplayInfo();
}
} // namespace OHOS::Rosen
