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

#include "foundation/appframework/arkui/uicontent/declarative_module_preloader.h"

#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"

namespace OHOS::Ace::Platform {
void InitAceModule(void* runtime)
{
    Framework::JsiDeclarativeEngineInstance::PreloadAceModule(runtime);
}

void DeclarativeModulePreloader::Preload(NativeEngine& runtime)
{
    InitAceModule(reinterpret_cast<void*>(&runtime));
}
} // namespace ::Ace::Platform