<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-03-30 | Updated: 2026-03-30 -->

# CarRenderFactory Module

## Purpose
汽车渲染工厂的主游戏模块。包含核心游戏逻辑、渲染系统和汽车相关的功能实现。

## Key Files
| File | Description |
|------|-------------|
| `CarRenderFactory.Build.cs` | 模块构建配置文件，定义模块依赖和编译设置 |
| `CarRenderFactory.h` | 模块头文件，声明模块类和导出宏 |
| `CarRenderFactory.cpp` | 模块实现文件，包含模块初始化代码 |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| (待创建) | 后续可创建: `Public/` (公开头文件), `Private/` (私有实现), `Classes/` (蓝图可访问类) |

## For AI Agents

### Working In This Directory
- 所有新类应该放在此模块下
- 公开 API 放在 `Public/` 或直接在模块根目录
- 私有实现放在 `Private/` 子目录
- 蓝图可访问的类放在 `Classes/` 目录

### Testing Requirements
- 新增类需要编译测试
- 确保模块能正确加载和卸载
- 测试编辑器和运行时两种环境

### Common Patterns
- 使用 `UCLASS()` 宏定义蓝图可访问的类
- 使用 `USTRUCT()` 定义可序列化的数据结构
- 使用 `UENUM()` 定义蓝图可用的枚举
- 使用 `UPROPERTY()` 暴露属性到编辑器

## Dependencies

### Internal
- 无内部依赖

### External
| Module | Purpose |
|--------|---------|
| `Core` | 引擎核心功能 (内存管理、容器、字符串等) |
| `CoreUObject` | UObject 系统 (反射、序列化、垃圾回收) |
| `Engine` | 引擎核心 (Actor、Component、World) |
| `InputCore` | 输入系统基础 |
| `EnhancedInput` | 增强输入系统 (UE5 新输入系统) |

## Architecture Notes

### 建议的目录结构
```
CarRenderFactory/
├── Public/                    # 公开头文件
│   ├── CarRenderFactory.h     # 模块头文件
│   ├── Core/                  # 核心系统
│   ├── Rendering/             # 渲染相关
│   └── Vehicles/              # 车辆相关
├── Private/                   # 私有实现
│   ├── CarRenderFactory.cpp   # 模块实现
│   ├── Core/
│   ├── Rendering/
│   └── Vehicles/
└── Classes/                   # 蓝图可访问的类
```

<!-- MANUAL: 模块特定说明可以添加在下方 -->
