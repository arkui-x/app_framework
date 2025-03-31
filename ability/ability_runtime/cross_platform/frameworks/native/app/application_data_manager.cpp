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

#include "application_data_manager.h"

namespace OHOS {
namespace AppExecFwk {
ApplicationDataManager& ApplicationDataManager::GetInstance()
{
    static ApplicationDataManager manager;
    return manager;
}

void ApplicationDataManager::AddErrorObserver(const std::shared_ptr<IErrorObserver>& observer)
{
    errorObserver_ = observer;
}

bool ApplicationDataManager::NotifyUnhandledException(const std::string& errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }
    return false;
}

bool ApplicationDataManager::NotifyCJUnhandledException(const std::string& errMsg)
{
    if (errorObserver_) {
        errorObserver_->OnUnhandledException(errMsg);
        return true;
    }
    return false;
}

void ApplicationDataManager::RemoveErrorObserver()
{
    errorObserver_ = nullptr;
}

bool ApplicationDataManager::NotifyExceptionObject(const AppExecFwk::ErrorObject& errorObj)
{
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }
    return false;
}

bool ApplicationDataManager::NotifyCJExceptionObject(const AppExecFwk::ErrorObject& errorObj)
{
    if (errorObserver_) {
        errorObserver_->OnExceptionObject(errorObj);
        return true;
    }
    return false;
}
}  // namespace AppExecFwk
}  // namespace OHOS
