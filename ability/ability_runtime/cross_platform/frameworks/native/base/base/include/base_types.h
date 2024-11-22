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
#ifndef OHOS_ABILITY_BASE_BASE_TYPES_H
#define OHOS_ABILITY_BASE_BASE_TYPES_H

#include <stdint.h>
#include <string.h>

namespace OHOS {
namespace AAFwk {
using HANDLE = uintptr_t;
using byte = char;
using zchar = char32_t;

using Uuid = struct Uuid {
    static constexpr int UUID_SUB5_SIZE = 12;

    unsigned int mData1;
    unsigned short mData2;
    unsigned short mData3;
    unsigned short mData4;
    unsigned char mData5[UUID_SUB5_SIZE];

    static const struct Uuid Empty;
};

using InterfaceID = Uuid;
using ClassID = Uuid;

inline bool operator==(const InterfaceID &iid1, /* [in] */
    const InterfaceID &iid2)                    /* [in] */
{
    return !memcmp(&iid1, &iid2, sizeof(InterfaceID));
}

inline bool operator!=(const InterfaceID &iid1, /* [in] */
    const InterfaceID &iid2)                    /* [in] */
{
    return memcmp(&iid1, &iid2, sizeof(InterfaceID));
}
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_BASE_TYPES_H
