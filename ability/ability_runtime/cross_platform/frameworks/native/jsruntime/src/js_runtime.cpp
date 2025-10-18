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

#ifdef DEBUG_MODE
#include "connect_server_manager.h"
#endif
#include "ecmascript/napi/include/jsnapi.h"
#include "event_handler.h"
#include "hilog.h"
#include "js_console_log.h"
#include "js_module_reader.h"
#include "js_runtime_utils.h"
#include "js_timer.h"
#include "js_worker.h"
#include "json_util.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "stage_asset_manager.h"

#include "uncaught_exception_callback.h"

#include "base/log/ace_trace.h"

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
const std::string PACKAGE_NAME = "packageName";
const std::string BUNDLE_NAME = "bundleName";
const std::string MODULE_NAME = "moduleName";
const std::string VERSION = "version";
const std::string ENTRY_PATH = "entryPath";
const std::string IS_SO = "isSO";
const std::string DEPENDENCY_ALIAS = "dependencyAlias";

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

    napi_value LoadJsModule(const std::string& path, std::vector<uint8_t>& buffer, bool needUpdate) override
    {
        std::string assetPath = BUNDLE_INSTALL_PATH + moduleName_ + MERGE_ABC_PATH;
        HILOG_INFO("LoadJsModule path %{public}s", path.c_str());
        HILOG_INFO("LoadJsModule assetPath %{public}s", assetPath.c_str());
        HILOG_INFO("LoadJsModule moduleName_ %{public}s", moduleName_.c_str());
        panda::JSNApi::SetAssetPath(vm_, assetPath);
        panda::JSNApi::SetModuleName(vm_, moduleName_);
        
        NativeEngine* engine = reinterpret_cast<NativeEngine*>(env_);
        bool result = engine->RunScriptBuffer(path.c_str(), buffer, false, needUpdate) != nullptr;
        env_ = reinterpret_cast<napi_env>(engine);
        
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
        if (!env_) {
            HILOG_ERROR("pointer is nullptr.");
            return nullptr;
        }
        return ArkNativeEngine::ArkValueToNapiValue(env_, exportObj);
    }

private:
    static int32_t PrintVmLog(int32_t, int32_t, const char*, const char*, const char* message)
    {
        HILOG_INFO("ArkLog: %{public}s", message);
        return 0;
    }

    void GetPkgContextInfoListMap(const std::map<std::string, std::vector<uint8_t>>& contextInfoMap,
        std::map<std::string, std::vector<std::vector<std::string>>>& pkgContextInfoMap,
        std::map<std::string, std::string>& pkgAliasMap)
    {
        for (const auto& [moduleName, buffer] : contextInfoMap) {
            std::vector<std::vector<std::string>> pkgContextInfoList;
            nlohmann::json jsonObject = nlohmann::json::parse(buffer.data(), nullptr, false);
            if (jsonObject.is_discarded()) {
                HILOG_ERROR("moduleName: %{public}s parse json error", moduleName.c_str());
                continue;
            }
            ParsePkgContextInfoJson(jsonObject, pkgContextInfoList, pkgAliasMap);
            pkgContextInfoMap[moduleName] = pkgContextInfoList;
        }
    }

    void ParsePkgContextInfoJson(nlohmann::json& jsonObject, std::vector<std::vector<std::string>>& pkgContextInfoList,
        std::map<std::string, std::string>& pkgAliasMap)
    {
        for (nlohmann::json::iterator jsonIt = jsonObject.begin(); jsonIt != jsonObject.end(); ++jsonIt) {
            std::vector<std::string> items;
            items.emplace_back(jsonIt.key());
            nlohmann::json itemObject = jsonIt.value();
            std::string pkgName = "";
            items.emplace_back(PACKAGE_NAME);
            if (itemObject[PACKAGE_NAME].is_null() || !itemObject[PACKAGE_NAME].is_string()) {
                items.emplace_back(pkgName);
            } else {
                pkgName = itemObject[PACKAGE_NAME].get<std::string>();
                items.emplace_back(pkgName);
            }

            ParsePkgContextInfoJsonString(itemObject, BUNDLE_NAME, items);
            ParsePkgContextInfoJsonString(itemObject, MODULE_NAME, items);
            ParsePkgContextInfoJsonString(itemObject, VERSION, items);
            ParsePkgContextInfoJsonString(itemObject, ENTRY_PATH, items);
            items.emplace_back(IS_SO);
            if (itemObject[IS_SO].is_null() || !itemObject[IS_SO].is_boolean()) {
                items.emplace_back("false");
            } else {
                bool isSo = itemObject[IS_SO].get<bool>();
                if (isSo) {
                    items.emplace_back("true");
                } else {
                    items.emplace_back("false");
                }
            }
            if (!itemObject[DEPENDENCY_ALIAS].is_null() && itemObject[DEPENDENCY_ALIAS].is_string()) {
                std::string pkgAlias = itemObject[DEPENDENCY_ALIAS].get<std::string>();
                if (!pkgAlias.empty()) {
                    pkgAliasMap[pkgAlias] = pkgName;
                }
            }
            pkgContextInfoList.emplace_back(items);
        }
    }

    void ParsePkgContextInfoJsonString(
        const nlohmann::json& itemObject, const std::string& key, std::vector<std::string>& items)
    {
        items.emplace_back(key);
        if (itemObject[key].is_null() || !itemObject[key].is_string()) {
            items.emplace_back("");
        } else {
            items.emplace_back(itemObject[key].get<std::string>());
        }
    }

    bool Initialize(const Runtime::Options& options) override
    {
        Ace::AceScopedTrace trace("ArkJsRuntimeInit");
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
        env_ =  reinterpret_cast<napi_env>(new ArkNativeEngine(vm_, static_cast<JsRuntime*>(this)));
        
#ifdef ANDROID_PLATFORM
        std::vector<std::string> paths;
        if (!options.appLibPath.empty()) {
            paths.push_back(options.appLibPath);
        }
        bool isDataLibPathEmpty = options.appDataLibPath.empty();
        if (!isDataLibPathEmpty) {
            paths.push_back(options.appDataLibPath);
        }
        if (paths.size() != 0) {
            ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
            engine->SetPackagePath("default", paths);
            if (Platform::StageAssetManager::GetInstance()->IsDynamicLoadLibs() && !isDataLibPathEmpty) {
                engine->SetPackagePath(options.appDataLibPath, paths);
            }
            env_ = reinterpret_cast<napi_env>(engine);
        }
#else
#ifdef IOS_PLATFORM
        if (!options.codePath.empty()) {
            ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
            engine->SetPackagePath("default", { options.codePath });
            env_ = reinterpret_cast<napi_env>(engine);
        }
#endif
        if (!options.appLibPath.empty()) {
            ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
            engine->SetPackagePath("default", { options.appLibPath });
            env_ = reinterpret_cast<napi_env>(engine);
        }
#endif
        bundleName_ = options.bundleName;
        isBundle_ = options.isBundle;
        panda::JSNApi::SetBundle(vm_, options.isBundle);
        panda::JSNApi::SetBundleName(vm_, options.bundleName);
        panda::JSNApi::SetHostResolveBufferTracker(vm_, JsModuleReader(options.bundleName));
        std::map<std::string, std::vector<std::vector<std::string>>> pkgContextInfoMap;
        std::map<std::string, std::string> pkgAliasMap;
        GetPkgContextInfoListMap(options.pkgContextInfoJsonBufferMap, pkgContextInfoMap, pkgAliasMap);
        panda::JSNApi::SetpkgContextInfoList(vm_, pkgContextInfoMap);
        panda::JSNApi::SetPkgAliasList(vm_, pkgAliasMap);
        panda::JSNApi::SetPkgNameList(vm_, options.packageNameList);
        return JsRuntime::Initialize(options);
    }
};

class UvLoopHandler : public AppExecFwk::FileDescriptorListener, public std::enable_shared_from_this<UvLoopHandler> {
public:
    explicit UvLoopHandler(uv_loop_t* uvLoop) : uvLoop_(uvLoop) {}

    void OnReadable(int32_t) override
    {
        OnTriggered();
    }

    void OnWritable(int32_t) override
    {
        OnTriggered();
    }

private:
    void OnTriggered()
    {
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
    Ace::AceScopedTrace trace("JsRuntimeCreate");
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
    napi_env env, const std::string& moduleName, const napi_value* argv, size_t argc)
{
    HILOG_INFO("JsRuntime::LoadSystemModule(%{public}s)", moduleName.c_str());
    if (env == nullptr) {
        HILOG_INFO("JsRuntime::LoadSystemModule: invalid env.");
        return std::unique_ptr<NativeReference>();
    }

    napi_value globalObj = nullptr;
    napi_get_global(env, &globalObj);
    napi_value propertyValue = nullptr;
    napi_get_named_property(env, globalObj, "requireNapi", &propertyValue);
    
    std::unique_ptr<NativeReference> methodRequireNapiRef_;
    napi_ref tmpRef = nullptr;
    napi_create_reference(env, propertyValue, 1, &tmpRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
    
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return nullptr;
    }
    napi_value className = nullptr;
    napi_create_string_utf8(env, moduleName.c_str(), moduleName.length(), &className);
    auto refValue = methodRequireNapiRef_->GetNapiValue();
    napi_value args[1] = { className };
    napi_value classValue = nullptr;
    napi_call_function(env, globalObj, refValue, 1, args, &classValue);
    napi_value instanceValue = nullptr;
    napi_new_instance(env, classValue, argc, argv, &instanceValue);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

bool JsRuntime::Initialize(const Options& options)
{
    HandleScope handleScope(*this);
    napi_value globalObj = nullptr;
    napi_get_global(env_, &globalObj);

    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return false;
    }

    InitConsoleLogModule(env_, globalObj);

    // Simple hook function 'isSystemplugin'
    const char* moduleName = "JsRuntime";
    BindNativeFunction(env_, globalObj, "isSystemplugin", moduleName,
        [](napi_env env_, napi_callback_info cbinfo) -> napi_value {
            return CreateJsUndefined(env_);
        });

    napi_value propertyValue = nullptr;
    napi_get_named_property(env_, globalObj, "requireNapi", &propertyValue);
    napi_ref tmpRef = nullptr;
    napi_create_reference(env_, propertyValue, 1, &tmpRef);
    methodRequireNapiRef_.reset(reinterpret_cast<NativeReference*>(tmpRef));
    if (!methodRequireNapiRef_) {
        HILOG_ERROR("Failed to create reference for global.requireNapi");
        return false;
    }

#ifdef SUPPORT_GRAPHICS
    if (options.loadAce) {
        NativeEngine* engine = reinterpret_cast<NativeEngine*>(env_);
        OHOS::Ace::Platform::DeclarativeModulePreloader::Preload(*engine);
        env_ = reinterpret_cast<napi_env>(engine);
    }
#endif

    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(options.eventRunner);

    uv_loop_s* uvLoop = nullptr;
    napi_get_uv_event_loop(env_, &uvLoop);

    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd < 0) {
        HILOG_ERROR("Failed to get backend fd from uv loop");
        return false;
    }

    uv_run(uvLoop, UV_RUN_NOWAIT);
    codePath_ = options.codePath;
    uint32_t events = AppExecFwk::FILE_DESCRIPTOR_INPUT_EVENT | AppExecFwk::FILE_DESCRIPTOR_OUTPUT_EVENT;
    eventHandler_->AddFileDescriptorListener(fd, events, std::make_shared<UvLoopHandler>(uvLoop));
    InitTimerModule(env_, globalObj);
    auto engine = reinterpret_cast<NativeEngine*>(env_);
    InitWorkerModule(*engine, codePath_, options.isDebugVersion, options.isBundle);
    env_ = reinterpret_cast<napi_env>(engine);
    return true;
}

void JsRuntime::Deinitialize()
{
    for (auto it = modules_.begin(); it != modules_.end(); it = modules_.erase(it)) {
        delete it->second;
        it->second = nullptr;
    }

    methodRequireNapiRef_.reset();
    
    uv_loop_s* uvLoop = nullptr;
    napi_get_uv_event_loop(env_, &uvLoop);
    auto fd = uvLoop != nullptr ? uv_backend_fd(uvLoop) : -1;
    if (fd >= 0 && eventHandler_ != nullptr) {
        eventHandler_->RemoveFileDescriptorListener(fd);
    }
    RemoveTask(TIMER_TASK);
}

napi_value JsRuntime::LoadJsBundle(const std::string& path, std::vector<uint8_t>& buffer)
{
    HILOG_INFO("LoadJsBundle path %{public}s", path.c_str());
    HILOG_INFO("LoadJsBundle moduleName_ %{public}s", moduleName_.c_str());
    napi_value globalObj = nullptr;
    napi_get_global(env_, &globalObj);
    napi_value exports = nullptr;
    napi_create_object(env_, &exports);
    napi_set_named_property(env_, globalObj, "exports", exports);
    
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env_);
    bool result = engine->RunScriptBuffer(path.c_str(), buffer, isBundle_) != nullptr;
    env_ = reinterpret_cast<napi_env>(engine);
    
    if (!result) {
        HILOG_ERROR("RunScriptBuffer failed path: %{public}s", path.c_str());
        return nullptr;
    }
    
    napi_value exportsObj = nullptr;
    napi_get_named_property(env_, globalObj, "exports", &exportsObj);
    if (exportsObj == nullptr) {
        HILOG_ERROR("Failed to get exports objcect: %{private}s", path.c_str());
        return nullptr;
    }

    napi_value exportObj = nullptr;
    napi_get_named_property(env_, exportsObj, "default", &exportObj);
    if (exportObj == nullptr) {
        HILOG_ERROR("Failed to get default objcect: %{private}s", path.c_str());
        return nullptr;
    }

    return exportObj;
}

std::unique_ptr<NativeReference> JsRuntime::LoadModule(const std::string& moduleName, const std::string& modulePath,
    std::vector<uint8_t>& buffer, const std::string& srcEntrance, bool esmodule)
{
    HILOG_INFO("JsRuntime::LoadModule(%{public}s, %{public}s", moduleName.c_str(), modulePath.c_str());
    HandleScope handleScope(*this);

    std::string path = moduleName;
    auto pos = path.find("::");
    if (pos != std::string::npos) {
        path.erase(pos, path.size() - pos);
        moduleName_ = path;
    }

    napi_value classValue = nullptr;

    bool isDynamicUpdate = IsNeedUpdate(moduleName_, modulePath);
    panda::JSNApi::SetBundleName(vm_, bundleName_);
    auto it = modules_.find(modulePath);
    if (it != modules_.end()) {
        classValue = it->second->GetNapiValue();
    } else {
        if (esmodule) {
            std::string fileName;
            std::string moduleNamePath = (moduleName_.find_last_of('.') != std::string::npos)
                                             ? moduleName_.substr(moduleName_.find_last_of('.') + 1)
                                             : moduleName_;
            fileName.append(moduleNamePath).append("/").append(srcEntrance);
            fileName.erase(fileName.rfind("."));
            fileName.append(".abc");
            std::regex pattern(std::string("\\.") + std::string("/"));
            fileName = std::regex_replace(fileName, pattern, "");
            classValue = LoadJsModule(fileName, buffer, isDynamicUpdate);
        } else {
            classValue = LoadJsBundle(modulePath, buffer);
        }
        if (classValue == nullptr) {
            return std::unique_ptr<NativeReference>();
        }

        napi_ref tmpRef = nullptr;
        napi_create_reference(env_, classValue, 1, &tmpRef);
        modules_.emplace(modulePath, reinterpret_cast<NativeReference*>(tmpRef));
    }

    napi_value instanceValue = nullptr;
    napi_new_instance(env_, classValue, 0, nullptr, &instanceValue);
    if (instanceValue == nullptr) {
        HILOG_ERROR("Failed to create object instance");
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    napi_create_reference(env_, instanceValue, 1, &resultRef);
    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

bool JsRuntime::IsNeedUpdate(const std::string& moduleName, const std::string& modulePath)
{
    bool isDynamicUpdate = Platform::StageAssetManager::GetInstance()->IsDynamicUpdateModule(moduleName);
    if (isDynamicUpdate) {
        auto it = modules_.find(modulePath);
        if (it != modules_.end()) {
            if (it->second != nullptr) {
                delete it->second;
                it->second = nullptr;
            }
            modules_.erase(it);
        }
    }
    return isDynamicUpdate;
}

std::unique_ptr<NativeReference> JsRuntime::LoadSystemModule(
    const std::string& moduleName, napi_value* const* argv, size_t argc)
{
    return nullptr;
}

napi_env JsRuntime::GetNapiEnv() const
{
    return env_;
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
    auto debuggerPostTask = [eventHandler = eventHandler_](
                                std::function<void()>&& task) { eventHandler->PostTask(task); };

    debugMode_ = StartDebugMode(bundleName_, needBreakPoint, instanceId_, debuggerPostTask);
}

bool JsRuntime::StartDebugMode(
    const std::string& bundleName, bool needBreakPoint, uint32_t instanceId, const DebuggerPostTask& debuggerPostTask)
{
#ifdef DEBUG_MODE
    ConnectServerManager::Get().StartConnectServer(bundleName);
    ConnectServerManager::Get().AddInstance(instanceId);
#endif
    StartDebuggerInWorkerModule();
    return true;
}

void JsRuntime::RegisterUncaughtExceptionHandler(const JsEnv::UncaughtExceptionInfo& uncaughtExceptionInfo)
{
    ArkNativeEngine* engine = reinterpret_cast<ArkNativeEngine*>(env_);
    if (engine != nullptr) {
        engine->RegisterNapiUncaughtExceptionHandler(
            JsEnv::NapiUncaughtExceptionCallback(uncaughtExceptionInfo.uncaughtTask, nullptr, env_));
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
