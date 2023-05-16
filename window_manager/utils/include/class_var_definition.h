/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing p ermissions and
 * limitations under the License.
 */

#ifndef OHOS_ROSEN_CLASS_VAR_DEFINITION_H
#define OHOS_ROSEN_CLASS_VAR_DEFINITION_H

namespace OHOS::Rosen {
#define DEFINE_VAR(type, memberName)                                               \
private:                                                                           \
    type memberName##_;

#define DEFINE_VAR_WITH_LOCK(type, memberName)                                     \
private:                                                                           \
    std::atomic<bool> memberName##atomicBool_ { false };                           \
    type memberName##_;

#define DEFINE_VAR_DEFAULT(type, memberName, defaultValue)                         \
private:                                                                           \
    type memberName##_ {defaultValue};

#define DEFINE_FUNC_GET(type, funcName, memberName)                                \
public:                                                                            \
    type Get##funcName() const                                                     \
    {                                                                              \
        return memberName##_;                                                      \
    }

#define DEFINE_FUNC_GET_WITH_LOCK(type, funcName, memberName)                      \
public:                                                                            \
    type Get##funcName()                                                           \
    {                                                                              \
        bool expect = false;                                                       \
        type res;                                                                  \
        while (!memberName##atomicBool_.compare_exchange_weak(expect, true,        \
            std::memory_order_relaxed)) {                                          \
            expect = false;                                                        \
        }                                                                          \
        res = memberName##_;                                                       \
        memberName##atomicBool_.store(false);                                      \
        return res;                                                                \
    }

#define DEFINE_FUNC_SET(type, funcName, memberName)                                \
public:                                                                            \
    void Set##funcName(type value)                                                 \
    {                                                                              \
        memberName##_ = value;                                                     \
    }

#define DEFINE_FUNC_SET_WITH_LOCK(type, funcName, memberName)                      \
public:                                                                            \
    void Set##funcName(type value)                                                 \
    {                                                                              \
        bool expect = false;                                                       \
        while (!memberName##atomicBool_.compare_exchange_weak(expect, true,        \
            std::memory_order_relaxed)) {                                          \
            expect = false;                                                        \
        }                                                                          \
                                                                                   \
        memberName##_ = value;                                                     \
        memberName##atomicBool_.store(false);                                      \
    }

#define DEFINE_VAR_FUNC_GET(type, funcName, memberName)                            \
    DEFINE_VAR(type, memberName)                                                   \
    DEFINE_FUNC_GET(type, funcName, memberName)

#define DEFINE_VAR_DEFAULT_FUNC_GET(type, funcName, memberName, defaultValue)      \
    DEFINE_VAR_DEFAULT(type, memberName, defaultValue)                             \
    DEFINE_FUNC_GET(type, funcName, memberName)

#define DEFINE_VAR_DEFAULT_FUNC_SET(type, funcName, memberName, defaultValue)      \
    DEFINE_VAR_DEFAULT(type, memberName, defaultValue)                             \
    DEFINE_FUNC_SET(type, funcName, memberName)

#define DEFINE_VAR_FUNC_GET_SET(type, funcName, memberName)                        \
    DEFINE_VAR(type, memberName)                                                   \
    DEFINE_FUNC_GET(type, funcName, memberName)                                    \
    DEFINE_FUNC_SET(type, funcName, memberName)

#define DEFINE_VAR_FUNC_GET_SET_WITH_LOCK(type, funcName, memberName)              \
    DEFINE_VAR_WITH_LOCK(type, memberName)                                         \
    DEFINE_FUNC_GET_WITH_LOCK(type, funcName, memberName)                          \
    DEFINE_FUNC_SET_WITH_LOCK(type, funcName, memberName)

#define DEFINE_VAR_DEFAULT_FUNC_GET_SET(type, funcName, memberName, defaultValue)  \
    DEFINE_VAR_DEFAULT(type, memberName, defaultValue)                             \
    DEFINE_FUNC_GET(type, funcName, memberName)                                    \
    DEFINE_FUNC_SET(type, funcName, memberName)
} // namespace OHOS::Rosen
#endif // OHOS_ROSEN_CLASS_VAR_DEFINITION_H
