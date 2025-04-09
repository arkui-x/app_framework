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

#include "js_error_manager.h"

#include <cstdint>
#include <unistd.h>

#include "ability_business_error.h"
#include "application_data_manager.h"
#include "event_runner.h"
#include "hilog.h"
#include "js_error_observer.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
struct JsLoopObserver {
    std::shared_ptr<AppExecFwk::EventRunner> mainRunner;
    std::shared_ptr<NativeReference> observerObject;
    napi_env env;
};
struct WorkItem {
    uv_work_t work;
    napi_ref ref;
    napi_env env;
    std::string instanceName;
    uint32_t instanceType;
    std::string name;
    std::string message;
    std::string stack;
};
struct GlobalObserverItem {
    napi_ref ref;
    napi_env env;
    bool operator<(const GlobalObserverItem& other) const
    {
        return ref < other.ref;
    }
};
static std::set<GlobalObserverItem> globalObserverList;
static std::set<GlobalObserverItem> globalPromiseList;
static std::map<napi_env, std::vector<std::pair<int32_t, std::shared_ptr<NativeReference>>>> observerList;
static std::mutex globalErrorMtx;
static std::mutex globalPromiseMtx;
static std::recursive_mutex errorMtx;
static std::shared_ptr<JsLoopObserver> loopObserver;
static std::once_flag registerCallbackFlag;
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr const char* ON_OFF_TYPE = "error";
constexpr const char* GLOBAL_ON_OFF_TYPE = "globalErrorOccurred";
constexpr const char* ON_OFF_TYPE_UNHANDLED_REJECTION = "unhandledRejection";
constexpr const char* GLOBAL_ON_OFF_TYPE_UNHANDLED_REJECTION = "globalUnhandledRejectionDetected";
constexpr const char* ON_OFF_TYPE_SYNC = "errorEvent";
constexpr const char* ON_OFF_TYPE_SYNC_LOOP = "loopObserver";
constexpr uint32_t INITITAL_REFCOUNT_ONE = 1;

thread_local std::set<napi_ref> unhandledRejectionObservers;
thread_local std::map<napi_ref, napi_ref> pendingUnHandledRejections;

napi_value AddRejection(napi_env env, napi_value promise, napi_value reason)
{
    napi_ref promiseRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, promise, INITITAL_REFCOUNT_ONE, &promiseRef));
    napi_ref reasonRef = nullptr;
    NAPI_CALL(env, napi_create_reference(env, reason, INITITAL_REFCOUNT_ONE, &reasonRef));
    pendingUnHandledRejections.insert(std::make_pair(promiseRef, reasonRef));
    return CreateJsUndefined(env);
}

napi_value RemoveRejection(napi_env env, napi_value promise)
{
    napi_value ret = CreateJsUndefined(env);
    auto iter = pendingUnHandledRejections.begin();
    while (iter != pendingUnHandledRejections.end()) {
        napi_value prom = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, iter->first, &prom));
        bool isEquals = false;
        NAPI_CALL(env, napi_strict_equals(env, promise, prom, &isEquals));
        if (isEquals) {
            NAPI_CALL(env, napi_delete_reference(env, iter->first));
            NAPI_CALL(env, napi_delete_reference(env, iter->second));
            pendingUnHandledRejections.erase(iter);
            return ret;
        }
        ++iter;
    }
    return ret;
}

napi_value UnhandledRejectionHandler(napi_env env, napi_value promise, napi_value reason)
{
    napi_value global = nullptr;
    NAPI_CALL(env, napi_get_global(env, &global));
    size_t argc = ARGC_TWO;
    napi_value args[] = {reason, promise};
    for (auto& iter : unhandledRejectionObservers) {
        napi_value cb = nullptr;
        NAPI_CALL(env, napi_get_reference_value(env, iter, &cb));
        napi_value result = nullptr;
        NAPI_CALL(env, napi_call_function(env, global, cb, argc, args, &result));
    }
    return CreateJsUndefined(env);
}

static napi_value OnUnhandledRejection(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_THREE;
    napi_value argv[ARGC_THREE] = {0};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));
    int32_t event = 0;
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &event));
    if (event == static_cast<int32_t>(UnhandledRejectionEvent::REJECT)) {
        return AddRejection(env, argv[INDEX_ONE], argv[INDEX_TWO]);
    }
    if (event == static_cast<int32_t>(UnhandledRejectionEvent::HANDLE)) {
        return RemoveRejection(env, argv[INDEX_ONE]);
    }
    ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    return CreateJsUndefined(env);
}

static napi_value NotifyUnhandledRejectionHandler(napi_env env, napi_callback_info info)
{
    if (!pendingUnHandledRejections.empty()) {
        auto iter = pendingUnHandledRejections.begin();
        while (iter != pendingUnHandledRejections.end()) {
            napi_value promise = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter->first, &promise));
            napi_value reason = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter->second, &reason));

            UnhandledRejectionHandler(env, promise, reason);

            NAPI_CALL(env, napi_delete_reference(env, iter->first));
            NAPI_CALL(env, napi_delete_reference(env, iter->second));
            iter = pendingUnHandledRejections.erase(iter);
        }
    }
    return CreateJsUndefined(env);
}

static bool IsobserverListNotEmpty()
{
    std::lock_guard<std::recursive_mutex> lock(errorMtx);
    return !observerList.empty();
}

static bool IsGlobalObserverListNotEmpty()
{
    std::lock_guard<std::mutex> lock(globalErrorMtx);
    return !globalObserverList.empty();
}

static bool IsErrorObserverListNotEmpty()
{
    return IsobserverListNotEmpty() || IsGlobalObserverListNotEmpty();
}

static bool IsGlobalPromiseListNotEmpty()
{
    std::lock_guard<std::mutex> lock(globalPromiseMtx);
    return !globalPromiseList.empty();
}

static std::string GetContent(napi_env env, napi_value exception, const std::string& name)
{
    napi_value tempContent;
    std::string content;
    napi_value propertyName = nullptr;
    std::string property = name;
    napi_create_string_utf8(env, property.c_str(), property.size(), &propertyName);
    napi_get_property(env, exception, propertyName, &tempContent);
    size_t length = 0;
    napi_get_value_string_utf8(env, tempContent, nullptr, 0, &length);
    content.resize(length);
    napi_get_value_string_utf8(env, tempContent, content.data(), content.size() + 1, &length);
    return content;
}

static napi_value CreateGlobalObject(napi_env env, WorkItem* item)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        return objValue;
    }
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, item->name));
    napi_set_named_property(env, objValue, "message", CreateJsValue(env, item->message));
    napi_set_named_property(env, objValue, "stack", CreateJsValue(env, item->stack));
    napi_set_named_property(env, objValue, "instanceName", CreateJsValue(env, item->instanceName));
    napi_set_named_property(env, objValue, "instanceType", CreateJsValue(env, item->instanceType));
    return objValue;
}

static void CallJsFunction(napi_env env, napi_value obj, const char* methodName, napi_value const* argv, size_t argc)
{
    if (obj == nullptr) {
        HILOG_ERROR("CallJsFunction null obj, methodName:%{public}s", methodName);
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, methodName, &method);
    if (method == nullptr) {
        HILOG_ERROR("CallJsFunction null method, methodName:%{public}s", methodName);
        return;
    }
    napi_value callResult = nullptr;
    napi_call_function(env, obj, method, argc, argv, &callResult);
}

static void DoFunctionCallback(uv_work_t* reqwork, int status)
{
    WorkItem* newItem = static_cast<WorkItem*>(reqwork->data);
    if (newItem == nullptr) {
        return;
    }
    napi_value global = nullptr;
    if (napi_get_global(newItem->env, &global) != napi_ok) {
        delete newItem;
        newItem = nullptr;
        return;
    }

    size_t argc = ARGC_ONE;
    napi_value args[] = {CreateGlobalObject(newItem->env, newItem)};

    napi_value function = nullptr;
    if (napi_get_reference_value(newItem->env, newItem->ref, &function) != napi_ok) {
        delete newItem;
        newItem = nullptr;
        return;
    }

    napi_value result = nullptr;
    if (napi_call_function(newItem->env, global, function, argc, args, &result) != napi_ok) {
        HILOG_INFO("DoFunctionCallback Do Callback Failed");
    }
    delete newItem;
    newItem = nullptr;
}

static void DoCallbackInRegesterThread(napi_env env, WorkItem& info)
{
    std::lock_guard<std::mutex> lock(globalErrorMtx);
    for (auto iter : globalObserverList) {
        uv_loop_t* loop = nullptr;
        if (napi_get_uv_event_loop(iter.env, &loop) != napi_ok) {
            continue;
        }
        WorkItem* item = new (std::nothrow) WorkItem();
        if (item == nullptr) {
            continue;
        }
        item->env = iter.env;
        item->ref = iter.ref;
        item->instanceName = info.instanceName;
        item->instanceType = info.instanceType;
        item->work.data = item;
        item->name = info.name;
        item->stack = info.stack;
        item->message = info.message;
        int ret = uv_queue_work(
            loop, &item->work, [](uv_work_t* reqwork) {}, DoFunctionCallback);
        if (ret != 0) {
            if (item != nullptr) {
                delete item;
                item = nullptr;
            }
        }
    }
}

static void DoGlobalCallback(napi_env env, napi_value exception, const std::string& instanceName, uint32_t type)
{
    {
        std::lock_guard<std::mutex> lock(globalErrorMtx);
        if (globalObserverList.empty()) {
            return;
        }
    }
    if (exception == nullptr) {
        return;
    }
    std::string name = GetContent(env, exception, "name");
    std::string stack = GetContent(env, exception, "stack");
    std::string message = GetContent(env, exception, "message");
    WorkItem item;
    item.name = name;
    item.message = message;
    item.stack = stack;
    item.instanceName = instanceName;
    item.instanceType = type;
    DoCallbackInRegesterThread(env, item);
}

static napi_value CreateJsErrorObject(napi_env env, const std::string& name, const std::string& message,
    const std::string& stack)
{
    napi_value objValue = nullptr;
    napi_create_object(env, &objValue);
    if (objValue == nullptr) {
        return objValue;
    }
    napi_set_named_property(env, objValue, "name", CreateJsValue(env, name));
    napi_set_named_property(env, objValue, "message", CreateJsValue(env, message));
    if (!stack.empty()) {
        napi_set_named_property(env, objValue, "stack", CreateJsValue(env, stack));
    }

    return objValue;
}

static void DoMainThreadOnException(napi_env env, const std::string& name, const std::string& message,
    const std::string& stack)
{
    std::lock_guard<std::recursive_mutex> lock(errorMtx);
    if (observerList.empty()) {
        return;
    }
    if (!observerList.count(env)) {
        return;
    }

    std::vector<std::pair<int32_t, std::shared_ptr<NativeReference>>> functions = observerList[env];
    for (auto& functionTemp : functions) {
        size_t argc = ARGC_ONE;
        napi_value args[] = {CreateJsErrorObject(env, name, message, stack)};
        CallJsFunction(env, functionTemp.second->GetNapiValue(), "onException", args, argc);
    }
}

static void DoMainThreadOnUnhandleException(napi_env env, const std::string& summary)
{
    std::lock_guard<std::recursive_mutex> lock(errorMtx);
    if (observerList.empty()) {
        return;
    }
    if (!observerList.count(env)) {
        return;
    }

    std::vector<std::pair<int32_t, std::shared_ptr<NativeReference>>> functions = observerList[env];
    for (auto& functionTemp : functions) {
        size_t argc = ARGC_ONE;
        napi_value args[] = {CreateJsValue(env, summary)};
        CallJsFunction(env, functionTemp.second->GetNapiValue(), "onUnhandledException", args, argc);
    }
}

static void DoWorkThreadCallback(napi_env env, napi_value exception)
{
    std::lock_guard<std::recursive_mutex> lock(errorMtx);
    if (observerList.empty()) {
        return;
    }
    if (exception == nullptr) {
        return;
    }
    if (!observerList.count(env)) {
        return;
    }
    std::string name = GetContent(env, exception, "name");
    std::string stack = GetContent(env, exception, "stack");
    std::string message = GetContent(env, exception, "message");
    std::vector<std::pair<int32_t, std::shared_ptr<NativeReference>>> functions = observerList[env];
    for (auto& functionTemp : functions) {
        size_t argc = ARGC_ONE;
        napi_value args[] = {CreateJsErrorObject(env, name, message, stack)};
        CallJsFunction(env, functionTemp.second->GetNapiValue(), "onException", args, argc);
    }
}

static bool ErrorManagerWorkerCallback(napi_env env, napi_value exception, const std::string& instanceName,
    uint32_t type)
{
    DoGlobalCallback(env, exception, instanceName, type);
    DoWorkThreadCallback(env, exception);
    return true;
}

static bool ErrorManagerMainWorkerCallback(napi_env env, const std::string& summary, const std::string& name,
    const std::string& message, const std::string& stack)
{
    DoMainThreadOnUnhandleException(env, summary);
    DoMainThreadOnException(env, name, message, stack);
    WorkItem item;
    item.name = name;
    item.message = message;
    item.stack = stack;
    item.instanceName = "";
    item.instanceType = 0;
    DoCallbackInRegesterThread(env, item);
    return true;
}

static bool PromiseManagerCallback(napi_env env, napi_value* args, const std::string& instanceName, uint32_t type)
{
    std::lock_guard<std::mutex> lock(globalPromiseMtx);
    if (globalPromiseList.empty()) {
        return false;
    }
    int32_t event = -1;
    napi_value reason = args[INDEX_TWO];
    napi_get_value_int32(env, args[INDEX_ZERO], &event);

    if (event != static_cast<int32_t>(UnhandledRejectionEvent::REJECT)) {
        return false;
    }

    std::string name = GetContent(env, reason, "name");
    std::string stack = GetContent(env, reason, "stack");
    std::string message = GetContent(env, reason, "message");

    for (auto iter : globalPromiseList) {
        uv_loop_t* loop = nullptr;
        if (napi_get_uv_event_loop(iter.env, &loop) != napi_ok) {
            continue;
        }
        WorkItem* item = new (std::nothrow) WorkItem();
        if (item == nullptr) {
            continue;
        }
        item->env = iter.env;
        item->ref = iter.ref;
        item->instanceName = instanceName;
        item->instanceType = type;
        item->work.data = item;
        item->name = name;
        item->stack = stack;
        item->message = message;
        int ret = uv_queue_work(
            loop, &item->work, [](uv_work_t* reqwork) {}, DoFunctionCallback);
        if (ret != 0) {
            if (item != nullptr) {
                delete item;
                item = nullptr;
            }
        }
    }
    return true;
}

class JsErrorManager final {
public:
    JsErrorManager() {}
    ~JsErrorManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        std::unique_ptr<JsErrorManager>(static_cast<JsErrorManager*>(data));
        ClearReference(env);
    }

    static napi_value On(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOn);
    }

    static napi_value Off(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsErrorManager, OnOff);
    }

    napi_value SetRejectionCallback(napi_env env) const
    {
        napi_value rejectCallback = nullptr;
        std::string rejectCallbackName = "OnUnhandledRejection";
        NAPI_CALL(env, napi_create_function(env,
                                            rejectCallbackName.c_str(),
                                            rejectCallbackName.size(),
                                            OnUnhandledRejection,
                                            nullptr, &rejectCallback));
        napi_ref rejectCallbackRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, rejectCallback, INITITAL_REFCOUNT_ONE, &rejectCallbackRef));

        napi_value checkCallback = nullptr;
        std::string checkCallbackName = "NotifyUnhandledRejectionHandler";
        NAPI_CALL(env, napi_create_function(env,
                                            checkCallbackName.c_str(),
                                            checkCallbackName.size(),
                                            NotifyUnhandledRejectionHandler,
                                            nullptr, &checkCallback));
        napi_ref checkCallbackRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, checkCallback, INITITAL_REFCOUNT_ONE, &checkCallbackRef));

        NAPI_CALL(env, napi_set_promise_rejection_callback(env, rejectCallbackRef, checkCallbackRef));

        return CreateJsUndefined(env);
    }

    static void ClearReference(napi_env env)
    {
        for (auto& iter : unhandledRejectionObservers) {
            napi_delete_reference(env, iter);
        }
        unhandledRejectionObservers.clear();

        auto iter = pendingUnHandledRejections.begin();
        while (iter != pendingUnHandledRejections.end()) {
            napi_delete_reference(env, iter->first);
            napi_delete_reference(env, iter->second);
            ++iter;
        }
        pendingUnHandledRejections.clear();
    }

private:
    napi_value OnOn(napi_env env, const size_t argc, napi_value* argv)
    {
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOnNew(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_SYNC_LOOP) {
            if (!AppExecFwk::EventRunner::IsAppMainThread()) {
                ThrowInvalidCallerError(env);
                return CreateJsUndefined(env);
            }
            return OnSetLoopWatch(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOnUnhandledRejection(env, argv[INDEX_ONE]);
        }
        if (type == GLOBAL_ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOnAllUnhandledRejection(env, argv[INDEX_ONE]);
        }
        if (type == GLOBAL_ON_OFF_TYPE) {
            if (argc != ARGC_TWO) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOnAll(env, argv[INDEX_ONE]);
        }
        return OnOnOld(env, argc, argv);
    }

    napi_value OnOnOld(napi_env env, const size_t argc, napi_value* argv)
    {
        if (argc != ARGC_TWO) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            ThrowInvalidParamError(env, "Parameter error: Parse type failed, must be a string error.");
            return CreateJsUndefined(env);
        }
        std::lock_guard<std::recursive_mutex> lock(errorMtx);
        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }
        napi_ref ref = nullptr;
        napi_create_reference(env, argv[INDEX_ONE], 1, &ref);
        observerList[env].push_back(
            std::make_pair(observerId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref))));
        return CreateJsValue(env, observerId);
    }

    napi_value OnOnUnhandledRejection(napi_env env, napi_value function)
    {
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }
        for (auto& iter : unhandledRejectionObservers) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter));
                unhandledRejectionObservers.erase(iter);
                break;
            }
        }
        napi_ref myCallRef = nullptr;
        NAPI_CALL(env, napi_create_reference(env, function, INITITAL_REFCOUNT_ONE, &myCallRef));
        unhandledRejectionObservers.insert(myCallRef);
        return nullptr;
    }

    napi_value OnOnNew(napi_env env, const size_t argc, napi_value* argv)
    {
        if (argc < ARGC_TWO) {
            HILOG_ERROR("OnOnNew invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_object)) {
            HILOG_ERROR("OnOnNew invalid param");
            ThrowInvalidParamError(env, "Parameter error: Parse observer failed, must be a ErrorObserver.");
            return CreateJsUndefined(env);
        }

        int32_t observerId = serialNumber_;
        if (serialNumber_ < INT32_MAX) {
            serialNumber_++;
        } else {
            serialNumber_ = 0;
        }

        if (observer_ == nullptr) {
            observer_ = std::make_shared<JsErrorObserver>(env);
            AppExecFwk::ApplicationDataManager::GetInstance().AddErrorObserver(observer_);
        }
        observer_->AddJsObserverObject(observerId, argv[INDEX_ONE], true);
        return CreateJsValue(env, observerId);
    }

    napi_value OnOff(napi_env env, size_t argc, napi_value* argv)
    {
        std::string type = ParseParamType(env, argc, argv);
        if (type == ON_OFF_TYPE_SYNC) {
            return OnOffNew(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_SYNC_LOOP) {
            if (!AppExecFwk::EventRunner::IsAppMainThread()) {
                ThrowInvalidCallerError(env);
                return CreateJsUndefined(env);
            }
            return OnRemoveLoopWatch(env, argc, argv);
        }
        if (type == ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO && argc != ARGC_ONE) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOffUnhandledRejection(env, argc, argv);
        }
        if (type == GLOBAL_ON_OFF_TYPE_UNHANDLED_REJECTION) {
            if (argc != ARGC_TWO && argc != ARGC_ONE) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOffAllUnhandledRejection(env, argv[ARGC_ONE]);
        }
        if (type == GLOBAL_ON_OFF_TYPE) {
            if (argc != ARGC_TWO && argc != ARGC_ONE) {
                ThrowInvalidNumParametersError(env);
                return CreateJsUndefined(env);
            }
            return OnOffAllError(env, argv[ARGC_ONE]);
        }
        return OnOffOld(env, argc, argv);
    }

    napi_value OnOnAll(napi_env env, napi_value function)
    {
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(globalErrorMtx);
        for (auto& iter : globalObserverList) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter.ref, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
                globalObserverList.erase(iter);
                break;
            }
        }
        GlobalObserverItem item;
        NAPI_CALL(env, napi_create_reference(env, function, INITITAL_REFCOUNT_ONE, &item.ref));
        item.env = env;
        globalObserverList.insert(item);
        return CreateJsUndefined(env);
    }

    napi_value OnOnAllUnhandledRejection(napi_env env, napi_value function)
    {
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(globalPromiseMtx);
        for (auto& iter : globalPromiseList) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter.ref, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
                globalPromiseList.erase(iter);
                break;
            }
        }
        GlobalObserverItem item;
        NAPI_CALL(env, napi_create_reference(env, function, INITITAL_REFCOUNT_ONE, &item.ref));
        item.env = env;
        globalPromiseList.insert(item);
        return CreateJsUndefined(env);
    }

    napi_value OnOffAllError(napi_env env, napi_value function)
    {
        auto res = CreateJsUndefined(env);
        if (function == nullptr) {
            std::lock_guard<std::mutex> lock(globalErrorMtx);
            for (auto& iter : globalObserverList) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
            }
            globalObserverList.clear();
            return res;
        }
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(globalErrorMtx);
        for (auto& iter : globalObserverList) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter.ref, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
                globalObserverList.erase(iter);
                return res;
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnOffAllUnhandledRejection(napi_env env, napi_value function)
    {
        auto res = CreateJsUndefined(env);
        if (function == nullptr) {
            std::lock_guard<std::mutex> lock(globalPromiseMtx);
            for (auto& iter : globalPromiseList) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
            }
            globalPromiseList.clear();
            return res;
        }
        if (!ValidateFunction(env, function)) {
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(globalPromiseMtx);
        for (auto& iter : globalPromiseList) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter.ref, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter.ref));
                globalPromiseList.erase(iter);
                return res;
            }
        }
        return CreateJsUndefined(env);
    }

    napi_value OnOffOld(napi_env env, size_t argc, napi_value* argv)
    {
        int32_t observerId = -1;
        if (argc != ARGC_TWO && argc != ARGC_THREE) {
            ThrowTooFewParametersError(env);
        } else {
            napi_get_value_int32(env, argv[INDEX_ONE], &observerId);
        }

        std::string type;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], type) || type != ON_OFF_TYPE) {
            ThrowInvalidParamError(env, "Parameter error: Parse type failed, must be a string error.");
            return CreateJsUndefined(env);
        }
        NapiAsyncTask::CompleteCallback complete = [observerId](napi_env env, NapiAsyncTask& task, int32_t status) {
            std::lock_guard<std::recursive_mutex> lock(errorMtx);
            if (!observerList.count(env) || observerId == -1) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return;
            }
            auto& envObservers = observerList[env];
            auto iter = std::find_if(envObservers.begin(), envObservers.end(),
                [observerId](const auto& item) {
                    return item.first == observerId;
                });
            if (iter == envObservers.end()) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID));
                return;
            }
            envObservers.erase(iter);
            if (envObservers.empty()) {
                observerList.erase(env);
            }
            task.ResolveWithNoError(env, CreateJsUndefined(env));
        };
        napi_value lastParam = (argc <= ARGC_TWO) ? nullptr : argv[INDEX_TWO];
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JSErrorManager::OnUnregisterErrorObserver",
            env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
        return result;
    }

    napi_value OnOffUnhandledRejection(napi_env env, size_t argc, napi_value* argv)
    {
        auto res = CreateJsUndefined(env);
        if (argc == ARGC_ONE) {
            for (auto& iter : unhandledRejectionObservers) {
                napi_delete_reference(env, iter);
            }
            unhandledRejectionObservers.clear();
            return res;
        }
        napi_value function = argv[INDEX_ONE];
        if (!ValidateFunction(env, function)) {
            return res;
        }
        for (auto& iter : unhandledRejectionObservers) {
            napi_value observer = nullptr;
            NAPI_CALL(env, napi_get_reference_value(env, iter, &observer));
            bool equals = false;
            NAPI_CALL(env, napi_strict_equals(env, observer, function, &equals));
            if (equals) {
                NAPI_CALL(env, napi_delete_reference(env, iter));
                unhandledRejectionObservers.erase(iter);
                return res;
            }
        }
        ThrowError(env, AbilityErrorCode::ERROR_CODE_OBSERVER_NOT_FOUND);
        return res;
    }

    napi_value OnOffNew(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc < ARGC_TWO) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        int32_t observerId = -1;
        if (!ConvertFromJsValue(env, argv[INDEX_ONE], observerId)) {
            ThrowInvalidParamError(env, "Parameter error: Parse observerId failed, must be a number.");
            return CreateJsUndefined(env);
        }
        if (observer_ == nullptr) {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
            return CreateJsUndefined(env);
        }
        if (observer_->RemoveJsObserverObject(observerId, true)) {
            HILOG_INFO("OnOffNew success");
        } else {
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_ID);
        }
        if (observer_->IsEmpty()) {
            AppExecFwk::ApplicationDataManager::GetInstance().RemoveErrorObserver();
            observer_ = nullptr;
        }
        return CreateJsUndefined(env);
    }

    static void CallJsFunction(napi_env env, napi_value obj, const char* methodName,
        napi_value const* argv, size_t argc)
    {
        if (obj == nullptr) {
            HILOG_ERROR("CallJsFunction null obj");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env, obj, methodName, &method);
        if (method == nullptr) {
            HILOG_ERROR("CallJsFunction null method");
            return;
        }
        napi_value callResult = nullptr;
        napi_call_function(env, obj, method, argc, argv, &callResult);
    }

    static void CallbackTimeout(int64_t number)
    {
        std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
            ([number](napi_env env, NapiAsyncTask& task, int32_t status) {
                if (loopObserver == nullptr) {
                    HILOG_ERROR("CallbackTimeout null loopObserver");
                    return;
                }
                if (loopObserver->env == nullptr) {
                    HILOG_ERROR("CallbackTimeout null env");
                    return;
                }
                if (loopObserver->observerObject == nullptr) {
                    HILOG_ERROR("CallbackTimeout null observer");
                    return;
                }
                napi_value jsValue[] = { CreateJsValue(loopObserver->env, number) };
                CallJsFunction(loopObserver->env, loopObserver->observerObject->GetNapiValue(), "onLoopTimeOut",
                    jsValue, ARGC_ONE);
            });
        napi_ref callback = nullptr;
        std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
        if (loopObserver && loopObserver->env) {
            NapiAsyncTask::Schedule("JsErrorObserver::CallbackTimeout",
                loopObserver->env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
        }
    }

    napi_value OnSetLoopWatch(napi_env env, size_t argc, napi_value* argv)
    {
        if (argc != ARGC_THREE) {
            HILOG_ERROR("OnSetLoopWatch invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        if (!CheckTypeForNapiValue(env, argv[INDEX_ONE], napi_number)) {
            HILOG_ERROR("OnSetLoopWatch invalid param");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse timeout, must be a number.");
            return CreateJsUndefined(env);
        }
        if (!CheckTypeForNapiValue(env, argv[INDEX_TWO], napi_object)) {
            HILOG_ERROR("OnSetLoopWatch invalid param");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse observer, must be a LoopObserver.");
            return CreateJsUndefined(env);
        }
        int64_t number;
        if (!ConvertFromJsNumber(env, argv[INDEX_ONE], number)) {
            HILOG_ERROR("OnSetLoopWatch parse timeout failed");
            ThrowInvalidParamError(env, "Parameter error: Failed to parse timeout, must be a number.");
            return CreateJsUndefined(env);
        }
        if (number <= 0) {
            HILOG_ERROR("OnSetLoopWatch timeout<=0");
            ThrowInvalidParamError(env, "Parameter error: The timeout cannot be less than 0.");
            return CreateJsUndefined(env);
        }

        if (loopObserver == nullptr) {
            loopObserver = std::make_shared<JsLoopObserver>();
        }
        loopObserver->mainRunner = AppExecFwk::EventRunner::GetMainEventRunner();
        napi_ref ref = nullptr;
        napi_create_reference(env, argv[INDEX_TWO], INITITAL_REFCOUNT_ONE, &ref);
        loopObserver->observerObject = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
        loopObserver->env = env;
        loopObserver->mainRunner->SetTimeout(number);
        loopObserver->mainRunner->SetTimeoutCallback(CallbackTimeout);
        return nullptr;
    }

    napi_value OnRemoveLoopWatch(napi_env env, size_t argc, napi_value* argv)
    {
        if (loopObserver) {
            loopObserver.reset();
            loopObserver = nullptr;
        }
        return nullptr;
    }

    std::string ParseParamType(napi_env env, const size_t argc, napi_value* argv)
    {
        std::string type;
        if (argc > INDEX_ZERO && ConvertFromJsValue(env, argv[INDEX_ZERO], type)) {
            return type;
        }
        return "";
    }

    bool ValidateFunction(napi_env env, napi_value function)
    {
        if (function == nullptr ||
            CheckTypeForNapiValue(env, function, napi_null) ||
            CheckTypeForNapiValue(env, function, napi_undefined)) {
            HILOG_ERROR("ValidateFunction invalid func");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
            return false;
        }
        return true;
    }

    int32_t serialNumber_ = 0;
    std::shared_ptr<JsErrorObserver> observer_;
};
} // namespace

napi_value JsErrorManagerInit(napi_env env, napi_value exportObj)
{
    if (env == nullptr || exportObj == nullptr) {
        HILOG_ERROR("JsErrorManagerInit null env or exportObj");
        return nullptr;
    }
    std::unique_ptr<JsErrorManager> jsErrorManager = std::make_unique<JsErrorManager>();
    jsErrorManager->SetRejectionCallback(env);
    napi_wrap(env, exportObj, jsErrorManager.release(), JsErrorManager::Finalizer, nullptr, nullptr);

    std::call_once(registerCallbackFlag, []() {
        NapiErrorManager::GetInstance()->RegisterHasOnErrorCallback(IsErrorObserverListNotEmpty);
        NapiErrorManager::GetInstance()->RegisterHasAllUnhandledRejectionCallback(IsGlobalPromiseListNotEmpty);
        NapiErrorManager::GetInstance()->RegisterOnErrorCallback(
            ErrorManagerWorkerCallback, ErrorManagerMainWorkerCallback);
        NapiErrorManager::GetInstance()->RegisterAllUnhandledRejectionCallback(PromiseManagerCallback);
    });

    const char* moduleName = "JsErrorManager";
    BindNativeFunction(env, exportObj, "on", moduleName, JsErrorManager::On);
    BindNativeFunction(env, exportObj, "off", moduleName, JsErrorManager::Off);
    return CreateJsUndefined(env);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
