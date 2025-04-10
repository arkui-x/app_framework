/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "time_util.h"

#include <chrono>
#include <ctime>
#include <sys/time.h>

namespace OHOS {
namespace HiviewDFX {
namespace TimeUtil {
namespace {
const std::string DEFAULT_DATE = "19700101";
}
uint64_t GetMilliseconds()
{
    auto now = std::chrono::system_clock::now();
    auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millisecs.count();
}

std::string GetTimeZone()
{
    struct timeval tv;
    if (gettimeofday(&tv, nullptr) != 0) {
        return std::string("");
    }
    time_t sysSec = tv.tv_sec;
    struct tm tmLocal;
    if (localtime_r(&sysSec, &tmLocal) == nullptr) {
        return std::string("");
    }
    constexpr size_t buffSize = 6; // for '+0800\0'
    char buff[buffSize] = { 0 };
    if (strftime(buff, buffSize, "%z", &tmLocal) == 0) {
        return std::string("");
    }
    return std::string(buff);
}
} // namespace TimeUtil
} // namespace HiviewDFX
} // namespace OHOS
