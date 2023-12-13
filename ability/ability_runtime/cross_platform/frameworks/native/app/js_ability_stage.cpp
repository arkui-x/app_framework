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

#include "js_ability_stage.h"

#include "ability.h"
#include "ability_delegator_registry.h"
#include "ability_stage_context.h"
#include "context.h"
#include "hilog.h"
#include "js_ability_stage_context.h"
#include "js_data_struct_converter.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
NativeValue* AttachAbilityStageContext(NativeEngine* engine, void* value, void*)
{
    HILOG_INFO("AttachAbilityStageContext");
    if (engine == nullptr || value == nullptr) {
        HILOG_WARN("invalid parameter, engine or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    NativeValue* object = CreateJsAbilityStageContext(*engine, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(engine, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->Get();
    NativeObject* nObject = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nObject == nullptr) {
        HILOG_WARN("LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(ptr);
    nObject->SetNativePointer(
        workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<Context>*>(data);
        },
        nullptr);
    return contextObj;
}

std::shared_ptr<AbilityStage> JsAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        HILOG_ERROR("invalid parameter, runtime is nullptr.");
        return nullptr;
    }

    std::string modulePath;
    auto& jsRuntime = static_cast<JsRuntime&>(*runtime);
    std::unique_ptr<NativeReference> moduleObj;
    bool esmodule = hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE;
    auto buffer = StageAssetManager::GetInstance()->GetModuleBuffer(hapModuleInfo.moduleName,
        modulePath, esmodule);
    if (!esmodule || !hapModuleInfo.srcEntrance.empty()) {
        if (esmodule) {
            modulePath = hapModuleInfo.name;
            modulePath.append("/");
            modulePath.append(hapModuleInfo.srcEntrance);
            modulePath.erase(modulePath.rfind("."));
            modulePath.append(".abc");
        }
        std::string moduleName(hapModuleInfo.moduleName);
        moduleName.append("::").append("AbilityStage");
        HILOG_INFO("moduleName: %{public}s", moduleName.c_str());

        moduleObj = jsRuntime.LoadModule(moduleName, modulePath, buffer,
            hapModuleInfo.srcEntrance, esmodule);
    }
    return std::make_shared<JsAbilityStage>(jsRuntime, std::move(moduleObj));
}

JsAbilityStage::JsAbilityStage(JsRuntime& jsRuntime, std::unique_ptr<NativeReference>&& jsAbilityStageObj)
    : jsRuntime_(jsRuntime), jsAbilityStageObj_(std::move(jsAbilityStageObj))
{}

JsAbilityStage::~JsAbilityStage() = default;

void JsAbilityStage::OnCreate() const
{
    AbilityStage::OnCreate();

    if (!jsAbilityStageObj_) {
        HILOG_WARN("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsAbilityStageObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("OnCreate, Failed to get AbilityStage object");
        return;
    }

    NativeValue* methodOnCreate = obj->GetProperty("onCreate");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    nativeEngine.CallFunction(value, methodOnCreate, nullptr, 0);
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        HILOG_DEBUG("Call AbilityDelegator::PostPerformStageStart");
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void JsAbilityStage::Init(const std::shared_ptr<Context>& context)
{
    AbilityStage::Init(context);

    if (!context) {
        HILOG_ERROR("context is nullptr");
        return;
    }

    if (!jsAbilityStageObj_) {
        HILOG_ERROR("AbilityStageObj is nullptr");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsAbilityStageObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    NativeValue* contextObj = CreateJsAbilityStageContext(engine, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(&engine, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->Get();
    NativeObject* nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    nativeObj->ConvertToNativeBindingObject(
        &engine, DetachCallbackFunc, AttachAbilityStageContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    obj->SetProperty("context", contextObj);
    HILOG_INFO("Set ability stage context");

    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<Context>*>(data);
        },
        nullptr);
}

void JsAbilityStage::OnConfigurationUpdate(const Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdate(configuration);

    if (!jsAbilityStageObj_) {
        HILOG_ERROR("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    auto stageContext = GetContext();
    if (stageContext == nullptr) {
        HILOG_ERROR("Not found stageContext");
        return;
    }
    auto config = stageContext->GetConfiguration();
    if (config == nullptr) {
        HILOG_ERROR("Not found config");
        return;
    }
    JsAbilityStageContext::ConfigurationUpdated(&nativeEngine, shellContextRef_, config);

    NativeValue* value = jsAbilityStageObj_->Get();
    if (value == nullptr) {
        HILOG_ERROR("OnConfigurationUpdate, Failed to get jsAbilityStage object");
        return;
    }
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("OnConfigurationUpdate, Failed to ConvertNativeValueTo object");
        return;
    }
    NativeValue* methodOnCreate = obj->GetProperty("onConfigurationUpdate");
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'OnConfigurationUpdate' from AbilityStage object");
        return;
    }

    NativeValue* argv[] = { CreateJsConfiguration(nativeEngine, *config) };
    nativeEngine.CallFunction(value, methodOnCreate, argv, ArraySize(argv));
}

std::shared_ptr<OHOS::AppExecFwk::DelegatorAbilityStageProperty> JsAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<OHOS::AppExecFwk::DelegatorAbilityStageProperty>();
    property->moduleName_ = GetHapModuleProp("name");
    property->srcEntrance_ = GetHapModuleProp("srcEntrance");
    property->object_ = jsAbilityStageObj_;
    return property;
}

std::string JsAbilityStage::GetHapModuleProp(const std::string &propName) const
{
    auto context = GetContext();
    if (!context) {
        HILOG_ERROR("Failed to get context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        HILOG_ERROR("Failed to get hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    HILOG_ERROR("Failed to GetHapModuleProp name = %{public}s", propName.c_str());
    return std::string();
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
