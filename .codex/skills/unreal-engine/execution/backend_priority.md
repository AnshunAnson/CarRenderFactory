# Execution Backend Priority (执行后端优先级)

> **权威源**: 本文件定义执行后端选择规则
> **版本**: 1.0

---

## 后端优先级顺序

对于可执行的 UE 自动化路径，使用以下后端顺序:

```
1. MCP Dual-Agent System (AudiR7S 项目首选)
   ├── ue-core-agent (McpAutomationBridge) — 引擎级操作
   └── umg-ui-agent (UmgMcp) — UMG Widget/UI 操作

2. Unreal-engine MCP (通用 MCP 单后端)

3. cli-anything-unrealengine (离线/批量操作)
   → 详见: cli_tool_layer.md

4. Native UE CLI (最终回退)
   ├── Build.bat
   ├── RunUAT.bat
   └── UnrealEditor-Cmd.exe
```

---

## MCP Dual-Agent 路由规则 (AudiR7S 项目)

当两个 MCP Agent 都可用时，按领域路由任务:

### 领域路由矩阵

| 任务领域 | 主力 Agent | 回退 | 核心工具 |
|---------|-----------|------|---------|
| UMG Widget CRUD / Layout / UI Animation | `umg-ui-agent` | `ue-core-agent` (有限) | `create_widget`, `apply_layout`, `set_animation_data` |
| Skeletal Animation / AB / BS / StateMachine | `ue-core-agent` | 无 | `create_animation_bp`, `create_state_machine` |
| Actor spawn/control/transform | `ue-core-agent` | 无 | `control_actor_spawn`, `set_transform` |
| Material pipeline (PBR/MF/MI/Landscape) | `ue-core-agent` | `umg-ui-agent` (仅 UI 材质) | `create_material`, `set_shading_model`, `MI params` |
| Editor control (PIE/screenshot/input) | `ue-core-agent` | 无 | `play_in_editor`, `screenshot`, `simulate_input` |
| BP SCS component tree | `ue-core-agent` | 无 | `blueprint_modify_scs` |
| BP graph editing (nodes/variables) | 视上下文 | 任一 | 见下方重叠规则 |
| Asset management (list/create/delete) | `ue-core-agent` | `umg-ui-agent` (仅 list) | `asset_action`, `list_assets` |

### BP Graph 编辑重叠解决

```
BP 编辑请求
├── 目标是 Widget Blueprint? (WBP_ 前缀 / 继承 UUserWidget)
│   ├── 是 → umg-ui-agent (更熟悉 Widget 事件绑定、UI 专用函数)
│   └── 否 → ue-core-agent (通用 BP)
├── 包含 SCS 组件操作?
│   ├── 是 → ue-core-agent (唯一拥有 SCS 能力)
│   └── 否 → 按上述规则
└── 仅编译操作?
    └── 任一 Agent 均可执行 (API 一致)
```

---

## 完整路由文档引用

| 文档 | 内容 |
|------|------|
| [references/mcp_agent_system.md](../references/mcp_agent_system.md) | 完整路由矩阵 + 协作协议 |
| [playbooks/mcp_agent_orchestration.md](../playbooks/mcp_agent_orchestration.md) | Agent 编排 Playbook |
| [.trae/agents/coordination-protocol.md](../../../agents/coordination-protocol.md) | 跨 Agent 协调契约 |

---

## 后端选择决策流程

```
任务请求
│
├── Step 1: 检查 MCP 可用性
│   ├── UE Editor 已启动? → 继续
│   └── 否 → 回退到 CLI 或 Native CLI
│
├── Step 2: 确定任务领域
│   ├── UI/Widget 相关? → umg-ui-agent
│   ├── 引擎/Actor/材质? → ue-core-agent
│   └── 混合任务? → 按协调协议协作
│
├── Step 3: 执行任务
│   └── 调用对应 MCP 工具
│
└── Step 4: 验证结果
    └── 返回执行状态
```

---

## 回退策略

### MCP 不可用时

| 场景 | 回退方案 |
|------|---------|
| Editor 未启动 | 使用 `cli-anything-unrealengine` 离线分析 |
| 特定插件未启用 | 使用 Native UE CLI 或提示用户启用插件 |
| MCP 工具不支持 | 使用 Native UE CLI |

### 输出声明

当不使用 MCP 时，必须在输出中声明回退原因:

```
Execution Backend: cli-anything
Fallback Reason: UE Editor 未启动，使用离线分析
```

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
