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

#include "js_worker.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <unistd.h>

#include "connect_server_manager.h"
#include "commonlibrary/c_utils/base/include/refbase.h"
#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif
#include "hilog.h"
#include "js_console_log.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#ifdef ANDROID_PLATFORM
#include "stage_asset_provider.h"
#endif
#include "commonlibrary/ets_utils/js_sys_module/console/console.h"

#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t ASSET_FILE_MAX_SIZE = 32 * 1024 * 1024;
const std::string BUNDLE_NAME_FLAG = "@bundle:";
const std::string CACHE_DIRECTORY = "el2";
#if defined(ANDROID_PLATFORM) && !defined(IOS_PLATFORM)
    constexpr char ARK_DEBUGGER_LIB_PATH[] = "libark_debugger.z.so";
#else
#ifdef APP_USE_ARM
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib/libark_debugger.z.so";
#else
constexpr char ARK_DEBUGGER_LIB_PATH[] = "/system/lib64/libark_debugger.z.so";
#endif
#endif // ANDROID_PLATFORM

bool g_debugMode = false;

void InitWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_INFO("InitWorkerFunc called");
    if (nativeEngine == nullptr) {
        HILOG_ERROR("Input nativeEngine is nullptr");
        return;
    }

    NativeObject* globalObj = ConvertNativeValueTo<NativeObject>(nativeEngine->GetGlobal());
    if (globalObj == nullptr) {
        HILOG_ERROR("Failed to get global object");
        return;
    }
    
    InitConsoleLogModule(*nativeEngine, *globalObj);

    if (g_debugMode) {
        uint64_t instanceId;
#if !defined(IOS_PLATFORM)
        instanceId = gettid();
#else
        pthread_threadid_np(0, &instanceId);
#endif
        std::string instanceName = "workerThread_" + std::to_string(instanceId);
        bool needBreakPoint = ConnectServerManager::Get().AddInstance(instanceId, instanceName);
        auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
        auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
        auto workerPostTask = [nativeEngine](std::function<void()>&& callback) {
            nativeEngine->CallDebuggerPostTaskFunc(std::move(callback));
        };
#if !defined(IOS_PLATFORM)
        panda::JSNApi::StartDebugger(ARK_DEBUGGER_LIB_PATH, vm, needBreakPoint, instanceId, workerPostTask);
#else
        panda::JSNApi::StartDebugger(nullptr, vm, needBreakPoint, instanceId, workerPostTask);
#endif
    }
}

void OffWorkerFunc(NativeEngine* nativeEngine)
{
    HILOG_INFO("OffWorkerFunc called");
    if (nativeEngine == nullptr) {
        HILOG_ERROR("Input nativeEngine is nullptr");
        return;
    }

    if (g_debugMode) {
        uint64_t instanceId;
#if !defined(IOS_PLATFORM)
        instanceId = gettid();
#else
        pthread_threadid_np(0, &instanceId);
#endif
        ConnectServerManager::Get().RemoveInstance(instanceId);
        auto arkNativeEngine = static_cast<ArkNativeEngine*>(nativeEngine);
        auto vm = const_cast<EcmaVM*>(arkNativeEngine->GetEcmaVm());
        panda::JSNApi::StopDebugger(vm);
    }
}

bool ReadAssetData(const std::string& filePath, std::vector<uint8_t>& content, bool isDebugVersion)
{
    char path[PATH_MAX];
#if defined(ANDROID_PLATFORM)
    content =  Platform::StageAssetProvider::GetInstance()->GetAbcPathBuffer(filePath);
    return true;
#else
    if (realpath(filePath.c_str(), path) == nullptr) {
        HILOG_ERROR("ReadAssetData realpath(%{private}s) failed, errno = %{public}d", filePath.c_str(), errno);
        return false;
    }
#endif
    std::ifstream stream(path, std::ios::binary | std::ios::ate);
    if (!stream.is_open()) {
        HILOG_ERROR("ReadAssetData failed to open file %{private}s", filePath.c_str());
        return false;
    }

    auto fileLen = stream.tellg();
    if (!isDebugVersion && fileLen > ASSET_FILE_MAX_SIZE) {
        HILOG_ERROR("ReadAssetData failed, file is too large");
        return false;
    }

    content.resize(fileLen);

    stream.seekg(0);
    stream.read(reinterpret_cast<char*>(content.data()), content.size());
    return true;
}

struct AssetHelper final {
    explicit AssetHelper(const std::string& codePath, bool isDebugVersion)
        : codePath_(codePath), isDebugVersion_(isDebugVersion)
    {
        if (!codePath_.empty() && codePath.back() != '/') {
            codePath_.append("/");
        }
    }

    void operator()(const std::string& uri, std::vector<uint8_t>& content, std::string &ami) const
    {
        if (uri.empty()) {
            HILOG_ERROR("Uri is empty.");
            return;
        }

        HILOG_INFO("RegisterAssetFunc called, uri: %{private}s", uri.c_str());
        size_t index = uri.find_last_of(".");
        if (index == std::string::npos) {
            HILOG_ERROR("Invalid uri");
            return;
        }
#if defined(ANDROID_PLATFORM)
        ami = uri.substr(0, index) + ".abc";
#else
        ami = codePath_ + uri.substr(0, index) + ".abc";
#endif
        HILOG_INFO("Get asset, ami: %{private}s", ami.c_str());
        if (!ReadAssetData(ami, content, isDebugVersion_)) {
            HILOG_ERROR("Get asset content failed.");
            return;
        }
    }

    std::string codePath_;
    bool isDebugVersion_ = false;
};

int32_t GetContainerId()
{
#ifdef SUPPORT_GRAPHICS
    int32_t scopeId = ContainerScope::CurrentId();
    return scopeId;
#else
    constexpr int32_t containerScopeDefaultId = 0;
    return containerScopeDefaultId;
#endif
}
void UpdateContainerScope(int32_t id)
{
#ifdef SUPPORT_GRAPHICS
ContainerScope::UpdateCurrent(id);
#endif
}
void RestoreContainerScope(int32_t id)
{
#ifdef SUPPORT_GRAPHICS
ContainerScope::UpdateCurrent(-1);
#endif
}
}

void InitWorkerModule(NativeEngine& engine, const std::string& codePath, bool isDebugVersion)
{
    engine.SetInitWorkerFunc(InitWorkerFunc);
    engine.SetOffWorkerFunc(OffWorkerFunc);
    engine.SetGetAssetFunc(AssetHelper(codePath, isDebugVersion));
    engine.SetGetContainerScopeIdFunc(GetContainerId);
    engine.SetInitContainerScopeFunc(UpdateContainerScope);
    engine.SetFinishContainerScopeFunc(RestoreContainerScope);
}

void StartDebuggerInWorkerModule()
{
    g_debugMode = true;
}
} // namespace AbilityRuntime
} // namespace OHOS