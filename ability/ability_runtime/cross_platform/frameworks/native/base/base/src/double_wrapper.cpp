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

#include "double_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Double, Object, IDouble);

ErrCode Double::GetValue(double &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Double::Equals(IObject &other) /* [in] */
{
    Double *otherObj = static_cast<Double *>(IDouble::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Double::ToString()
{
    return std::to_string(value_);
}

sptr<IDouble> Double::Box(double value) /* [in] */
{
    sptr<IDouble> object = new Double(value);
    return object;
}

double Double::Unbox(IDouble *object) /* [in] */
{
    double value;
    object->GetValue(value);
    return value;
}

sptr<IDouble> Double::Parse(const std::string &str) /* [in] */
{
    sptr<IDouble> object;
    std::size_t idx;
    double value = std::stod(str, &idx);
    if (idx != 0) {
        object = new Double(value);
    }
    return object;
}
}  // namespace AAFwk
}  // namespace OHOS