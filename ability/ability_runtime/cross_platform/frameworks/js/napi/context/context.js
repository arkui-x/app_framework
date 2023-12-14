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

class Context {
    constructor(obj) {
        this.__context_impl__ = obj
    }

    get resourceManager() {
        return this.__context_impl__.resourceManager
    }

    get applicationInfo() {
        return this.__context_impl__.applicationInfo
    }

    get cacheDir() {
        return this.__context_impl__.cacheDir
    }

    get tempDir() {
        return this.__context_impl__.tempDir
    }

    get filesDir() {
        return this.__context_impl__.filesDir
    }

    get databaseDir() {
        return this.__context_impl__.databaseDir
    }

    get preferencesDir() {
        return this.__context_impl__.preferencesDir
    }

    get bundleCodeDir() {
        return this.__context_impl__.bundleCodeDir
    }

    get stageMode() {
        return true;
    }
    
    getApplicationContext() {
        return this.__context_impl__.getApplicationContext()
    }

    createModuleContext(moduleName) {
        return this.__context_impl__.createModuleContext(moduleName)
    }
}

export default Context
