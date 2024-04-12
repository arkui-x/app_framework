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

#ifndef ABILITY_RUNTIME_HILOG_WRAPPER_H
#define ABILITY_RUNTIME_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "base/log/log.h"

#ifdef HILOG_FATAL
#undef HILOG_FATAL
#endif

#ifdef HILOG_ERROR
#undef HILOG_ERROR
#endif

#ifdef HILOG_WARN
#undef HILOG_WARN
#endif

#ifdef HILOG_INFO
#undef HILOG_INFO
#endif

#ifdef HILOG_DEBUG
#undef HILOG_DEBUG
#endif

#ifdef LOG_LABEL
#undef LOG_LABEL
#endif

#define HILOG_FATAL(fmt, ...) PRINT_LOG(FATAL, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define HILOG_ERROR(fmt, ...) PRINT_LOG(ERROR, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define HILOG_WARN(fmt, ...) PRINT_LOG(WARN, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define HILOG_INFO(fmt, ...) PRINT_LOG(INFO, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define HILOG_DEBUG(fmt, ...) PRINT_LOG(DEBUG, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#else

#define HILOG_FATAL(...)
#define HILOG_ERROR(...)
#define HILOG_WARN(...)
#define HILOG_INFO(...)
#define HILOG_DEBUG(...)
#endif // CONFIG_HILOG

#endif // ABILITY_RUNTIME_HILOG_WRAPPER_H
