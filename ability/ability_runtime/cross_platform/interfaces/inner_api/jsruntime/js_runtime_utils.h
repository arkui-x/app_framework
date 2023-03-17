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

#ifndef OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H

#include <cstdint>
#include <memory>
#include <sstream>
#include <type_traits>

#include "js_runtime.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
template<class T>
inline T* ConvertNativeValueTo(NativeValue* value)
{
    return (value != nullptr) ? static_cast<T*>(value->GetInterface(T::INTERFACE_ID)) : nullptr;
}

template<typename T, size_t N>
inline constexpr size_t ArraySize(T (&)[N]) noexcept
{
    return N;
}

template<class T>
inline NativeValue* CreateJsValue(NativeEngine& engine, const T& value)
{
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        return engine.CreateBoolean(value);
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        return engine.CreateNumber(value);
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        return engine.CreateString(value.c_str(), value.length());
    } else if constexpr (std::is_enum_v<ValueType>) {
        return engine.CreateNumber(static_cast<std::make_signed_t<ValueType>>(value));
    } else if constexpr (std::is_same_v<ValueType, const char*>) {
        return (value != nullptr) ? engine.CreateString(value, strlen(value)) : engine.CreateUndefined();
    }
    return engine.CreateUndefined();
}

template<class T>
inline bool ConvertFromJsValue(NativeEngine& engine, NativeValue* jsValue, T& value)
{
    if (jsValue == nullptr) {
        return false;
    }

    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (std::is_same_v<ValueType, bool>) {
        auto boolValue = ConvertNativeValueTo<NativeBoolean>(jsValue);
        if (boolValue == nullptr) {
            return false;
        }
        value = *boolValue;
        return true;
    } else if constexpr (std::is_arithmetic_v<ValueType>) {
        auto numberValue = ConvertNativeValueTo<NativeNumber>(jsValue);
        if (numberValue == nullptr) {
            return false;
        }
        value = *numberValue;
        return true;
    } else if constexpr (std::is_same_v<ValueType, std::string>) {
        auto stringValue = ConvertNativeValueTo<NativeString>(jsValue);
        if (stringValue == nullptr) {
            return false;
        }
        size_t len = stringValue->GetLength() + 1;
        auto buffer = std::make_unique<char[]>(len);
        size_t strLength = 0;
        stringValue->GetCString(buffer.get(), len, &strLength);
        value = buffer.get();
        return true;
    } else if constexpr (std::is_enum_v<ValueType>) {
        auto numberValue = ConvertNativeValueTo<NativeNumber>(jsValue);
        if (numberValue == nullptr) {
            return false;
        }
        value = static_cast<ValueType>(static_cast<std::make_signed_t<ValueType>>(*numberValue));
        return true;
    }
}

template<class T>
NativeValue* CreateNativeArray(NativeEngine& engine, const std::vector<T>& data)
{
    NativeValue* arrayValue = engine.CreateArray(data.size());
    NativeArray* array = ConvertNativeValueTo<NativeArray>(arrayValue);
    uint32_t index = 0;
    for (const T& item : data) {
        array->SetElement(index++, CreateJsValue(engine, item));
    }
    return arrayValue;
}

NativeValue* CreateJsError(NativeEngine& engine, int32_t errCode, const std::string& message = std::string());
void BindNativeFunction(
    NativeEngine& engine, NativeObject& object, const char* name, const char* moduleName, NativeCallback func);
void BindNativeProperty(NativeObject& object, const char* name, NativeCallback getter);
void* GetNativePointerFromCallbackInfo(const NativeEngine* engine, NativeCallbackInfo* info, const char* name);

void SetNamedNativePointer(
    NativeEngine& engine, NativeObject& object, const char* name, void* ptr, NativeFinalize func);
void* GetNamedNativePointer(NativeEngine& engine, NativeObject& object, const char* name);

template<class T>
T* CheckParamsAndGetThis(NativeEngine* engine, NativeCallbackInfo* info, const char* name = nullptr)
{
    return static_cast<T*>(GetNativePointerFromCallbackInfo(engine, info, name));
}

class HandleScope final {
public:
    explicit HandleScope(JsRuntime& jsRuntime);
    explicit HandleScope(NativeEngine& engine);
    ~HandleScope();

    HandleScope(const HandleScope&) = delete;
    HandleScope(HandleScope&&) = delete;
    HandleScope& operator=(const HandleScope&) = delete;
    HandleScope& operator=(HandleScope&&) = delete;

private:
    NativeScopeManager* scopeManager_ = nullptr;
    NativeScope* nativeScope_ = nullptr;
};

class HandleEscape final {
public:
    explicit HandleEscape(JsRuntime& jsRuntime);
    explicit HandleEscape(NativeEngine& engine);
    ~HandleEscape();

    NativeValue* Escape(NativeValue* value);

    HandleEscape(const HandleEscape&) = delete;
    HandleEscape(HandleEscape&&) = delete;
    HandleEscape& operator=(const HandleEscape&) = delete;
    HandleEscape& operator=(HandleEscape&&) = delete;

private:
    NativeScopeManager* scopeManager_ = nullptr;
    NativeScope* nativeScope_ = nullptr;
};

class AsyncTask final {
public:
    using ExecuteCallback = std::function<void()>;
    using CompleteCallback = std::function<void(NativeEngine&, AsyncTask&, int32_t)>;

    static void Schedule(const std::string& name, NativeEngine& engine, std::unique_ptr<AsyncTask>&& task);

    AsyncTask(NativeDeferred* deferred, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    AsyncTask(NativeReference* callbackRef, std::unique_ptr<ExecuteCallback>&& execute,
        std::unique_ptr<CompleteCallback>&& complete);
    ~AsyncTask();

    void Resolve(NativeEngine& engine, NativeValue* value);
    void Reject(NativeEngine& engine, NativeValue* error);
    void ResolveWithNoError(NativeEngine& engine, NativeValue* value);
    void ResolveWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value);
    void RejectWithCustomize(NativeEngine& engine, NativeValue* error, NativeValue* value);

private:
    static void Execute(NativeEngine* engine, void* data);
    static void Complete(NativeEngine* engine, int32_t status, void* data);

    bool Start(const std::string& name, NativeEngine& engine);

    std::unique_ptr<NativeDeferred> deferred_;
    std::unique_ptr<NativeReference> callbackRef_;
    std::unique_ptr<NativeAsyncWork> work_;
    std::unique_ptr<ExecuteCallback> execute_;
    std::unique_ptr<CompleteCallback> complete_;
};

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, AsyncTask::CompleteCallback&& complete, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam,
    AsyncTask::ExecuteCallback&& execute, nullptr_t, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(NativeEngine& engine, NativeValue* lastParam, nullptr_t,
    AsyncTask::CompleteCallback&& complete, NativeValue** result);

std::unique_ptr<AsyncTask> CreateAsyncTaskWithLastParam(
    NativeEngine& engine, NativeValue* lastParam, nullptr_t, nullptr_t, NativeValue** result);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_RUNTIME_UTILS_H
