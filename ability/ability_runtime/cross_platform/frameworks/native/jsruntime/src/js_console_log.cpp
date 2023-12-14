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
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr uint32_t JS_CONSOLE_LOG_MAX_LOG_LEN = 1024;
enum class LogLevel : uint32_t {
    DEBUG = 0,
    INFO,
    WARN,
    ERROR,
    FATAL,
};

std::string MakeLogContent(NativeCallbackInfo& info)
{
    std::string content;

    for (size_t i = 0; i < info.argc; i++) {
        NativeValue* value = info.argv[i];
        if (value->TypeOf() != NATIVE_STRING) {
            value = value->ToString();
        }

        NativeString* str = ConvertNativeValueTo<NativeString>(value);
        if (str == nullptr) {
            HILOG_ERROR("Failed to convert to string object");
            continue;
        }

        size_t bufferLen = str->GetLength();
        if (bufferLen >= JS_CONSOLE_LOG_MAX_LOG_LEN) {
            HILOG_INFO("Log length exceeds maximum");
            return content;
        }

        auto buff = new (std::nothrow) char[bufferLen + 1];
        if (buff == nullptr) {
            HILOG_ERROR("Failed to allocate buffer, size = %zu", bufferLen + 1);
            break;
        }

        size_t strLen = 0;
        str->GetCString(buff, bufferLen + 1, &strLen);
        if (!content.empty()) {
            content.append(" ");
        }
        content.append(buff);
        delete[] buff;
    }

    return content;
}

template<LogLevel LEVEL>
NativeValue* ConsoleLog(NativeEngine* engine, NativeCallbackInfo* info)
{
    if (engine == nullptr || info == nullptr) {
        HILOG_ERROR("engine or callback info is nullptr");
        return nullptr;
    }

    std::string content = MakeLogContent(*info);
    switch (LEVEL) {
        case LogLevel::INFO:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        case LogLevel::DEBUG:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        case LogLevel::WARN:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        case LogLevel::ERROR:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        case LogLevel::FATAL:
            HILOG_INFO("%{public}s", content.c_str());
            break;
        default:
            HILOG_INFO("%{public}s", content.c_str());
            break;
    }

    return engine->CreateUndefined();
}
} // namespace

void InitConsoleLogModule(NativeEngine& engine, NativeObject& globalObject)
{
    NativeValue* consoleValue = engine.CreateObject();
    NativeObject* consoleObj = ConvertNativeValueTo<NativeObject>(consoleValue);
    if (consoleObj == nullptr) {
        HILOG_ERROR("Failed to create console object");
        return;
    }
    const char* moduleName = "console";
    BindNativeFunction(engine, *consoleObj, "log", moduleName, ConsoleLog<LogLevel::INFO>);
    BindNativeFunction(engine, *consoleObj, "debug", moduleName, ConsoleLog<LogLevel::DEBUG>);
    BindNativeFunction(engine, *consoleObj, "info", moduleName, ConsoleLog<LogLevel::INFO>);
    BindNativeFunction(engine, *consoleObj, "warn", moduleName, ConsoleLog<LogLevel::WARN>);
    BindNativeFunction(engine, *consoleObj, "error", moduleName, ConsoleLog<LogLevel::ERROR>);
    BindNativeFunction(engine, *consoleObj, "fatal", moduleName, ConsoleLog<LogLevel::FATAL>);

    globalObject.SetProperty("console", consoleValue);
}
} // namespace AbilityRuntime
} // namespace OHOS