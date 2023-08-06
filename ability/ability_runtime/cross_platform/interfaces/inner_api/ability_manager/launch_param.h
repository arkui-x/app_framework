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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_INNER_API_ABILITY_MANAGER_LAUNCH_PARAM_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_INNER_API_ABILITY_MANAGER_LAUNCH_PARAM_H

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
/**
 * @enum LaunchReason
 * LaunchReason defines the reason of launching ability.
 */
enum LaunchReason {
    LAUNCHREASON_UNKNOWN = 0,
    LAUNCHREASON_START_ABILITY,
    LAUNCHREASON_CALL,
    LAUNCHREASON_CONTINUATION,
    LAUNCHREASON_APP_RECOVERY
};

/**
 * @enum LastExitReason
 * LastExitReason defines the reason of last exist.
 */
enum LastExitReason {
    LASTEXITREASON_UNKNOWN = 0,
    LASTEXITREASON_ABILITY_NOT_RESPONDING,
    LASTEXITREASON_NORMAL
};

/**
 * @struct LaunchParam
 * LaunchParam is used to save information about ability launch param.
 */
struct LaunchParam {
    LaunchReason launchReason = LaunchReason::LAUNCHREASON_UNKNOWN;
    LastExitReason lastExitReason = LastExitReason::LASTEXITREASON_NORMAL;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_INNER_API_ABILITY_MANAGER_LAUNCH_PARAM_H
