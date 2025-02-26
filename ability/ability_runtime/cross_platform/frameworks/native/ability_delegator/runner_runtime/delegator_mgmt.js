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

function isSystemplugin(shortName, moduleType) {
    const plugin = moduleType === 'ohos' ? globalThis.ohosplugin : globalThis.systemplugin;
    if (typeof (plugin) !== 'undefined') {
      let target = plugin;
      for (let key of shortName.split('.')) {
        target = target[key];
        if (!target) {
          break;
        }
      }
      return typeof (target) !== 'undefined';
    }
}


let global = globalThis;
globalThis.exports = {default: {}};
globalThis.$appDefine$ = function (page, packageName, parseContent) {
    const module = {exports: {}};
    parseContent({}, module.exports, module);
    globalThis.exports.default = module.exports;
};
globalThis.$appBootstrap$ = function() {};