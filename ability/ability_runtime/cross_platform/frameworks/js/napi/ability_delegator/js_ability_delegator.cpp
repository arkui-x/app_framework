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
#include "napi_common_util.h"
#include "js_error_utils.h"

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

napi_value ThrowJsError(napi_env env, int32_t errCode)
{
    napi_value error = CreateJsError(env, errCode, errorMap[errCode]);
    napi_throw(env, error);
    return CreateJsUndefined(env);
}

void ResolveWithNoError(napi_env env, NapiAsyncTask &task, napi_value value = nullptr)
{
    if (value == nullptr) {
        task.Resolve(env, CreateJsNull(env));
    } else {
        task.Resolve(env, value);
    }
}

napi_value AttachAppContext(napi_env env, void *value, void *)
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

    napi_value object = CreateJsBaseContext(env, ptr, true);
    napi_coerce_to_native_binding_object(env, object, DetachCallbackFunc, AttachAppContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Platform::Context>(ptr);
    napi_wrap(env, object, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Platform::Context> *>(data);
        }, nullptr, nullptr);
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

void JSAbilityDelegator::Finalizer(napi_env env, void *data, void *hint)
{
    HILOG_INFO("enter");
    std::unique_ptr<JSAbilityDelegator>(static_cast<JSAbilityDelegator *>(data));
}

napi_value JSAbilityDelegator::AddAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityMonitor);
}

napi_value JSAbilityDelegator::WaitAbilityMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityMonitor);
}

napi_value JSAbilityDelegator::AddAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnAddAbilityStageMonitor);
}

napi_value JSAbilityDelegator::RemoveAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnRemoveAbilityStageMonitor);
}

napi_value JSAbilityDelegator::WaitAbilityStageMonitor(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnWaitAbilityStageMonitor);
}

napi_value JSAbilityDelegator::GetAppContext(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAppContext);
}

napi_value JSAbilityDelegator::GetAbilityState(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetAbilityState);
}

napi_value JSAbilityDelegator::GetCurrentTopAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnGetCurrentTopAbility);
}

napi_value JSAbilityDelegator::StartAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnStartAbility);
}

napi_value JSAbilityDelegator::DoAbilityForeground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityForeground);
}

napi_value JSAbilityDelegator::DoAbilityBackground(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnDoAbilityBackground);
}

napi_value JSAbilityDelegator::Print(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrint);
}

napi_value JSAbilityDelegator::PrintSync(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnPrintSync);
}

napi_value JSAbilityDelegator::FinishTest(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JSAbilityDelegator, OnFinishTest);
}

napi_value JSAbilityDelegator::OnAddAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor)) {
        HILOG_ERROR("Parse addAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "addAbilityMonitor failed."));
            return;
        }
        delegator->AddAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnAddAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted)) {
        HILOG_ERROR("Parse addAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnAddAbilityStageMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "addAbilityStageMonitor failed."));
            return;
        }
        delegator->AddAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnAddAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return result;
}

napi_value JSAbilityDelegator::OnRemoveAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    if (!ParseAbilityMonitorPara(env, info, monitor)) {
        HILOG_ERROR("Parse removeAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) mutable {
        HILOG_INFO("OnRemoveAbilityMonitor NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "removeAbilityMonitor failed."));
            return;
        }
        delegator->RemoveAbilityMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env,
        lastParam, nullptr, std::move(complete), &result));

    if (AbilityDelegatorRegistry::GetAbilityDelegator()) {
        for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
            std::shared_ptr<NativeReference> jsMonitor = iter->first;
            bool isEquals = false;
            napi_strict_equals(env, (info.argv[INDEX_ZERO]), jsMonitor->GetNapiValue(), &isEquals);
            if (isEquals) {
                g_monitorRecord.erase(iter);
                break;
            }
        }
    }
    return result;
}

napi_value JSAbilityDelegator::OnRemoveAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    bool isExisted = false;
    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    if (!ParseAbilityStageMonitorPara(env, info, monitor, isExisted)) {
        HILOG_ERROR("Parse removeAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete =
        [monitor](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnRemoveAbilityStageMonitor NapiAsyncTask is called");

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "removeAbilityStageMonitor failed."));
            return;
        }
        delegator->RemoveAbilityStageMonitor(monitor);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnRemoveAbilityStageMonitor", env,
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));

    if (isExisted) {
        RemoveStageMonitorRecord(env, info.argv[INDEX_ZERO]);
    }
    return result;
}

napi_value JSAbilityDelegator::OnWaitAbilityMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityMonitorPara(env, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto abilityObjectBox = std::make_shared<AbilityObjectBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityObjectBox]() {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback is called");
        if (!abilityObjectBox) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid abilityObjectBox");
            return;
        }

        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid delegator");
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

    NapiAsyncTask::CompleteCallback complete = [abilityObjectBox](napi_env env, NapiAsyncTask &task, int32_t status)
    {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask CompleteCallback is called");
        if (abilityObjectBox && !abilityObjectBox->object_.expired()) {
            ResolveWithNoError(env, task,abilityObjectBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "waitAbilityMonitor failed."));
        }
    };

    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnWaitAbilityStageMonitor(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::shared_ptr<AbilityStageMonitor> monitor = nullptr;
    TimeoutCallback opt {false, false};
    int64_t timeout = 0;
    if (!ParseWaitAbilityStageMonitorPara(env, info, monitor, opt, timeout)) {
        HILOG_ERROR("Parse waitAbilityStageMonitor parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto abilityStageObjBox = std::make_shared<AbilityStageObjBox>();
    NapiAsyncTask::ExecuteCallback execute = [monitor, timeout, opt, abilityStageObjBox]() {
        HILOG_INFO("OnWaitAbilityStageMonitor NapiAsyncTask ExecuteCallback is called");
        if (!abilityStageObjBox) {
            HILOG_ERROR("OnWaitAbilityStageMonitor NapiAsyncTask ExecuteCallback, Invalid abilityStageObjBox");
            return;
        }
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("OnWaitAbilityMonitor NapiAsyncTask ExecuteCallback, Invalid delegator");
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

    NapiAsyncTask::CompleteCallback complete = [abilityStageObjBox](
        napi_env env, NapiAsyncTask &task, int32_t status)
    {
        HILOG_INFO("OnWaitAbilityMonitor NapiAsyncTask CompleteCallback is called");
        if (abilityStageObjBox && !abilityStageObjBox->object_.expired()) {
            ResolveWithNoError(env, task, abilityStageObjBox->object_.lock()->GetNapiValue());
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "waitAbilityStageMonitor failed."));
        }
    };
    napi_value lastParam = nullptr;
    if (opt.hasCallbackPara) {
        lastParam = opt.hasTimeoutPara ? info.argv[INDEX_TWO] : info.argv[INDEX_ONE];
    }
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnWaitAbilityStageMonitor",
        env, CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnPrint(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [msg](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnPrint NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "print failed."));
            return;
        }
        delegator->Print(msg);
        ResolveWithNoError(env, task);
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnPrint",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnPrintSync(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    if (!ParsePrintPara(env, info, msg)) {
        HILOG_ERROR("Parse print parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("Invalid delegator");
        return CreateJsUndefined(env);
    }

    delegator->Print(msg);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::OnGetAppContext(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return CreateJsNull(env);
    }
    std::shared_ptr<AbilityRuntime::Platform::Context> context = delegator->GetAppContext();
    if (!context) {
        HILOG_ERROR("context is null");
        return CreateJsNull(env);
    }
    napi_value value = CreateJsBaseContext(env, context, false);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::Platform::Context>(context);
    napi_coerce_to_native_binding_object(env, value, DetachCallbackFunc, AttachAppContext, workContext, nullptr);
    napi_wrap(env, value, workContext,
        [](napi_env, void *data, void *) {
            HILOG_INFO("Finalizer for weak_ptr app context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Platform::Context> *>(data);
        }, nullptr, nullptr);
    return value;
}

napi_value JSAbilityDelegator::OnGetAbilityState(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return CreateJsUndefined(env);
    }

    std::string fullName;
    if (!ParseAbilityParaInfo(env, info.argv[INDEX_ZERO], fullName)) {
        HILOG_ERROR("Parse ability parameter failed");
        return CreateJsUndefined(env);
    }

    auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
    if (!delegator) {
        HILOG_ERROR("delegator is null");
        return CreateJsUndefined(env);
    }
    AbilityDelegator::AbilityState lifeState = delegator->GetAbilityState(fullName);
    AbilityLifecycleState abilityLifeState = AbilityLifecycleState::UNINITIALIZED;
    AbilityLifecycleStateToJs(lifeState, abilityLifeState);
    return CreateJsValue(env, static_cast<int>(abilityLifeState));
}

napi_value JSAbilityDelegator::OnGetCurrentTopAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    if (info.argc >= ARGC_ONE && !AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ZERO], napi_function)) {
        HILOG_ERROR("Parse getCurrentTopAbility parameter failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [this](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnGetCurrentTopAbility NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            HILOG_ERROR("Invalid delegator");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "getCurrentTopAbility failed."));
            return;
        }

        auto property = delegator->GetCurrentTopAbility();
        if (!property || property->object_.expired()) {
            HILOG_ERROR("Invalid property");
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "getCurrentTopAbility failed."));
        } else {
            {
                std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
                g_abilityRecordInfo.emplace(property->object_, property->fullName_);
            }
            ResolveWithNoError(env, task, property->object_.lock()->GetNapiValue());
        }
    };

    napi_value lastParam = (info.argc >= ARGC_ONE) ? info.argv[INDEX_ZERO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnGetCurrentTopAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnStartAbility(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    AAFwk::Want want;
    if (!ParseStartAbilityPara(env, info, want)) {
        HILOG_ERROR("Parse startAbility parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [want](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnStartAbility NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "startAbility failed."));
            return;
        }
        int result = delegator->StartAbility(want);
        if (result) {
            task.Reject(env, CreateJsError(env, result, "startAbility failed."));
        } else {
            ResolveWithNoError(env, task);
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnStartAbility",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnDoAbilityForeground(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string fullName;
    if (!ParseAbilityCommonPara(env, info, fullName)) {
        HILOG_ERROR("Parse doAbilityForeground parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [fullName](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityForeground NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityForeground failed."));
            return;
        }
        if (delegator->DoAbilityForeground(fullName)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityForeground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityForeground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnDoAbilityBackground(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string fullName;
    if (!ParseAbilityCommonPara(env, info, fullName)) {
        HILOG_ERROR("Parse doAbilityBackground parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [fullName](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnDoAbilityBackground NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityBackground failed."));
            return;
        }
        if (delegator->DoAbilityBackground(fullName)) {
            ResolveWithNoError(env, task, CreateJsNull(env));
        } else {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "doAbilityBackground failed."));
        }
    };

    napi_value lastParam = (info.argc > ARGC_ONE) ? info.argv[INDEX_ONE] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JSAbilityDelegator::OnDoAbilityBackground",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::OnFinishTest(napi_env env, NapiCallbackInfo& info)
{
    HILOG_INFO("enter, argc = %{public}d", static_cast<int32_t>(info.argc));

    std::string msg;
    int64_t code = 0;
    if (!ParseFinishTestPara(env, info, msg, code)) {
        HILOG_ERROR("Parse finishTest parameters failed");
        return ThrowJsError(env, INCORRECT_PARAMETERS);
    }

    NapiAsyncTask::CompleteCallback complete = [msg, code](napi_env env, NapiAsyncTask &task, int32_t status) {
        HILOG_INFO("OnFinishTest NapiAsyncTask is called");
        auto delegator = AbilityDelegatorRegistry::GetAbilityDelegator();
        if (!delegator) {
            task.Reject(env, CreateJsError(env, COMMON_FAILED, "finishTest failed."));
            return;
        }
        delegator->FinishUserTest(msg, code);
        ResolveWithNoError(env, task);
    };
    napi_value lastParam = (info.argc > ARGC_TWO) ? info.argv[INDEX_TWO] : nullptr;
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSAbilityDelegator::OnFinishTest",
        env, CreateAsyncTaskWithLastParam(env, lastParam, nullptr, std::move(complete), &result));
    return result;
}

napi_value JSAbilityDelegator::ParseMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter, monitorRecord size = %{public}zu", g_monitorRecord.size());

    for (auto iter = g_monitorRecord.begin(); iter != g_monitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            HILOG_ERROR("monitor existed");
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }

    napi_value abilityNameValue = nullptr;
    napi_get_named_property(env, value, "abilityName", &abilityNameValue);
    if (abilityNameValue == nullptr) {
        HILOG_ERROR("Failed to get property abilityName");
        return nullptr;
    }

    std::string abilityName;
    if (!ConvertFromJsValue(env, abilityNameValue, abilityName)) {
        return nullptr;
    }

    std::string moduleName = "";
    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue != nullptr && !ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        HILOG_WARN("Failed to get property moduleName");
        moduleName = "";
    }

    std::shared_ptr<JSAbilityMonitor> abilityMonitor = nullptr;
    if (moduleName.empty()) {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName);
    } else {
        abilityMonitor = std::make_shared<JSAbilityMonitor>(abilityName, moduleName);
    }

    abilityMonitor->SetJsAbilityMonitorEnv(env);
    abilityMonitor->SetJsAbilityMonitor(value);

    monitor = std::make_shared<AbilityMonitor>(abilityName, abilityMonitor);
    std::shared_ptr<NativeReference> reference = nullptr;
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    g_monitorRecord.emplace(reference, monitor);

    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStageMonitorPara(
    napi_env env, napi_value value, std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    HILOG_INFO("enter, stageMonitorRecord size = %{public}zu", g_stageMonitorRecord.size());

    isExisted = false;
    for (auto iter = g_stageMonitorRecord.begin(); iter != g_stageMonitorRecord.end(); ++iter) {
        std::shared_ptr<NativeReference> jsMonitor = iter->first;
        bool isEquals = false;
        napi_strict_equals(env, value, jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            HILOG_WARN("AbilityStage monitor existed");
            isExisted = true;
            monitor = iter->second;
            return monitor ? CreateJsNull(env) : nullptr;
        }
    }
    napi_value moduleNameValue = nullptr;
    napi_get_named_property(env, value, "moduleName", &moduleNameValue);
    if (moduleNameValue == nullptr) {
        HILOG_ERROR("Failed to get property moduleName");
        return nullptr;
    }
    std::string moduleName;
    if (!ConvertFromJsValue(env, moduleNameValue, moduleName)) {
        HILOG_ERROR("Failed to get moduleName from JsValue");
        return nullptr;
    }
    napi_value srcEntranceValue = nullptr;
    napi_get_named_property(env, value, "srcEntrance", &srcEntranceValue);
    if (srcEntranceValue == nullptr) {
        HILOG_ERROR("Failed to get property srcEntrance");
        return nullptr;
    }
    std::string srcEntrance;
    if (!ConvertFromJsValue(env, srcEntranceValue, srcEntrance)) {
        HILOG_ERROR("Failed to get srcEntrance from JsValue");
        return nullptr;
    }

    monitor = std::make_shared<AbilityStageMonitor>(moduleName, srcEntrance);
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityParaInfo(napi_env env, napi_value value, std::string &fullName)
{
    HILOG_INFO("enter");

    std::unique_lock<std::mutex> lck(g_mutexAbilityRecord);
    for (auto iter = g_abilityRecordInfo.begin(); iter != g_abilityRecordInfo.end();) {
        if (iter->first.expired()) {
            iter = g_abilityRecordInfo.erase(iter);
            continue;
        }
        
        bool isEqual = false;
        napi_strict_equals(env, iter->first.lock()->GetNapiValue(), value, &isEqual);
        if (isEqual) {
            fullName = iter->second;
            HILOG_INFO("Ability exist");
            return fullName.c_str() ? CreateJsNull(env) : nullptr;
        }

        ++iter;
    }

    HILOG_ERROR("Ability doesn't exist");
    fullName = "";
    return nullptr;
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

napi_value JSAbilityDelegator::ParseAbilityMonitorPara(
    napi_env env, NapiCallbackInfo& info, std::shared_ptr<AbilityMonitor> &monitor)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Parse monitor parameters failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("ParseAbilityMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, bool &isExisted)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Parse stage monitor parameters failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("ParseAbilityStageMonitorPara, Parse callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseMonitorPara(env, info.argv[INDEX_ZERO], monitor)) {
        HILOG_ERROR("Monitor parse parameters failed");
        return nullptr;
    }

    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseWaitAbilityStageMonitorPara(napi_env env, NapiCallbackInfo& info,
    std::shared_ptr<AbilityStageMonitor> &monitor, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    bool isExisted = false;
    if (!ParseStageMonitorPara(env, info.argv[INDEX_ZERO], monitor, isExisted)) {
        HILOG_ERROR("Stage monitor parse parameters failed");
        return nullptr;
    }
    if (!ParseTimeoutCallbackPara(env, info, opt, timeout)) {
        HILOG_ERROR("TimeoutCallback parse parameters failed");
        return nullptr;
    }
    if (!isExisted) {
        AddStageMonitorRecord(env, info.argv[INDEX_ZERO], monitor);
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseTimeoutCallbackPara(
    napi_env env, NapiCallbackInfo& info, TimeoutCallback &opt, int64_t &timeout)
{
    HILOG_INFO("enter");

    opt.hasCallbackPara = false;
    opt.hasTimeoutPara = false;

    if (info.argc >= ARGC_TWO) {
        if (ConvertFromJsValue(env, info.argv[INDEX_ONE], timeout)) {
            opt.hasTimeoutPara = true;
        } else {
            if (info.argv[INDEX_ONE] == nullptr) {
                HILOG_WARN("info.argv[1] is null");
            } else if (AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
                opt.hasCallbackPara = true;
                return CreateJsNull(env);
            } else {
                return nullptr;
            }
        }

        if (info.argc > ARGC_TWO) {
            if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
                if (info.argv[INDEX_TWO] == nullptr) {
                    HILOG_WARN("info.argv[2] is null");
                    return CreateJsNull(env);
                }
                return nullptr;
            }
            opt.hasCallbackPara = true;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParsePrintPara(napi_env env, NapiCallbackInfo& info, std::string &msg)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse callback parameter failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseAbilityCommonPara(
    napi_env env, NapiCallbackInfo& info, std::string &fullName)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ParseAbilityParaInfo(env, info.argv[INDEX_ZERO], fullName)) {
        HILOG_ERROR("Parse ability parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse ability callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseStartAbilityPara(
    napi_env env, NapiCallbackInfo& info, AAFwk::Want &want)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_ONE) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!OHOS::AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want)) {
        HILOG_ERROR("Parse want parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_ONE) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_ONE], napi_function)) {
            HILOG_ERROR("Parse StartAbility callback parameters failed");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

napi_value JSAbilityDelegator::ParseFinishTestPara(
    napi_env env, NapiCallbackInfo& info, std::string &msg, int64_t &code)
{
    HILOG_INFO("enter");
    if (info.argc < ARGC_TWO) {
        HILOG_ERROR("Incorrect number of parameters");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], msg)) {
        HILOG_ERROR("Parse msg parameter failed");
        return nullptr;
    }

    if (!ConvertFromJsValue(env, info.argv[INDEX_ONE], code)) {
        HILOG_ERROR("Parse code para parameter failed");
        return nullptr;
    }

    if (info.argc > ARGC_TWO) {
        if (!AppExecFwk::IsTypeForNapiValue(env, info.argv[INDEX_TWO], napi_function)) {
            HILOG_ERROR("Incorrect Callback Function type");
            return nullptr;
        }
    }
    return CreateJsNull(env);
}

void JSAbilityDelegator::AddStageMonitorRecord(
    napi_env env, napi_value value, const std::shared_ptr<AbilityStageMonitor> &monitor)
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
    napi_ref ref = nullptr;
    napi_create_reference(env, value, 1, &ref);
    reference.reset(reinterpret_cast<NativeReference*>(ref));
    {
        std::unique_lock<std::mutex> lck(g_mtxStageMonitorRecord);
        g_stageMonitorRecord.emplace(reference, monitor);
    }
    HILOG_INFO("g_stageMonitorRecord added, size = %{public}zu", g_stageMonitorRecord.size());
}

void JSAbilityDelegator::RemoveStageMonitorRecord(napi_env env, napi_value value)
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
        bool isEquals = false;
        napi_strict_equals(env, value,jsMonitor->GetNapiValue(), &isEquals);
        if (isEquals) {
            g_stageMonitorRecord.erase(iter);
            HILOG_INFO("g_stageMonitorRecord removed, size = %{public}zu", g_stageMonitorRecord.size());
            break;
        }
    }
}
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
