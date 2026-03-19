/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef HIVIEWDFX_HILOG_BASE_C_H
#define HIVIEWDFX_HILOG_BASE_C_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Log domain, indicates the log service domainID. Different LogType have different domainID ranges.
 * Log type of LOG_APP: 0-0xFFFF
 * Log type of LOG_CORE & LOG_INIT: 0xD000000-0xD0FFFFF
 */
#ifndef LOG_DOMAIN
#define LOG_DOMAIN 0
#endif

/**
 * Log tag, indicates the log service tag, you can customize the tag as needed, usually the keyword of the module.
 */
#ifndef LOG_TAG
#define LOG_TAG NULL
#endif

/* Log type */
typedef enum {
    /* min log type */
    LOG_TYPE_MIN = 0,
    /* Used by app log. */
    LOG_APP = 0,
    /* Log to kmsg, only used by init phase. */
    LOG_INIT = 1,
    /* Used by core service, framework. */
    LOG_CORE = 3,
    /* Used by kmsg log. */
    LOG_KMSG = 4,
    /* Not print in release version. */
    LOG_ONLY_PRERELEASE = 5,
    /* max log type */
    LOG_TYPE_MAX
} LogType;

/* Log level */
typedef enum {
    /* min log level */
    LOG_LEVEL_MIN = 0,
    /* Designates lower priority log. */
    LOG_DEBUG = 3,
    /* Designates useful information. */
    LOG_INFO = 4,
    /* Designates hazardous situations. */
    LOG_WARN = 5,
    /* Designates very serious errors. */
    LOG_ERROR = 6,
    /* Designates major fatal anomaly. */
    LOG_FATAL = 7,
    /* max log level */
    LOG_LEVEL_MAX,
} LogLevel;

int HiLogBasePrint(LogType type, LogLevel level, unsigned int domain, const char *tag, const char *fmt, ...)
    __attribute__((__format__(os_log, 5, 6)));

/**
 * @brief Hilog base interface of different log level.
 * DEBUG: Designates lower priority log.
 * INFO: Designates useful information.
 * WARN: Designates hazardous situations.
 * ERROR: Designates very serious errors.
 * FATAL: Designates major fatal anomaly.
 *
 * @param type enum:LogType
 */
#define HILOG_BASE_DEBUG(type, ...) ((void)HiLogBasePrint((type), LOG_DEBUG, LOG_DOMAIN, LOG_TAG, __VA_ARGS__))

#define HILOG_BASE_INFO(type, ...) ((void)HiLogBasePrint((type), LOG_INFO, LOG_DOMAIN, LOG_TAG, __VA_ARGS__))

#define HILOG_BASE_WARN(type, ...) ((void)HiLogBasePrint((type), LOG_WARN, LOG_DOMAIN, LOG_TAG, __VA_ARGS__))

#define HILOG_BASE_ERROR(type, ...) ((void)HiLogBasePrint((type), LOG_ERROR, LOG_DOMAIN, LOG_TAG, __VA_ARGS__))

#define HILOG_BASE_FATAL(type, ...) ((void)HiLogBasePrint((type), LOG_FATAL, LOG_DOMAIN, LOG_TAG, __VA_ARGS__))

/**
 * @brief Check whether log of a specified domain, tag and level can be printed.
 *
 * @param domain macro:LOG_DOMAIN
 * @param tag macro:LOG_TAG
 * @param level enum:LogLevel
 */
bool HiLogBaseIsLoggable(unsigned int domain, const char *tag, LogLevel level);

void HiLogRecordSnapshot(int lines, int64_t time);

#ifdef __cplusplus
}
#endif

#ifdef HILOG_RAWFORMAT
#include "hilog_base/log_base_inner.h"
#endif

#endif  // HIVIEWDFX_HILOG_BASE_C_H
