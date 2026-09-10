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

const windowenv = requireInternal('windowenv') as WindowEnv;
const hilog = requireInternal('hilog') as Hilog;
const HILOG_DOMAIN = 0x04209;
const HILOG_TAG = 'WMSEnvAvoidArea';

function isIosPlatform(): boolean {
  try {
    return typeof windowenv.isIosPlatform === 'function' && windowenv.isIosPlatform();
  } catch (error) {
    hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] get platform failed, ${error.message}`);
    return false;
  }
}

const IS_IOS_PLATFORM = isIosPlatform();

interface UIEnvAvoidArea {
  statusBar: window.AvoidArea;
  cutout: window.AvoidArea;
  keyboard: window.AvoidArea;
  navigationIndicator: window.AvoidArea;
}

enum AvoidAreaType {
  TYPE_SYSTEM = 0,
  TYPE_CUTOUT = 1,
  TYPE_SYSTEM_GESTURE = 2,
  TYPE_KEYBOARD = 3,
  TYPE_NAVIGATION_INDICATOR = 4,
}

function createEmptyRect(): window.Rect {
  return { left: 0, top: 0, width: 0, height: 0 };
}

function createEmptyAvoidArea(): window.AvoidArea {
  return {
    visible: false,
    leftRect: createEmptyRect(),
    topRect: createEmptyRect(),
    rightRect: createEmptyRect(),
    bottomRect: createEmptyRect(),
  };
}

function cloneRect(rect: window.Rect): window.Rect {
  return { left: rect.left, top: rect.top, width: rect.width, height: rect.height };
}

function cloneAvoidArea(area: window.AvoidArea): window.AvoidArea {
  return {
    visible: area.visible,
    leftRect: cloneRect(area.leftRect),
    topRect: cloneRect(area.topRect),
    rightRect: cloneRect(area.rightRect),
    bottomRect: cloneRect(area.bottomRect),
  };
}

function normalizeAvoidAreaForEnv(type: AvoidAreaType, area: window.AvoidArea): window.AvoidArea {
  if (!IS_IOS_PLATFORM ||
    (type !== AvoidAreaType.TYPE_SYSTEM && type !== AvoidAreaType.TYPE_NAVIGATION_INDICATOR)) {
    return area;
  }

  const normalizedArea = cloneAvoidArea(area);
  normalizedArea.bottomRect = createEmptyRect();
  return normalizedArea;
}

function getWindowAvoidAreaForEnv(win: window.Window, type: AvoidAreaType): window.AvoidArea {
  return normalizeAvoidAreaForEnv(type, win.getWindowAvoidArea(type));
}

@ObservedV2
class WindowAvoidAreaPxEnv implements IEnvironmentValue<UIEnvAvoidArea> {
  @Trace statusBar: window.AvoidArea = createEmptyAvoidArea();
  @Trace cutout: window.AvoidArea = createEmptyAvoidArea();
  @Trace keyboard: window.AvoidArea = createEmptyAvoidArea();
  @Trace navigationIndicator: window.AvoidArea = createEmptyAvoidArea();

  #win: window.Window;

  get value() {
    return this;
  }

  constructor(context: UIContext) {
    try {
      this.#win = windowenv.findWindow(context.getWindowName());
      this.statusBar = getWindowAvoidAreaForEnv(this.#win, AvoidAreaType.TYPE_SYSTEM);
      this.cutout = getWindowAvoidAreaForEnv(this.#win, AvoidAreaType.TYPE_CUTOUT);
      this.keyboard = getWindowAvoidAreaForEnv(this.#win, AvoidAreaType.TYPE_KEYBOARD);
      this.navigationIndicator = getWindowAvoidAreaForEnv(this.#win, AvoidAreaType.TYPE_NAVIGATION_INDICATOR);
      this.#win.on('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] px env constructor failed, ${error.message}`);
    }
  }

  #avoidAreaChangeCallback = ({ type, area }: window.AvoidAreaOptions) => {
    const normalizedArea = normalizeAvoidAreaForEnv(type, area);
    switch (type) {
      case AvoidAreaType.TYPE_SYSTEM:
        this.statusBar = normalizedArea;
        break;
      case AvoidAreaType.TYPE_CUTOUT:
        this.cutout = normalizedArea;
        break;
      case AvoidAreaType.TYPE_KEYBOARD:
        this.keyboard = normalizedArea;
        break;
      case AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
        this.navigationIndicator = normalizedArea;
        break;
    }
  };

  update() {}

  destroy(): void {
    try {
      if (!this.#win) {
        return;
      }
      this.#win.off('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] px env destroy failed, ${error.message}`);
    }
  }
}

@ObservedV2
class WindowAvoidAreaVpEnv implements IEnvironmentValue<UIEnvAvoidArea> {
  @Trace statusBar: window.AvoidArea = createEmptyAvoidArea();
  @Trace cutout: window.AvoidArea = createEmptyAvoidArea();
  @Trace keyboard: window.AvoidArea = createEmptyAvoidArea();
  @Trace navigationIndicator: window.AvoidArea = createEmptyAvoidArea();

  #win: window.Window;
  #context: UIContext;
  #areaInPx: Map<AvoidAreaType, window.AvoidArea> = new Map();
  #isDensityUpdateRegistered = false;

  get value() {
    return this;
  }

  constructor(context: UIContext) {
    try {
      this.#context = context;
      this.#win = windowenv.findWindow(context.getWindowName());
      for (const type of [AvoidAreaType.TYPE_SYSTEM, AvoidAreaType.TYPE_CUTOUT, AvoidAreaType.TYPE_KEYBOARD, AvoidAreaType.TYPE_NAVIGATION_INDICATOR]) {
        this.#areaInPx.set(type, getWindowAvoidAreaForEnv(this.#win, type));
      }
      this.update();
      this.#win.on('avoidAreaChange', this.#avoidAreaChangeCallback);
      try {
        this.#context.getUIObserver().on('densityUpdate', this.#densityUpdateCallback);
        this.#isDensityUpdateRegistered = true;
      } catch (error) {
        hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] vp env densityUpdate listener unavailable, ${error.message}`);
      }
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] vp env constructor failed, ${error.message}`);
    }
  }

  #translateRectToVp(rect: window.Rect): window.Rect {
    const vpRect = createEmptyRect();
    vpRect.left = this.#context?.px2vp(rect.left) ?? rect.left;
    vpRect.top = this.#context?.px2vp(rect.top) ?? rect.top;
    vpRect.width = this.#context?.px2vp(rect.width) ?? rect.width;
    vpRect.height = this.#context?.px2vp(rect.height) ?? rect.height;
    return vpRect;
  }

  #translateAvoidAreaToVp(area: window.AvoidArea): window.AvoidArea {
    const vpArea = createEmptyAvoidArea();
    vpArea.visible = area.visible;
    vpArea.leftRect = this.#translateRectToVp(area.leftRect);
    vpArea.topRect = this.#translateRectToVp(area.topRect);
    vpArea.rightRect = this.#translateRectToVp(area.rightRect);
    vpArea.bottomRect = this.#translateRectToVp(area.bottomRect);
    return vpArea;
  }

  #avoidAreaChangeCallback = ({ type, area }: window.AvoidAreaOptions) => {
    const normalizedArea = normalizeAvoidAreaForEnv(type, area);
    this.#areaInPx.set(type, normalizedArea);
    switch (type) {
      case AvoidAreaType.TYPE_SYSTEM:
        this.statusBar = this.#translateAvoidAreaToVp(normalizedArea);
        break;
      case AvoidAreaType.TYPE_CUTOUT:
        this.cutout = this.#translateAvoidAreaToVp(normalizedArea);
        break;
      case AvoidAreaType.TYPE_KEYBOARD:
        this.keyboard = this.#translateAvoidAreaToVp(normalizedArea);
        break;
      case AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
        this.navigationIndicator = this.#translateAvoidAreaToVp(normalizedArea);
        break;
    }
  };

  #densityUpdateCallback = () => {
    this.update();
  };

  update() {
    if (!this.#context) {
      return;
    }

    if (this.#areaInPx.has(AvoidAreaType.TYPE_SYSTEM)) {
      this.statusBar = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_SYSTEM));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_CUTOUT)) {
      this.cutout = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_CUTOUT));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_KEYBOARD)) {
      this.keyboard = this.#translateAvoidAreaToVp(this.#areaInPx.get(AvoidAreaType.TYPE_KEYBOARD));
    }
    if (this.#areaInPx.has(AvoidAreaType.TYPE_NAVIGATION_INDICATOR)) {
      this.navigationIndicator = this.#translateAvoidAreaToVp(
        this.#areaInPx.get(AvoidAreaType.TYPE_NAVIGATION_INDICATOR)
      );
    }
  }

  destroy(): void {
    try {
      this.#areaInPx.clear();
      if (this.#isDensityUpdateRegistered) {
        this.#context.getUIObserver().off('densityUpdate', this.#densityUpdateCallback);
        this.#isDensityUpdateRegistered = false;
      }
      if (!this.#win) {
        return;
      }
      this.#win.off('avoidAreaChange', this.#avoidAreaChangeCallback);
    } catch (error) {
      hilog.error(HILOG_DOMAIN, HILOG_TAG, `[env] vp env destroy failed, ${error.message}`);
    }
  }
}

export default {
  WindowAvoidAreaPxEnv,
  WindowAvoidAreaVpEnv
};
