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
#ifndef OHOS_ABILITY_BASE_LONG_WRAPPER_H
#define OHOS_ABILITY_BASE_LONG_WRAPPER_H

#include "base_obj.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Long final : public Object, public ILong {
public:
    inline Long(long value) : value_(value)
    {}

    inline ~Long()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(long &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<ILong> Box(long value); /* [in] */

    static long Unbox(ILong *object); /* [in] */

    static sptr<ILong> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'J';

private:
    long value_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_LONG_WRAPPER_H
