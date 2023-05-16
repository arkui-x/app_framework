/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_DISPLAY_MANAGER_H
#define OHOS_JS_DISPLAY_MANAGER_H

#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "display_manager.h"

namespace OHOS {
namespace Rosen {
NativeValue* InitDisplayState(NativeEngine* engine);
NativeValue* InitOrientation(NativeEngine* engine);
NativeValue* InitDisplayErrorCode(NativeEngine* engine);
NativeValue* InitDisplayError(NativeEngine* engine);
NativeValue* JsDisplayManagerInit(NativeEngine* engine, NativeValue* exportObj);

class JsDisplayManager {
public:
    explicit JsDisplayManager(NativeEngine* engine) {
    }

    ~JsDisplayManager() = default;

    static void Finalizer(NativeEngine* engine, void* data, void* hint);
    static NativeValue* GetDefaultDisplaySync(NativeEngine* engine, NativeCallbackInfo* info);

private:
    NativeValue* OnGetDefaultDisplaySync(NativeEngine& engine, NativeCallbackInfo& info);

private:
    std::mutex mtx_;
};
}  // namespace Rosen
}  // namespace OHOS

#endif
