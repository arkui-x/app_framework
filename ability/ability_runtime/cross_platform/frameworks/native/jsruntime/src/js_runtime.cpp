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
#ifndef IOS_PLATFORM
#include <sys/epoll.h>
#else
#include <sys/event.h>
#endif
#include <unistd.h>

#include "ecmascript/napi/include/jsnapi.h"
#include "event_handler.h"
#include "hilog.h"
#include "connect_server_manager.h"
#include "js_console_log.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
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
constexpr char TIMER_TASK[] = "uv_timer_task";
constexpr char MERGE_ABC_PATH[] = "/ets/modules.abc";
constexpr char BUNDLE_INSTALL_PATH[] = "/data/storage/el1/bundle/";

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
        if (vm_ != nullptr) {
            panda::JSNApi::DestroyJSVM(vm_);
            vm_ = nullptr;
        }
        Deinitialize();
    }

    bool RunScript(const std::string& srcPath, const std::string& hapPath, bool useCommonChunk) override
    {
        return true;
    }

    NativeValue* LoadJsModule(const std::string& path, std::vector<uint8_t>& buffer) override
    {
        std::string assetPath = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
        HILOG_INFO("LoadJsModule path %{public}s", path.c_str());
        HILOG_INFO("LoadJsModule assetPath %{public}s", assetPath.c_str());
        HILOG_INFO("LoadJsModule moduleName_ %{public}s", moduleName_.c_str());
        panda::JSNApi::SetAssetPath(vm_, assetPath);
        panda::JSNApi::SetModuleName(vm_, moduleName_);
        bool result = nativeEngine_->RunScriptBuffer(path.c_str(), buffer, false) != nullptr;
        if (!result) {
            HILOG_ERROR("RunScriptBuffer failed path: %{public}s", path.c_str());
            return nullptr;
        }

        if (!vm_) {
            HILOG_ERROR("pointer is nullptr.");
            return nullptr;
        }
        panda::Local<panda::ObjectRef> exportObj = panda::JSNApi::GetExportObject(vm_, path, "default");
        if (exportObj->IsNull()) {
            HILOG_ERROR("Get export object failed");
            return nullptr;
        }
        if (!nativeEngine_) {
            HILOG_ERROR("pointer is nullptr.");
            return nullptr;
        }
        return ArkNativeEngine::ArkValueToNativeValue(
            static_cast<ArkNativeEngine*>(nativeEngine_.get()), exportObj);
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
        panda::JSNApi::SetBundle(vm_, options.isBundle);
        panda::JSNApi::SetBundleName(vm_, options.bundleName);
        return JsRuntime::Initialize(options);
    }

    panda::ecmascript::EcmaVM* vm_ = nullptr;
};

class UvLoopHandler : public AppExecFwk::FileDescriptorListener, public std::enable_shared_from_this<UvLoopHandler> {
public:
    explicit UvLoopHandler(uv_loop_t* uvLoop) : uvLoop_(uvLoop) {}

    void OnReadable(int32_t) override
    {
        HILOG_INFO("UvLoopHandler::OnReadable is triggered");
        OnTriggered();
    }

    void OnWritable(int32_t) override
    {
        HILOG_INFO("UvLoopHandler::OnWritable is triggered");
        OnTriggered();
    }

private:
    void OnTriggered()
    {
        HILOG_INFO("UvLoopHandler::OnTriggered is triggered");

        auto fd = uv_backend_fd(uvLoop_);
#ifndef IOS_PLATFORM
        struct epoll_event ev;
        do {
            uv_run(uvLoop_, UV_RUN_NOWAIT);
        } while (epoll_wait(fd, &ev, 1, 0) > 0);
#else
        uv_run(uvLoop_, UV_RUN_NOWAIT);
#endif
        auto eventHandler = GetOwner();
        if (!eventHandler) {
            return;
        }

        int32_t timeout = uv_backend_timeout(uvLoop_);
        if (timeout < 0) {
            if (haveTimerTask_) {
                eventHandler->RemoveTask(TIMER_TASK);
            }
            return;
        }

        int64_t timeStamp = static_cast<int64_t>(uv_now(uvLoop_)) + timeout;
        if (timeStamp == lastTimeStamp_) {
            return;
        }

        if (haveTimerTask_) {
            eventHandler->RemoveTask(TIMER_TASK);
        }

        auto callback = [wp = weak_from_this()] {
            auto sp = wp.lock();
            if (sp) {
                // Timer task is triggered, so there is no timer task now.
                sp->haveTimerTask_ = false;
                sp->OnTriggered();
            }
        };
        eventHandler->PostTask(callback, TIMER_TASK, timeout);
        lastTimeStamp_ = timeStamp;
        haveTimerTask_ = true;
    }

    uv_loop_t* uvLoop_ = nullptr;
    int64_t lastTimeStamp_ = 0;
    bool haveTimerTask_ = false;
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

    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(options.eventRunner);

    auto uvLoop = nativeEngine_->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        HILOG_ERROR("Failed to get backend fd from uv loop");
        return false;
    }

    uv_run(uvLoop, UV_RUN_NOWAIT);
    codePath_ = options.codePath;
    uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
    eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<UvLoopHandler>(uvLoop));
    InitTimerModule(*nativeEngine_, *globalObj);
    InitWorkerModule(*nativeEngine_, codePath_, options.isDebugVersion);
    return true;
}

void JsRuntime::Deinitialize()
{
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();

    auto uvLoop = nativeEngine_->GetUVLoop();
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);
    nativeEngine_.reset();
}

NativeValue* JsRuntime::LoadJsBundle(const std::string& path, std::vector<uint8_t>& buffer)
{
    HILOG_INFO("LoadJsBundle path %{public}s", path.c_str());
    HILOG_INFO("LoadJsBundle moduleName_ %{public}s", moduleName_.c_str());
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

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName,
    const std::string& modulePath, std::vector<uint8_t>& buffer, const std::string& srcEntrance, bool esmodule)
{
    HILOG_INFO("JsRuntime::LoadModule(%{public}s, %{public}s, %{public}s)",
        moduleName.c_str(), modulePath.c_str(), srcEntrance.c_str());
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
        if (esmodule) {
            std::string fileName;
            fileName.append(moduleName_).append("/").append(srcEntrance);
            fileName.erase(fileName.rfind("."));
            fileName.append(".abc");
            std::regex pattern(std::string("\\.") + std::string("/"));
            fileName = std::regex_replace(fileName, pattern, "");
            classValue = LoadJsModule(fileName, buffer);
        } else {
            classValue = LoadJsBundle(modulePath, buffer);
        }
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

void JsRuntime::PostTask(const std::function<void()>& task, const std::string& name, int64_t delayTime)
{
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(task, name, delayTime);
    }
}

void JsRuntime::RemoveTask(const std::string& name)
{
    if (eventHandler_ != nullptr) {
        eventHandler_->RemoveTask(name);
    }
}

std::atomic<bool> JsRuntime::hasInstance(false);

void JsRuntime::StartDebugMode(bool needBreakPoint)
{
    if (debugMode_) {
        HILOG_INFO("Already in debug mode");
        return;
    }

    // Set instance id to tid after the first instance.
    if (JsRuntime::hasInstance.exchange(true, std::memory_order_relaxed)) {
        uint64_t tid;
#if !defined(IOS_PLATFORM)
            tid = gettid();
#else
            pthread_threadid_np(0, &tid);
#endif
        instanceId_ = static_cast<uint32_t>(tid);
    }

    HILOG_INFO("Ark VM is starting debug mode [%{public}s]", needBreakPoint ? "break" : "normal");
    auto debuggerPostTask = [eventHandler = eventHandler_](std::function<void()>&& task) {
        eventHandler->PostTask(task);
    };

    debugMode_ = StartDebugMode(bundleName_, needBreakPoint, instanceId_, debuggerPostTask);
}

bool JsRuntime::StartDebugMode(const std::string& bundleName, bool needBreakPoint, uint32_t instanceId,
    const DebuggerPostTask& debuggerPostTask)
{
    ConnectServerManager::Get().StartConnectServer(bundleName);
    ConnectServerManager::Get().AddInstance(instanceId);
    StartDebuggerInWorkerModule();
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
