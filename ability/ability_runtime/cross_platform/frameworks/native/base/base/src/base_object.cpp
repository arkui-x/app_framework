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

#include "base_obj.h"

#include <cstdio>

#include "securec.h"

#include "light_refbase.h"

namespace OHOS {
namespace AAFwk {
const InterfaceID g_IID_IObject = {
    0x8321f710, 0xa0c0, 0x4cbe, 0xbfbc, {0x5, 0xa, 0x7, 0x8, 0xf, 0x1, 0x3, 0x1, 0x2, 0xb, 0x1, 0xb}
};

const InterfaceID g_IID_IWeakReference = {
    0x26ab1978, 0x1d11, 0x4a4f, 0x826d, {0x6, 0x1, 0x7, 0x8, 0x5, 0xc, 0x0, 0x4, 0x8, 0xc, 0xc, 0xa}
};

const InterfaceID g_IID_IWeakReferenceSource = {
    0xbc3f5250, 0x34d7, 0x42d2, 0x9b40, {0xf, 0xf, 0xc, 0xe, 0x8, 0x3, 0xf, 0xd, 0x4, 0x0, 0x6, 0x1}
};

class WeakReferenceImpl final : public LightRefCountBase, public IWeakReference {
public:
    WeakReferenceImpl(IInterface *object, WeakRefCounter *wkRef) : object_(object), wkRef_(wkRef)
    {}

    virtual ~WeakReferenceImpl()
    {}

    void IncStrongRef(const void *id = nullptr) override;

    void DecStrongRef(const void *id = nullptr) override;

    IInterface *Query(const InterfaceID &iid) override;

    InterfaceID GetInterfaceID(IInterface *object) override;

    ErrCode Resolve(const InterfaceID &iid, IInterface **object) override;

private:
    IInterface *object_ = nullptr;
    WeakRefCounter *wkRef_ = nullptr;
};

void WeakReferenceImpl::IncStrongRef(const void *id)
{
    LightRefCountBase::IncStrongRef(id);
}

void WeakReferenceImpl::DecStrongRef(const void *id)
{
    LightRefCountBase::DecStrongRef(id);
}

IInterface *WeakReferenceImpl::Query(const InterfaceID &iid)
{
    if ((iid == g_IID_IWeakReference) || (iid == g_IID_IInterface)) {
        return static_cast<IWeakReference *>(this);
    }
    return nullptr;
}

InterfaceID WeakReferenceImpl::GetInterfaceID(IInterface *object)
{
    if (object == static_cast<IWeakReference *>(this)) {
        return g_IID_IWeakReference;
    }
    return InterfaceID::Empty;
}

ErrCode WeakReferenceImpl::Resolve(const InterfaceID &iid, IInterface **object)
{
    VALIDATE_NOT_NULL(object);

    if ((object_ != nullptr) && wkRef_->AttemptIncStrongRef(this)) {
        *object = object_->Query(iid);
        if (*object == nullptr) {
            object_->DecStrongRef(this);
        }
    } else {
        *object = nullptr;
    }
    return ERR_OK;
}

void Object::IncStrongRef(const void *id)
{
    RefBase::IncStrongRef(id);
}

void Object::DecStrongRef(const void *id)
{
    RefBase::DecStrongRef(id);
}

IInterface *Object::Query(const InterfaceID &iid)
{
    if (iid == g_IID_IObject) {
        return static_cast<IObject *>(this);
    }
    if (iid == g_IID_IWeakReferenceSource) {
        return static_cast<IWeakReferenceSource *>(this);
    }
    if (iid == g_IID_IInterface) {
        return static_cast<IWeakReferenceSource *>(this);
    }
    return nullptr;
}

InterfaceID Object::GetInterfaceID(IInterface *object)
{
    if (object == static_cast<IObject *>(this)) {
        return g_IID_IObject;
    }
    if (object == static_cast<IWeakReferenceSource *>(this)) {
        return g_IID_IWeakReferenceSource;
    }
    return InterfaceID::Empty;
}

ClassID Object::GetClassID()
{
    return ClassID::Empty;
}

int Object::GetHashCode()
{
    return reinterpret_cast<HANDLE>(this);
}

bool Object::Equals(IObject &other)
{
    Object *otherObj = static_cast<Object *>(&other);
    return this == otherObj;
}

ErrCode Object::GetWeakReference(sptr<IWeakReference> &weakRef)
{
    void *addr = calloc(sizeof(WeakReferenceImpl), 1);
    if (addr == nullptr) {
        return ERR_NO_MEMORY;
    }
    weakRef = new (addr) WeakReferenceImpl(static_cast<IObject *>(this), CreateWeakRef(addr));
    return ERR_OK;
}

std::string Object::ToString()
{
    static constexpr int BUFFER_MAX = 1024;
    std::string str("");
    char *buf = new (std::nothrow) char[BUFFER_MAX];
    if (buf != nullptr) {
        int ret = snprintf_s(buf, BUFFER_MAX, BUFFER_MAX - 1, "[Object %p]", this);
        if (ret >= 0) {
            str = buf;
        }
        delete[] buf;
        buf = nullptr;
    }
    return str;
}

bool Object::Equals(IInterface &obj1, IInterface &obj2)
{
    if (IInterface::Query(&obj1) == IInterface::Query(&obj2)) {
        return true;
    }

    IObject *o1 = IObject::Query(obj1);
    if (o1 == nullptr) {
        return false;
    }
    return o1->Equals(*(IObject::Query(obj2)));
}

std::string Object::ToString(IInterface &object)
{
    IObject *obj = IObject::Query(object);
    return ((obj != nullptr) ? obj->ToString() : "");
}
}  // namespace AAFwk
}  // namespace OHOS