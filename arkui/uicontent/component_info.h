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

#ifndef FOUNDATION_APPFRAMEWORK_UICONTENT_COMMON_COMPONENT_INFO_H
#define FOUNDATION_APPFRAMEWORK_UICONTENT_COMMON_COMPONENT_INFO_H

#include <string>

namespace OHOS::Ace::Platform {
struct ComponentInfo {
public:
  std::string compid;
  std::string text;
  std::string type;
  float top = 0.0f;
  float width = 0.0f;
  float left = 0.0f;
  float height = 0.0f;
  bool clickable = true;
  bool checkable = true;
  bool checked = true;
  bool selected = true;
  bool scrollable = true;
  bool enabled = true;
  bool focused = true;
  bool longClickable = true;
};
} // namespace OHOS::Ace::Platform
#endif