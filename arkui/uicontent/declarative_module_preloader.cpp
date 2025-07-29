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

#include "base/log/ace_trace.h"
#include "base/thread/background_task_executor.h"
#include "core/components/common/properties/text_style.h"
#include "core/components_ng/render/font_collection.h"
#include "core/components_ng/render/paragraph.h"
#include "frameworks/bridge/declarative_frontend/engine/jsi/jsi_declarative_engine.h"
#include "stage_application_info_adapter.h"

namespace OHOS::Ace::Platform {

void InitAceModule(void *runtime)
{
    Framework::JsiDeclarativeEngineInstance::PreloadAceModule(runtime);
}

void DeclarativeModulePreloader::Preload(NativeEngine &runtime)
{
    BackgroundTaskExecutor::GetInstance().PostTask([]() {
        ACE_SCOPED_TRACE("PreLayout");
        std::string language;
        std::string country;
        std::string script;
        AbilityRuntime::Platform::StageApplicationInfoAdapter::GetInstance()->GetLocale(language, country, script);
        AceApplicationInfo::GetInstance().SetLocale(language, country, script, "");
        NG::ParagraphStyle paraStyle;
        auto paragraph = NG::Paragraph::Create(paraStyle, NG::FontCollection::Current());
        TextStyle textStyle;
        paragraph->PushStyle(textStyle);
        paragraph->AddText(u"abc123\u4e2d\u6587");
        paragraph->Build();
        paragraph->Layout(100);
    });
    ACE_SCOPED_TRACE("PreloadAceModule");
    InitAceModule(reinterpret_cast<void *>(&runtime));
}

void DeclarativeModulePreloader::PreloadWorker(NativeEngine& runtime)
{
    Framework::JsiDeclarativeEngineInstance::PreloadAceModuleWorker(reinterpret_cast<void*>(&runtime));
}

} // namespace OHOS::Ace::Platform
