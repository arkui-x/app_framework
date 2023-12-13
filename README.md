# ArkUI-X应用框架适配层

## 简介

ArkUI-X 应用框架（AppFramework）适配层是在不同平台上模拟OpenHarmony应用框架的运行环境，将应用的生命周期以及开发模型适配到对应的系统平台上，确保应用的开发模式保持兼容。

应用框架适配层主要包含以下模块：
- AbilityRuntime：Ability基础运行环境，提供Stage模型的基础能力
- ArkUI接口层：ArkUI实例的启动对接
- Graphic2D：2D图形适配层
- ResourceManagement：资源管理
- WindowManager：窗口管理模块

## 目录介绍

目录结构如下图所示：

```
/foundation/appframework
├── ability                       # Ability基础运行环境
├── arkui                         # ArkUI相关接口
├── graphic_2d                    # 2D图形适配层
├── icu_data                      # ICU国际化数据配置
├── resource_management           # 资源管理模块
└── window_manager                # 窗口管理模块
```

## 框架开发指导

更多框架开发相关内容可参考：
- [框架开发者文档](https://gitee.com/arkui-x/docs/blob/master/zh-cn/framework-dev/README.md)