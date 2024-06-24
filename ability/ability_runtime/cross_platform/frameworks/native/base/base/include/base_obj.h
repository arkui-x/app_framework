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
#ifndef OHOS_ABILITY_BASE_BASE_OBJECT_H
#define OHOS_ABILITY_BASE_BASE_OBJECT_H

#include "base_interfaces.h"
#include "refbase.h"

namespace OHOS {
namespace AAFwk {
INTERFACE(IObject, 8321f710 - a0c0 - 4cbe-bfbc - 5a78f1312b1b)
{
    inline static IObject *Query(IInterface & object) /* [in] */
    {
        return static_cast<IObject *>(object.Query(g_IID_IObject));
    }

    virtual ClassID GetClassID() = 0;

    virtual int GetHashCode() = 0;

    virtual bool Equals(IObject & other) = 0; /* [in] */

    virtual std::string ToString() = 0;
};

INTERFACE(IWeakReference, 26ab1978 - 1d11 - 4a4f - 826d - 61785c048cca)
{
    inline static IWeakReference *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IWeakReference *>(object->Query(g_IID_IWeakReference));
    }

    virtual ErrCode Resolve(const InterfaceID &iid, /* [in] */
        IInterface **object) = 0;                   /* [out] */
};

INTERFACE(IWeakReferenceSource, bc3f5250 - 34d7 - 42d2 - 9b40 - ffce83fd4061)
{
    inline static IWeakReferenceSource *Query(IInterface * object) /* [in] */
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IWeakReferenceSource *>(object->Query(g_IID_IWeakReferenceSource));
    }

    virtual ErrCode GetWeakReference(sptr<IWeakReference> & weakRef) = 0; /* [out] */
};

class Object : public virtual RefBase, public IObject, public IWeakReferenceSource {
public:
    void IncStrongRef(const void *id = nullptr) override; /* [in] */

    void DecStrongRef(const void *id = nullptr) override; /* [in] */

    IInterface *Query(const InterfaceID &iid) override; /* [in] */

    InterfaceID GetInterfaceID(IInterface *object) override; /* [in] */

    ClassID GetClassID() override;

    int GetHashCode() override;

    bool Equals(IObject &other) override; /* [in] */

    ErrCode GetWeakReference(sptr<IWeakReference> &weakRef) override; /* [out] */

    std::string ToString() override;

    static bool Equals(IInterface &obj1, /* [in] */
        IInterface &obj2);               /* [in] */

    static std::string ToString(IInterface &object); /* [in] */
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_BASE_OBJECT_H
