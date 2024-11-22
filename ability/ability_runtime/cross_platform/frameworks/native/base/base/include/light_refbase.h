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
#ifndef OHOS_ABILITY_BASE_LIGHT_REFBASE_H
#define OHOS_ABILITY_BASE_LIGHT_REFBASE_H

#include <atomic>

namespace OHOS {
namespace AAFwk {
class LightRefCountBase {
public:
    inline LightRefCountBase();

    inline int IncStrongRef(const void *id = nullptr); /* [in] */

    inline int DecStrongRef(const void *id = nullptr); /* [in] */

    inline int GetRefCount() const;

protected:
    inline virtual ~LightRefCountBase();

private:
    std::atomic<int> refCount_;
};

LightRefCountBase::LightRefCountBase() : refCount_(0)
{}

LightRefCountBase::~LightRefCountBase()
{}

int LightRefCountBase::IncStrongRef(const void *id) /* [in] */
{
    const int beforeCount = refCount_.fetch_add(1, std::memory_order_relaxed);
    return beforeCount + 1;
}

int LightRefCountBase::DecStrongRef(const void *id) /* [in] */
{
    const int beforeCount = refCount_.fetch_sub(1, std::memory_order_release);
    if (beforeCount - 1 == 0) {
        delete this;
    }
    return beforeCount - 1;
}

int LightRefCountBase::GetRefCount() const
{
    return refCount_.load(std::memory_order_relaxed);
}
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_BASE_LIGHT_REFBASE_H
