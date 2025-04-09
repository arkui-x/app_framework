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

#ifndef OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H

#include <string>

#include "ierror_observer.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {
class ApplicationDataManager {
public:
    static ApplicationDataManager& GetInstance();
    void AddErrorObserver(const std::shared_ptr<IErrorObserver>& observer);
    bool NotifyUnhandledException(const std::string& errMsg);
    bool NotifyCJUnhandledException(const std::string& errMsg);
    void RemoveErrorObserver();
    bool NotifyExceptionObject(const AppExecFwk::ErrorObject& errorObj);
    bool NotifyCJExceptionObject(const AppExecFwk::ErrorObject& errorObj);

private:
    ApplicationDataManager() = default;
    ~ApplicationDataManager() = default;
    DISALLOW_COPY_AND_MOVE(ApplicationDataManager);
    std::shared_ptr<IErrorObserver> errorObserver_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_APPLICATION_DATA_MANAGER_H
