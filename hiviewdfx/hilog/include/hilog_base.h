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

#ifndef HILOG_BASE_H
#define HILOG_BASE_H

#include <stdint.h>
#include <stddef.h>

#define SOCKET_FILE_DIR "/dev/unix/socket/"
#define INPUT_SOCKET_NAME "hilogInput"

#define MAX_LOG_LEN 4096 /* maximum length of a log, include '\0' */
#define MAX_TAG_LEN 32   /* log tag size, include '\0' */

/*
 * header of log message from libhilog to hilogd
 */
typedef struct __attribute__((__packed__)) {
    uint16_t len;
    uint16_t version : 3;
    uint16_t type : 4; /* APP,CORE,INIT,SEC etc */
    uint16_t level : 3;
    uint16_t tagLen : 6; /* include '\0' */
    uint32_t tv_sec;
    uint32_t tv_nsec;
    uint32_t mono_sec;
    uint32_t pid;
    uint32_t tid;
    uint32_t domain;
    char tag[]; /* shall be end with '\0' */
} HilogMsg;

#if defined(__GNUC__) && (__GNUC__ >= 4)
#define HILOG_PUBLIC_API __attribute__((visibility("default")))
#define HILOG_LOCAL_API __attribute__((visibility("hidden")))
#else
#define HILOG_PUBLIC_API
#define HILOG_LOCAL_API
#endif

#endif /* HILOG_BASE_H */
