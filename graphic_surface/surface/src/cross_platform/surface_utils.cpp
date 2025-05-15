/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include "cross_platform/surface_utils.h"

namespace OHOS {
SurfaceUtils& SurfaceUtils::GetInstance()
{
    static SurfaceUtils instance;
    return instance;
}

SurfaceUtils::~SurfaceUtils()
{
    nativeWindowCache_.clear();
}

void* SurfaceUtils::GetNativeWindow(uint64_t uniqueId)
{
    std::lock_guard<std::mutex> lockGuard(nativeWindowLock_);
    auto iter = nativeWindowCache_.find(uniqueId);
    if (iter == nativeWindowCache_.end()) {
        return nullptr;
    }
    return iter->second;
}

SurfaceError SurfaceUtils::AddNativeWindow(uint64_t uniqueId, void *nativeWindow)
{
    if (nativeWindow == nullptr) {
        return GSERROR_INVALID_ARGUMENTS;
    }
    std::lock_guard<std::mutex> lockGuard(nativeWindowLock_);
    if (nativeWindowCache_.count(uniqueId) == 0) {
        nativeWindowCache_[uniqueId] = nativeWindow;
        return GSERROR_OK;
    }
    return GSERROR_OK;
}

SurfaceError SurfaceUtils::RemoveNativeWindow(uint64_t uniqueId)
{
    std::lock_guard<std::mutex> lockGuard(nativeWindowLock_);
    nativeWindowCache_.erase(uniqueId);
    return GSERROR_OK;
}
} // namespace OHOS
