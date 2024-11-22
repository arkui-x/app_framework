/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "string_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(String, Object, IString);

ErrCode String::GetString(std::string &str) /* [out] */
{
    VALIDATE_NOT_NULL(&str);

    str = string_;
    return ERR_OK;
}

bool String::Equals(IObject &other) /* [in] */
{
    String *otherObj = static_cast<String *>(IString::Query(&other));
    return otherObj != nullptr && otherObj->string_ == string_;
}

std::string String::ToString()
{
    return string_;
}

sptr<IString> String::Box(const std::string &str) /* [in] */
{
    sptr<IString> object = new String(str);
    return object;
}

std::string String::Unbox(IString *object) /* [in] */
{
    std::string str;
    object->GetString(str);
    return str;
}

sptr<IString> String::Parse(const std::string &str) /* [in] */
{
    sptr<IString> object = new String(str);
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS