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

#include "connect_server_manager.h"

#include <dlfcn.h>
#include <unistd.h>
#include "connect_inspector.h"
#include "hilog.h"

namespace OHOS::AbilityRuntime {
namespace {
std::string GetInstanceMapMessage(const std::string& messageType, int32_t instanceId, const std::string& instanceName)
{
    std::string message;
    message.append("{\"type\":\"");
    message.append(messageType);
    message.append("\",\"instanceId\":");
    message.append(std::to_string(instanceId));
    message.append(",\"name\":\"");
    message.append(instanceName);
    message.append("\",\"tid\":");
#if !defined(IOS_PLATFORM)
    message.append(std::to_string(gettid()));
#else
    uint64_t tid;
    pthread_threadid_np(0, &tid);
    message.append(std::to_string(tid));
#endif
    message.append("}");
    return message;
}
}
using namespace OHOS::ArkCompiler;
using StartServer = void (*)(const std::string&);
using SendMessage = void (*)(const std::string&);
using SendLayoutMessage = void (*)(const std::string&);
using StopServer = void (*)(const std::string&);
using StoreMessage = void (*)(int32_t, const std::string&);
using StoreInspectorInfo = void (*)(const std::string&, const std::string&);
using SetSwitchCallBack = void (*)(const std::function<void(bool)> &setStatus,
    const std::function<void(int32_t)> &createLayoutInfo, int32_t instanceId);
using RemoveMessage = void (*)(int32_t);
using WaitForConnection = bool (*)();

SendMessage g_sendMessage = nullptr;
SendLayoutMessage g_sendLayoutMessage = nullptr;
RemoveMessage g_removeMessage = nullptr;
StoreInspectorInfo g_storeInspectorInfo = nullptr;
StoreMessage g_storeMessage = nullptr;
SetSwitchCallBack g_setSwitchCallBack = nullptr;
WaitForConnection g_waitForConnection = nullptr;

ConnectServerManager::~ConnectServerManager()
{
    StopConnectServer();
    CloseConnectServerSo();
}

ConnectServerManager& ConnectServerManager::Get()
{
    static ConnectServerManager connectServerManager;
    return connectServerManager;
}

bool ConnectServerManager::InitFunc()
{
#if !defined(IOS_PLATFORM)
    g_sendMessage = reinterpret_cast<SendMessage>(dlsym(handlerConnectServerSo_, "SendMessage"));
    g_storeMessage = reinterpret_cast<StoreMessage>(dlsym(handlerConnectServerSo_, "StoreMessage"));
    g_removeMessage = reinterpret_cast<RemoveMessage>(dlsym(handlerConnectServerSo_, "RemoveMessage"));
    g_setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    g_sendLayoutMessage = reinterpret_cast<SendLayoutMessage>(dlsym(handlerConnectServerSo_, "SendLayoutMessage"));
    g_storeInspectorInfo = reinterpret_cast<StoreInspectorInfo>(dlsym(handlerConnectServerSo_, "StoreInspectorInfo"));
    g_waitForConnection = reinterpret_cast<WaitForConnection>(dlsym(handlerConnectServerSo_, "WaitForConnection"));
#else
    using namespace OHOS::ArkCompiler;
    g_sendMessage = reinterpret_cast<SendMessage>(&Toolchain::SendMessage);
    g_storeMessage = reinterpret_cast<StoreMessage>(&Toolchain::StoreMessage);
    g_removeMessage = reinterpret_cast<RemoveMessage>(&Toolchain::RemoveMessage);
    g_setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(&Toolchain::SetSwitchCallBack);
    g_sendLayoutMessage = reinterpret_cast<SendLayoutMessage>(&Toolchain::SendLayoutMessage);
    g_storeInspectorInfo = reinterpret_cast<StoreInspectorInfo>(&Toolchain::StoreInspectorInfo);
    g_waitForConnection = reinterpret_cast<WaitForConnection>(&Toolchain::WaitForConnection);
#endif
    if (g_sendMessage == nullptr || g_storeMessage == nullptr || g_removeMessage == nullptr) {
        CloseConnectServerSo();
        return false;
    }

    if (g_storeInspectorInfo == nullptr || g_setSwitchCallBack == nullptr || g_waitForConnection == nullptr ||
        g_sendLayoutMessage == nullptr) {
        CloseConnectServerSo();
        return false;
    }
    return true;
}

void ConnectServerManager::StartConnectServer(const std::string& bundleName)
{
    HILOG_DEBUG("ConnectServerManager::StartConnectServer Start connect server");
#if !defined(IOS_PLATFORM)
#if defined(ANDROID_PLATFORM)
    const std::string soDir = "libconnectserver_debugger.so";
#else
    const std::string soDir = "libconnectserver_debugger.z.so";
#endif // ANDROID_PLATFORM
    handlerConnectServerSo_ = dlopen(soDir.c_str(), RTLD_LAZY);
    if (handlerConnectServerSo_ == nullptr) {
        HILOG_ERROR("ConnectServerManager::StartConnectServer failed to open register library");
        return;
    }
    auto startServer = reinterpret_cast<StartServer>(dlsym(handlerConnectServerSo_, "StartServer"));
#else
    auto startServer = reinterpret_cast<StartServer>(&ArkCompiler::Toolchain::StartServer);
#endif // IOS_PLATFORM
    if (startServer == nullptr|| !InitFunc()) {
        HILOG_ERROR("ConnectServerManager::StartConnectServer failed to find symbol 'StartServer'");
        return;
    }
    bundleName_ = bundleName;
    startServer(bundleName_);
}

void ConnectServerManager::CloseConnectServerSo()
{
#if !defined(IOS_PLATFORM)
    if (handlerConnectServerSo_ == nullptr) {
        HILOG_ERROR("ConnectServerManager::CloseConnectServerSo handlerConnectServerSo_ is nullptr");
        return;
    }
    dlclose(handlerConnectServerSo_);
    handlerConnectServerSo_ = nullptr;
#endif
}

void ConnectServerManager::StopConnectServer()
{
    HILOG_DEBUG("ConnectServerManager::StopConnectServer Stop connect server");
#if !defined(IOS_PLATFORM)
    if (handlerConnectServerSo_ == nullptr) {
        HILOG_ERROR("ConnectServerManager::StopConnectServer handlerConnectServerSo_ is nullptr");
        return;
    }
    auto stopServer = reinterpret_cast<StopServer>(dlsym(handlerConnectServerSo_, "StopServer"));
#else
    auto stopServer = reinterpret_cast<StopServer>(&ArkCompiler::Toolchain::StopServer);
#endif
    if (stopServer != nullptr) {
        stopServer(bundleName_);
    } else {
        HILOG_ERROR("ConnectServerManager::StopConnectServer failed to find symbol 'StopServer'");
    }
}

bool ConnectServerManager::AddInstance(int32_t instanceId, const std::string& instanceName)
{
    HILOG_DEBUG("ConnectServerManager::AddInstance Add instance to connect server");

    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto result = instanceMap_.try_emplace(instanceId, instanceName);
        if (!result.second) {
            HILOG_WARN("ConnectServerManager::AddInstance Instance %{public}d already added", instanceId);
            return false;
        }
    }

    auto setSwitchCallBack = reinterpret_cast<SetSwitchCallBack>(
        dlsym(handlerConnectServerSo_, "SetSwitchCallBack"));
    if (setSwitchCallBack == nullptr) {
        HILOG_ERROR("ConnectServerManager::AddInstance failed to find symbol 'setSwitchCallBack'");
        return false;
    }
    setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);

    // Get the message including information of new instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("addInstance", instanceId, instanceName);

    if (!g_waitForConnection()) { // g_waitForConnection : the res means the connection state of the connect server
        g_sendMessage(message); // if connected, message will be sent immediately.
    } else { // if not connected, message will be stored and sent later when "connected" coming.
        g_storeMessage(instanceId, message);
    }
    g_setSwitchCallBack([this](bool status) { setStatus_(status); },
        [this](int32_t containerId) { createLayoutInfo_(containerId); }, instanceId);
    return true;
}

void ConnectServerManager::RemoveInstance(int32_t instanceId)
{
    HILOG_DEBUG("ConnectServerManager::RemoveInstance Remove instance to connect server");
    std::string instanceName;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = instanceMap_.find(instanceId);
        if (it == instanceMap_.end()) {
            HILOG_WARN("ConnectServerManager::RemoveInstance Instance %{public}d is not found", instanceId);
            return;
        }

        instanceName = std::move(it->second);
        instanceMap_.erase(it);
    }
    // Get the message including information of deleted instance, which will be send to IDE.
    std::string message = GetInstanceMapMessage("destroyInstance", instanceId, instanceName);

    if (!g_waitForConnection()) {
        g_sendMessage(message);
    } else {
        g_removeMessage(instanceId);
    }
}

void ConnectServerManager::SendInspector(const std::string& jsonTreeStr, const std::string& jsonSnapshotStr)
{
    LOGI("ConnectServerManager SendInspector Start");
    g_sendLayoutMessage(jsonTreeStr);
    g_sendLayoutMessage(jsonSnapshotStr);
    g_storeInspectorInfo(jsonTreeStr, jsonSnapshotStr);
}

void ConnectServerManager::SetLayoutInspectorCallback(
    const std::function<void(int32_t)>& createLayoutInfo, const std::function<void(bool)>& setStatus)
{
    createLayoutInfo_ = createLayoutInfo;
    setStatus_ = setStatus;
}

std::function<void(int32_t)> ConnectServerManager::GetLayoutInspectorCallback()
{
    return createLayoutInfo_;
}
} // namespace OHOS::AbilityRuntime