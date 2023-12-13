/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef MOCK_NATIVE_INCLUDE_ERRORS_H_
#define MOCK_NATIVE_INCLUDE_ERRORS_H_

#include <cerrno>

namespace OHOS {
/**
 * ErrCode layout
 *
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Bit |31|30|29|28|27|26|25|24|23|22|21|20|19|18|17|16|15|14|13|12|11|10|09|08|07|06|05|04|03|02|01|00|
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |Field|Reserved|        Subsystem      |  Module      |                  Code                         |
 * +-----+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 */

using ErrCode = int;

enum {
    SUBSYS_COMMON = 0,
    SUBSYS_AAFWK = 1,
    SUBSYS_ACCOUNT = 2,
    SUBSYS_AI = 3,
    SUBSYS_APPEXECFWK = 4,
    SUBSYS_APPLICATIONS = 5,
    SUBSYS_ARVR = 6,
    SUBSYS_ARVRHARDWARE = 7,
    SUBSYS_BARRIERFREE = 8,
    SUBSYS_BIOMETRICS = 9,
    SUBSYS_CCRUNTIME = 10,
    SUBSYS_COMMUNICATION = 11,
    SUBSYS_DFX = 12,
    SUBSYS_DISTRIBUTEDDATAMNG = 13,
    SUBSYS_DISTRIBUTEDSCHEDULE = 14,
    SUBSYS_DRIVERS = 15,
    SUBSYS_GLOBAL = 16,
    SUBSYS_GRAPHIC = 17,
    SUBSYS_HBS = 18,
    SUBSYS_IAWARE = 19,
    SUBSYS_IDE = 20,
    SUBSYS_INTELLIACCESSORIES = 21,
    SUBSYS_INTELLISPEAKER = 22,
    SUBSYS_INTELLITV = 23,
    SUBSYS_IOT = 24,
    SUBSYS_IOTHARDWARE = 25,
    SUBSYS_IVIHARDWARE = 26,
    SUBSYS_KERNEL = 27,
    SUBSYS_LOCATION = 28,
    SUBSYS_MSDP = 29,
    SUBSYS_MULTIMEDIA = 30,
    SUBSYS_MULTIMODAINPUT = 31,
    SUBSYS_NOTIFICATION = 32,
    SUBSYS_POWERMNG = 33,
    SUBSYS_ROUTER = 34,
    SUBSYS_SECURITY = 35,
    SUBSYS_SENSORS = 36,
    SUBSYS_SMALLSERVICES = 37,
    SUBSYS_SOURCECODETRANSFORMER = 38,
    SUBSYS_STARTUP = 39,
    SUBSYS_TELEPONY = 40,
    SUBSYS_UPDATE = 41,
    SUBSYS_USB = 42,
    SUBSYS_WEARABLE = 43,
    SUBSYS_WEARABLEHARDWARE = 44,
    SUBSYS_IVI = 45
    // new type
};

// be used to init the subsystem errorno.
constexpr ErrCode ErrCodeOffset(unsigned int subsystem, unsigned int module = 0)
{
    constexpr int SUBSYSTEM_BIT_NUM = 21;
    constexpr int MODULE_BIT_NUM = 16;
    return (subsystem << SUBSYSTEM_BIT_NUM) | (module << MODULE_BIT_NUM);
}

// offset of common error, only be used in this file.
constexpr ErrCode BASE_ERR_OFFSET = ErrCodeOffset(SUBSYS_COMMON);

enum {
    ERR_OK = 0,
    ERR_NO_MEMORY = BASE_ERR_OFFSET + ENOMEM,
    ERR_INVALID_OPERATION = BASE_ERR_OFFSET + ENOSYS,
    ERR_INVALID_VALUE = BASE_ERR_OFFSET + EINVAL,
    ERR_NAME_NOT_FOUND = BASE_ERR_OFFSET + ENOENT,
    ERR_PERMISSION_DENIED = BASE_ERR_OFFSET + EPERM,
    ERR_NO_INIT = BASE_ERR_OFFSET + ENODEV,
    ERR_ALREADY_EXISTS = BASE_ERR_OFFSET + EEXIST,
    ERR_DEAD_OBJECT = BASE_ERR_OFFSET + EPIPE,
    ERR_OVERFLOW = BASE_ERR_OFFSET + EOVERFLOW,
    ERR_ENOUGH_DATA = BASE_ERR_OFFSET + ENODATA,
    ERR_WOULD_BLOCK = BASE_ERR_OFFSET + EWOULDBLOCK,
    ERR_TIMED_OUT = BASE_ERR_OFFSET + ETIMEDOUT
};

#define SUCCEEDED(errCode) ((errCode) == ERR_OK)
#define FAILED(errCode) ((errCode) != ERR_OK)
} // namespace OHOS

#endif // MOCK_NATIVE_INCLUDE_ERRORS_H_
