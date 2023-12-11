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
napi_value AttachAbilityStageContext(napi_env env, void *value, void *)
{
    HILOG_INFO("AttachAbilityStageContext");
    if (env == nullptr || value == nullptr) {
        HILOG_WARN("invalid parameter, env or value is nullptr.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityContext>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_WARN("invalid context.");
        return nullptr;
    }
    napi_value object = CreateJsAbilityStageContext(env, ptr, nullptr, nullptr);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &object, 1);
    if (systemModule == nullptr) {
        HILOG_WARN("invalid systemModule.");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_WARN("LoadSystemModuleByEngine or ConvertNativeValueTo failed.");
        return nullptr;
    }
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<Context> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

bool JsAbilityStage::UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    for (auto &md: hapModuleInfo.metadata) {
        if (md.name == "USE_COMMON_CHUNK") {
            if (md.value != "true") {
                HILOG_WARN("USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
                return false;
            }
            return true;
        }
    }
    return false;
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
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("OnCreate, Failed to get AbilityStage object");
        return;
    }

    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, obj, "onCreate", &methodOnCreate);
    if (methodOnCreate == nullptr) {
        HILOG_ERROR("Failed to get 'onCreate' from AbilityStage object");
        return;
    }
    napi_call_function(env, obj, methodOnCreate, 0, nullptr, nullptr);
    
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
    auto env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsAbilityStageObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        HILOG_ERROR("Failed to get AbilityStage object");
        return;
    }

    napi_value contextObj = CreateJsAbilityStageContext(env, context, nullptr, nullptr);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AbilityStageContext", &contextObj, 1);
    if (shellContextRef_ == nullptr) {
        HILOG_ERROR("Failed to get LoadSystemModuleByEngine");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        HILOG_ERROR("Failed to get context native object");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAbilityStageContext, workContext, nullptr);
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    HILOG_INFO("Set ability stage context");

    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            HILOG_DEBUG("Finalizer for weak_ptr ability stage context is called");
            delete static_cast<std::weak_ptr<Context>*>(data);
        },
        nullptr, nullptr);
}

void JsAbilityStage::OnConfigurationUpdate(const Configuration& configuration)
{
    AbilityStage::OnConfigurationUpdate(configuration);

    if (!jsAbilityStageObj_) {
        HILOG_ERROR("Not found AbilityStage.js");
        return;
    }

    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

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
    JsAbilityStageContext::ConfigurationUpdated(env, shellContextRef_, config);

    napi_value value = jsAbilityStageObj_->GetNapiValue();
    
    napi_value methodOnCreate = nullptr;
    napi_get_named_property(env, value, "onConfigurationUpdate", &methodOnCreate);

    napi_value argv[] = { CreateJsConfiguration(env, *config) };
    napi_value callResult = nullptr;
    napi_call_function(env, value, methodOnCreate, ArraySize(argv), argv, &callResult);
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
