/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "js_timer.h"

#include <atomic>
#include <string>
#include <vector>

#include "hilog.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_common.h"

#ifdef SUPPORT_GRAPHICS
#include "core/common/container_scope.h"
#endif

#ifdef SUPPORT_GRAPHICS
using OHOS::Ace::ContainerScope;
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
class JsTimer;
    
std::atomic<uint32_t> g_callbackId(1);

constexpr size_t MIN_SIZE = 2;
    
class JsTimer final {
public:
    JsTimer(JsRuntime& jsRuntime, const std::shared_ptr<NativeReference>& jsFunction, const std::string &name,
        int64_t interval, bool isInterval)
        : jsRuntime_(jsRuntime), jsFunction_(jsFunction), name_(name), interval_(interval), isInterval_(isInterval)
    {}

    ~JsTimer() = default;

    void operator()() const
    {
        if (isInterval_) {
            jsRuntime_.PostTask(*this, name_, interval_);
        }
#ifdef SUPPORT_GRAPHICS
        // call js function
        ContainerScope containerScope(containerScopeId_);
#endif
        HandleScope handleScope(jsRuntime_);

        std::vector<napi_value> args_;
        args_.reserve(jsArgs_.size());
        for (auto arg : jsArgs_) {
            args_.emplace_back(arg->GetNapiValue());
        }

        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value result = nullptr;
        napi_get_undefined(env, &result);
        napi_call_function(env, result, jsFunction_->GetNapiValue(), args_.size(), args_.data(), nullptr);
    }

    void PushArgs(const std::shared_ptr<NativeReference>& ref)
    {
        jsArgs_.emplace_back(ref);
    }

private:
    JsRuntime& jsRuntime_;
    std::shared_ptr<NativeReference> jsFunction_;
    std::vector<std::shared_ptr<NativeReference>> jsArgs_;
    std::string name_;
    int64_t interval_ = 0;
    bool isInterval_ = false;
#ifdef SUPPORT_GRAPHICS
    int32_t containerScopeId_ = ContainerScope::CurrentId();
#endif
};

napi_value StartTimeoutOrInterval(napi_env env, napi_callback_info info, bool isInterval)
{
    if (env == nullptr || info == nullptr) {
        HILOG_ERROR("StartTimeoutOrInterval, env or callback info is nullptr.");
        return nullptr;
    }
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    // parameter check, must have at least 1 params
    if (argc < 1 || !CheckTypeForNapiValue(env, argv[0], napi_function)) {
        HILOG_ERROR("Set callback timer failed with invalid parameter.");
        return CreateJsUndefined(env);
    }

    // parse parameter
    napi_ref ref = nullptr;
    napi_create_reference(env, argv[0], 1, &ref);
    std::shared_ptr<NativeReference> jsFunction(reinterpret_cast<NativeReference*>(ref));
    uint32_t callbackId = g_callbackId.fetch_add(1, std::memory_order_relaxed);
    std::string name = "JsRuntimeTimer_";
    name.append(std::to_string(callbackId));

    // create timer task
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    JsRuntime& jsRuntime = *reinterpret_cast<JsRuntime*>(engine->GetJsEngine());
    env = reinterpret_cast<napi_env>(engine);

    int64_t delayTime = 0;
    if (argc < MIN_SIZE || !CheckTypeForNapiValue(env, argv[1], napi_number)) {
        JsTimer task(jsRuntime, jsFunction, name, delayTime, isInterval);
        jsRuntime.PostTask(task, name, delayTime);
        return CreateJsValue(env, callbackId);
    }
    napi_get_value_int64(env, argv[1], &delayTime);
    JsTimer task(jsRuntime, jsFunction, name, delayTime, isInterval);
    for (size_t index = 2; index < argc; ++index) {
        napi_ref ref = nullptr;
        napi_create_reference(env, argv[index], 1, &ref);
        task.PushArgs(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    }
    jsRuntime.PostTask(task, name, delayTime);
    return CreateJsValue(env, callbackId);
}

napi_value StartTimeout(napi_env env, napi_callback_info info)
{
    return StartTimeoutOrInterval(env, info, false);
}

napi_value StartInterval(napi_env env, napi_callback_info info)
{
    return StartTimeoutOrInterval(env, info, true);
}

napi_value StopTimeoutOrInterval(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        HILOG_ERROR("Stop timeout or interval failed with env or callback info is nullptr.");
        return nullptr;
    }
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr));

    // parameter check, must have at least 1 param
    if (argc < 1 || !CheckTypeForNapiValue(env, argv[0], napi_number)) {
        HILOG_ERROR("Clear callback timer failed with invalid parameter.");
        return CreateJsUndefined(env);
    }

    uint32_t callbackId = 0;
    napi_get_value_uint32(env, argv[0], &callbackId);
    std::string name = "JsRuntimeTimer_";
    name.append(std::to_string(callbackId));
    NativeEngine* engine = reinterpret_cast<NativeEngine*>(env);
    JsRuntime& jsRuntime = *reinterpret_cast<JsRuntime*>(engine->GetJsEngine());
    env = reinterpret_cast<napi_env>(engine);
    jsRuntime.RemoveTask(name);
    return CreateJsUndefined(env);
}
}

void InitTimerModule(napi_env env, napi_value globalObject)
{
    HILOG_DEBUG("InitTimerModule begin.");
    const char *moduleName = "JsTimer";
    BindNativeFunction(env, globalObject, "setTimeout", moduleName, StartTimeout);
    BindNativeFunction(env, globalObject, "setInterval", moduleName, StartInterval);
    BindNativeFunction(env, globalObject, "clearTimeout", moduleName, StopTimeoutOrInterval);
    BindNativeFunction(env, globalObject, "clearInterval", moduleName, StopTimeoutOrInterval);
}
} // namespace AbilityRuntime
} // namespace OHOS