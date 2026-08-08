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
const WMS_LAYOUT_DOMAIN = 0x04208;
const WMS_LAYOUT_TAG = 'WMSEnvSize';

@ObservedV2
class WindowSizePxEnv implements IEnvironmentValue<window.Size> {
  @Trace public width: number;
  @Trace public height: number;
  #win: window.Window;

  #sizeChangeCallback = (size: window.Size): void => {
    this.update(size);
  };

  constructor(context: UIContext) {
    const defaultSize: window.Size = {
      width: 0,
      height: 0
    };

    if (!context) {
      this.update(defaultSize);
      return;
    }

    try {
      this.#win = windowenv.findWindow(context.getWindowName());
      const props = this.#win.getWindowProperties();
      this.update({ width: props.windowRect.width, height: props.windowRect.height });
      this.#win.on('windowSizeChange', this.#sizeChangeCallback);
    } catch (error) {
      this.update(defaultSize);
      hilog.error(WMS_LAYOUT_DOMAIN, WMS_LAYOUT_TAG, `[env] px env constructor failed, ${error.message}`);
    }
  }

  get value(): window.Size {
    return {
      width: this.width,
      height: this.height,
    };
  }

  update(size: window.Size): void {
    this.width = size.width;
    this.height = size.height;
  }

  destroy(): void {
    try {
      if (!this.#win) {
        return;
      }
      this.#win.off('windowSizeChange', this.#sizeChangeCallback);
    } catch (error) {
      hilog.error(WMS_LAYOUT_DOMAIN, WMS_LAYOUT_TAG, `[env] failed to unregister window size change listener, ${error.message}`);
    }
  }
}

@ObservedV2
class WindowSizeVpEnv implements IEnvironmentValue<window.SizeInVp> {
  @Trace public width: number;
  @Trace public height: number;
  #win: window.Window;
  #context: UIContext;
  #widthPx = 0;
  #heightPx = 0;
  #isDensityUpdateRegistered = false;

  #updateVpSize(): void {
    if (!this.#context) {
      return;
    }
    this.update({
      widthVp: this.#context.px2vp(this.#widthPx),
      heightVp: this.#context.px2vp(this.#heightPx)
    });
  }

  #densityChangeCallback = (density: uiObserver.DensityInfo): void => {
    this.#updateVpSize();
  };

  #sizeChangeCallback = (size: window.Size): void => {
    this.#widthPx = size.width;
    this.#heightPx = size.height;
    this.#updateVpSize();
  };

  constructor(context: UIContext) {
    const defaultSizeInVp: window.SizeInVp = {
      widthVp: 0.0,
      heightVp: 0.0
    };

    if (!context) {
      this.update(defaultSizeInVp);
      return;
    }

    try {
      this.#context = context;
      this.#win = windowenv.findWindow(context.getWindowName());
      const props = this.#win.getWindowProperties();
      this.#widthPx = props.windowRect.width;
      this.#heightPx = props.windowRect.height;
      this.#updateVpSize();
      this.#win.on('windowSizeChange', this.#sizeChangeCallback);
      try {
        this.#context.getUIObserver().on('densityUpdate', this.#densityChangeCallback);
        this.#isDensityUpdateRegistered = true;
      } catch (error) {
        hilog.error(WMS_LAYOUT_DOMAIN, WMS_LAYOUT_TAG, `[env] vp env densityUpdate listener unavailable, ${error.message}`);
      }
    } catch (error) {
      this.update(defaultSizeInVp);
      hilog.error(WMS_LAYOUT_DOMAIN, WMS_LAYOUT_TAG, `[env] vp env constructor failed, ${error.message}`);
    }
  }

  get value(): window.SizeInVp {
    return {
      widthVp: this.width,
      heightVp: this.height,
    };
  }

  update(size: window.SizeInVp): void {
    this.width = size.widthVp;
    this.height = size.heightVp;
  }

  destroy(): void {
    try {
      if (!this.#win) {
        return;
      }
      this.#win.off('windowSizeChange', this.#sizeChangeCallback);
      if (this.#isDensityUpdateRegistered) {
        this.#context.getUIObserver().off('densityUpdate', this.#densityChangeCallback);
        this.#isDensityUpdateRegistered = false;
      }
    } catch (error) {
      hilog.error(WMS_LAYOUT_DOMAIN, WMS_LAYOUT_TAG, `[env] failed to unregister listener, ${error.message}`);
    }
  }
}

export default {
  WindowSizePxEnv,
  WindowSizeVpEnv,
};
