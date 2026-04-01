<!-- Generated: 2026-03-30 | Updated: 2026-03-30 -->

# CarRenderFactory Source

## Purpose
汽车渲染工厂项目的源代码根目录。这是一个基于 Unreal Engine 5.7 的项目，专注于汽车渲染和可视化功能开发。

## Key Files
| File | Description |
|------|-------------|
| `CarRenderFactory.Target.cs` | 游戏运行时构建目标配置，定义打包和运行时的编译设置 |
| `CarRenderFactoryEditor.Target.cs` | 编辑器构建目标配置，定义编辑器环境的编译设置 |

## Subdirectories
| Directory | Purpose |
|-----------|---------|
| `CarRenderFactory/` | 主游戏模块，包含核心游戏逻辑和功能实现 (见 `CarRenderFactory/AGENTS.md`) |

## For AI Agents

### Working In This Directory
- 这是 UE5 项目的 Source 目录，所有 C++ 模块都放在这里
- 新建模块需要在此目录下创建新的子文件夹
- 每个模块需要包含 `.Build.cs` 文件来定义依赖关系

### Testing Requirements
- 编译前确保 UE5.7 引擎已正确安装
- 使用 Rider for Unreal 或 Visual Studio 进行编译
- 运行 `UBT` (Unreal Build Tool) 进行构建验证

### Common Patterns
- 模块命名遵循 PascalCase 规范
- 每个模块需要 `ModuleName.h` 和 `ModuleName.cpp` 入口文件
- `.Target.cs` 文件定义构建目标类型和引擎版本

## Dependencies

### Internal
- `CarRenderFactory/` - 主游戏模块

### External
- Unreal Engine 5.7 - 游戏引擎
- UnrealBuildTool - 构建系统

<!-- MANUAL: 项目特定说明可以添加在下方 -->
