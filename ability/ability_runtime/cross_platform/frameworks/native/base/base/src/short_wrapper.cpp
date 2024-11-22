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

#include "short_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Short, Object, IShort);

ErrCode Short::GetValue(short &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Short::Equals(IObject &other) /* [in] */
{
    Short *otherObj = static_cast<Short *>(IShort::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Short::ToString()
{
    return std::to_string(value_);
}

sptr<IShort> Short::Box(short value) /* [in] */
{
    sptr<IShort> object = new Short(value);
    return object;
}

short Short::Unbox(IShort *object) /* [in] */
{
    short value;
    object->GetValue(value);
    return value;
}

sptr<IShort> Short::Parse(const std::string &str) /* [in] */
{
    sptr<IShort> object;
    std::size_t idx;
    short value = (short)std::stoi(str, &idx);
    if (idx != 0) {
        object = new Short(value);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS