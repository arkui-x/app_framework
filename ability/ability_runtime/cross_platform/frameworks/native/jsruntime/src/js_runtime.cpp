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

#include "js_runtime.h"

#include <atomic>
#include <cerrno>
#include <climits>
#include <cstdlib>
#include <regex>
#include <unistd.h>

#include "ecmascript/napi/include/jsnapi.h"
#include "hilog.h"
#include "js_console_log.h"
#include "js_runtime_utils.h"
#include "js_worker.h"
#include "native_engine/impl/ark/ark_native_engine.h"

#ifdef SUPPORT_GRAPHICS
#include "foundation/appframework/arkui/uicontent/declarative_module_preloader.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t DEFAULT_GC_POOL_SIZE = 0x10000000; // 256MB
constexpr int32_t DEFAULT_ARK_PROPERTIES = -1;
constexpr size_t DEFAULT_GC_THREAD_NUM = 7;
constexpr size_t DEFAULT_LONG_PAUSE_TIME = 40;

class ArkJsRuntime : public JsRuntime {
public:
    ArkJsRuntime()
    {
        HILOG_INFO("ArkJsRuntime::ArkJsRuntime call constructor.");
        isArkEngine_ = true;
    }

    ~ArkJsRuntime() override
    {
        HILOG_INFO("ArkJsRuntime::ArkJsRuntime call destructor.");
        Deinitialize();

        if (vm_ != nullptr) {
            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
    }

    bool RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk) override
    {
        return true;
    }

    NativeValue* LoadJsModule(const std::string& path, const std::string& hapPath) override
    {
        return nullptr;
    }

private:
    static int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
    {
        HILOG_INFO("ArkLog: %{public}s", message);
        return 0;
    }

    bool Initialize(const Runtime::Options& options) override
    {
        panda::RuntimeOption pandaOption;
        pandaOption.SetArkProperties(DEFAULT_ARK_PROPERTIES);
        pandaOption.SetGcThreadNum(DEFAULT_GC_THREAD_NUM);
        pandaOption.SetLongPauseTime(DEFAULT_LONG_PAUSE_TIME);
        pandaOption.SetGcType(panda::RuntimeOption::GC_TYPE::GEN_GC);
        pandaOption.SetGcPoolSize(DEFAULT_GC_POOL_SIZE);
        pandaOption.SetLogLevel(panda::RuntimeOption::LOG_LEVEL::INFO);
        pandaOption.SetLogBufPrint(PrintVmLog);
        pandaOption.SetEnableAsmInterpreter(true);
        pandaOption.SetAsmOpcodeDisableRange("");
        pandaOption.SetEnableAOT(true);
        pandaOption.SetEnableProfile(false);
        vm_ = panda::JSNApi::CreateJSVM(pandaOption);
        if (vm_ == nullptr) {
            return false;
        }

        nativeEngine_ = std::make_unique<ArkNativeEngine>(vm_, static_cast<JsRuntime*>(this));

        isBundle_ = options.isBundle;
        return JsRuntime::Initialize(options);
    }

    panda::ecmascript::EcmaVM* vm_ = nullptr;
};
} // namespace

std::unique_ptr<Runtime> JsRuntime::Create(const Runtime::Options& options)
{
    std::unique_ptr<JsRuntime> instance = std::make_unique<ArkJsRuntime>();
    if (instance == nullptr) {
        HILOG_INFO("instance is nullptr");
        return nullptr;
    }

    if (!instance->Initialize(options)) {
        return std::unique_ptr<Runtime>();
    }
    return instance;
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModuleByEngine(
    NativeEngine* engine, const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    if (engine == nullptr) {
        HILOG_INFO("JsRuntime::LoadSystemModule: invalid engine.");
        return std::unique_ptr<NativeReference>();
    }

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(engine->GetGlobal());
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    methodRequireNapiRef_.reset(engine->CreateReference(globalObj->GetProperty("requireNapi"), 1));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return nullptr;
    }
    NativeValue* className = engine->CreateString(moduleName.c_str(), moduleName.length());
    NativeValue* classValue = engine->CallFunction(engine->GetGlobal(), methodRequireNapiRef_->Get(), &className, 1);
    NativeValue* instanceValue = engine->CreateInstance(classValue, argv, argc);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(engine->CreateReference(instanceValue, 1));
}

bool JsRuntime::Initialize(const Options& options)
{
    HandleScope handleScope(*this);

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    InitConsoleLogModule(*nativeEngine_, *globalObj);

    // Simple hook function 'isSystemplugin'
    const char* moduleName = "JsRuntime";
    BindNativeFunction(*nativeEngine_, *globalObj, "isSystemplugin", moduleName,
        [](NativeEngine* engine, NativeCallbackInfo* info) -> NativeValue* { return engine->CreateUndefined(); });

    methodRequireNapiRef_.reset(nativeEngine_->CreateReference(globalObj->GetProperty("requireNapi"), 1));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return false;
    }

#ifdef SUPPORT_GRAPHICS
    if (options.loadAce) {
        OHOS::Ace::Platform::DeclarativeModulePreloader::Preload(*nativeEngine_);
    }
#endif

    auto uvLoop = nativeEngine_->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        HILOG_ERROR("Failed to get backend fd from uv loop");
        return false;
    }

    uv_run(uvLoop, UV_RUN_NOWAIT);
    return true;
}

void JsRuntime::Deinitialize()
{
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();
    nativeEngine_.reset();
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, std::vector<uint8_t>& buffer)
{
    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine_->GetGlobal());
    NativeValue* exports = nativeEngine_->CreateObject();
    globalObj->SetProperty("exports", exports);

    bool result = nativeEngine_->RunScriptBuffer(path.c_str(), buffer, isBundle_) != nullptr;
    if (!result) {
        HILOG_ERROR("RunScriptBuffer failed path: %{public}s", path.c_str());
        return nullptr;
    }

    NativeObject* exportsObj = ConvertNativeValueTo<NativeObject>(globalObj->GetProperty("exports"));
    if (exportsObj == nullptr) {
        HILOG_ERROR("Failed to get exports objcect: %{private}s", path.c_str());
        return nullptr;
    }

    NativeValue* exportObj = exportsObj->GetProperty("default");
    if (exportObj == nullptr) {
        HILOG_ERROR("Failed to get default objcect: %{private}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(
    const std::string& moduleName, const std::string& modulePath, std::vector<uint8_t>& buffer)
{
    HILOG_INFO("JsRuntime::LoadModule(%{public}s, %{public}s)", moduleName.c_str(), modulePath.c_str());
    HandleScope handleScope(*this);

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }

    NativeValue* classValue = nullptr;

    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->Get();
    } else {
        classValue = LoadJsBundle(modulePath, buffer);
        if (classValue == nullptr) {
            return std::unique_ptr<NativeReference>();
        }

        modules_.emplace(modulePath, nativeEngine_->CreateReference(classValue, 1));
    }

    NativeValue* instanceValue = nativeEngine_->CreateInstance(classValue, nullptr, 0);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(nativeEngine_->CreateReference(instanceValue, 1));
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, NativeValue* const* argv, size_t argc)
{
    return nullptr;
}

bool JsRuntime::RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk)
{
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
