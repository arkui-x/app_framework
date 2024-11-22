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

#include "long_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Long, Object, ILong);

ErrCode Long::GetValue(long &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Long::Equals(IObject &other) /* [in] */
{
    Long *otherObj = static_cast<Long *>(ILong::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Long::ToString()
{
    return std::to_string(value_);
}

sptr<ILong> Long::Box(long value) /* [in] */
{
    sptr<ILong> object = new Long(value);
    return object;
}

long Long::Unbox(ILong *object) /* [in] */
{
    long value;
    object->GetValue(value);
    return value;
}

sptr<ILong> Long::Parse(const std::string &str) /* [in] */
{
    sptr<ILong> object;
    std::size_t idx;
    long value = std::stol(str, &idx);
    if (idx != 0) {
        object = new Long(value);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS