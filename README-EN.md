# ArkUI-X Application Framework Adaptation Layer

## Introduction

The ArkUI-X Application Framework (AppFramework) adaptation layer simulates the OpenHarmony app framework's runtime environment across different platforms. It adapts the application lifecycle and development model to specific system platforms, ensuring compatibility in development paradigms while maintaining consistent application behavior.

The application framework adaptation layer primarily includes these core modules:

- AbilityRuntime：The basic runtime environment for Ability, providing the basic capabilities of the Stage model.
- ArkUI Interface Layer: Interface for starting ArkUI instances.
- Graphic2D：2D Graphics Adaptation Layer
- ResourceManagement：Resource Management
- WindowManager：Window Management Module

## Directory Introduction

The directory structure is shown in the figure below:

```
/foundation/appframework
├── ability                       # Basic Runtime Environment for Ability
├── arkui                         # ArkUI-related interfaces
├── graphic_2d                    # 2D Graphics Adaptation Layer
├── icu_data                      # ICU Internationalization Data Configuration
├── resource_management           # Resource Management Module
└── window_manager                # Window Management Module
```

## Framework development guidance

For more framework development related content, please refer to:

- [Framework Developer Documentation](https://gitcode.com/arkui-x/docs/blob/master/en/framework-dev/README.md)
