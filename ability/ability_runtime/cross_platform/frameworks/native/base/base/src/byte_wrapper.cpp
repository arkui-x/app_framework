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

#include "byte_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Byte, Object, IByte);

ErrCode Byte::GetValue(byte &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Byte::Equals(IObject &other) /* [in] */
{
    Byte *otherObj = static_cast<Byte *>(IByte::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Byte::ToString()
{
    return std::to_string(value_);
}

sptr<IByte> Byte::Box(byte value) /* [in] */
{
    sptr<IByte> object = new Byte(value);
    return object;
}

byte Byte::Unbox(IByte *object) /* [in] */
{
    byte value;
    object->GetValue(value);
    return value;
}

sptr<IByte> Byte::Parse(const std::string &str) /* [in] */
{
    sptr<IByte> object;
    std::size_t idx;
    byte value = (byte)std::stoi(str, &idx);
    if (idx != 0) {
        object = new Byte(value);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS