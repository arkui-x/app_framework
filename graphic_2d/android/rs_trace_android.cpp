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

#include <android/trace.h>
#include "platform/common/rs_log.h"
#include "rs_trace_crossplatform.h"

namespace OHOS {
namespace Rosen {
void RosenTraceBegin(const char* name)
{
    if (name != nullptr) {
        ATrace_beginSection(name);
    }
}

void RosenTraceBegin(std::string name)
{
    ATrace_beginSection(name.c_str());
}

void RosenTraceEnd()
{
    ATrace_endSection();
}

ScopedTrace::ScopedTrace(const char *name) {
    if (name != nullptr) {
        ATrace_beginSection(name);
    }
}

ScopedTrace::ScopedTrace(std::string name) {
    ATrace_beginSection(name.c_str());
}

ScopedTrace::~ScopedTrace() {
    ATrace_endSection();
}

} // namespace Rosen
} // namespace OHOS
