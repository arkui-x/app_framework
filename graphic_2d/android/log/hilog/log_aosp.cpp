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

#include "log.h"
#include <android/log.h>
#include <cstring>
#include <string>

namespace OHOS {
namespace HiviewDFX {
namespace {
int LogPrint(const android_LogPriority &logPriority, const char *tag, const char *fmt, va_list args)
{
    std::string str(fmt);
    for (auto pos = str.find("{public}", 0); pos != std::string::npos; pos = str.find("{public}", pos)) {
        str.erase(pos, 8); // 8 is the length of "{public}"
    }
    __android_log_vprint(logPriority, tag, str.c_str(), args);
    return 0;
}
}

int HiLog::Debug(const HiLogLabel &label, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(ANDROID_LOG_DEBUG, label.tag, fmt, args);
    va_end(args);
    return 0;
}
int HiLog::Info(const HiLogLabel &label, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(ANDROID_LOG_INFO, label.tag, fmt, args);
    va_end(args);
    return 0;
}
int HiLog::Warn(const HiLogLabel &label, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(ANDROID_LOG_WARN, label.tag, fmt, args);
    va_end(args);
    return 0;
}
int HiLog::Error(const HiLogLabel &label, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(ANDROID_LOG_ERROR, label.tag, fmt, args);
    va_end(args);
    return 0;
}
int HiLog::Fatal(const HiLogLabel &label, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(ANDROID_LOG_FATAL, label.tag, fmt, args);
    va_end(args);
    return 0;
}

}  // namespace AppExecFwk
}  // namespace OHOS
