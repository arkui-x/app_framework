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
#include "napi_app_event_watcher.h"

#include "app_event_store.h"
#include "hiappevent_base.h"
#include "hilog/log.h"
#include "napi_util.h"
#include "uv.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002D07

#undef LOG_TAG
#define LOG_TAG "NapiWatcher"

namespace OHOS {
namespace HiviewDFX {
namespace {
constexpr size_t CALLBACK_PARAM_NUM = 3;
constexpr size_t RECEIVE_PARAM_NUM = 2;

void DeleteEventMappingSync(int64_t observerSeq, const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    std::vector<int64_t> eventSeqs;
    for (const auto& event : events) {
        eventSeqs.emplace_back(event->GetSeq());
    }
    if (!AppEventStore::GetInstance().DeleteData(observerSeq, eventSeqs)) {
        HILOG_ERROR(LOG_CORE, "failed to delete mapping data, seq=%l{public}d, event num=%{public}zu", observerSeq,
            eventSeqs.size());
    }
}

} // namespace
OnTriggerContext::~OnTriggerContext()
{
    if (onTrigger != nullptr) {
        napi_delete_reference(env, onTrigger);
    }
    if (holder != nullptr) {
        napi_delete_reference(env, holder);
    }
}

OnReceiveContext::~OnReceiveContext()
{
    if (onReceive != nullptr) {
        napi_delete_reference(env, onReceive);
    }
}

WatcherContext::~WatcherContext()
{
    if (triggerContext != nullptr) {
        delete triggerContext;
    }
    if (receiveContext != nullptr) {
        delete receiveContext;
    }
}

NapiAppEventWatcher::NapiAppEventWatcher(
    const std::string& name, const std::vector<AppEventFilter>& filters, TriggerCondition cond)
    : AppEventWatcher(name, filters, cond), context_(nullptr)
{}

NapiAppEventWatcher::~NapiAppEventWatcher()
{
    HILOG_DEBUG(LOG_CORE, "start to destroy NapiAppEventWatcher object");
    if (context_ == nullptr) {
        return;
    }
    napi_env env = nullptr;
    if (context_->receiveContext != nullptr) {
        env = context_->receiveContext->env;
    } else if (context_->triggerContext != nullptr) {
        env = context_->triggerContext->env;
    } else {
        delete context_;
        context_ = nullptr;
        return;
    }
    auto task = [contextData = context_] () {
        HILOG_DEBUG(LOG_CORE, "start to destroy WatcherContext object");
        delete contextData;
    };
    if (napi_send_event(env, task, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
        delete context_;
        context_ = nullptr;
    }
}

void NapiAppEventWatcher::InitHolder(const napi_env env, const napi_value holder)
{
    if (context_ == nullptr) {
        context_ = new (std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new (std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            return;
        }
    }
    context_->triggerContext->env = env;
    context_->triggerContext->holder = NapiUtil::CreateReference(env, holder);
}

void NapiAppEventWatcher::OnTrigger(const TriggerCondition& triggerCond)
{
    HILOG_DEBUG(LOG_CORE, "onTrigger start");
    if (context_ == nullptr || context_->triggerContext == nullptr) {
        HILOG_ERROR(LOG_CORE, "onTrigger context is null");
        return;
    }
    context_->triggerContext->row = triggerCond.row;
    context_->triggerContext->size = triggerCond.size;
    auto onTriggerWork = [triggerContext = context_->triggerContext] () {
        auto context = static_cast<OnTriggerContext*>(triggerContext);
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(context->env, &scope);
        if (scope == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to open handle scope");
            return;
        }
        napi_value callback = NapiUtil::GetReferenceValue(context->env, context->onTrigger);
        if (callback == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            napi_close_handle_scope(context->env, scope);
            return;
        }
        napi_value argv[CALLBACK_PARAM_NUM] = {
            NapiUtil::CreateInt32(context->env, context->row),
            NapiUtil::CreateInt32(context->env, context->size),
            NapiUtil::GetReferenceValue(context->env, context->holder)
        };
        napi_value ret = nullptr;
        if (napi_call_function(context->env, nullptr, callback, CALLBACK_PARAM_NUM, argv, &ret) != napi_ok) {
            HILOG_ERROR(LOG_CORE, "failed to call onTrigger function");
        }
        napi_close_handle_scope(context->env, scope);
    };
    if (napi_send_event(context_->triggerContext->env, onTriggerWork, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

void NapiAppEventWatcher::InitTrigger(const napi_env env, const napi_value triggerFunc)
{
    if (context_ == nullptr) {
        context_ = new (std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->triggerContext == nullptr) {
        context_->triggerContext = new (std::nothrow) OnTriggerContext();
        if (context_->triggerContext == nullptr) {
            return;
        }
    }
    context_->triggerContext->env = env;
    context_->triggerContext->onTrigger = NapiUtil::CreateReference(env, triggerFunc);
}

void NapiAppEventWatcher::InitReceiver(const napi_env env, const napi_value receiveFunc)
{
    if (context_ == nullptr) {
        context_ = new (std::nothrow) WatcherContext();
        if (context_ == nullptr) {
            return;
        }
    }
    if (context_->receiveContext == nullptr) {
        context_->receiveContext = new (std::nothrow) OnReceiveContext();
        if (context_->receiveContext == nullptr) {
            return;
        }
    }
    context_->receiveContext->env = env;
    context_->receiveContext->onReceive = NapiUtil::CreateReference(env, receiveFunc);
}

void NapiAppEventWatcher::OnEvents(const std::vector<std::shared_ptr<AppEventPack>>& events)
{
    HILOG_DEBUG(LOG_CORE, "onEvents start, seq=%l{public}d, event num=%{public}zu", GetSeq(), events.size());
    if (context_ == nullptr || context_->receiveContext == nullptr || events.empty()) {
        HILOG_ERROR(LOG_CORE, "onReceive context is null or events is empty");
        return;
    }
    context_->receiveContext->domain = events[0]->GetDomain();
    context_->receiveContext->events = events;
    context_->receiveContext->observerSeq = GetSeq();
    auto onReceiveWork = [receiveContext = context_->receiveContext] () {
        auto context = static_cast<OnReceiveContext*>(receiveContext);
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(context->env, &scope);
        if (scope == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to open handle scope");
            return;
        }
        napi_value callback = NapiUtil::GetReferenceValue(context->env, context->onReceive);
        if (callback == nullptr) {
            HILOG_ERROR(LOG_CORE, "failed to get callback from the context");
            napi_close_handle_scope(context->env, scope);
            return;
        }
        napi_value argv[RECEIVE_PARAM_NUM] = {
            NapiUtil::CreateString(context->env, context->domain),
            NapiUtil::CreateEventGroups(context->env, context->events)
        };
        napi_value ret = nullptr;
        if (napi_call_function(context->env, nullptr, callback, RECEIVE_PARAM_NUM, argv, &ret) == napi_ok) {
            DeleteEventMappingSync(context->observerSeq, context->events);
        } else {
            HILOG_ERROR(LOG_CORE, "failed to call onReceive function");
        }
        napi_close_handle_scope(context->env, scope);
    };
    if (napi_send_event(context_->receiveContext->env, onReceiveWork, napi_eprio_high) != napi_status::napi_ok) {
        HILOG_ERROR(LOG_CORE, "failed to SendEvent.");
    }
}

bool NapiAppEventWatcher::IsRealTimeEvent(std::shared_ptr<AppEventPack> event)
{
    return (context_ != nullptr && context_->receiveContext != nullptr);
}
} // namespace HiviewDFX
} // namespace OHOS
