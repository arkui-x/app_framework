/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

class BusinessError extends Error {
  constructor(code) {
    let msg = '';
    if (errMap.has(code)) {
      msg = errMap.get(code);
    } else {
      msg = ERROR_MSG_INNER_ERROR;
    }
    super(msg);
    this.code = code;
  }
}

class EventHub {
    constructor() {
      this.eventMap = {};
    }
  
    on(event, callback) {
      if ((typeof (event) !== 'string') || (typeof (callback) !== 'function')) {
        throw new BusinessError(ERROR_CODE_INVALID_PARAM);
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
        throw new BusinessError(ERROR_CODE_INVALID_PARAM);
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
        throw new BusinessError(ERROR_CODE_INVALID_PARAM);
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

class ApplicationContext {
    constructor(obj) {
        this.__contextImpl__ = obj;
        this.__contextImpl__.eventHub = new EventHub();
    }

    on(type, callback) {
        return this.__contextImpl__.on(type, callback);
    }

    off(type, callbackId, callback) {
        return this.__contextImpl__.off(type, callbackId, callback);
    }

    getApplicationContext() {
        return this.__contextImpl__.getApplicationContext();
    }

    createModuleContext(moduleName) {
        return this.__contextImpl__.createModuleContext(moduleName);
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

    getRunningProcessInformation(callback) {
        return this.__contextImpl__.getRunningProcessInformation(callback);
    }

    setColorMode(colorMode) {
        return this.__contextImpl__.setColorMode(colorMode);
    }

    setFont(font) {
      return this.__contextImpl__.setFont(font);
    }
    
    setFontSizeScale(fontSizeScale) {
      return this.__contextImpl__.setFontSizeScale(fontSizeScale);
    }

    setLanguage(language) {
      return this.__contextImpl__.setLanguage(language);
    }
}

export default ApplicationContext;
