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

#ifndef OHOS_WM_INCLUDE_FUTURE_H
#define OHOS_WM_INCLUDE_FUTURE_H

#include <condition_variable>
#include "window_hilog.h"

namespace OHOS::Rosen {
template<class T>
class Future {
public:
    T GetResult(long timeOut)
    {
        std::unique_lock <std::mutex> lock(mutex_);
        if (!conditionVariable_.wait_for(lock, std::chrono::milliseconds(timeOut), [this] { return IsReady(); })) {
            WLOGE("wait for %{public}ld, timeout.", timeOut);
        }
        return FetchResult();
    }

protected:
    virtual bool IsReady() = 0;

    virtual T FetchResult() = 0;

    virtual void Call(T) = 0;

    void FutureCall(T t)
    {
        std::unique_lock <std::mutex> lock(mutex_);
        Call(t);
        conditionVariable_.notify_one();
    }

    std::mutex mutex_;

private:
    std::condition_variable conditionVariable_;
};

template<class T>
class RunnableFuture : public Future<T> {
public:
    void SetValue(T res)
    {
        Future<T>::FutureCall(res);
    }
    void Reset(T defaultValue)
    {
        flag_ = false;
        result_ = defaultValue;
    }
    void ResetLock(T defaultValue)
    {
        std::unique_lock <std::mutex> lock(Future<T>::mutex_);
        flag_ = false;
        result_ = defaultValue;
    }
protected:
    void Call(T res) override
    {
        if (!flag_) {
            flag_ = true;
            result_ = res;
        }
    }
    bool IsReady() override
    {
        return flag_;
    }
    T FetchResult() override
    {
        return result_;
    }
private:
    bool flag_ {false};
    T result_;
};
} // namespace OHOS::Rosen
#endif // OHOS_WM_INCLUDE_FUTURE_H
