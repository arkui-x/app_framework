/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "ability_info.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AAFwk {
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

static napi_value InitLaunchTypeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "SINGLETON", static_cast<int32_t>(AppExecFwk::LaunchMode::SINGLETON)));
    NAPI_CALL(env, SetEnumItem(env, object, "STANDARD", static_cast<int32_t>(AppExecFwk::LaunchMode::STANDARD)));
    NAPI_CALL(env, SetEnumItem(env, object, "MULTITON", static_cast<int32_t>(AppExecFwk::LaunchMode::STANDARD)));

    return object;
}

static napi_value BundleManagerExport(napi_env env, napi_value exports)
{
    napi_value launchType = InitLaunchTypeObject(env);
    NAPI_ASSERT(env, launchType != nullptr, "failed to create launch type object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("LaunchType", launchType),
    };

    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");

    return exports;
}

static napi_module bundle_manager_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = BundleManagerExport,
    .nm_modname = "bundle.bundleManager",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void BundleManagerRegister(void)
{
    napi_module_register(&bundle_manager_module);
}
}
}