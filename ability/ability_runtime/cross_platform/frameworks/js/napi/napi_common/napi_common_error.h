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

#ifndef OHOS_ABILITY_RUNTIME_NAPI_COMMON_ERROR_H
#define OHOS_ABILITY_RUNTIME_NAPI_COMMON_ERROR_H

#include <string>

namespace OHOS {
namespace AppExecFwk {
#define NAPI_ERR_NO_ERROR 0
#define NAPI_ERR_NO_WINDOW (-1)
#define NAPI_ERR_NO_PERMISSION (-100)
#define NAPI_ERR_INNER_DATA (-101)           // Internal data error, data allocation fail
#define NAPI_ERR_ACE_ABILITY (-102)          // Failed to acquire ability object
#define NAPI_ERR_LONG_CALLBACK (-103)        // Failed to acquire long callback object
#define NAPI_ERR_PARAM_INVALID (-104)        // Parameter is invalid
#define NAPI_ERR_ABILITY_TYPE_INVALID (-105) // Type of ability is invalid
#define NAPI_ERR_ABILITY_CALL_INVALID (-106) // Call the ability interface, the return value is wrong
} // namespace AppExecFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_NAPI_COMMON_ERROR_H
