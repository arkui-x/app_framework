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
#ifndef OHOS_ABILITY_BASE_BASE_IINTERFACES_H
#define OHOS_ABILITY_BASE_BASE_IINTERFACES_H
#include <memory>
#include <string>

#include "base_def.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
INTERFACEID(IInterface)
INTERFACE_ID(00000000 - 0000 - 0000 - 0000 - 000000000001)
INTERFACE_INNER IInterface
{
    inline static IInterface *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return object->Query(g_IID_IInterface);
    }

    virtual void IncStrongRef(const void *id = nullptr) = 0; /* [in] */

    virtual void DecStrongRef(const void *id = nullptr) = 0; /* [in] */

    virtual IInterface *Query(const InterfaceID &iid) = 0; /* [in] */

    virtual InterfaceID GetInterfaceID(IInterface * object) = 0; /* [in] */

    virtual ~IInterface() {}
};

INTERFACE(IBoolean, 492ef6c0 - e122 - 401d - 80c4 - bb65e2325766)
{
    inline static IBoolean *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IBoolean *>(object->Query(g_IID_IBoolean));
    }

    virtual ErrCode GetValue(bool &value) = 0; /* [out] */
};

INTERFACE(IChar, 6da72e7c - d353 - 4d7b - 85ef - 2146add34219)
{
    inline static IChar *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IChar *>(object->Query(g_IID_IChar));
    }

    virtual ErrCode GetValue(zchar & value) = 0; /* [out] */
};

INTERFACE(IByte, 2c9fd6ff - 73f7 - 4a22 - 9ea8 - 1e37b2213ca1)
{
    inline static IByte *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IByte *>(object->Query(g_IID_IByte));
    }

    virtual ErrCode GetValue(byte & value) = 0; /* [out] */
};

INTERFACE(IShort, 54fe1291 - 0813 - 43c9 - b6f7 - b008edef7f34)
{
    inline static IShort *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IShort *>(object->Query(g_IID_IShort));
    }

    virtual ErrCode GetValue(short &value) = 0; /* [out] */
};

INTERFACE(IInteger, 0672d282 - 7cdb - 49b8 - 92c7 - 1af35e2d4949)
{
    inline static IInteger *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IInteger *>(object->Query(g_IID_IInteger));
    }

    virtual ErrCode GetValue(int &value) = 0; /* [out] */
};

INTERFACE(ILong, d7550828 - 2eaf - 4281 - 8604 - 42a82ab9bcdc)
{
    inline static ILong *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<ILong *>(object->Query(g_IID_ILong));
    }

    virtual ErrCode GetValue(long &value) = 0; /* [out] */
};

INTERFACE(IFloat, b5428638 - ca53 - 4a27 - 95a0 - 3f24e54a58d5)
{
    inline static IFloat *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IFloat *>(object->Query(g_IID_IFloat));
    }

    virtual ErrCode GetValue(float &value) = 0; /* [out] */
};

INTERFACE(IDouble, 596dc69a - ae7d - 42ad - b00f - 0d5eb01a2557)
{
    inline static IDouble *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IDouble *>(object->Query(g_IID_IDouble));
    }

    virtual ErrCode GetValue(double &value) = 0; /* [out] */
};

INTERFACE(IString, 69644bf4 - dd20 - 417b - 9055 - 5a26e3e1793b)
{
    inline static IString *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IString *>(object->Query(g_IID_IString));
    }

    virtual ErrCode GetString(std::string & str) = 0; /* [out] */
};

INTERFACE(IArray, 875b9da6 - 9913 - 4370 - 8847 - e1961be6e560)
{
    inline static IArray *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IArray *>(object->Query(g_IID_IArray));
    }

    virtual ErrCode Get(long index,   /* [in] */
        sptr<IInterface> &value) = 0; /* [out] */

    virtual ErrCode GetLength(long &size) = 0; /* [out] */

    virtual ErrCode GetType(InterfaceID & id) = 0; /* [out] */

    virtual ErrCode Set(long index, /* [in] */
        IInterface *value) = 0;     /* [in] */
};

INTERFACE(IPacMap, f92066fd - fd0c - 401b - a3f6 - 626da3bac9d5)
{
    inline static IPacMap *Query(IInterface * object)
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IPacMap *>(object->Query(g_IID_IPacMap));
    }

    virtual bool FromString(const std::string &str) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_BASE_IINTERFACES_H
