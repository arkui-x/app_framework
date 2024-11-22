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

#include "int_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Integer, Object, IInteger);

ErrCode Integer::GetValue(int &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Integer::Equals(IObject &other) /* [in] */
{
    Integer *otherObj = static_cast<Integer *>(IInteger::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Integer::ToString()
{
    return std::to_string(value_);
}

sptr<IInteger> Integer::Box(int value) /* [in] */
{
    sptr<IInteger> object = new Integer(value);
    return object;
}

int Integer::Unbox(IInteger *object) /* [in] */
{
    int value;
    object->GetValue(value);
    return value;
}

sptr<IInteger> Integer::Parse(const std::string &str) /* [in] */
{
    sptr<IInteger> object;
    std::size_t idx;
    int value = std::stoi(str, &idx);
    if (idx != 0) {
        object = new Integer(value);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS