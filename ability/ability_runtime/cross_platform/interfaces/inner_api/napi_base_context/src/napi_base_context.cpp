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

#include "napi_base_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
napi_status IsStageContext(napi_env env, napi_value object, bool& stageMode)
{
    stageMode = true;
    return napi_ok;
}

std::shared_ptr<Context> GetStageModeContext(napi_env env, napi_value object)
{
    void* wrapped = nullptr;
    napi_status ret = napi_unwrap(env, object, &wrapped);
    if (ret != napi_ok) {
        return nullptr;
    }

    auto weakContext = static_cast<std::weak_ptr<Context>*>(wrapped);
    return weakContext != nullptr ? weakContext->lock() : nullptr;
}
}  // namespace Platform
}  // namespace AbilityRuntime
}  // namespace OHOS
