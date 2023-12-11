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

#ifndef FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_LIFECYCLE_CALLBACK_H
#define FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;

namespace OHOS {
namespace AbilityRuntime {
namespace Platform {
class AbilityLifecycleCallback {
public:
    virtual ~AbilityLifecycleCallback() {}
    /**
     * Called back when the ability is started for initialization.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityCreate(const std::shared_ptr<NativeReference>& ability) = 0;

    /**
     * Called back when the window stage is created.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to create.
     * @StageModelOnly
     */
    virtual void OnWindowStageCreate(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage) = 0;

    /**
     * Called back when the window stage is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @param windowStage: Indicates the window stage to destroy.
     * @StageModelOnly
     */
    virtual void OnWindowStageDestroy(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage) = 0;

    /**
     * Called back when the ability is destroy.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityDestroy(const std::shared_ptr<NativeReference>& ability) = 0;

    /**
     * Called back when the ability is foreground.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityForeground(const std::shared_ptr<NativeReference>& ability) = 0;

    /**
     * Called back when the ability is background.
     *
     * @since 9
     * @syscap SystemCapability.Ability.AbilityRuntime.AbilityCore
     * @param ability: Indicates the ability to register for listening.
     * @StageModelOnly
     */
    virtual void OnAbilityBackground(const std::shared_ptr<NativeReference>& ability) = 0;
};

class JsAbilityLifecycleCallback : public AbilityLifecycleCallback,
                                   public std::enable_shared_from_this<JsAbilityLifecycleCallback> {
public:
    explicit JsAbilityLifecycleCallback(napi_env env);
    void OnAbilityCreate(const std::shared_ptr<NativeReference>& ability) override;
    void OnWindowStageCreate(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage) override;
    void OnWindowStageDestroy(
        const std::shared_ptr<NativeReference>& ability, const std::shared_ptr<NativeReference>& windowStage) override;
    void OnAbilityDestroy(const std::shared_ptr<NativeReference>& ability) override;
    void OnAbilityForeground(const std::shared_ptr<NativeReference>& ability) override;
    void OnAbilityBackground(const std::shared_ptr<NativeReference>& ability) override;
    int32_t Register(napi_value jsCallback);
    bool UnRegister(int32_t callbackId);
    bool IsEmpty() const;
    static int32_t serialNumber_;

private:
    napi_env env_ = nullptr;
    std::shared_ptr<NativeReference> jsCallback_;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacks_;
    void CallJsMethod(const std::string& methodName, const std::shared_ptr<NativeReference>& ability);
    void CallWindowStageJsMethod(const std::string& methodName, const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage);
    void CallJsMethodInnerCommon(const std::string& methodName, const std::shared_ptr<NativeReference>& ability,
        const std::shared_ptr<NativeReference>& windowStage,
        const std::map<int32_t, std::shared_ptr<NativeReference>> callbacks);
};
} // namespace Platform
} // namespace AbilityRuntime
} // namespace OHOS
#endif // FOUNDATION_ABILITY_RUNTIME_CROSS_PLATFORM_INTERFACES_KITS_NATIVE_APPKIT_ABILITY_LIFECYCLE_CALLBACK_H
