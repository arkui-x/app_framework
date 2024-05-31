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

#ifndef FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_PROFILE_H
#define FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_PROFILE_H

#include "appexecfwk_errors.h"
#include "inner_bundle_info.h"

namespace OHOS {
namespace AppExecFwk {
enum OverlayType : int32_t {
    OVERLAY_INTERNAL_BUNDLE = 1,
    OVERLAY_EXTERNAL_BUNDLE,
    NON_OVERLAY_TYPE,
};

struct OverlayMsg {
    OverlayType type = NON_OVERLAY_TYPE;
    bool isAppPriorityExisted = false;
    bool isModulePriorityExisted = false;
};

class ModuleProfile {
public:
    ErrCode TransformTo(const std::vector<uint8_t>& buf, InnerBundleInfo& innerBundleInfo) const;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // FOUNDATION_APPEXECFWK_SERVICES_BUNDLEMGR_INCLUDE_MODULE_PROFILE_H
