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
#include <vector>
namespace OHOS::Ace::Platform {
struct ComponentInfo {
public:
  std::string text;
  std::string type;
  std::string compid;
  float top = 0.0f;
  float width = 0.0f;
  float left = 0.0f;
  float height = 0.0f;
  bool clickable = false;
  bool checkable = false;
  bool checked = false;
  bool selected = false;
  bool scrollable = false;
  bool enabled = false;
  bool focused = false;
  bool longClickable = false;
  std::vector<ComponentInfo> children;
};
} // namespace OHOS::Ace::Platform
#endif