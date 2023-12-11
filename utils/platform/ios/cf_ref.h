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

#ifndef OHOS_PLATFORM_IOS_CF_REF_H_
#define OHOS_PLATFORM_IOS_CF_REF_H_

#include <CoreFoundation/CoreFoundation.h>

namespace OHOS {

template <class T>
class CFRef {
 public:
  CFRef() : instance_(nullptr) {}

  CFRef(T instance) : instance_(instance) {}

  ~CFRef() {
    if (instance_ != nullptr) {
      CFRelease(instance_);
    }
    instance_ = nullptr;
  }

  void Reset(T instance) {
    if (instance_ == instance) {
      return;
    }
    if (instance_ != nullptr) {
      CFRelease(instance_);
    }
    instance_ = instance;
  }
  operator T() const { return instance_; }
  operator bool() const { return instance_ != nullptr; }

 private:
  T instance_;
  CFRef(const CFRef&) = delete;
  CFRef& operator=(const CFRef&) = delete;
};
} // namespace ohos

#endif // OHOS_PLATFORM_IOS_CF_REF_H_
