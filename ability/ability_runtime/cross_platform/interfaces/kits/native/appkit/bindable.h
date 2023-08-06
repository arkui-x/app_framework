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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_BINDABLE_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_BINDABLE_H

#include <memory>

namespace OHOS {
namespace AbilityRuntime {
class Runtime;
namespace Platform {
class BindingObject final {
public:
    template<class T>
    BindingObject(Runtime& runtime, T* ptr) : runtime_(runtime), object_(ptr, SimpleRelease<T>)
    {}
    ~BindingObject() = default;

    template<class T>
    T* Get()
    {
        return static_cast<T*>(object_.get());
    }

    void Reset()
    {
        object_.reset();
    }

    Runtime& GetRuntime()
    {
        return runtime_;
    }

    BindingObject(const BindingObject&) = delete;
    BindingObject& operator=(const BindingObject&) = delete;
    BindingObject(BindingObject&&) = delete;
    BindingObject& operator=(BindingObject&&) = delete;

private:
    template<class T>
    static void SimpleRelease(void* ptr)
    {
        // There is no need to delete here
    }

    Runtime& runtime_;
    std::unique_ptr<void, void (*)(void*)> object_;
};

class Bindable {
public:
    virtual ~Bindable() = default;

    template<class T>
    void Bind(Runtime& runtime, T* object)
    {
        object_ = std::make_unique<BindingObject>(runtime, object);
    }

    const std::unique_ptr<BindingObject>& GetBindingObject() const
    {
        return object_;
    }

protected:
    Bindable() = default;

private:
    std::unique_ptr<BindingObject> object_;
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_BINDABLE_H
