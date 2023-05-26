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

#ifndef OHOS_SINGLETON_CONTAINER_H
#define OHOS_SINGLETON_CONTAINER_H

#include <any>
#include <cstdint>
#include <map>
#include <set>
#include <string>

#include "wm_single_instance.h"
namespace OHOS {
namespace Rosen {
class SingletonContainer {
WM_DECLARE_SINGLE_INSTANCE_BASE(SingletonContainer);

public:
    void AddSingleton(const std::string& name, void* instance);

    void SetSingleton(const std::string& name, void* instance);

    void* GetSingleton(const std::string& name);

    void* DependOn(const std::string& instance, const std::string& name);

    static bool IsDestroyed()
    {
        return SingletonContainer::GetInstance().destroyed_;
    }

    template<class T>
    static T& Get()
    {
        std::string nameT = __PRETTY_FUNCTION__;
        nameT = nameT.substr(nameT.find("T = "));
        nameT = nameT.substr(sizeof("T ="), nameT.length() - sizeof("T = "));
        if (SingletonContainer::GetInstance().GetSingleton(nameT) == nullptr) {
            return T::GetInstance();
        }
        return *(reinterpret_cast<T*>(SingletonContainer::GetInstance().GetSingleton(nameT)));
    }

    template<class T>
    static void Set(T& instance)
    {
        std::string nameT = __PRETTY_FUNCTION__;
        nameT = nameT.substr(nameT.find("T = "));
        nameT = nameT.substr(sizeof("T ="), nameT.length() - sizeof("T = "));
        SingletonContainer::GetInstance().SetSingleton(nameT, &instance);
    }

private:
    SingletonContainer() = default;

    virtual ~SingletonContainer();

    struct Singleton {
        void* value;
        int32_t refCount;
    };
    std::map<std::string, int32_t> stringMap;
    std::map<int32_t, SingletonContainer::Singleton> singletonMap;
    std::map<int32_t, std::set<int32_t>> dependencySetMap;
    bool destroyed_ { false };
};
} // namespace Rosen
} // namespace OHOS
#endif // FRAMEWORKS_WM_INCLUDE_SINGLETON_CONTAINER_H
