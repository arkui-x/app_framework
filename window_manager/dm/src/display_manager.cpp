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

#include "display_manager.h"

#include <chrono>
#include <cinttypes>

#include "dm_common.h"
#include "display_info.h"
#include "singleton_delegator.h"
#include "hilog.h"

namespace OHOS::Rosen {
namespace {
    const static uint32_t MAX_INTERVAL_US = 5000;
}
WM_IMPLEMENT_SINGLE_INSTANCE(DisplayManager)

class DisplayManager::Impl : public RefBase {
public:
    Impl(std::recursive_mutex& mutex) : mutex_(mutex) {}
    ~Impl();
    static inline SingletonDelegator<DisplayManager> delegator;
    sptr<Display> GetDefaultDisplaySync();
private:
    bool UpdateDisplayInfoLocked(sptr<DisplayInfo>);
    void Clear();

    DisplayId defaultDisplayId_ = DISPLAY_ID_INVALID;
    std::map<DisplayId, sptr<Display>> displayMap_;
    std::recursive_mutex& mutex_;
};

void DisplayManager::Impl::Clear()
{
}

DisplayManager::Impl::~Impl()
{
    Clear();
}

DisplayManager::DisplayManager() : pImpl_(new Impl(mutex_))
{
}

DisplayManager::~DisplayManager()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    destroyed_ = true;
}

sptr<Display> DisplayManager::Impl::GetDefaultDisplaySync()
{
    HILOG_INFO("DisplayManager::Impl::GetDefaultDisplaySync : Start...");
    static std::chrono::steady_clock::time_point lastRequestTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestTime).count();
    if (defaultDisplayId_ != DISPLAY_ID_INVALID && interval < MAX_INTERVAL_US) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = displayMap_.find(defaultDisplayId_);
        if (iter != displayMap_.end()) {
            HILOG_INFO("DisplayManager::Impl::GetDefaultDisplaySync : Get display from display map");
            return displayMap_[defaultDisplayId_];
        }
    }

    HILOG_INFO("DisplayManager::Impl::GetDefaultDisplaySync : allocate new display info");
  
    DisplayInfo *displayInfo = new DisplayInfo();
    if (displayInfo == nullptr) {
        HILOG_INFO("DisplayManager::Impl::GetDefaultDisplaySync : allocate new display info failed!");
        return nullptr;
    }
    
    auto displayId = displayInfo->GetDisplayId();
    HILOG_INFO("DisplayManager::Impl::GetDefaultDisplaySync : get displayId = %{public}d", displayId);
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (!UpdateDisplayInfoLocked(displayInfo)) {
        displayMap_.erase(displayId);
        return nullptr;
    }
    lastRequestTime = currentTime;
    defaultDisplayId_ = displayId;
    return displayMap_[displayId];
}

sptr<Display> DisplayManager::GetDefaultDisplaySync()
{
    return pImpl_->GetDefaultDisplaySync();
}

bool DisplayManager::Impl::UpdateDisplayInfoLocked(sptr<DisplayInfo> displayInfo)
{
    HILOG_INFO("DisplayManager::Impl::UpdateDisplayInfoLocked : Start...");
    if (displayInfo == nullptr) {
        HILOG_ERROR("DisplayManager::Impl::UpdateDisplayInfoLocked : displayInfo is null");
        return false;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    HILOG_DEBUG("DisplayManager::Impl::UpdateDisplayInfoLocked : displayId=%{public}d", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        HILOG_ERROR("DisplayManager::Impl::UpdateDisplayInfoLocked : displayId is invalid.");
        return false;
    }
    auto iter = displayMap_.find(displayId);
    if (iter != displayMap_.end() && iter->second != nullptr) {
        HILOG_DEBUG("get screen in screen map");
        iter->second->UpdateDisplayInfo(displayInfo);
        return true;
    }
    sptr<Display> display = new Display("", displayInfo);
    displayMap_[displayId] = display;
    return true;
}

} // namespace OHOS::Rosen