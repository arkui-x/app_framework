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

#ifndef HILOG_H
#define HILOG_H

// Log domain
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0
#endif

// Log tag
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

// Log type
typedef enum {
    LOG_TYPE_MIN = 0,
    LOG_APP = 0,
    LOG_INIT = 1,
    // Used by core service, framework.
    LOG_CORE = 3,
    LOG_KMSG = 4,
    LOG_TYPE_MAX
} LogType;

// Log level
typedef enum {
    LOG_LEVEL_MIN = 0,
    LOG_DEBUG = 3,
    LOG_INFO = 4,
    LOG_WARN = 5,
    LOG_ERROR = 6,
    LOG_FATAL = 7,
    LOG_LEVEL_MAX,
} LogLevel;

namespace OHOS::HiviewDFX {
using HiLogLabel = struct {
    LogType type;
    unsigned int domain;
    const char *tag;
};

class HiLog final {
public:
    static int Debug(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(os_log, 2, 3)));
    static int Info(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(os_log, 2, 3)));
    static int Warn(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(os_log, 2, 3)));
    static int Error(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(os_log, 2, 3)));
    static int Fatal(const HiLogLabel &label, const char *fmt, ...) __attribute__((__format__(os_log, 2, 3)));
};
}

#endif