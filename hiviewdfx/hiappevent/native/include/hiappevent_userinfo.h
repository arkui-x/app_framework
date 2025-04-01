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
#ifndef FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_USERINFO_H
#define FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_USERINFO_H

#include <string>
#include <unordered_map>

#include "base_type.h"
#include "nocopyable.h"

namespace OHOS {
namespace HiviewDFX {
namespace HiAppEvent {
class UserInfo : public NoCopyable {
public:
    static UserInfo& GetInstance();

    int SetUserId(const std::string& name, const std::string& value);
    int GetUserId(const std::string& name, std::string& out);
    int RemoveUserId(const std::string& name);
    int SetUserProperty(const std::string& name, const std::string& value);
    int GetUserProperty(const std::string& name, std::string& out);
    int RemoveUserProperty(const std::string& name);
    void ClearData();

private:
    UserInfo();
    ~UserInfo() = default;

private:
    std::unordered_map<std::string, std::string> userIds_;
    std::unordered_map<std::string, std::string> userProperties_;
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // FOUNDATION_APPFRAMEWORK_HIVIEWDFX_HIAPPEVENT_NATIVE_INCLUDE_HIAPPEVENT_USERINFO_H
