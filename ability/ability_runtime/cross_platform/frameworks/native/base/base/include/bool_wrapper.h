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
#ifndef OHOS_ABILITY_BASE_BOOLEAN_WRAPPER_H
#define OHOS_ABILITY_BASE_BOOLEAN_WRAPPER_H

#include "base_obj.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Boolean final : public Object, public IBoolean {
public:
    inline Boolean(bool value) : value_(value)
    {}

    inline ~Boolean()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(bool &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IBoolean> Box(bool value); /* [in] */

    static bool Unbox(IBoolean *object); /* [in] */

    static sptr<IBoolean> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'Z';

private:
    bool value_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_BOOLEAN_WRAPPER_H
