/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "load_module_helper.h"

#include "hilog.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "stage_asset_manager.h"

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
namespace {
static const std::string ONLOAD_METHOD_NAME = "onLoad";
}
void LoadModuleHelper::LoadModule(const std::unique_ptr<Runtime>& runtime,
    const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo, const std::string& entryPath)
{
    if (entryPath.empty()) {
        HILOG_ERROR("LoadModule::LoadModule entryPath is empty");
        return;
    }

    if (hapModuleInfo == nullptr) {
        HILOG_ERROR("LoadModule::LoadModule hapModuleInfo is nullptr");
        return;
    }

    bool esModule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    if (!esModule) {
        HILOG_ERROR("LoadModule::LoadModule the module is not esModule");
        return;
    }

    std::string modulePath;
    auto buffer = StageAssetManager::GetInstance()->GetModuleBuffer(hapModuleInfo->moduleName, modulePath, esModule);
    if (buffer.empty()) {
        HILOG_ERROR("LoadModule::LoadModule GetModuleBuffer error");
        return;
    }

    std::string moduleName(hapModuleInfo->moduleName);
    moduleName.append("::").append(hapModuleInfo->moduleName);
    modulePath = hapModuleInfo->moduleName;
    modulePath.append("/");
    modulePath.append(entryPath);
    modulePath.erase(modulePath.rfind("."));
    modulePath.append(".abc");
    if (runtime == nullptr) {
        HILOG_ERROR("LoadModule::LoadModule runtime is nullptr");
        return;
    }

    auto moduleObj = static_cast<JsRuntime&>(*runtime).LoadModule(
        moduleName, modulePath, buffer, entryPath, esModule);
    if (moduleObj == nullptr) {
        HILOG_ERROR("LoadModule::LoadModule JsRuntime LoadModule failed");
        return;
    }
    napi_env env = static_cast<JsRuntime&>(*runtime).GetNapiEnv();
    CallObjectMethod(env, moduleObj.get(), ONLOAD_METHOD_NAME);
}

void LoadModuleHelper::CallObjectMethod(napi_env env, NativeReference* moduleRef, const std::string& methodName)
{
    if (env == nullptr || moduleRef == nullptr || methodName.empty()) {
        HILOG_ERROR("CallObjectMethod env or moduleRef or methodName is nullptr");
        return;
    }
    napi_value obj = moduleRef->GetNapiValue();
    if (obj == nullptr) {
        HILOG_ERROR("CallObjectMethod moduleRef GetNapiValue is nullptr");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env, obj, methodName.c_str(), &method);
    if (method == nullptr) {
        HILOG_ERROR("CallObjectMethod napi_value method is nullptr");
        return;
    }
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, method, &valueType);
    if (valueType != napi_function) {
        HILOG_ERROR("CallObjectMethod napi_value method is not napi_function");
        return;
    }
    napi_value result = nullptr;
    napi_call_function(env, obj, method, 0, nullptr, &result);
}
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
