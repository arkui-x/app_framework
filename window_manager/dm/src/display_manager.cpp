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

#include "display_manager.h"

#include <chrono>
#include <cinttypes>

#include "dm_common.h"
#include "display_info.h"
#include "display_manager_agent.h"
#include "singleton_delegator.h"
#include "window_hilog.h"

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
    bool IsFoldable();
    FoldStatus GetFoldStatus();
    FoldDisplayMode GetFoldDisplayMode();
    DMError RegisterDisplayListener(sptr<DisplayListener> listener);
    DMError UnregisterDisplayListener(sptr<DisplayListener> listener);
    DMError RegisterFoldStatusListener(sptr<FoldStatusListener> listener);
    DMError UnregisterFoldStatusListener(sptr<FoldStatusListener> listener);
    DMError RegisterDisplayModeListener(sptr<DisplayModeListener> listener);
    DMError UnregisterDisplayModeListener(sptr<DisplayModeListener> listener);
    void NotifyDisplayListenerCallback(uint32_t notifyType, uint64_t displayId);
    void NotifyFoldStatusChangeCallback(const std::string& instanceName, uint32_t foldStatus);
private:
    bool UpdateDisplayInfoLocked(sptr<DisplayInfo>);
    void Clear();

    DisplayId defaultDisplayId_ = DISPLAY_ID_INVALID;
    std::map<DisplayId, sptr<Display>> displayMap_;
    std::recursive_mutex& mutex_;
    std::set<sptr<DisplayListener>> displayListeners_;
    std::set<sptr<FoldStatusListener>> foldStatusListeners_;
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
    WLOGI("DisplayManager::Impl::GetDefaultDisplaySync : Start...");
    static std::chrono::steady_clock::time_point lastRequestTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastRequestTime).count();
    if (defaultDisplayId_ != DISPLAY_ID_INVALID && interval < MAX_INTERVAL_US) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        auto iter = displayMap_.find(defaultDisplayId_);
        if (iter != displayMap_.end()) {
            WLOGI("DisplayManager::Impl::GetDefaultDisplaySync : Get display from display map");
            return displayMap_[defaultDisplayId_];
        }
    }

    WLOGI("DisplayManager::Impl::GetDefaultDisplaySync : allocate new display info");
  
    DisplayInfo *displayInfo = new DisplayInfo();
    if (displayInfo == nullptr) {
        WLOGI("DisplayManager::Impl::GetDefaultDisplaySync : allocate new display info failed!");
        return nullptr;
    }
    
    auto displayId = displayInfo->GetDisplayId();
    WLOGI("DisplayManager::Impl::GetDefaultDisplaySync : get displayId = %{public}d", displayId);
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
    WLOGI("DisplayManager::Impl::UpdateDisplayInfoLocked : Start...");
    if (displayInfo == nullptr) {
        WLOGE("DisplayManager::Impl::UpdateDisplayInfoLocked : displayInfo is null");
        return false;
    }
    DisplayId displayId = displayInfo->GetDisplayId();
    WLOGD("DisplayManager::Impl::UpdateDisplayInfoLocked : displayId=%{public}d", displayId);
    if (displayId == DISPLAY_ID_INVALID) {
        WLOGE("DisplayManager::Impl::UpdateDisplayInfoLocked : displayId is invalid.");
        return false;
    }
    auto iter = displayMap_.find(displayId);
    if (iter != displayMap_.end() && iter->second != nullptr) {
        WLOGD("get screen in screen map");
        iter->second->UpdateDisplayInfo(displayInfo);
        return true;
    }
    sptr<Display> display = new Display("", displayInfo);
    displayMap_[displayId] = display;
    return true;
}

bool DisplayManager::IsFoldable()
{
    return pImpl_->IsFoldable();
}

bool DisplayManager::Impl::IsFoldable()
{
    return DisplayManagerAgent::GetInstance()->IsFoldable();
}

FoldStatus DisplayManager::GetFoldStatus()
{
    return pImpl_->GetFoldStatus();
}

FoldStatus DisplayManager::Impl::GetFoldStatus()
{
    uint32_t foldStatus = DisplayManagerAgent::GetInstance()->GetFoldStatus();
    return static_cast<FoldStatus>(foldStatus);
}

FoldDisplayMode DisplayManager::GetFoldDisplayMode()
{
    return pImpl_->GetFoldDisplayMode();
}

FoldDisplayMode DisplayManager::Impl::GetFoldDisplayMode()
{
    return FoldDisplayMode::UNKNOWN;
}

DMError DisplayManager::RegisterDisplayListener(sptr<DisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGE("RegisterDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayListener(listener);
}

DMError DisplayManager::Impl::RegisterDisplayListener(sptr<DisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    DisplayManagerAgent::GetInstance()->RegisterDisplayListener();
    displayListeners_.insert(listener);
    return DMError::DM_OK;
}

DMError DisplayManager::UnregisterDisplayListener(sptr<DisplayListener> listener)
{
    if (listener == nullptr) {
        WLOGE("UnregisterDisplayListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayListener(sptr<DisplayListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(displayListeners_.begin(), displayListeners_.end(), listener);
    if (iter == displayListeners_.end()) {
        WLOGE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    displayListeners_.erase(iter);
    if (displayListeners_.empty()) {
        DisplayManagerAgent::GetInstance()->UnregisterDisplayListener();
    }
    return DMError::DM_OK;
}

DMError DisplayManager::RegisterFoldStatusListener(sptr<FoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGE("FoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterFoldStatusListener(listener);
}

DMError DisplayManager::Impl::RegisterFoldStatusListener(sptr<FoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::string instanceName = DisplayManagerAgent::GetInstance()->RegisterFoldStatusListener();
    listener->setInstanceName(instanceName);
    foldStatusListeners_.insert(listener);
    return DMError::DM_OK;
}

DMError DisplayManager::UnregisterFoldStatusListener(sptr<FoldStatusListener> listener)
{
    if (listener == nullptr) {
        WLOGE("UnregisterFoldStatusListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterFoldStatusListener(listener);
}

DMError DisplayManager::Impl::UnregisterFoldStatusListener(sptr<FoldStatusListener> listener)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto iter = std::find(foldStatusListeners_.begin(), foldStatusListeners_.end(), listener);
    if (iter == foldStatusListeners_.end()) {
        WLOGE("could not find this listener");
        return DMError::DM_ERROR_NULLPTR;
    }
    foldStatusListeners_.erase(iter);
    if (foldStatusListeners_.empty()) {
        DisplayManagerAgent::GetInstance()->UnRegisterFoldStatusListener();
    }
    return DMError::DM_OK;
}

DMError DisplayManager::RegisterDisplayModeListener(sptr<DisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGE("DisplayModeListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->RegisterDisplayModeListener(listener);
}

DMError DisplayManager::Impl::RegisterDisplayModeListener(sptr<DisplayModeListener> listener)
{
    return DMError::DM_OK;
}

DMError DisplayManager::UnregisterDisplayModeListener(sptr<DisplayModeListener> listener)
{
    if (listener == nullptr) {
        WLOGE("UnregisterPrivateWindowListener listener is nullptr.");
        return DMError::DM_ERROR_NULLPTR;
    }
    return pImpl_->UnregisterDisplayModeListener(listener);
}

DMError DisplayManager::Impl::UnregisterDisplayModeListener(sptr<DisplayModeListener> listener)
{
    return DMError::DM_OK;
}

void DisplayManager::NotifyDisplayListenerCallback(uint32_t notifyType, uint64_t displayId)
{
    pImpl_->NotifyDisplayListenerCallback(notifyType, displayId);
}

void DisplayManager::Impl::NotifyDisplayListenerCallback(uint32_t notifyType, uint64_t displayId)
{
    std::set<sptr<DisplayListener>> displayListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        displayListeners = displayListeners_;
    }
    for (auto listener : displayListeners) {
        switch (notifyType) {
            case static_cast<int32_t>(DisplayChangeType::ADD):
                listener->OnCreate(displayId);
                break;
            case static_cast<int32_t>(DisplayChangeType::REMOVE):
                listener->OnDestroy(displayId);
                break;
            case static_cast<int32_t>(DisplayChangeType::CHANGE):
                listener->OnChange(displayId);
                break;
            default:
                WLOGE("The display listener return type is unknown.");
                break;
        }
    }
}

void DisplayManager::NotifyFoldStatusChangeCallback(const std::string& instanceName, uint32_t foldStatus)
{
    pImpl_->NotifyFoldStatusChangeCallback(instanceName, foldStatus);
}

void DisplayManager::Impl::NotifyFoldStatusChangeCallback(const std::string& instanceName, uint32_t foldStatus)
{
    std::set<sptr<FoldStatusListener>> foldStatusListeners;
    {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        foldStatusListeners = foldStatusListeners_;
    }
    for (auto& listener : foldStatusListeners) {
        if (listener->getInstanceName() == instanceName) {
            listener->OnFoldStatusChanged(static_cast<FoldStatus>(foldStatus));
        }
    }
}

} // namespace OHOS::Rosen