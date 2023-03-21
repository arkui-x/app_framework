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

#include <string>
#include "js_runtime_utils.h"
#include "virtual_rs_window.h"
#include "js_window_stage.h"

namespace OHOS {
namespace Rosen {
using namespace AbilityRuntime;
JsWindowStage::JsWindowStage(const std::shared_ptr<Rosen::WindowStage>& windowStage)
    : windowStage_(windowStage)
{
}

JsWindowStage::~JsWindowStage()
{
}

NativeValue* JsWindowStage::LoadContent(NativeEngine* engine, NativeCallbackInfo* info)
{
    JsWindowStage* me = CheckParamsAndGetThis<JsWindowStage>(engine, info);
    return (me != nullptr) ? me->OnLoadContent(*engine, *info) : nullptr;
}

static void LoadContentTask(std::shared_ptr<NativeReference> contentStorage, std::string contextUrl,
    std::shared_ptr<Window> weakWindow, NativeEngine& engine, AsyncTask& task)
{
    NativeValue* nativeStorage =  (contentStorage == nullptr) ? nullptr : contentStorage->Get();
    int ret = weakWindow->SetUIContent(contextUrl, &engine, nativeStorage, false, nullptr);
    if (ret == 0) {
        task.Resolve(engine, engine.CreateUndefined());
    } else {
        task.Reject(engine, CreateJsError(engine, static_cast<int32_t>(ret), "Window load content failed"));
    }
}

NativeValue* JsWindowStage::OnLoadContent(NativeEngine& engine, NativeCallbackInfo& info)
{
    int errCode = 0;
    std::string contextUrl;
    if (!ConvertFromJsValue(engine, info.argv[0], contextUrl)) {
        errCode = -1;
    }
    NativeValue* storage = nullptr;
    NativeValue* callBack = nullptr;
    NativeValue* value1 = info.argv[1];
    NativeValue* value2 = info.argv[2]; // 2: param index
    if (value1->TypeOf() == NATIVE_FUNCTION) {
        callBack = value1;
    } else if (value1->TypeOf() == NATIVE_OBJECT) {
        storage = value1;
    }
    if (value2->TypeOf() == NATIVE_FUNCTION) {
        callBack = value2;
    }
    if (errCode == -1) {
        engine.Throw(CreateJsError(engine, -1));
        return engine.CreateUndefined();
    }
    std::shared_ptr<NativeReference> contentStorage = (storage == nullptr) ? nullptr :
        std::shared_ptr<NativeReference>(engine.CreateReference(storage, 1));
    AsyncTask::CompleteCallback complete =
        [weak = windowStage_, contentStorage, contextUrl, errCode](
            NativeEngine& engine, AsyncTask& task, int32_t status) {
            auto weakScene = weak.lock();
            if (weakScene == nullptr) {
                task.Reject(engine, CreateJsError(engine, -1));
                return;
            }
            auto win = weakScene->GetMainWindow();
            if (win == nullptr) {
                task.Reject(engine, CreateJsError(engine, -1));
                return;
            }
            LoadContentTask(contentStorage, contextUrl, win, engine, task);
        };
    NativeValue* result = nullptr;
    AsyncTask::Schedule("JsWindowStage::OnLoadContent",
        engine, CreateAsyncTaskWithLastParam(engine, callBack, nullptr, std::move(complete), &result));
    return result;
}

void JsWindowStage::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    std::unique_ptr<JsWindowStage>(static_cast<JsWindowStage*>(data));
}

NativeValue* CreateJsWindowStage(NativeEngine& engine,
    std::shared_ptr<Rosen::WindowStage> WindowStage)
{
    NativeValue* objValue = engine.CreateObject();
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);

    std::unique_ptr<JsWindowStage> jsWindowStage = std::make_unique<JsWindowStage>(WindowStage);
    object->SetNativePointer(jsWindowStage.release(), JsWindowStage::Finalizer, nullptr);

    const char *moduleName = "JsWindowStage";
    BindNativeFunction(engine,
        *object, "loadContent", moduleName, JsWindowStage::LoadContent);

    return objValue;
}
}  // namespace Rosen
}  // namespace OHOS
