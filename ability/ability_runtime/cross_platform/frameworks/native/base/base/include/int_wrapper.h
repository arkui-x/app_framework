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
#ifndef OHOS_ABILITY_BASE_INTEGER_WRAPPER_H
#define OHOS_ABILITY_BASE_INTEGER_WRAPPER_H

#include "base_obj.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Integer final : public Object, public IInteger {
public:
    inline Integer(int value) : value_(value)
    {}

    inline ~Integer()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(int &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IInteger> Box(int value); /* [in] */

    static int Unbox(IInteger *object); /* [in] */

    static sptr<IInteger> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'I';

private:
    int value_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_INTEGER_WRAPPER_H