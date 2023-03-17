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

#include "js_ability_stage_context.h"

#include "hilog.h"
#include "js_context_utils.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {

void JsAbilityStageContext::Finalizer(NativeEngine* engine, void* data, void* hint)
{
    HILOG_INFO("JsAbilityStageContext::Finalizer is called");
    std::unique_ptr<JsAbilityStageContext>(static_cast<JsAbilityStageContext*>(data));
}

NativeValue* CreateJsAbilityStageContext(
    NativeEngine& engine, std::shared_ptr<Context> context, DetachCallback detach, AttachCallback attach)
{
    HILOG_INFO("CreateJsAbilityStageContext called.");
    NativeValue* objValue = CreateJsBaseContext(engine, context);
    NativeObject* object = ConvertNativeValueTo<NativeObject>(objValue);
    if (context == nullptr) {
        return objValue;
    }
    std::unique_ptr<JsAbilityStageContext> jsAbilityStageContext = std::make_unique<JsAbilityStageContext>(context);
    object->SetNativePointer(jsAbilityStageContext.release(), JsAbilityStageContext::Finalizer, nullptr);
    return objValue;
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS