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

#include "js_ability_delegator.h"

#include <mutex>
#include "ability_delegator_registry.h"
#include "foundation/appframework/ability/ability_runtime/cross_platform/interfaces/kits/native/ability/js_ability.h"
#include "hilog_wrapper.h"
#include "js_ability_delegator_utils.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "napi_common_want.h"

namespace OHOS {
namespace AbilityDelegatorJs {
struct AbilityObjectBox {
    std::weak_ptr<NativeReference> object_;
};
struct AbilityStageObjBox {
    std::weak_ptr<NativeReference> object_;
};

constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;

using namespace OHOS::AbilityRuntime;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityMonitor>> g_monitorRecord;
std::map<std::shared_ptr<NativeReference>, std::shared_ptr<AbilityStageMonitor>> g_stageMonitorRecord;
std::map<std::weak_ptr<NativeReference>, sptr<IRemoteObject>, std::owner_less<>> g_abilityRecord;
std::map<std::weak_ptr<NativeReference>, std::string, std::owner_less<>> g_abilityRecordInfo;
std::mutex g_mutexAbilityRecord;
std::mutex g_mtxStageMonitorRecord;

enum ERROR_CODE {
    INCORRECT_PARAMETERS    = 401,
};

std::unordered_map<int32_t, std::string> errorMap = {
    {INCORRECT_PARAMETERS,  "Incorrect parameters."},
};

constexpr int COMMON_FAILED = 16000100;

NativeValue *ThrowJsError(NativeEngine& engine, int32_t errCode)
{
    NativeValue *error = CreateJsError(engine, errCode, errorMap[errCode]);
    engine.Throw(error);
    return engine.CreateUndefined();
}

void ResolveWithNoError(NativeEngine &engine, AsyncTask &task, NativeValue *value = nullptr)
{
    if (value == nullptr) {
        task.ResolveWithNoError(engine, engine.CreateUndefined());
    } else {
        task.ResolveWithNoError(engine, value);
    }
}

NativeValue *AttachAppContext(NativeEngine *engine, void *value, void *)
{
    HILOG_INFO("AttachAppContext");
    if (value == nullptr) {
        HILOG_ERROR("invalid parameter.");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::Platform::Context>*>(value)->lock();
    if (ptr == nullptr) {
        HILOG_ERROR("invalid context.");
        return nullptr;
    }

    NativeValue *object = CreateJsBaseContext(*engine, ptr, true);
    NativeObject *nObject = ConvertNativeValueTo<NativeObject>(object);
    if (nObject == nullptr) {
        HILOG_ERROR("nObject is nullptr.");
        return nullptr;
    }
    nObject->ConvertToNativeBindingObject(engine, DetachCallbackFunc, AttachAppContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Platform::Context>(ptr);
    nObject->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Platform::Context> *>(data);
        },
        nullptr);
    return object;
}

JSAbilityDelegator::JSAbilityDelegator()
{
    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (delegator) {
        auto clearFunc = [](const std::shared_ptr<ADelegatorAbilityProperty> &property) {
            HILOG_INFO("Clear function is called");
            if (!property) {
                HILOG_ERROR("Invalid property");
                return;
            }

            std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
            for (auto it = g_abilityRecordInfo.begin(); it != g_abilityRecordInfo.end();) {
                if (it->second == property->fullName_) {
                    it = g_abilityRecordInfo.erase(it);
                    continue;
                }
                ++it;
            }
        };

        delegator->RegisterClearFunc(clearFunc);
    }
}

void JSAbilityDelegator::Finalizer(NativeEngine *engine, void *data, void *hint)
{
    HILOG_INFO("enter");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

NativeValue *JSAbilityDelegator::AddAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnAddAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::RemoveAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnRemoveAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::WaitAbilityMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnWaitAbilityMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::AddAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnAddAbilityStageMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::RemoveAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnRemoveAbilityStageMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::WaitAbilityStageMonitor(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnWaitAbilityStageMonitor(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAppContext(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAppContext(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetAbilityState(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetAbilityState(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::GetCurrentTopAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnGetCurrentTopAbility(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::StartAbility(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnStartAbility(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityForeground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityForeground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::DoAbilityBackground(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnDoAbilityBackground(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::Print(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnPrint(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::PrintSync(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnPrintSync(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::FinishTest(NativeEngine *engine, NativeCallbackInfo *info)
{
    JSAbilityDelegator *me = CheckParamsAndGetThis<JSAbilityDelegator>(engine, info);
    return (me != nullptr) ? me->OnFinishTest(*engine, *info) : nullptr;
}

NativeValue *JSAbilityDelegator::OnAddAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(engine, info, monitor)) {
        HILOG_ERROR("Parse addAbilityMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityMonitor AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "addAbilityMonitor failed."));
            return;
        }
        delegator->AddAbilityMonitor(monitor);
        ResolveWithNoError(engine, task);
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityMonitor",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnAddAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(engine, info, monitor, isExisted)) {
        HILOG_ERROR("Parse addAbilityStageMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityStageMonitor AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "addAbilityStageMonitor failed."));
            return;
        }
        delegator->AddAbilityStageMonitor(monitor);
        ResolveWithNoError(engine, task);
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityStageMonitor",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));

    if (!isExisted) {
        AddStageMonitorRecord(engine, info.argv[INDEX_ZERO], monitor);
    }
    return result;
}

NativeValue *JSAbilityDelegator::OnRemoveAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(engine, info, monitor)) {
        HILOG_ERROR("Parse removeAbilityMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete =
        [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) mutable {
        HILOG_INFO("OnRemoveAbilityMonitor AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "removeAbilityMonitor failed."));
            return;
        }
        delegator->RemoveAbilityMonitor(monitor);
        ResolveWithNoError(engine, task);
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityMonitor",
        engine, CreateAsyncTaskWithLastParam(engine,
        lastParam, nullptr, std::move(complete), &result));

    if (AbilityDelegatorRegistry::GetAbilityDelegator()) {
        for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
            std::shared_ptr<NativeReference> jsMonitor = iter->first;
            if ((info.argv[INDEX_ZERO])->StrictEquals(jsMonitor->Get())) {
                g_monitorRecord.erase(iter);
                break;
            }
        }
    }
    return result;
}

NativeValue *JSAbilityDelegator::OnRemoveAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(engine, info, monitor, isExisted)) {
        HILOG_ERROR("Parse removeAbilityStageMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete =
        [monitor](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnRemoveAbilityStageMonitor AsyncTask is called");

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "removeAbilityStageMonitor failed."));
            return;
        }
        delegator->RemoveAbilityStageMonitor(monitor);
        ResolveWithNoError(engine, task);
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityStageMonitor", engine,
        CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));

    if (isExisted) {
        RemoveStageMonitorRecord(info.argv[INDEX_ZERO]);
    }
    return result;
}

NativeValue *JSAbilityDelegator::OnWaitAbilityMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityMonitorPara(engine, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    auto abilityObjectBox = std::make_shared<AbilityObjectBox>();
    AsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityObjectBox]() {
        HILOG_INFO("OnWaitAbilityMonitor AsyncTask ExecuteCallback is called");
        if (!abilityObjectBox) {
            HILOG_ERROR("OnWaitAbilityMonitor AsyncTask ExecuteCallback, Invalid abilityObjectBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor AsyncTask ExecuteCallback, Invalid delegator");
            return;
        }

        std::shared_ptr<ADelegatorAbilityProperty> property = opt.hasTimeoutPara ?
            delegator->WaitAbilityMonitor(monitor, timeout) : delegator->WaitAbilityMonitor(monitor);
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            return;
        }

        abilityObjectBox->object_ = property->object_;
        std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
        g_abilityRecordInfo.emplace(property->object_, property->fullName_);
    };

    AsyncTask::CompleteCallback complete = [abilityObjectBox](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnWaitAbilityMonitor AsyncTask CompleteCallback is called");
        if (abilityObjectBox && !abilityObjectBox->object_.expired()) {
            ResolveWithNoError(engine, task, abilityObjectBox->object_.lock()->Get());
        } else {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "waitAbilityMonitor failed."));
        }
    };

    NativeValue *lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityMonitor",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnWaitAbilityStageMonitor(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityStageMonitorPara(engine, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityStageMonitor parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    auto abilityStageObjBox = std::make_shared<AbilityStageObjBox>();
    AsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityStageObjBox]() {
        HILOG_INFO("OnWaitAbilityStageMonitor AsyncTask ExecuteCallback is called");
        if (!abilityStageObjBox) {
            HILOG_ERROR("OnWaitAbilityStageMonitor AsyncTask ExecuteCallback, Invalid abilityStageObjBox");
            return;
        }
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor AsyncTask ExecuteCallback, Invalid delegator");
            return;
        }
        std::shared_ptr<DelegatorAbilityStageProperty> result;
        result = opt.hasTimeoutPara ?
            delegator->WaitAbilityStageMonitor(monitor, timeout) : delegator->WaitAbilityStageMonitor(monitor);
        if (!result || result->object_.expired()) {
            HILOG_ERROR("WaitAbilityStageMonitor failed");
            return;
        }
        abilityStageObjBox->object_ = result->object_;
    };

    AsyncTask::CompleteCallback complete = [abilityStageObjBox](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnWaitAbilityMonitor AsyncTask CompleteCallback is called");
        if (abilityStageObjBox && !abilityStageObjBox->object_.expired()) {
            ResolveWithNoError(engine, task, abilityStageObjBox->object_.lock()->Get());
        } else {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "waitAbilityStageMonitor failed."));
        }
    };
    NativeValue *lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityStageMonitor",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnPrint(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(engine, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [msg](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnPrint AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "print failed."));
            return;
        }
        delegator->Print(msg);
        ResolveWithNoError(engine, task);
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnPrint",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnPrintSync(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(engine, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("Invalid delegator");
        return engine.CreateUndefined();
    }

    delegator->Print(msg);
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::OnGetAppContext(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return engine.CreateNull();
    }
    std::shared_ptr<AbilityRuntime::Platform::Context> context = delegator->GetAppContext();
    if (!context) {
        HILOG_ERROR("context is null");
        return engine.CreateNull();
    }
    NativeValue *value = CreateJsBaseContext(engine, context, false);
    NativeObject *nativeObj = ConvertNativeValueTo<NativeObject>(value);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get context native object");
        return engine.CreateNull();
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Platform::Context>(context);
    nativeObj->ConvertToNativeBindingObject(&engine, DetachCallbackFunc, AttachAppContext, workContext, nullptr);
    nativeObj->SetNativePointer(
        workContext,
        [](NativeEngine *, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Platform::Context> *>(data);
        },
        nullptr);
    return value;
}

NativeValue *JSAbilityDelegator::OnGetAbilityState(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return engine.CreateUndefined();
    }

    std::string fullname;
    if (!ParseAbilityParaInfo(engine, info.argv[INDEX_ZERO], fullname)) {
        HILOG_ERROR("Parse ability parameter failed");
        return engine.CreateUndefined();
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return engine.CreateNull();
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(fullname);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToJs(lifeState, abilityLifeState);
    return engine.CreateNumber(static_cast<int>(abilityLifeState));
}

NativeValue *JSAbilityDelegator::OnGetCurrentTopAbility(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc >= ARGC_ONE && info.argv[INDEX_ZERO]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
        HILOG_ERROR("Parse getCurrentTopAbility parameter failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [this](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetCurrentTopAbility AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("Invalid delegator");
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "getCurrentTopAbility failed."));
            return;
        }

        auto property = delegator->GetCurrentTopAbility();
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "getCurrentTopAbility failed."));
        } else {
            {
                std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
                g_abilityRecordInfo.emplace(property->object_, property->fullName_);
            }
            ResolveWithNoError(engine, task, property->object_.lock()->Get());
        }
    };

    NativeValue *lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnGetCurrentTopAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnStartAbility(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    AAFwk::Want want;
    if (!ParseStartAbilityPara(engine, info, want)) {
        HILOG_ERROR("Parse startAbility parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [want](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnStartAbility AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "startAbility failed."));
            return;
        }
        int result = delegator->StartAbility(want);
        if (result) {
            task.Reject(engine, CreateJsError(engine, result, "startAbility failed."));
        } else {
            ResolveWithNoError(engine, task);
        }
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnStartAbility",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityForeground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string fullname;
    if (!ParseAbilityCommonPara(engine, info, fullname)) {
        HILOG_ERROR("Parse doAbilityForeground parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [fullname](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityForeground AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "doAbilityForeground failed."));
            return;
        }
        if (delegator->DoAbilityForeground(fullname)) {
            ResolveWithNoError(engine, task, engine.CreateNull());
        } else {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "doAbilityForeground failed."));
        }
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnDoAbilityForeground",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnDoAbilityBackground(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string fullname;
    if (!ParseAbilityCommonPara(engine, info, fullname)) {
        HILOG_ERROR("Parse doAbilityBackground parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [fullname](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityBackground AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "doAbilityBackground failed."));
            return;
        }
        if (delegator->DoAbilityBackground(fullname)) {
            ResolveWithNoError(engine, task, engine.CreateNull());
        } else {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "doAbilityBackground failed."));
        }
    };

    NativeValue *lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnDoAbilityBackground",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::OnFinishTest(NativeEngine &engine, NativeCallbackInfo &info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    int64_t code = 0;
    if (!ParseFinishTestPara(engine, info, msg, code)) {
        HILOG_ERROR("Parse finishTest parameters failed");
        return ThrowJsError(engine, INCORRECT_PARAMETERS);
    }

    AsyncTask::CompleteCallback complete = [msg, code](NativeEngine &engine, AsyncTask &task, int32_t status) {
        HILOG_INFO("OnFinishTest AsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(engine, CreateJsError(engine, COMMON_FAILED, "finishTest failed."));
            return;
        }
        delegator->FinishUserTest(msg, code);
        ResolveWithNoError(engine, task);
    };
    NativeValue *lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    NativeValue *result = nullptr;
    AsyncTask::Schedule("JSAbilityDelegator::OnFinishTest",
        engine, CreateAsyncTaskWithLastParam(engine, lastParam, nullptr, std::move(complete), &result));
    return result;
}

NativeValue *JSAbilityDelegator::ParseMonitorPara(
    NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter, monitorRecord size = %{public}zu", g_monitorRecord.size());

    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if (value->StrictEquals(jsMonitor->Get())) {
            HILOG_ERROR("monitor existed");
            monitor = iter->second;
            return monitor ? engine.CreateNull() : nullptr;
        }
    }

    NativeObject *object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    auto abilityNameValue = object->GetProperty("abilityName");
    if (abilityNameValue == nullptr) {
        HILOG_ERROR("Failed to get property abilityName");
        return nullptr;
    }

    std::string abilityName;
    if (!ConvertFromJsValue(engine, abilityNameValue, abilityName)) {
        return nullptr;
    }

    std::string moduleName = "";
    auto moduleNameValue = object->GetProperty("moduleName");
    if (moduleNameValue != nullptr && !ConvertFromJsValue(engine, moduleNameValue, moduleName)) {
        HILOG_WARN("Failed to get property moduleName");
        moduleName = "";
    }

    std::shared_ptr<JSAbilityMonitor> abilityMonitor = nullptr;
    if (moduleName.empty()) {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
    } else {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName, moduleName);
    }

    abilityMonitor->SetJsAbilityMonitorEnv(&engine);
    abilityMonitor->SetJsAbilityMonitor(value);

    monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(value, 1));
    g_monitorRecord.emplace(reference, monitor);

    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseStageMonitorPara(
    NativeEngine &engine, NativeValue *value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    HILOG_INFO("enter, stageMonitorRecord size = %{public}zu", g_stageMonitorRecord.size());

    isExisted = false;
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if (value->StrictEquals(jsMonitor->Get())) {
            HILOG_WARN("AbilityStage monitor existed");
            isExisted = true;
            monitor = iter->second;
            return monitor ? engine.CreateNull() : nullptr;
        }
    }
    NativeObject *object = ConvertNativeValueTo<NativeObject>(value);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }
    auto moduleNameValue = object->GetProperty("moduleName");
    if (moduleNameValue == nullptr) {
        HILOG_ERROR("Failed to get property moduleName");
        return nullptr;
    }
    std::string moduleName;
    if (!ConvertFromJsValue(engine, moduleNameValue, moduleName)) {
        HILOG_ERROR("Failed to get moduleName from JsValue");
        return nullptr;
    }
    auto srcEntranceValue = object->GetProperty("srcEntrance");
    if (srcEntranceValue == nullptr) {
        HILOG_ERROR("Failed to get property srcEntrance");
        return nullptr;
    }
    std::string srcEntrance;
    if (!ConvertFromJsValue(engine, srcEntranceValue, srcEntrance)) {
        HILOG_ERROR("Failed to get srcEntrance from JsValue");
        return nullptr;
    }

    monitor = std::make_shared<AbilityStageMonitor>(moduleName, srcEntrance);
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseAbilityParaInfo(NativeEngine &engine, NativeValue *value, std::string &fullname)
{
    HILOG_INFO("enter");

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    for (auto iter = g_abilityRecordInfo.begin(); iter != g_abilityRecordInfo.end();) {
        if (iter->first.expired()) {
            iter = g_abilityRecordInfo.erase(iter);
            continue;
        }

        if (value->StrictEquals(iter->first.lock()->Get())) {
            fullname = iter->second;
            HILOG_INFO("Ability exist");
            return fullname.c_str() ? engine.CreateNull() : nullptr;
        }

        ++iter;
    }

    HILOG_ERROR("Ability doesn't exist");
    fullname = "";
    return nullptr;
}

NativeValue *JSAbilityDelegator::CreateAbilityObject(NativeEngine &engine, const sptr<IRemoteObject> &remoteObject)
{
    HILOG_INFO("enter");

    if (!remoteObject) {
        return nullptr;
    }

    NativeValue *objValue = engine.CreateObject();
    NativeObject *object = ConvertNativeValueTo<NativeObject>(objValue);
    if (object == nullptr) {
        HILOG_ERROR("Failed to get object");
        return nullptr;
    }

    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(objValue, 1));

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    g_abilityRecord[reference] = remoteObject;
    return objValue;
}

void JSAbilityDelegator::AbilityLifecycleStateToJs(
    const AbilityDelegator::AbilityState &lifeState, AbilityLifecycleState &abilityLifeState)
{
    HILOG_INFO("enter and lifeState = %{public}d", static_cast<int32_t>(lifeState));
    switch (lifeState) {
        case AbilityDelegator::AbilityState::STARTED:
            abilityLifeState = AbilityLifecycleState::CREATE;
            break;
        case AbilityDelegator::AbilityState::FOREGROUND:
            abilityLifeState = AbilityLifecycleState::FOREGROUND;
            break;
        case AbilityDelegator::AbilityState::BACKGROUND:
            abilityLifeState = AbilityLifecycleState::BACKGROUND;
            break;
        case AbilityDelegator::AbilityState::STOPPED:
            abilityLifeState = AbilityLifecycleState::DESTROY;
            break;
        default:
            abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
            break;
    }
}

NativeValue *JSAbilityDelegator::ParseAbilityMonitorPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Parse monitor parameters failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("ParseAbilityMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseAbilityStageMonitorPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseStageMonitorPara(engine, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Parse stage monitor parameters failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("ParseAbilityStageMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseWaitAbilityMonitorPara(NativeEngine &engine, NativeCallbackInfo &info,
    std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(engine, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Monitor parse parameters failed");
        return nullptr;
    }

    if (!ParseTimeoutCallbackPara(engine, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseWaitAbilityStageMonitorPara(NativeEngine &engine, NativeCallbackInfo &info,
    std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    bool isExisted = false;
    if (!ParseStageMonitorPara(engine, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Stage monitor parse parameters failed");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(engine, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    if (!isExisted) {
        AddStageMonitorRecord(engine, info.argv[INDEX_ZERO], monitor);
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseTimeoutCallbackPara(
    NativeEngine &engine, NativeCallbackInfo &info, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");

    opt.hasCallbackPara = false;
    opt.hasTimeoutPara = false;

    if (info.argc >= ARGC_TWO) {
        if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], timeout)) {
            if (info.argv[INDEX_ONE] == nullptr || info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
                HILOG_ERROR("Parse parameter argv[1] failed");
                return nullptr;
            }
            opt.hasCallbackPara = true;
            return engine.CreateNull();
        }
        opt.hasTimeoutPara = true;

        if (info.argc > ARGC_TWO) {
            if (info.argv[INDEX_TWO]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
                HILOG_ERROR("Parse parameter argv[2] failed");
                return (info.argv[INDEX_TWO] == nullptr) ? engine.CreateNull() : nullptr;
            }
            opt.hasCallbackPara = true;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParsePrintPara(NativeEngine &engine, NativeCallbackInfo &info, std::string &msg)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse callback parameter failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseAbilityCommonPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::string &fullname)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseAbilityParaInfo(engine, info.argv[INDEX_ZERO], fullname)) {
        HILOG_ERROR("Parse ability parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse ability callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseStartAbilityPara(
    NativeEngine &engine, NativeCallbackInfo &info, AAFwk::Want &want)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!OHOS::AppExecFwk::UnwrapWant(
        reinterpret_cast<napi_env>(&engine), reinterpret_cast<napi_value>(info.argv[INDEX_ZERO]), want)) {
        HILOG_ERROR("Parse want parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (info.argv[INDEX_ONE]->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Parse StartAbility callback parameters failed");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

NativeValue *JSAbilityDelegator::ParseFinishTestPara(
    NativeEngine &engine, NativeCallbackInfo &info, std::string &msg, int64_t &code)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (!ConvertFromJsValue(engine, info.argv[INDEX_ONE], code)) {
        HILOG_ERROR("Parse code para parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_TWO) {
        if ((info.argv[INDEX_TWO])->TypeOf() != NativeValueType::NATIVE_FUNCTION) {
            HILOG_ERROR("Incorrect Callback Function type");
            return nullptr;
        }
    }
    return engine.CreateNull();
}

void JSAbilityDelegator::AddStageMonitorRecord(
    NativeEngine &engine, NativeValue *value, const std::shared_ptr<AbilityStageMonitor> &monitor)
{
    if (!value) {
        HILOG_ERROR("UpdateStageMonitorRecord value is empty");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        HILOG_ERROR("AbilityDelegator is null");
        return;
    }
    std::shared_ptr<NativeReference> reference = nullptr;
    reference.reset(engine.CreateReference(value, 1));
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        g_stageMonitorRecord.emplace(reference, monitor);
    }
    HILOG_INFO("g_stageMonitorRecord added, size = %{public}zu", g_stageMonitorRecord.size());
}

void JSAbilityDelegator::RemoveStageMonitorRecord(NativeValue *value)
{
    if (!value) {
        HILOG_ERROR("UpdateStageMonitorRecord value is empty");
        return;
    }
    if (!AbilityDelegatorRegistry::GetAbilityDelegator()) {
        HILOG_ERROR("AbilityDelegator is null");
        return;
    }
    std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        if (value->StrictEquals(jsMonitor->Get())) {
            g_stageMonitorRecord.erase(iter);
            HILOG_INFO("g_stageMonitorRecord removed, size = %{public}zu", g_stageMonitorRecord.size());
            break;
        }
    }
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
