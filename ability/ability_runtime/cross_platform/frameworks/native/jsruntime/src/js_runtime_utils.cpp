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

#include "js_runtime_utils.h"

#include "hilog.h"
#include "js_runtime.h"
namespace OHOS {
namespace AbilityRuntime {
namespace {
std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    std::unique_ptr<AsyncTask::ExecuteCallback>&& execute, std::unique_ptr<AsyncTask::CompleteCallback>&& complete,
    NativeValue** result)
{
    if (lastParam == nullptr || lastParam->TypeOf() != NATIVE_FUNCTION) {
        NativeDeferred* nativeDeferred = nullptr;
        *result = engine.CreatePromise(&nativeDeferred);
        return std::make_unique<AsyncTask>(nativeDeferred, std::move(execute), std::move(complete));
    } else {
        *result = engine.CreateUndefined();
        NativeReference* callbackRef = engine.CreateReference(lastParam, 1);
        return std::make_unique<AsyncTask>(callbackRef, std::move(execute), std::move(complete));
    }
}
} // namespace

// Help Functions
NativeValue* CreateJsError(NativeEngine& engine, int32_t errCode, const std::string& message)
{
    return engine.CreateError(CreateJsValue(engine, errCode), CreateJsValue(engine, message));
}

void BindNativeFunction(
    NativeEngine& engine, NativeObject& object, const char* name, const char* moduleName, NativeCallback func)
{
    std::string fullName(moduleName);
    fullName += ".";
    fullName += name;
    object.SetProperty(name, engine.CreateFunction(fullName.c_str(), fullName.length(), func, nullptr));
}

void BindNativeProperty(NativeObject& object, const char* name, NativeCallback getter)
{
    NativePropertyDescriptor property;
    property.utf8name = name;
    property.name = nullptr;
    property.method = nullptr;
    property.getter = getter;
    property.setter = nullptr;
    property.value = nullptr;
    property.attributes = napi_default;
    property.data = nullptr;
    object.DefineProperty(property);
}

void* GetNativePointerFromCallbackInfo(const NativeEngine* engine, NativeCallbackInfo* info, const char* name)
{
    if (engine == nullptr || info == nullptr) {
        return nullptr;
    }

    NativeObject* object = ConvertNativeValueTo<NativeObject>(info->thisVar);
    if (object != nullptr && name != nullptr) {
        object = ConvertNativeValueTo<NativeObject>(object->GetProperty(name));
    }
    return (object != nullptr) ? object->GetNativePointer() : nullptr;
}

void SetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name, void* ptr, NativeFinalize func)
{
    NativeValue* value = engine.CreateObject();
    NativeObject* newObject = ConvertNativeValueTo<NativeObject>(value);
    if (newObject == nullptr) {
        return;
    }
    newObject->SetNativePointer(ptr, func, nullptr);
    object.SetProperty(name, value);
}

void* GetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name)
{
    NativeObject* namedObj = ConvertNativeValueTo<NativeObject>(object.GetProperty(name));
    return (namedObj != nullptr) ? namedObj->GetNativePointer() : nullptr;
}

// Handle Scope
HandleScope::HandleScope(JsRuntime& jsRuntime)
{
    scopeManager_ = jsRuntime.GetNativeEngine().GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->Open();
    }
}

HandleScope::HandleScope(NativeEngine& engine)
{
    scopeManager_ = engine.GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->Open();
    }
}

HandleScope::~HandleScope()
{
    if (nativeScope_ != nullptr) {
        scopeManager_->Close(nativeScope_);
        nativeScope_ = nullptr;
    }
    scopeManager_ = nullptr;
}

// Handle Escape
HandleEscape::HandleEscape(JsRuntime& jsRuntime)
{
    scopeManager_ = jsRuntime.GetNativeEngine().GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->OpenEscape();
    }
}

HandleEscape::HandleEscape(NativeEngine& engine)
{
    scopeManager_ = engine.GetScopeManager();
    if (scopeManager_ != nullptr) {
        nativeScope_ = scopeManager_->OpenEscape();
    }
}

HandleEscape::~HandleEscape()
{
    if (nativeScope_ != nullptr) {
        scopeManager_->CloseEscape(nativeScope_);
        nativeScope_ = nullptr;
    }
    scopeManager_ = nullptr;
}

NativeValue* HandleEscape::Escape(NativeValue* value)
{
    if (nativeScope_ != nullptr) {
        scopeManager_->Escape(nativeScope_, value);
    }
    return value;
}

// Async Task
AsyncTask::AsyncTask(NativeDeferred* deferred, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : deferred_(deferred), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::AsyncTask(NativeReference* callbackRef, std::unique_ptr<AsyncTask::ExecuteCallback>&& execute,
    std::unique_ptr<AsyncTask::CompleteCallback>&& complete)
    : callbackRef_(callbackRef), execute_(std::move(execute)), complete_(std::move(complete))
{}

AsyncTask::~AsyncTask() = default;

void AsyncTask::Schedule(const std::string& name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task)
{
    if (task && task->Start(name, engine)) {
        task.release();
    }
}

void AsyncTask::Resolve(NativeEngine& engine, NativeValue* value)
{
    HILOG_INFO("AsyncTask::Resolve is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            CreateJsError(engine, 0),
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_INFO("AsyncTask::Resolve is called end.");
}

void AsyncTask::ResolveWithNoError(NativeEngine& engine, NativeValue* value)
{
    HILOG_INFO("AsyncTask::Resolve is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            engine.CreateNull(),
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_INFO("AsyncTask::Resolve is called end.");
}

void AsyncTask::Reject(NativeEngine& engine, NativeValue* error)
{
    if (deferred_) {
        deferred_->Reject(error);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            engine.CreateUndefined(),
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
}

void AsyncTask::ResolveWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value)
{
    HILOG_INFO("AsyncTask::ResolveWithCustomize is called");
    if (deferred_) {
        deferred_->Resolve(value);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_INFO("AsyncTask::ResolveWithCustomize is called end.");
}

void AsyncTask::RejectWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value)
{
    HILOG_INFO("AsyncTask::RejectWithCustomize is called");
    if (deferred_) {
        deferred_->Reject(error);
        deferred_.reset();
    }
    if (callbackRef_) {
        NativeValue* argv[] = {
            error,
            value,
        };
        engine.CallFunction(engine.CreateUndefined(), callbackRef_->Get(), argv, ArraySize(argv));
        callbackRef_.reset();
    }
    HILOG_INFO("AsyncTask::RejectWithCustomize is called end.");
}

void AsyncTask::Execute(NativeEngine* engine, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    auto me = static_cast<AsyncTask*>(data);
    if (me->execute_ && *(me->execute_)) {
        (*me->execute_)();
    }
}

void AsyncTask::Complete(NativeEngine* engine, int32_t status, void* data)
{
    if (engine == nullptr || data == nullptr) {
        return;
    }
    std::unique_ptr<AsyncTask> me(static_cast<AsyncTask*>(data));
    if (me->complete_ && *(me->complete_)) {
        HandleScope handleScope(*engine);
        (*me->complete_)(*engine, *me, status);
    }
}

bool AsyncTask::Start(const std::string& name, NativeEngine& engine)
{
    work_.reset(engine.CreateAsyncWork(name, Execute, Complete, this));
    return work_->Queue();
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam,
        std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)),
        std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(
    NativeEngine& engine, NativeValue* lastParam, AsyncTask::ExecuteCallback&& execute, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, std::make_unique<AsyncTask::ExecuteCallback>(std::move(execute)), nullptr, result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam, nullptr_t,
    AsyncTask::CompleteCallback&& complete, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(
        engine, lastParam, nullptr, std::make_unique<AsyncTask::CompleteCallback>(std::move(complete)), result);
}

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(
    NativeEngine& engine, NativeValue* lastParam, nullptr_t, nullptr_t, NativeValue** result)
{
    return CreateAsyncTaskWithLastParam(engine, lastParam, std::unique_ptr<AsyncTask::ExecuteCallback>(),
        std::unique_ptr<AsyncTask::CompleteCallback>(), result);
}
} // namespace AbilityRuntime
} // namespace OHOS
