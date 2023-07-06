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

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "native_engine/native_engine.h"
#include "res_common.h"

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

static napi_value InitColorModeObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "COLOR_MODE_NOT_SET", Global::Resource::ColorMode::COLOR_MODE_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, object, "COLOR_MODE_DARK", Global::Resource::ColorMode::DARK));
    NAPI_CALL(env, SetEnumItem(env, object, "COLOR_MODE_LIGHT", Global::Resource::ColorMode::LIGHT));

    return object;
}

static napi_value InitDirectionObject(napi_env env)
{
    napi_value object;
    NAPI_CALL(env, napi_create_object(env, &object));

    NAPI_CALL(env, SetEnumItem(env, object, "DIRECTION_NOT_SET", Global::Resource::Direction::DIRECTION_NOT_SET));
    NAPI_CALL(env, SetEnumItem(env, object, "DIRECTION_VERTICAL", Global::Resource::Direction::DIRECTION_VERTICAL));
    NAPI_CALL(env, SetEnumItem(env, object, "DIRECTION_HORIZONTAL", Global::Resource::Direction::DIRECTION_HORIZONTAL));

    return object;
}

static napi_value ConfigurationConstantInit(napi_env env, napi_value exports)
{
    napi_value colorMode = InitColorModeObject(env);
    NAPI_ASSERT(env, colorMode != nullptr, "failed to create color mode object");

    napi_value direction = InitDirectionObject(env);
    NAPI_ASSERT(env, direction != nullptr, "failed to create direction object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("ColorMode", colorMode),
        DECLARE_NAPI_PROPERTY("Direction", direction),
    };

    napi_status status = napi_define_properties(env, exports, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exports");

    return exports;
}

static napi_module _module = { .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ConfigurationConstantInit,
    .nm_modname = "app.ability.ConfigurationConstant",
    .nm_priv = (static_cast<void*>(0)),
    .reserved = { 0 } };

extern "C" __attribute__((constructor)) void NAPI_app_ability_ConfigurationConstant_AutoRegister(void)
{
    napi_module_register(&_module);
}
} // namespace AAFwk
} // namespace OHOS
