/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "resource_manager_addon.h"

#if !defined(__ARKUI_CROSS__)
#include "ability.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/appkit/ability_runtime/context/context.h"
#include "js_runtime_utils.h"
#include "hisysevent_adapter.h"
#include "hitrace_meter.h"
#endif
#include "hilog/log.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "res_common.h"
#include "resource_manager_napi_async_impl.h"

namespace OHOS {
namespace Global {
namespace Resource {

#if !defined(__ARKUI_CROSS__)
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

using namespace OHOS::AppExecFwk;

static void ExecuteGetResMgr(napi_env env, void* data)
{
    if (data == nullptr) {
        return;
    }
    ResMgrDataContext *asyncContext = static_cast<ResMgrDataContext*>(data);

    asyncContext->createValueFunc_ = [](napi_env env, ResMgrDataContext &context) -> napi_value {
        std::string traceVal = "Create ResourceManager";
        StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
        napi_value result = ResourceManagerAddon::Create(env, context.bundleName_, context.resMgr_, nullptr);
        FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
        if (result == nullptr) {
            context.SetErrorMsg("Failed to get ResourceManagerAddon");
            ReportInitResourceManagerFail(context.bundleName_, "failed to get ResourceManagerAddon");
            return nullptr;
        }
        return result;
    };
}

Ability* GetGlobalAbility(napi_env env)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to get global");
        return nullptr;
    }

    napi_value abilityObj;
    status = napi_get_named_property(env, global, "ability", &abilityObj);
    if (status != napi_ok || abilityObj == nullptr) {
        HiLog::Warn(LABEL, "Failed to get ability property");
        return nullptr;
    }

    Ability* ability = nullptr;
    status = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (status == napi_ok && ability != nullptr) {
        return ability;
    }

    return nullptr;
}

static bool InitAsyncContext(napi_env env, const std::string &bundleName, Ability* ability,
    const std::shared_ptr<AbilityRuntime::Context>& context, ResMgrDataContext &asyncContext)
{
    std::shared_ptr<ResourceManager> resMgr;
    if (ability != nullptr) {
        if (bundleName.empty()) {
            resMgr = ability->GetResourceManager();
        } else {
            std::shared_ptr<Context> bundleContext = ability->CreateBundleContext(bundleName, 0);
            if (bundleContext != nullptr) {
                resMgr = bundleContext->GetResourceManager();
            }
        }
    } else if (context != nullptr) {
        if (bundleName.empty()) {
            resMgr = context->GetResourceManager();
        } else {
            std::shared_ptr<OHOS::AbilityRuntime::Context> bundleContext = context->CreateBundleContext(bundleName);
            if (bundleContext != nullptr) {
                resMgr = bundleContext->GetResourceManager();
            }
        }
    }
    asyncContext.resMgr_ = resMgr;
    asyncContext.bundleName_ = bundleName;
    return resMgr != nullptr;
}

static napi_value getResult(napi_env env, std::unique_ptr<ResMgrDataContext> &asyncContext,
    std::string &bundleName, const std::shared_ptr<AbilityRuntime::Context> &abilityRuntimeContext)
{
    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }

    if (!InitAsyncContext(env, bundleName, GetGlobalAbility(env), abilityRuntimeContext, *asyncContext)) {
        HiLog::Error(LABEL, "init async context failed");
        ReportInitResourceManagerFail(bundleName, "failed to init async context");
        return nullptr;
    }

    napi_value resource = nullptr;
    napi_create_string_utf8(env, "getResourceManager", NAPI_AUTO_LENGTH, &resource);
    napi_status status = napi_create_async_work(env, nullptr, resource, ExecuteGetResMgr,
        ResourceManagerNapiAsyncImpl::Complete, static_cast<void*>(asyncContext.get()), &asyncContext->work_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to create async work for getResourceManager %{public}d", status);
        return result;
    }
    status = napi_queue_async_work(env, asyncContext->work_);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to queue async work for getResourceManager %{public}d", status);
        return result;
    }
    asyncContext.release();
    return result;
}

static napi_value GetResourceManager(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);

    std::unique_ptr<ResMgrDataContext> asyncContext = std::make_unique<ResMgrDataContext>();
    std::shared_ptr<AbilityRuntime::Context> abilityRuntimeContext;
    std::string bundleName;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
        if (i == 0 && valueType == napi_object) {
            using WeakContextPtr = std::weak_ptr<AbilityRuntime::Context> *;
            WeakContextPtr objContext;
            napi_status status = napi_unwrap(env, argv[0], reinterpret_cast<void **>(&objContext));
            if (status != napi_ok || objContext == nullptr) {
                HiLog::Error(LABEL, "Failed to get objContext");
                return nullptr;
            }
            auto context = objContext->lock();
            if (context == nullptr) {
                HiLog::Error(LABEL, "Failed to get context");
                return nullptr;
            }
            abilityRuntimeContext = context;
        } else if ((i == 0 || i == 1) && valueType == napi_string) {
            size_t len = 0;
            napi_status status = napi_get_value_string_utf8(env, argv[i], nullptr, 0, &len);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to get bundle name length");
                return nullptr;
            }
            std::vector<char> buf(len + 1);
            status = napi_get_value_string_utf8(env, argv[i], buf.data(), len + 1, &len);
            if (status != napi_ok) {
                HiLog::Error(LABEL, "Failed to get bundle name");
                return nullptr;
            }
            bundleName = buf.data();
        } else if ((i == 0 || i == 1 || i == 2) && valueType == napi_function) { // 2 means the third parameter
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }

    napi_value result = getResult(env, asyncContext, bundleName, abilityRuntimeContext);
    return result;
}
#endif

static napi_value GetSystemResourceManager(napi_env env, napi_callback_info info)
{
    return ResourceManagerAddon::GetSystemResMgr(env);
}

static napi_status SetEnumItem(napi_env env, napi_value object, const char* name, int32_t value)
{
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, object, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitDirectionObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "DIRECTION_VERTICAL", DIRECTION_VERTICAL));
    NAPI_CALL(env, SetEnumItem(env, object, "DIRECTION_HORIZONTAL", DIRECTION_HORIZONTAL));
    return object;
}

static napi_value InitDeviceTypeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_PHONE", DEVICE_PHONE));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_TABLET", DEVICE_TABLET));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_CAR", DEVICE_CAR));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_PC", DEVICE_PAD));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_TV", DEVICE_TV));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_WEARABLE", DEVICE_WEARABLE));
    NAPI_CALL(env, SetEnumItem(env, object, "DEVICE_TYPE_2IN1", DEVICE_TWOINONE));
    return object;
}

static napi_value InitScreenDensityObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_SDPI", SCREEN_DENSITY_SDPI));
    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_MDPI", SCREEN_DENSITY_MDPI));
    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_LDPI", SCREEN_DENSITY_LDPI));
    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_XLDPI", SCREEN_DENSITY_XLDPI));
    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_XXLDPI", SCREEN_DENSITY_XXLDPI));
    NAPI_CALL(env, SetEnumItem(env, object, "SCREEN_XXXLDPI", SCREEN_DENSITY_XXXLDPI));
    return object;
}

static napi_value InitColorModeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "DARK", DARK));
    NAPI_CALL(env, SetEnumItem(env, object, "LIGHT", LIGHT));
    return object;
}

static napi_value ResMgrInit(napi_env env, napi_value exports)
{
#if !defined(__ARKUI_CROSS__)
    std::string traceVal = "GetResourceManager";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_property_descriptor creatorProp[] = {
        DECLARE_NAPI_FUNCTION("getResourceManager", GetResourceManager),
        DECLARE_NAPI_FUNCTION("getSystemResourceManager", GetSystemResourceManager),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(creatorProp) / sizeof(creatorProp[0]),
        creatorProp);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    napi_property_descriptor creatorProp[] = {
        DECLARE_NAPI_FUNCTION("getSystemResourceManager", GetSystemResourceManager),
    };
    napi_status status = napi_define_properties(env, exports, sizeof(creatorProp) / sizeof(creatorProp[0]),
        creatorProp);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "Failed to set getResourceManager at init");
        return nullptr;
    }

    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_PROPERTY("Direction", InitDirectionObject(env)),
        DECLARE_NAPI_PROPERTY("DeviceType", InitDeviceTypeObject(env)),
        DECLARE_NAPI_PROPERTY("ScreenDensity", InitScreenDensityObject(env)),
        DECLARE_NAPI_PROPERTY("ColorMode", InitColorModeObject(env)),
    };

    status = napi_define_properties(env, exports, sizeof(static_prop) / sizeof(static_prop[0]), static_prop);
    if (status != napi_ok) {
        HiLog::Error(LABEL, "failed to define properties for exports");
        return nullptr;
    }

    return exports;
}

static napi_module g_resourceManagerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ResMgrInit,
    .nm_modname = "resourceManager",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void ResMgrRegister()
{
    napi_module_register(&g_resourceManagerModule);
}
} // namespace Resource
} // namespace Global
} // namespace OHOS