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
#ifndef OHOS_ABILITY_BASE_FLOAT_WRAPPER_H
#define OHOS_ABILITY_BASE_FLOAT_WRAPPER_H

#include "base_obj.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Float final : public Object, public IFloat {
public:
    inline Float(float value) : value_(value)
    {}

    inline ~Float()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(float &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IFloat> Box(float value); /* [in] */

    static float Unbox(IFloat *object); /* [in] */

    static sptr<IFloat> Parse(const std::string &str); /* [in] */

public:
    static constexpr char SIGNATURE = 'F';

private:
    float value_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_FLOAT_WRAPPER_H