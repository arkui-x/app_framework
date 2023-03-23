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

#ifndef OHOS_JS_WINDOW_STAGE_H
#define OHOS_JS_WINDOW_STAGE_H
#include <map>
#include "native_engine/native_engine.h"
#include "native_engine/native_reference.h"
#include "native_engine/native_value.h"
#include "window_stage.h"
namespace OHOS {
namespace Rosen {
NativeValue* CreateJsWindowStage(NativeEngine& engine, std::shared_ptr<Rosen::WindowStage> WindowStage);
class JsWindowStage {
public:
    explicit JsWindowStage(const std::shared_ptr<Rosen::WindowStage>& WindowStage);
    ~JsWindowStage();
    static NativeValue* LoadContent(NativeEngine* engine, NativeCallbackInfo* info);
    static void Finalizer(NativeEngine* engine, void* data, void* hint);

private:
    NativeValue* OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info);

    std::weak_ptr<Rosen::WindowStage> windowStage_;
};
}  // namespace Rosen
}  // namespace OHOS
#endif  // OHOS_JS_WINDOW_STAGE_H
