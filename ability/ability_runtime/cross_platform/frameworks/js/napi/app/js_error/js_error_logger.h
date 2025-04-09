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

#ifndef OHOS_ABILITY_RUNTIME_JS_ERROR_LOGGER_H
#define OHOS_ABILITY_RUNTIME_JS_ERROR_LOGGER_H

#include <vector>

#include "ierror_observer.h"

namespace OHOS {
namespace HiviewDFX {
    class AppEventPack;
}
namespace AppExecFwk {
    struct ErrorObject;
}
namespace AbilityRuntime {
struct JsErrorInfo {
    std::string appRunningId;
    int32_t pid;
    int32_t uid;
    std::string bundleName;
    std::string versionName;
};

class JsErrorLogger {
public:
    static void SendExceptionToJsError(const JsErrorInfo& info, const AppExecFwk::ErrorObject& object, bool foreground);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ERROR_LOGGER_H
