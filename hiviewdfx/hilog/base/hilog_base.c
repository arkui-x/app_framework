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

#include "hilog_base/log_base.h"
#include <hilog_base.h>
#include <vsnprintf_s_p.h>

#include <errno.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>

#define LOG_LEN 3
#define ERROR_FD 2

static const int SOCKET_TYPE = SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC;
static const struct sockaddr_un SOCKET_ADDR = {AF_UNIX, SOCKET_FILE_DIR INPUT_SOCKET_NAME};
static const int INVALID_FD = -1;
static const int CAS_FAIL = -1;
static const int CAS_INVALID_PARAM = -2;
static const int CAS_SUCCESS = 0;
static const int INVALID_RESULT = -1;
static atomic_int g_socketFd = INVALID_FD;

static void CloseSocket(void)
{
    int socketFd = atomic_load(&g_socketFd);
    if (socketFd >= 0) {
        close(socketFd);
        atomic_store(&g_socketFd, INVALID_FD);
    }
}

__attribute__((constructor)) static void CleanSocketInit(void)
{
    pthread_atfork(NULL, NULL, CloseSocket);
}


__attribute__((destructor)) static void CleanSocket(void)
{
    CloseSocket();
}

static int GenerateSocketFd(void)
{
    int socketFd = TEMP_FAILURE_RETRY(socket(AF_UNIX, SOCKET_TYPE, 0));
    if (socketFd == INVALID_FD) {
        dprintf(ERROR_FD, "HiLogBase: Can't create socket! Errno: %d\n", errno);
        return INVALID_FD;
    }
    long int result =
        TEMP_FAILURE_RETRY(connect(socketFd, (const struct sockaddr *)(&SOCKET_ADDR), sizeof(SOCKET_ADDR)));
    if (result == INVALID_RESULT) {
        dprintf(ERROR_FD, "HiLogBase: Can't connect to server. Errno: %d\n", errno);
        close(socketFd);
        return INVALID_FD;
    }
    return socketFd;
}
 
static int CASGlobalSocketFd(int socketFd)
{
    if (socketFd == INVALID_FD) {
        return CAS_INVALID_PARAM;
    }
    int expected = INVALID_FD;
    // we should use CAS to avoid multi-thread problem
    if (!atomic_compare_exchange_strong(&g_socketFd, &expected, socketFd)) {
        // failure CAS: other threads execute to this branch to close extra fd
        return CAS_FAIL;
    }
    // success CAS: only one thread can execute to this branch
    return CAS_SUCCESS;
}

static int SendMessage(HilogMsg *header, const char *tag, uint16_t tagLen, const char *fmt, uint16_t fmtLen)
{
    bool releaseSocket = false;
    int socketFd = INVALID_FD;
    // read fd by using atomic operation
    socketFd = atomic_load(&g_socketFd);
    if (socketFd == INVALID_FD) {
        socketFd = GenerateSocketFd();
        int result = CASGlobalSocketFd(socketFd);
        if (result == CAS_INVALID_PARAM) {
            return INVALID_RESULT;
        } else if (result == CAS_FAIL) {
            releaseSocket = true;
        }
    }

    struct timespec ts = {0};
    (void)clock_gettime(CLOCK_REALTIME, &ts);
    struct timespec ts_mono = {0};
    (void)clock_gettime(CLOCK_MONOTONIC, &ts_mono);
    header->tv_sec = (uint32_t)(ts.tv_sec);
    header->tv_nsec = (uint32_t)(ts.tv_nsec);
    header->mono_sec = (uint32_t)(ts_mono.tv_sec);
    header->len = sizeof(HilogMsg) + tagLen + fmtLen;
    header->tagLen = tagLen;

    struct iovec vec[LOG_LEN] = {0};
    vec[0].iov_base = header;                   // 0 : index of hos log header
    vec[0].iov_len = sizeof(HilogMsg);          // 0 : index of hos log header
    vec[1].iov_base = (void *)((char *)(tag));  // 1 : index of log tag
    vec[1].iov_len = tagLen;                    // 1 : index of log tag
    vec[2].iov_base = (void *)((char *)(fmt));  // 2 : index of log content
    vec[2].iov_len = fmtLen;                    // 2 : index of log content
    int ret = TEMP_FAILURE_RETRY(writev(socketFd, vec, LOG_LEN));
    if (releaseSocket) {
        close(socketFd);
    }
    return ret;
}

void HiLogRecordSnapshot(int lines, int64_t time)
{
    return;
}

int HiLogBasePrintArgs(
    const LogType type, const LogLevel level, const unsigned int domain, const char *tag, const char *fmt, va_list ap)
{
    char buf[MAX_LOG_LEN] = {0};

    vsnprintfp_s(buf, MAX_LOG_LEN, MAX_LOG_LEN - 1, true, fmt, ap);

    size_t tagLen = strnlen(tag, MAX_TAG_LEN - 1);
    size_t logLen = strnlen(buf, MAX_LOG_LEN - 1);
    HilogMsg header = {0};
    header.type = type;
    header.level = level;
#ifndef __RECV_MSG_WITH_UCRED_
    header.pid = getprocpid();
#endif
    header.tid = (uint32_t)(gettid());
    header.domain = domain;

    return SendMessage(&header, tag, tagLen + 1, buf, logLen + 1);
}

int HiLogBasePrint(LogType type, LogLevel level, unsigned int domain, const char *tag, const char *fmt, ...)
{
    if (!HiLogBaseIsLoggable(domain, tag, level)) {
        return -1;
    }

    int ret;
    va_list ap;
    va_start(ap, fmt);
    ret = HiLogBasePrintArgs(type, level, domain, tag, fmt, ap);
    va_end(ap);
    return ret;
}

bool HiLogBaseIsLoggable(unsigned int domain, const char *tag, LogLevel level)
{
    if ((level <= LOG_LEVEL_MIN) || (level >= LOG_LEVEL_MAX) || tag == NULL) {
        return false;
    }
    return true;
}