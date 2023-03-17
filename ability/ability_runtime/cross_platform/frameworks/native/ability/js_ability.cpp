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
#include "js_ability.h"

#include "hilog.h"
#include "js_ability_context.h"
#include "js_runtime.h"
#include "js_want_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
NativeValue* AttachJsAbilityContext(NativeEngine* engine, void* value, void*)
{
    HILOG_INFO("AttachJsAbilityContext");
    if (value == nullptr) {
        HILOG_WARN("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue* object = CreateJsAbilityContext(*engine, ptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.AbilityContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject* nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachJsAbilityContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(ptr);
    nObject->SetNativePointer(
        workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityContext>*>(data);
        },
        nullptr);
    return contextObj;
}

std::shared_ptr<Ability> JsAbility::Create(const std::unique_ptr<Runtime>& runtime)
{
    return std::make_shared<JsAbility>(static_cast<JsRuntime&>(*runtime));
}

JsAbility::JsAbility(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}

JsAbility::~JsAbility() = default;

void JsAbility::Init(const std::shared_ptr<AppExecFwk::AbilityInfo>& abilityInfo)
{
    HILOG_INFO("Init begin.");
    if (!abilityInfo) {
        HILOG_ERROR("abilityInfo is nullptr");
        return;
    }

    std::string moduleName(abilityInfo->moduleName);
    HILOG_INFO("moduleName: %{public}s abilityName: %{public}s", moduleName.c_str(), abilityInfo->name.c_str());
    std::string modulePath;
    auto abilityBuffer =
        StageAssetManager::GetInstance().GetModuleAbilityBuffer(moduleName, abilityInfo->name, modulePath);
    HILOG_INFO("modulePath: %{public}s ", modulePath.c_str());

    jsAbilityObj_ = jsRuntime_.LoadModule(moduleName, modulePath, abilityBuffer);
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to convert Ability object");
        return;
    }

    auto abilityContext = GetAbilityContext();
    if (abilityContext == nullptr) {
        HILOG_ERROR("abilityContext is nullptr");
        return;
    }
    auto& engine = jsRuntime_.GetNativeEngine();
    NativeValue* contextObj = CreateJsAbilityContext(engine, abilityContext);
    shellContextRef_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(&engine, "application.AbilityContext", &contextObj, 1).release());
    contextObj = shellContextRef_->Get();
    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get ability native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(abilityContext);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachJsAbilityContext, workContext, nullptr);
    abilityContext->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);

    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr ability context is called");
            delete static_cast<std::weak_ptr<AbilityContext>*>(data);
        },
        nullptr);
}

void JsAbility::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsAbility::CallObjectMethod: %{public}s", name);

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to convert Ability object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty(name);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from Ability object", name);
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, argv, argc);
}

void JsAbility::OnCreate(const Want& want)
{
    HILOG_INFO("OnCreate begin.");
    Ability::OnCreate(want);
    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }

    NativeValue* jsWant = CreateJsWant(nativeEngine, want);
    obj->SetProperty("launchWant", jsWant);
    obj->SetProperty("lastRequestWant", jsWant);
    NativeValue* objValue = nativeEngine.CreateObject();
    NativeValue* argv[] = {
        jsWant,
        objValue,
    };
    CallObjectMethod("onCreate", argv, ArraySize(argv));
}

void JsAbility::OnDestory()
{
    HILOG_INFO("OnDestory begin.");
    Ability::OnDestory();
    CallObjectMethod("onDestroy");
}

void JsAbility::OnNewWant(const Want& want)
{
    HILOG_INFO("OnNewWant begin.");
    Ability::OnNewWant(want);
}

void JsAbility::OnForeground(const Want& want)
{
    HILOG_INFO("OnForeground begin.");
    Ability::OnForeground(want);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }
    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to convert Ability object");
        return;
    }
    NativeValue* jsWant = CreateJsWant(nativeEngine, want);
    obj->SetProperty("lastRequestWant", jsWant);
    CallObjectMethod("onForeground", &jsWant, 1);
}

void JsAbility::OnBackground()
{
    HILOG_INFO("OnBackground begin.");
    Ability::OnBackground();
    CallObjectMethod("onBackground");
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS