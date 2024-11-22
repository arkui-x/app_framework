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
#ifndef OHOS_ABILITY_BASE_ZCHAR_WRAPPER_H
#define OHOS_ABILITY_BASE_ZCHAR_WRAPPER_H

#include "base_obj.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
class Char final : public Object, public IChar {
public:
    inline Char(zchar value) : value_(value)
    {}

    inline ~Char()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(zchar &value) override; /* [out] */

    bool Equals(IObject &other) override; /* [in] */

    std::string ToString() override;

    static sptr<IChar> Box(zchar value); /* [in] */

    static zchar Unbox(IChar *object); /* [in] */

    static sptr<IChar> Parse(const std::string &str); /* [in] */

    static int GetByteSize(zchar c); /* [in] */

    static void WriteUTF8Bytes(char *dst, /* [in] */
        zchar c,                          /* [in] */
        int size);                        /* [in] */

    static zchar GetChar(const std::string &str, /* [in] */
        int index);                              /* [in] */

    static constexpr char SIGNATURE = 'C';

    static constexpr zchar INVALID_CHAR = 0x110000;

private:
    static zchar GetCharInternal(const unsigned char *cur, /* [in] */
        int &size);                                        /* [in] */

    static constexpr int MIN_CODE_POINT = 0x000000;
    static constexpr int MAX_CODE_POINT = 0x10FFFF;
    static constexpr int MIN_HIGH_SURROGATE = 0xD800;
    static constexpr int MAX_HIGH_SURROGATE = 0xDBFF;
    static constexpr int MIN_LOW_SURROGATE = 0xDC00;
    static constexpr int MAX_LOW_SURROGATE = 0xDFFF;

    static constexpr int BYTE_COUNT_1 = 1;
    static constexpr int BYTE_COUNT_2 = 2;
    static constexpr int BYTE_COUNT_3 = 3;
    static constexpr int BYTE_COUNT_4 = 4;

    static constexpr zchar BYTE_MASK = 0x000000BF;
    static constexpr zchar BYTE_MARK = 0x00000080;
    static constexpr int BYTE_SHIFT = 6;

    // (00-7f) 7bit -> 0xxxxxxx, bit mask is 0x00000000
    // (c0-df)(80-bf) 11bit -> 110yyyyx 10xxxxxx, bit mask is 0x000000C0
    // (e0-ef)(80-bf)(80-bf) 16bit -> 1110yyyy 10yxxxxx 10xxxxxx, bit mask is 0x000000E0
    // (f0-f7)(80-bf)(80-bf)(80-bf) 21bit -> 11110yyy 10yyxxxx 10xxxxxx 10xxxxxx, bit mask is 0x000000F0
    static constexpr zchar FIRST_BYTE_MARK[] = {0x00000000, 0x00000000, 0x000000C0, 0x000000E0, 0x000000F0};

    zchar value_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif
