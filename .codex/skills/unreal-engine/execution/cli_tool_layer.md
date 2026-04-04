# CLI Tool Layer (辅助工具层)

> **定位**: `cli-anything-unrealengine` 是 **Tool (工具)**, 不是 Agent
> 
> 它由 SoloCoder 或执行 Agent 按需调用，无独立身份、无职责边界、无自主决策能力。

---

## 验证状态

| 项目 | 值 |
|------|-----|
| **评分** | ⭐4/5 (85%通过率) |
| **详细报告** | [references/cli_validation_report.md](../references/cli_validation_report.md) |
| **路径** | `D:\UE\UE_5.7\agent-harness` |
| **启动** | `Set-Location D:\UE\UE_5.7\agent-harness; cli-anything-unrealengine --json <command>` |

---

## CLI vs MCP 调用决策规则

| 场景 | 用 CLI | 用 MCP | 决策依据 |
|------|--------|--------|----------|
| **项目健康检查** (任务开始前) | ✅ `project info` / `stats code` | ❌ | Editor 未启动或仅需元数据 |
| **UMG 离线结构分析** (CodeReview 阶段) | ✅ `ui umg-read` (.uasset 解析) | ⚠️ `get_widget_tree`(需Editor在线) | 无需运行时数据, 快速静态扫描 |
| **插件/模块状态确认** | ✅ `plugin list` / `modules` | ❌ | 纯信息查询 |
| **资产盘点** | ✅ `asset list --type Blueprint` | ⚠️ `list_assets`(需Editor) | 批量统计场景 |
| **编译触发** | ✅ `build editor --config Development` | ❌ | CLI 独有能力 |
| **配置审计** | ✅ `config list` | ❌ | INI section 列表 |
| **Widget 创建/编辑** | ❌ | ✅ `create_widget` / `apply_layout` | 需要实时操作 Editor |
| **Actor 操作** | ❌ | ✅ `spawn_actor` / `set_transform` | 需要引擎运行时 |
| **材质节点编辑** | ❌ | ✅ `add_node` / `connect_nodes` | 需要编辑器交互 |
| **PIE 测试/截图** | ❌ | ✅ `play_in_editor` / `screenshot` | 需要引擎运行时 |

---

## 核心命令速查

| 命令 | 用途 | 典型触发时机 |
|------|------|-------------|
| `project info` | 项目元数据 (名称/引擎/模块/插件) | 任务开始前一次性检查 |
| `stats project/code/content/build` | 项目规模全景 (资产数/代码行/空间占用) | CodeReview 前置分析 |
| `ui umg-read <path>` | **离线 UMG .uasset 解析** (named_widgets/generated_classes/references) | CodeReview 离线审查 / 迁移前分析 |
| `plugin list` | 插件启用状态 | 确认 MCP 插件可用性 |
| `build editor --config Development` | 触发 Editor 目标编译 | C++ 代码变更后 |
| `asset list --type <Type>` | 按类型过滤资产列表 | 资产盘点 / 清理 |
| `config list` | INI 配置文件 section 列表 | 配置排查 |
| `clean ddc/binaries/intermediate` | 缓存清理 | 编译问题排查 (需关闭Editor) |

---

## Bootstrap (PowerShell)

```powershell
Set-Location D:\UE\UE_5.7\agent-harness
cli-anything-unrealengine --json project info "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

---

## 典型使用场景

### 场景 1: 任务前置健康检查

```powershell
# 检查项目状态
cli-anything-unrealengine --json project info "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 检查插件状态
cli-anything-unrealengine --json plugin list "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 检查代码规模
cli-anything-unrealengine --json stats code "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

### 场景 2: CodeReview 离线分析

```powershell
# 离线解析 UMG 结构
cli-anything-unrealengine --json ui umg-read "/Game/AUDI_Content/UI/Core/WBP_WidgetBase"

# 输出包含: named_widgets, generated_classes, references
```

### 场景 3: 资产盘点

```powershell
# 列出所有 Blueprint 资产
cli-anything-unrealengine --json asset list --type Blueprint "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 列出所有 Widget Blueprint
cli-anything-unrealengine --json asset list --type WidgetBlueprint "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

---

## Fallback 策略

如果 harness 不可用或命令覆盖不足:
1. 使用 Native UE CLI
2. 在 `Verification Evidence` 中声明回退原因

### Native UE CLI 参考

```powershell
# 生成项目文件
GenerateProjectFiles.bat -project=D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject

# 打包游戏
RunUAT.bat BuildCookRun -project=... -platform=Win64 -clientconfig=Shipping

# 启动 Editor
UnrealEditor.exe D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject
```

---

## 与 Agent 的关系

```
SoloCoder / 执行 Agent
        │
        ├── 需要离线分析? ──→ CLI Tool
        │                       └── 返回静态数据
        │
        └── 需要实时操作? ──→ MCP Agent
                                └── 返回执行结果
```

**关键原则**:
- CLI Tool 是被动工具，由 Agent 按需调用
- CLI Tool 不做决策，只返回数据
- CLI Tool 不替代 MCP，是补充方案

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
