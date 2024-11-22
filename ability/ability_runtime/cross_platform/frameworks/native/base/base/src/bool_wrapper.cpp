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

#include "bool_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Boolean, Object, IBoolean);
ErrCode Boolean::GetValue(bool &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Boolean::Equals(IObject &other) /* [in] */
{
    Boolean *otherObj = static_cast<Boolean *>(IBoolean::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Boolean::ToString()
{
    return value_ ? "true" : "false";
}

sptr<IBoolean> Boolean::Box(bool value) /* [in] */
{
    sptr<IBoolean> object = new Boolean(value);
    return object;
}

bool Boolean::Unbox(IBoolean *object) /* [in] */
{
    bool value = false;
    object->GetValue(value);
    return value;
}

sptr<IBoolean> Boolean::Parse(const std::string &str) /* [in] */
{
    sptr<IBoolean> object;
    if (str == "true") {
        object = new Boolean(true);
    } else if (str == "false") {
        object = new Boolean(false);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS