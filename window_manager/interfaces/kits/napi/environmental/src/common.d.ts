/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

declare function requireInternal(k: string): unknown;

type ConstructorV2 = {
  new (...args: unknown[]): unknown;
};

declare function ObservedV2<T extends ConstructorV2>(BaseClass: T): T;

interface IEnvironmentValue<T> {
  value: T;
  update(newValue: T): void;
  destroy(): void;
}

declare const Trace: (target: Object, propertyKey: string) => void;

interface Callback<T> {
  (data: T): void;
}

interface Hilog {
  error(domain: number, tag: string, message: string): void;
}

declare namespace window {
  interface Size {
    width: number;
    height: number;
  }

  interface SizeInVp {
    widthVp: number;
    heightVp: number;
  }

  interface Rect {
    left: number;
    top: number;
    width: number;
    height: number;
  }

  interface AvoidArea {
    visible: boolean;
    leftRect: Rect;
    topRect: Rect;
    rightRect: Rect;
    bottomRect: Rect;
  }

  interface AvoidAreaOptions {
    type: number;
    area: AvoidArea;
  }

  enum WindowEventType {
    WINDOW_SHOWN = 1,
    WINDOW_ACTIVE = 2,
    WINDOW_INACTIVE = 3,
    WINDOW_HIDDEN = 4,
    WINDOW_DESTROYED = 7
  }

  interface WindowDensityInfo {
    systemDensity: number;
    defaultDensity: number;
    customDensity: number;
  }

  interface SystemDensity {
    systemDensity: number;
  }

  interface DisplayId {
    displayId: number;
  }

  interface Window {
    getWindowProperties(): { windowRect: Size };
    getWindowAvoidArea(type: number): AvoidArea;
    getWindowDensityInfo(): window.WindowDensityInfo;
    on(type: 'windowSizeChange', callback: Callback<Size>): void;
    off(type: 'windowSizeChange', callback?: Callback<Size>): void;
    on(type: 'avoidAreaChange', callback: Callback<AvoidAreaOptions>): void;
    off(type: 'avoidAreaChange', callback?: Callback<AvoidAreaOptions>): void;
    on(type: 'windowEvent', callback: Callback<WindowEventType>): void;
    off(type: 'windowEvent', callback?: Callback<WindowEventType>): void;
    on(type: 'windowHighlightChange', callback: Callback<boolean>): void;
    off(type: 'windowHighlightChange', callback?: Callback<boolean>): void;
    on(type: 'systemDensityChange', callback: Callback<number>): void;
    off(type: 'systemDensityChange', callback?: Callback<number>): void;
    on(type: 'displayIdChange', callback: Callback<number>): void;
    off(type: 'displayIdChange', callback?: Callback<number>): void;
  }
}

declare namespace uiObserver {
  class DensityInfo {
    context: UIContext;
    density: number;
  }
}

interface WindowEnv {
  findWindow(name: string): window.Window;
  getDisplayId(value: number): number;
  isIosPlatform?(): boolean;
}

declare class UIContext {
  getWindowName(): string;
  getUIObserver(): UIObserver;
  px2vp(value: number): number;
  getId(): number;
}

declare class UIObserver {
  on(type: 'densityUpdate', callback: Callback<uiObserver.DensityInfo>): void;
  off(type: 'densityUpdate', callback?: Callback<uiObserver.DensityInfo>): void;
}
