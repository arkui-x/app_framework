/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

let Context = requireNapi('application.Context');

class AbilityContext extends Context {
    constructor(obj) {
        super(obj);
        this.abilityInfo = obj.abilityInfo;
        this.currentHapModuleInfo = obj.currentHapModuleInfo;
        this.config = obj.config;
        this.windowStage = obj.windowStage;
    }

    onUpdateConfiguration(config) {
        this.config = config;
    }

    startAbility(want, options, callback) {
        return this.__contextImpl__.startAbility(want, options, callback);
    }

    terminateSelf(callback) {
        return this.__contextImpl__.terminateSelf(callback);
    }

    startAbilityForResult(want, options, callback) {
        return this.__contextImpl__.startAbilityForResult(want, options, callback);
    }

    terminateSelfWithResult(abilityResult, callback) {
        return this.__contextImpl__.terminateSelfWithResult(abilityResult, callback);
    }

    reportDrawnCompleted(callback) {
        return this.__contextImpl__.reportDrawnCompleted(callback);
    }
}

export default AbilityContext;
