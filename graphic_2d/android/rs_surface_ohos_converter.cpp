/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "platform/common/rs_log.h"
#include "platform/drawing/rs_surface_converter.h"
#include "rs_surface_android.h"

namespace OHOS::Rosen {
sptr<Surface> RSSurfaceConverter::ConvertToOhosSurface(std::shared_ptr<RSSurface> surface)
{
    if (surface == nullptr) {
        ROSEN_LOGE("nullptr input");
        return nullptr;
    }

    auto derivedPtr = std::static_pointer_cast<RSSurfaceAndroid>(surface);
    return derivedPtr->GetSurface();
}
} // namespace OHOS::Rosen
