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
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_WM_SINGLE_INSTANCE_H
#define OHOS_WM_SINGLE_INSTANCE_H
namespace OHOS {
namespace Rosen {
#define WM_DECLARE_SINGLE_INSTANCE_BASE(className)          \
public:                                                     \
    static className& GetInstance();                        \
    className(const className&) = delete;                   \
    className& operator= (const className&) = delete;       \
    className(className&&) = delete;                        \
    className& operator= (className&&) = delete;            \

#define WM_DECLARE_SINGLE_INSTANCE(className)  \
    WM_DECLARE_SINGLE_INSTANCE_BASE(className) \
protected:                                     \
    className() = default;                     \
    virtual ~className() = default;            \

#define WM_IMPLEMENT_SINGLE_INSTANCE(className) \
className& className::GetInstance()             \
{                                               \
    static className instance;                  \
    return instance;                            \
}                                               \

} // namespace OHOS
}
#endif // OHOS_SINGLE_INSTANCE_H
