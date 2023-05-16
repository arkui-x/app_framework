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

#ifndef OHOS_SINGLETON_DELEGATOR_H
#define OHOS_SINGLETON_DELEGATOR_H

#include "singleton_container.h"

#define MOCKABLE virtual

namespace OHOS {
namespace Rosen {
template<class T>
class SingletonDelegator {
public:
    SingletonDelegator()
    {
        nameT = __PRETTY_FUNCTION__;
        nameT = nameT.substr(nameT.find("T = "));
        nameT = nameT.substr(sizeof("T ="), nameT.length() - sizeof("T = "));
        SingletonContainer::GetInstance().AddSingleton(nameT, &T::GetInstance());
    }
    ~SingletonDelegator() = default;

    template<class S>
    S& Dep()
    {
        std::string nameS = __PRETTY_FUNCTION__;
        nameS = nameS.substr(nameS.find("S = "));
        nameS = nameS.substr(sizeof("S ="), nameS.length() - sizeof("S = "));

        auto ret = SingletonContainer::Get<S>();
        SingletonContainer::GetInstance().DependOn(nameT, nameS);
        return ret;
    }

private:
    std::string nameT;
};
} // namespace Rosen
} // namespace OHOS

#endif // FRAMEWORKS_WM_INCLUDE_SINGLETON_DELEGATOR_H
