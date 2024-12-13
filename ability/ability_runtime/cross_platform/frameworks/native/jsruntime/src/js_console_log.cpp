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

#include "js_console_log.h"

#include <string>

#include "hilog.h"
#include "napi/native_common.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint32_t JS_CONSOLE_LOG_MAX_LOG_LEN = 1024;
constexpr uint32_t JS_CONSOLE_LOG_DOMAIN = 0xFEFE;
enum class LogLevel : uint32_t {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

std::string MakeLogContent(napi_env env, napi_callback_info info)
{
    std::string content;
    
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    for (size_t i = 0; i < argc; i++) {
        napi_value value = argv[i];
        if (!CheckTypeForNapiValue(env, value, napi_string)) {
            napi_value resultStr = nullptr;
            napi_coerce_to_string(env, value, &resultStr);
            value = resultStr;
        }

        if (value == nullptr) {
            HILOG_ERROR("Failed to convert to string object");
            continue;
        }

        size_t bufferLen = 0;
        napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &bufferLen);
        if (status != napi_ok || bufferLen == 0 || bufferLen >= JS_CONSOLE_LOG_MAX_LOG_LEN) {
            HILOG_DEBUG("Log length exceeds maximum");
            return content;
        }

        auto buff = new (std::nothrow) char[bufferLen + 1];
        if (buff == nullptr) {
            HILOG_ERROR("Failed to allocate buffer, size = %zu", bufferLen + 1);
            break;
        }

        size_t strLen = 0;
        napi_get_value_string_utf8(env, value, buff, bufferLen + 1, &strLen);
        if (!content.empty()) {
            content.append(" ");
        }
        content.append(buff);
        delete [] buff;
    }

    return content;
}

template<LogLevel LEVEL>
napi_value ConsoleLog(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        HILOG_ERROR("env or callback info is nullptr");
        return nullptr;
    }

    std::string content = MakeLogContent(env, info);
    switch (LEVEL) {
        case LogLevel::INFO:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        case LogLevel::DEBUG:
            HILOG_DEBUG("%{public}s", content.c_str());
            break;
        case LogLevel::WARN:
            HILOG_WARN("%{public}s", content.c_str());
            break;
        case LogLevel::ERROR:
            HILOG_ERROR("%{public}s", content.c_str());
            break;
        case LogLevel::FATAL:
            HILOG_FATAL("%{public}s", content.c_str());
            break;
        default:
            HILOG_INFO("%{public}s", content.c_str());
            break;
    }

    return CreateJsUndefined(env);
}
} // namespace

void InitConsoleLogModule(napi_env env, napi_value globalObject)
{
    napi_value consoleObj = nullptr;
    napi_create_object(env, &consoleObj);
    if (consoleObj == nullptr) {
        HILOG_ERROR("Failed to create console object");
        return;
    }
    const char* moduleName = "console";
    BindNativeFunction(env, consoleObj, "log", moduleName, ConsoleLog<LogLevel::INFO>);
    BindNativeFunction(env, consoleObj, "debug", moduleName, ConsoleLog<LogLevel::DEBUG>);
    BindNativeFunction(env, consoleObj, "info", moduleName, ConsoleLog<LogLevel::INFO>);
    BindNativeFunction(env, consoleObj, "warn", moduleName, ConsoleLog<LogLevel::WARN>);
    BindNativeFunction(env, consoleObj, "error", moduleName, ConsoleLog<LogLevel::ERROR>);
    BindNativeFunction(env, consoleObj, "fatal", moduleName, ConsoleLog<LogLevel::FATAL>);

    napi_set_named_property(env, globalObject, "console", consoleObj);
}
} // namespace AbilityRuntime
} // namespace OHOS