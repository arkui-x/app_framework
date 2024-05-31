/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_THREAD_LOCAL_DATA_H
#define BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_THREAD_LOCAL_DATA_H

#include <mutex>
#include <thread>
#include <unordered_map>

namespace OHOS {
namespace AppExecFwk {
/*
 * Tool class, used to save thread local data.
 */
template<typename T>
class ThreadLocalData {
public:
    ThreadLocalData() = default;
    ~ThreadLocalData() = default;

    /*
     * Override type conversion method.
     */
    inline operator T() const
    {
        return Current();
    }

    /*
     * Override operator equal to save data.
     */
    inline const ThreadLocalData<T>& operator=(const T& data)
    {
        Save(data);
        return *this;
    }

    /*
     * Override operator equal to nullptr, which can used to discard the saved data.
     */
    inline const ThreadLocalData<T>& operator=(std::nullptr_t)
    {
        Discard();
        return *this;
    }

private:
    inline T Current() const
    {
        std::lock_guard<std::mutex> lock(mapLock_);
        auto it = dataMap_.find(std::this_thread::get_id());
        if (it == dataMap_.end()) {
            return T();
        } else {
            return it->second;
        }
    }

    inline void Save(const T& data)
    {
        std::lock_guard<std::mutex> lock(mapLock_);
        dataMap_[std::this_thread::get_id()] = data;
    }

    inline void Discard()
    {
        std::lock_guard<std::mutex> lock(mapLock_);
        (void)dataMap_.erase(std::this_thread::get_id());
    }

    mutable std::mutex mapLock_;
    std::unordered_map<std::thread::id, T> dataMap_;
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // #ifndef BASE_EVENTHANDLER_FRAMEWORKS_EVENTHANDLER_INCLUDE_THREAD_LOCAL_DATA_H
