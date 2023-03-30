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
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_want_utils.h"
#include "js_window_stage.h"
#include "window_view_adapter.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
NativeValue* AttachJsAbilityContext(NativeEngine* engine, void* value, void*)
{
    HILOG_INFO("AttachJsAbilityContext");
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr");
        return nullptr;
    }
    auto sptrContext = reinterpret_cast<std::weak_ptr<AbilityContext>*>(value)->lock();
    if (sptrContext == nullptr) {
        HILOG_ERROR("sptrContext is nullptr");
        return nullptr;
    }
    NativeValue* object = CreateJsAbilityContext(*engine, sptrContext);
    if (object == nullptr) {
        HILOG_ERROR("object is nullptr");
        return nullptr;
    }
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.AbilityContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_ERROR("invalid systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject* nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_ERROR("nObject is nullptr");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachJsAbilityContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityContext>(sptrContext);
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
        StageAssetManager::GetInstance()->GetModuleAbilityBuffer(moduleName, abilityInfo->name, modulePath);
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
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();
    NativeValue* contextObj = CreateJsAbilityContext(engine, abilityContext);
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr");
        return;
    }
    shellContextRef_ = std::shared_ptr<NativeReference>(
        JsRuntime::LoadSystemModuleByEngine(&engine, "application.AbilityContext", &contextObj, 1).release());
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("shellContextRef_ is nullptr");
        return;
    }
    contextObj = shellContextRef_->Get();
    if (contextObj == nullptr) {
        HILOG_ERROR("contextObj is nullptr");
        return;
    }
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
        HILOG_ERROR("Not found Ability.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityObj_->Get();
    if (value == nullptr) {
        HILOG_ERROR("value is nullptr");
        return;
    }
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

    if (windowStage_ == nullptr) {
        windowStage_ = std::make_shared<Rosen::WindowStage>();
        if (windowStage_ == nullptr) {
            HILOG_ERROR("windowStage_ is nullptr");
            return;
        }
        windowStage_->Init(GetAbilityContext(), WindowViewAdapter::GetInstance()->GetWindowView(GetInstanceName()),
            WindowViewAdapter::GetInstance()->GetJniEnv().get());
    }

    if (!jsAbilityObj_) {
        HILOG_WARN("Not found Ability.js");
        return;
    }

    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("OnCreate applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnAbilityCreate(jsAbilityObj_);

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    NativeValue* value = jsAbilityObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }
    NativeValue* jsWant = CreateJsWant(nativeEngine, want);
    if (jsWant == nullptr) {
        HILOG_ERROR("jsWant is nullptr");
        return;
    }
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
    OnWindowStageDestroy();
    Ability::OnDestory();
    CallObjectMethod("onDestroy");
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("onDestroy applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnAbilityDestroy(jsAbilityObj_);
}

void JsAbility::OnNewWant(const Want& want)
{
    HILOG_INFO("OnNewWant begin.");
    Ability::OnNewWant(want);
    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    if (jsAbilityObj_ == nullptr) {
        HILOG_ERROR("Failed to get Ability object");
        return;
    }
    NativeValue* value = jsAbilityObj_->Get();
    if (value == nullptr) {
        HILOG_ERROR("Failed to get jsAbility object");
        return;
    }
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to convert Ability object");
        return;
    }
    NativeValue* jsWant = CreateJsWant(nativeEngine, want);
    if (jsWant == nullptr) {
        HILOG_ERROR("Failed to create JsWant object");
        return;
    }
    CallObjectMethod("onNewWant", &jsWant, 1);
}

void JsAbility::OnForeground(const Want& want)
{
    HILOG_INFO("OnForeground begin.");
    OnWindowStageCreated();
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
    if (jsWant == nullptr) {
        HILOG_ERROR("jsWant is nullptr");
        return;
    }
    CallObjectMethod("onForeground", &jsWant, 1);
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("onForeground applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnAbilityForeground(jsAbilityObj_);
}

void JsAbility::OnBackground()
{
    HILOG_INFO("OnBackground begin.");
    Ability::OnBackground();
    CallObjectMethod("onBackground");
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("OnBackground applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnAbilityBackground(jsAbilityObj_);
}

void JsAbility::OnWindowStageCreated()
{
    HILOG_INFO("OnWindowStageCreated begin");
    Ability::OnWindowStageCreated();

    auto jsAppWindowStage = CreateJsWindowStage();
    if (jsAppWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsAppWindowStage object by LoadSystemModule");
        return;
    }
    NativeValue* argv[] = { jsAppWindowStage->Get() };
    CallObjectMethod("onWindowStageCreate", argv, ArraySize(argv));
    jsWindowStageObj_ = std::shared_ptr<NativeReference>(jsAppWindowStage.release());
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("onWindowStageCreate applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnWindowStageCreate(jsAbilityObj_, jsWindowStageObj_);
}

void JsAbility::OnWindowStageDestroy()
{
    HILOG_INFO("OnWindowStageDestroy begin");
    Ability::OnWindowStageDestroy();
    CallObjectMethod("onWindowStageDestroy");
    auto applicationContext = ApplicationContext::GetInstance();
    if (applicationContext == nullptr) {
        HILOG_ERROR("OnWindowStageDestroy applicationContext is nullptr");
        return;
    }
    applicationContext->DispatchOnWindowStageDestroy(jsAbilityObj_, jsWindowStageObj_);
}

std::unique_ptr<NativeReference> JsAbility::CreateJsWindowStage()
{
    HandleScope handleScope(jsRuntime_);
    if (windowStage_ == nullptr) {
        HILOG_ERROR("windowStage_ is nullptr");
        return nullptr;
    }

    auto& engine = jsRuntime_.GetNativeEngine();
    auto jsWindowStage = Rosen::CreateJsWindowStage(engine, windowStage_);
    if (jsWindowStage == nullptr) {
        HILOG_ERROR("Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(&engine, "application.WindowStage", &jsWindowStage, 1);
}

void JsAbility::OnConfigurationUpdate(const Configuration& configuration)
{
    Ability::OnConfigurationUpdate(configuration);
    HILOG_INFO("OnConfigurationUpdate called.");

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();
    auto stageContext = GetAbilityContext();
    if (stageContext == nullptr) {
        HILOG_ERROR("Not found stageContext");
        return;
    }
    auto config = stageContext->GetConfiguration();
    if (!config) {
        HILOG_ERROR("configuration is nullptr.");
        return;
    }
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("shellContextRef_ is nullptr");
        return;
    }
    JsAbilityContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, config);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS