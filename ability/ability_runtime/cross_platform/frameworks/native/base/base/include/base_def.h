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
#ifndef OHOS_ABILITY_BASE_BASE_DEF_H
#define OHOS_ABILITY_BASE_BASE_DEF_H

#include "base_types.h"
#include "errors.h"

namespace OHOS {
namespace AAFwk {
#define INTERFACE_INNER struct

#define REFCOUNT_ADD(i)      \
    if (i) {                 \
        (i)->IncStrongRef(); \
    }

#define REFCOUNT_RELEASE(i)  \
    if (i) {                 \
        (i)->DecStrongRef(); \
    }

#define VALIDATE_NOT_NULL(i)      \
    if ((i) == nullptr) {         \
        return ERR_INVALID_VALUE; \
    }

#define INTERFACEID(id) extern const InterfaceID g_IID_##id;

#define INTERFACE(id, x)                 \
    extern const InterfaceID g_IID_##id; \
    INTERFACE_INNER id : public IInterface

#define INTERFACE2(id, pid, x)               \
    do {                                     \
        extern const InterfaceID g_IID_##id; \
        INTERFACE_INNER id : public##pid     \
    } while (0);

#define CLASS(id, x)                   \
    do {                               \
        extern const ClassID CID_##id; \
        class id : public Object       \
    } while (0);

#define INTERFACE_ID(x)
#define CLASS_ID(x)

#ifndef IINTERFACE_DECL
#define IINTERFACE_DECL()                                 \
    void IncStrongRef(const void *id = nullptr) override; \
                                                          \
    void DecStrongRef(const void *id = nullptr) override; \
                                                          \
    IInterface *Query(const InterfaceID & iid) override;  \
                                                          \
    InterfaceID GetInterfaceID(IInterface *object) override;

#endif

#ifndef IINTERFACE_IMPL_1
#define IINTERFACE_IMPL_1(ClassName, SuperclassName, InterfaceName) \
    void ClassName::IncStrongRef(const void *id)                    \
    {                                                               \
        Object::IncStrongRef(id);                                   \
    }                                                               \
                                                                    \
    void ClassName::DecStrongRef(const void *id)                    \
    {                                                               \
        Object::DecStrongRef(id);                                   \
    }                                                               \
                                                                    \
    IInterface *ClassName::Query(const InterfaceID & iid)           \
    {                                                               \
        if (iid == g_IID_##InterfaceName) {                         \
            return static_cast<InterfaceName *>(this);              \
        }                                                           \
        if (iid == g_IID_IInterface) {                              \
            return static_cast<InterfaceName *>(this);              \
        }                                                           \
        return SuperclassName::Query(iid);                          \
    }                                                               \
                                                                    \
    InterfaceID ClassName::GetInterfaceID(IInterface *object)       \
    {                                                               \
        if (object == static_cast<InterfaceName *>(this)) {         \
            return g_IID_##InterfaceName;                           \
        }                                                           \
        return SuperclassName::GetInterfaceID(object);              \
    }
#endif

#ifndef IINTERFACE_IMPL_2
#define IINTERFACE_IMPL_2(ClassName, SuperclassName, InterfaceName1, InterfaceName2) \
    void ClassName::IncStrongRef(const void *id)                                     \
    {                                                                                \
        Object::IncStrongRef(id);                                                    \
    }                                                                                \
                                                                                     \
    void ClassName::DecStrongRef(const void *id)                                     \
    {                                                                                \
        Object::DecStrongRef(id);                                                    \
    }                                                                                \
    IInterface *ClassName::Query(const InterfaceID & iid)                            \
    {                                                                                \
        if (iid == g_IID_##InterfaceName1) {                                         \
            return static_cast<InterfaceName1 *>(this);                              \
        }                                                                            \
        if (iid == g_IID_##InterfaceName2) {                                         \
            return static_cast<InterfaceName2 *>(this);                              \
        }                                                                            \
        if (iid == g_IID_IInterface) {                                               \
            return static_cast<InterfaceName1 *>(this);                              \
        }                                                                            \
        return SuperclassName::Query(iid);                                           \
    }                                                                                \
    InterfaceID ClassName::GetInterfaceID(IInterface *object)                        \
    {                                                                                \
        if (object == static_cast<InterfaceName1 *>(this)) {                         \
            return g_IID_##InterfaceName1;                                           \
        }                                                                            \
        if (object == static_cast<InterfaceName2 *>(this)) {                         \
            return g_IID_##InterfaceName2;                                           \
        }                                                                            \
        return SuperclassName::GetInterfaceID(object);                               \
    }
#endif

#ifndef OBJECT_DECL
#define OBJECT_DECL() ClassID GetClassID() override;
#endif

#ifndef OBJECT_IMPL
#define OBJECT_IMPL(ClassName)           \
    do {                                 \
        ClassID(ClassName)::GetClassID() \
        {                                \
            return CID_##ClassName;      \
        }                                \
    } while (0);
#endif
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_BASE_DEF_H
