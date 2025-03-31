/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
#define OHOS_ABILITY_RUNTIME_JS_RUNTIME_H

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "js_module_reader.h"
#include "native_engine/native_engine.h"
#include "runtime.h"
#include "js_module_reader.h"

class NativeEngine;
namespace OHOS {
namespace AppExecFwk {
class EventHandler;
} // namespace AppExecFwk

namespace JsEnv {
struct UncaughtExceptionInfo;
class NapiUncaughtExceptionCallback;
} // namespace JsEnv

namespace AbilityRuntime {
class TimerTask;
class ModSourceMap;

inline void* DetachCallbackFunc(napi_env env, void* value, void*)
{
    return value;
}

class JsRuntime : public Runtime {
public:
    static std::unique_ptr<Runtime> Create(const Options& options);

    static std::unique_ptr<NativeReference> LoadSystemModuleByEngine(
        napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc);

    ~JsRuntime() override = default;
    
    napi_env GetNapiEnv() const;

    Language GetLanguage() const override
    {
        return Language::JS;
    }

    std::unique_ptr<NativeReference> LoadModule(
        const std::string& moduleName, const std::string& modulePath, std::vector<uint8_t>& buffer,
        const std::string& srcEntrance, bool esmodule);
    std::unique_ptr<NativeReference> LoadSystemModule(
        const std::string& moduleName, napi_value* const* argv = nullptr, size_t argc = 0);
    void PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime);
    void RemoveTask(const std::string& name);
    virtual bool RunScript(const std::string& path, const std::string& hapPath, bool useCommonChunk = false) = 0;
    void StartDebugMode(bool needBreakPoint);
    bool StartDebugMode(const std::string& bundleName, bool needBreakPoint, uint32_t instanceId,
        const DebuggerPostTask& debuggerPostTask = {});
    void RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo);
    bool IsNeedUpdate(const std::string& moduleName, const std::string& modulePath);

protected:
    JsRuntime() = default;

    virtual bool Initialize(const Options& options);
    void Deinitialize();

    napi_value LoadJsBundle(const std::string& path, std::vector<uint8_t>& buffer);
    virtual napi_value LoadJsModule(const std::string& path, std::vector<uint8_t>& buffer, bool needUpdate) = 0;

    bool isArkEngine_ = false;
    bool preloaded_ = false;
    bool debugMode_ = false;
    bool isBundle_ = true;
    napi_env env_ = nullptr;
    std::string codePath_;
    std::string appLibPath_;
    std::string moduleName_;
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::unordered_map<std::string, NativeReference*> modules_;
    static std::atomic<bool> hasInstance;

    std::string bundleName_;
    uint32_t instanceId_ = 0;
    panda::ecmascript::EcmaVM* vm_ = nullptr;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_RUNTIME_H
