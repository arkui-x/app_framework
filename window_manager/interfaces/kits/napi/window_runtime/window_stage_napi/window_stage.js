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
      this.__window_stage__ = obj;
    }
  
    setUIContent(context, url, storage) {
      return this.__window_stage__.setUIContent(context, url, storage);
    }
  
    loadContent(url, storage, asyncCallback) {
      return this.__window_stage__.loadContent(url, storage, asyncCallback);
    }

    getWindowMode(asyncCallback) {
      return this.__window_stage__.getWindowMode(asyncCallback);
    }
  
    getMainWindow(asyncCallback) {
      return this.__window_stage__.getMainWindow(asyncCallback);
    }
  
    getMainWindowSync() {
      return this.__window_stage__.getMainWindowSync();
    }
  
    createSubWindow(windowName, asyncCallback) {
      return this.__window_stage__.createSubWindow(windowName, asyncCallback);
    }
  
    getSubWindow(asyncCallback) {
      return this.__window_stage__.getSubWindow(asyncCallback);
    }
  
    on(type, callback) {
      return this.__window_stage__.on(type, callback);
    }
  
    off(type, callback) {
      return this.__window_stage__.off(type, callback);
    }
  
    setShowOnLockScreen(showOnLockScreen) {
      return this.__window_stage__.setShowOnLockScreen(showOnLockScreen);
    }
  
    disableWindowDecor() {
      return this.__window_stage__.disableWindowDecor();
    }
}
  
export default WindowStage;
