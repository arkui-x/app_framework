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

#ifndef FOUNDATION_APPFRAMEWORK_UICONTENT_DECLARATIVE_MODULE_PRELOADER_H
#define FOUNDATION_APPFRAMEWORK_UICONTENT_DECLARATIVE_MODULE_PRELOADER_H

#include <memory>
#include <string>

#include "base/utils/macros.h"

class NativeEngine;
namespace OHOS::Ace::Platform {
class DeclarativeModulePreloader {
public:
    static void Preload(NativeEngine& runtime);
};
} // namespace OHOS::Ace::Platform
#endif // FOUNDATION_APPFRAMEWORK_UICONTENT_DECLARATIVE_MODULE_PRELOADER_H