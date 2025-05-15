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

#include "cross_platform/external_window.h"
#include "cross_platform/surface_utils.h"

using namespace OHOS;
int32_t OH_NativeWindow_CreateNativeWindowFromSurfaceId(uint64_t surfaceId, OHNativeWindow **window)
{
    if (window == nullptr) {
        return OHOS::SURFACE_ERROR_INVALID_PARAM;
    }

    auto &utils = SurfaceUtils::GetInstance();
    *window = reinterpret_cast<OHNativeWindow *>(utils.GetNativeWindow(surfaceId));
    if (*window != nullptr) {
        return OHOS::GSERROR_OK;
    }
    return OHOS::SURFACE_ERROR_INVALID_PARAM;
}