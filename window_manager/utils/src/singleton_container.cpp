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

#include "singleton_container.h"
#include <mutex>
#include "window_hilog.h"

namespace OHOS {
namespace Rosen {
WM_IMPLEMENT_SINGLE_INSTANCE(SingletonContainer)

SingletonContainer::~SingletonContainer()
{
    destroyed_ = true;
    while (singletonMap.empty() == false) {
        auto it = singletonMap.begin();
        while (it != singletonMap.end()) {
            if (it->second.refCount > 0) {
                it++;
                continue;
            }

            if (dependencySetMap.find(it->first) != dependencySetMap.end()) {
                for (auto mid : dependencySetMap[it->first]) {
                    singletonMap[mid].refCount--;
                }
                dependencySetMap.erase(it->first);
            }

            for (const auto &[k, v] : stringMap) {
                if (v == it->first) {
                    WLOGD("remove %{public}s", k.c_str());
                    break;
                }
            }
            singletonMap.erase(it++);
        }
    }
}

void SingletonContainer::AddSingleton(const std::string& name, void* instance)
{
    if (stringMap.find(name) == stringMap.end()) {
        static int32_t nextId = 0;
        singletonMap[nextId].value = instance;
        singletonMap[nextId].refCount = 0;
        WLOGI("add %{public}s", name.c_str());
        stringMap[name] = nextId++;
    } else {
        WLOGE("add failed: %{public}s", name.c_str());
    }
}

void SingletonContainer::SetSingleton(const std::string& name, void* instance)
{
    if (stringMap.find(name) == stringMap.end()) {
        AddSingleton(name, instance);
    } else {
        WLOGI("set %{public}s", name.c_str());
        singletonMap[stringMap[name]].value = instance;
    }
}

void* SingletonContainer::GetSingleton(const std::string& name)
{
    if (stringMap.find(name) == stringMap.end()) {
        WLOGE("can not get %{public}s", name.c_str());
        return nullptr;
    }
    return singletonMap[stringMap[name]].value;
}

void* SingletonContainer::DependOn(const std::string& instance, const std::string& name)
{
    auto& instanceDependencySet = dependencySetMap[stringMap[instance]];
    if (instanceDependencySet.find(stringMap[name]) == instanceDependencySet.end()) {
        WLOGD("%{public}s DependOn %{public}s", instance.c_str(), name.c_str());
        instanceDependencySet.insert(stringMap[name]);
        singletonMap[stringMap[name]].refCount++;
    }
    return GetSingleton(name);
}
} // namespace Rosen
} // namespace OHOS
