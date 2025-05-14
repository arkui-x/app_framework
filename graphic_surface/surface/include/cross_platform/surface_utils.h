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

#ifndef FRAMEWORKS_SURFACE_INCLUDE_CROSS_PLATFORM_SURFACE_UTILS_H
#define FRAMEWORKS_SURFACE_INCLUDE_CROSS_PLATFORM_SURFACE_UTILS_H

#include <unordered_map>
#include <mutex>
#include "graphic_common.h"

namespace OHOS {
class SurfaceUtils {
public:
    SurfaceUtils(const SurfaceUtils&) = delete;
    SurfaceUtils& operator=(const SurfaceUtils&) = delete;
    static SurfaceUtils& GetInstance();
    void* GetNativeWindow(uint64_t uniqueId);
    SurfaceError AddNativeWindow(uint64_t uniqueId, void *nativeWindow);
    SurfaceError RemoveNativeWindow(uint64_t uniqueId);

private:
    SurfaceUtils() = default;
    ~SurfaceUtils();
    std::mutex nativeWindowLock_;
    std::unordered_map<uint64_t, void*> nativeWindowCache_;
};
} // namespace OHOS
#endif // FRAMEWORKS_SURFACE_INCLUDE_CROSS_PLATFORM_SURFACE_UTILS_H
