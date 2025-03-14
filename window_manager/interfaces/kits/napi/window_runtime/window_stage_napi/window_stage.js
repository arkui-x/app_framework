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

class WindowStage {
    constructor(obj) {
      this.__windowStage__ = obj;
    }
  
    setUIContent(context, url, storage) {
      return this.__windowStage__.setUIContent(context, url, storage);
    }
  
    loadContent(url, storage, asyncCallback) {
      return this.__windowStage__.loadContent(url, storage, asyncCallback);
    }

    loadContentByName(name, storage, asyncCallback) {
      return this.__windowStage__.loadContentByName(name, storage, asyncCallback);
    }

    getWindowMode(asyncCallback) {
      return this.__windowStage__.getWindowMode(asyncCallback);
    }
  
    getMainWindow(asyncCallback) {
      return this.__windowStage__.getMainWindow(asyncCallback);
    }
  
    getMainWindowSync() {
      return this.__windowStage__.getMainWindowSync();
    }
  
    createSubWindow(windowName, asyncCallback) {
      return this.__windowStage__.createSubWindow(windowName, asyncCallback);
    }
  
    getSubWindow(asyncCallback) {
      return this.__windowStage__.getSubWindow(asyncCallback);
    }
  
    on(type, callback) {
      return this.__windowStage__.on(type, callback);
    }
  
    off(type, callback) {
      return this.__windowStage__.off(type, callback);
    }
  
    setShowOnLockScreen(showOnLockScreen) {
      return this.__windowStage__.setShowOnLockScreen(showOnLockScreen);
    }
  
    disableWindowDecor() {
      return this.__windowStage__.disableWindowDecor();
    }
}
  
export default WindowStage;
