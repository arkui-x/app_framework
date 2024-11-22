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

#ifndef OHOS_WM_INCLUDE_WINDOW_HILOG_H
#define OHOS_WM_INCLUDE_WINDOW_HILOG_H

#include "hilog.h"
#if defined RUNTIME_MODE_RELEASE
#define WLOGD(fmt, ...)
#define WLOGI(fmt, ...)
#define WLOGW(fmt, ...) PRINT_LOG(WARN, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define WLOGE(fmt, ...) PRINT_LOG(ERROR, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)

#else
#define WLOGD(fmt, ...) PRINT_LOG(DEBUG, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define WLOGI(fmt, ...) PRINT_LOG(INFO, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define WLOGW(fmt, ...) PRINT_LOG(WARN, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#define WLOGE(fmt, ...) PRINT_LOG(ERROR, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, ##__VA_ARGS__)
#endif
#endif // FRAMEWORKS_WM_INCLUDE_WINDOW_HILOG_H
