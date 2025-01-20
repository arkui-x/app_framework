/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_BASE_ARRAY_WRAPPER_H
#define OHOS_ABILITY_BASE_ARRAY_WRAPPER_H

#include <functional>
#include <vector>

#include "base_obj.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {
class Array : public Object, public IArray {
public:
    Array(long size, const InterfaceID &id);

    inline ~Array()
    {}

    IINTERFACE_DECL();

    ErrCode Get(long index,                /* [in] */
        sptr<IInterface> &value) override; /* [out] */

    ErrCode GetLength(long &size) override; /* [out] */

    ErrCode GetType(InterfaceID &id) override; /* [out] */

    ErrCode Set(long index,          /* [in] */
        IInterface *value) override; /* [in] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IArray> Parse(const std::string &arrayStr); /* [in] */

    static bool IsBooleanArray(IArray *array); /* [in] */

    static bool IsCharArray(IArray *array); /* [in] */

    static bool IsByteArray(IArray *array); /* [in] */

    static bool IsShortArray(IArray *array); /* [in] */

    static bool IsIntegerArray(IArray *array); /* [in] */

    static bool IsLongArray(IArray *array); /* [in] */

    static bool IsFloatArray(IArray *array); /* [in] */

    static bool IsDoubleArray(IArray *array); /* [in] */

    static bool IsStringArray(IArray *array); /* [in] */

    static bool IsWantParamsArray(IArray *array); /* [in] */

    static void ForEach(IArray *array,           /* [in] */
        std::function<void(IInterface *)> func); /* [in] */

    static sptr<IArray> ParseCrossPlatformArray(const nlohmann::json& arrayStr); /* [in] */

    static constexpr char SIGNATURE = '[';

private:
    static void ParseElement(IArray *array,                  /* [in] */
        std::function<sptr<IInterface>(std::string &)> func, /* [in] */
        const std::string &values,                           /* [in] */
        long size);                                          /* [in] */

    std::vector<sptr<IInterface>> values_;
    long size_;
    InterfaceID typeId_;

    static sptr<IArray> ParseString(const std::string &values, long size);
    static sptr<IArray> ParseBoolean(const std::string &values, long size);
    static sptr<IArray> ParseByte(const std::string &values, long size);
    static sptr<IArray> ParseShort(const std::string &values, long size);
    static sptr<IArray> ParseInteger(const std::string &values, long size);
    static sptr<IArray> ParseLong(const std::string &values, long size);
    static sptr<IArray> ParseFloat(const std::string &values, long size);
    static sptr<IArray> ParseDouble(const std::string &values, long size);
    static sptr<IArray> ParseChar(const std::string &values, long size);
    static sptr<IArray> ParseArray(const std::string &values, long size);
    static sptr<IArray> ParseWantParams(const std::string &values, long size);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_ARRAY_WRAPPER_H
