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

    /**
     * @brief Get the default display object by means of sync.
     *
     * @return Default display id.
     */
    sptr<Display> GetDefaultDisplaySync();
    
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