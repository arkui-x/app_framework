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

class EventHub {
    constructor() {
      this.eventMap = {};
    }
  
    on(event, callback) {
      if ((typeof (event) !== 'string') || (typeof (callback) !== 'function')) {
        return;
      }
      if (!this.eventMap[event]) {
        this.eventMap[event] = [];
      }
      if (this.eventMap[event].indexOf(callback) === -1) {
        this.eventMap[event].push(callback);
      }
    }
  
    off(event, callback) {
      if (typeof (event) !== 'string') {
        return;
      }
      if (this.eventMap[event]) {
        if (callback) {
          let cbArray = this.eventMap[event];
          let index = cbArray.indexOf(callback);
          if (index > -1) {
            for (; index + 1 < cbArray.length; index++) {
              cbArray[index] = cbArray[index + 1];
            }
            cbArray.pop();
          }
        } else {
          delete this.eventMap[event];
        }
      }
    }
  
    emit(event, ...args) {
      if (typeof (event) !== 'string') {
        return;
      }
      if (this.eventMap[event]) {
        const cloneArray = [...this.eventMap[event]];
        const len = cloneArray.length;
        for (let i = 0; i < len; ++i) {
          cloneArray[i].apply(this, args);
        }
      }
    }
}

class Context {
    constructor(obj) {
        this.__contextImpl__ = obj;
        this.__contextImpl__.eventHub = new EventHub();
    }

    get resourceManager() {
        return this.__contextImpl__.resourceManager;
    }

    get applicationInfo() {
        return this.__contextImpl__.applicationInfo;
    }

    get cacheDir() {
        return this.__contextImpl__.cacheDir;
    }

    get tempDir() {
        return this.__contextImpl__.tempDir;
    }

    get filesDir() {
        return this.__contextImpl__.filesDir;
    }

    get databaseDir() {
        return this.__contextImpl__.databaseDir;
    }

    get preferencesDir() {
        return this.__contextImpl__.preferencesDir;
    }

    get bundleCodeDir() {
        return this.__contextImpl__.bundleCodeDir;
    }

    get eventHub() {
      return this.__contextImpl__.eventHub;
    }

    get stageMode() {
        return true;
    }
    
    getApplicationContext() {
        return this.__contextImpl__.getApplicationContext();
    }

    createModuleContext(moduleName) {
        return this.__contextImpl__.createModuleContext(moduleName);
    }
}

export default Context;
