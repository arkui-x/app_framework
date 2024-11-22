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

#include "zchar_wrapper.h"

namespace OHOS {
namespace AAFwk {
IINTERFACE_IMPL_1(Char, Object, IChar);

ErrCode Char::GetValue(zchar &value) /* [out] */
{
    VALIDATE_NOT_NULL(&value);

    value = value_;
    return ERR_OK;
}

bool Char::Equals(IObject &other) /* [in] */
{
    Char *otherObj = static_cast<Char *>(IChar::Query(&other));
    return otherObj != nullptr && otherObj->value_ == value_;
}

std::string Char::ToString()
{
    constexpr int MAX_BYTE_SIZE = 4;

    int byteSize = GetByteSize(value_);
    char buf[MAX_BYTE_SIZE + 1];
    WriteUTF8Bytes(buf, value_, byteSize);
    buf[byteSize] = '\0';
    return std::string(buf);
}

sptr<IChar> Char::Box(zchar value) /* [in] */
{
    sptr<IChar> object = new Char(value);
    return object;
}

zchar Char::Unbox(IChar *object) /* [in] */
{
    zchar value;
    object->GetValue(value);
    return value;
}

sptr<IChar> Char::Parse(const std::string &str) /* [in] */
{
    return Box(GetChar(str, 0));
}

int Char::GetByteSize(zchar c) /* [in] */
{
    if ((c > MAX_CODE_POINT) || (c >= MIN_HIGH_SURROGATE && c <= MAX_LOW_SURROGATE)) {
        return 0;
    }

    int bsize = BYTE_COUNT_4;
    if (c < 0x00000080) {
        bsize = BYTE_COUNT_1;
    } else if (c < 0x00000800) {
        bsize = BYTE_COUNT_2;
    } else if (c < 0x00010000) {
        bsize = BYTE_COUNT_3;
    }
    return bsize;
}

void Char::WriteUTF8Bytes(char *dst, /* [in] */
    zchar c,                         /* [in] */
    int size)                        /* [in] */
{
    uint32_t uc = static_cast<uint32_t>(c);
    dst += size;
    switch (size) {
        /* note: everything falls through. */
        case BYTE_COUNT_4: {
            *(--dst) = ((uc | BYTE_MARK) & BYTE_MASK);
            uc >>= BYTE_SHIFT;
        }
            [[clang::fallthrough]];
        case BYTE_COUNT_3: {
            *(--dst) = ((uc | BYTE_MARK) & BYTE_MASK);
            uc >>= BYTE_SHIFT;
        }
            [[clang::fallthrough]];
        case BYTE_COUNT_2: {
            *(--dst) = ((uc | BYTE_MARK) & BYTE_MASK);
            uc >>= BYTE_SHIFT;
        }
            [[clang::fallthrough]];
        case BYTE_COUNT_1: {
            *(--dst) = (uc | FIRST_BYTE_MARK[size]);
        }
            [[clang::fallthrough]];
        default:
            break;
    }
}

zchar Char::GetChar(const std::string &str, /* [in] */
    int index)                              /* [in] */
{
    if (str.empty() || index < 0) {
        return INVALID_CHAR;
    }

    int bsize;
    const char *p = str.c_str();
    const char *end = p + str.length() + 1;
    while (*p && p < end) {
        zchar unicode = GetCharInternal((unsigned char *)p, bsize);
        if (bsize == 0 || p + bsize >= end) {
            break;
        }
        
        if (index == 0) {
            return unicode;
        }
        p += bsize;
        index -= 1;
    }

    return INVALID_CHAR;
}

zchar Char::GetCharInternal(const unsigned char *cur, /* [in] */
    int &size)                                        /* [in] */
{
    if (isascii(*cur) != 0) {
        size = 1;
        return *cur;
    }

    const unsigned char firstChar = *cur++;
    zchar result = firstChar;
    zchar mask, ignoreMask;
    unsigned int num2Read;
    for (num2Read = 1, mask = 0x40, ignoreMask = 0xFFFFFF80; (firstChar & mask);
         num2Read++, ignoreMask |= mask, mask >>= 1) {
        result = (result << BYTE_SHIFT) + (*cur++ & 0x3F);
    }
    ignoreMask |= mask;
    result &= ~(ignoreMask << (BYTE_SHIFT * (num2Read - 1)));
    size = static_cast<int>(num2Read);
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS