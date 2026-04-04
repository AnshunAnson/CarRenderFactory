---
name: unreal-engine
description: Unreal Engine single-entry skill with Hardness (H/M/L) and subagent orchestration for discovery, implementation, and verification.
source:
  - https://github.com/DSTN2000/claude-unreal-engine-skill
  - https://github.com/sickn33/antigravity-awesome-skills/tree/main/skills/unreal-engine-cpp-pro
  - https://github.com/404kidwiz/claude-supercode-skills/tree/main/game-developer-skill
synced_at: 2026-03-31
---

# Unreal Engine Skill — 入口索引

> **版本**: 2.0 (原子化拆分版)
> **定位**: 本文件是 **导航索引** — 不包含实质规则定义，仅提供索引和查找
> **权威源映射**: 每个章节下方标注了完整内容的存放位置

---

## Purpose

Provide a single Unreal Engine skill entrypoint with deterministic Hardness governance, auditable delegation, and strict verification gates.

## When To Use

- Any request involving Unreal Engine, UE5, `.uproject`, C++ gameplay systems, Blueprint + C++ integration, Enhanced Input, GAS, replication, or plugin behavior.

---

## Non-Negotiables

- Zero assumptions: never invent asset names, module names, plugin states, or engine version.
- Discovery first, then implementation.
- Writer/reviewer separation for `M/H`.
- `M/H` requests are incomplete unless verification evidence is present.
- If reviewer separation is impossible, output `Status: BLOCKED` (never self-approve).
- Evaluate scriptability first; if a task is scriptable, prefer script/harness execution over manual repeated steps.
- Agent orchestration must strictly follow AgentHardness philosophy (intensity, delegation depth, and evidence depth must match risk/complexity).

---

## 核心框架索引

| 框架 | 内容 | 权威源位置 |
|------|------|-----------|
| **Hardness Framework** | L/M/H 级别定义 + 选择规则 + 合规规则 | [frameworks/hardness.md](./frameworks/hardness.md) |
| **Delegation Framework** | 委托门禁 + 动态团队生成 + Fallback 策略 | [frameworks/delegation.md](./frameworks/delegation.md) |
| **Scriptability Framework** | 脚本化决策规则 + 执行后端优先级 | [frameworks/scriptability.md](./frameworks/scriptability.md) |

---

## 执行层索引

| 模块 | 内容 | 权威源位置 |
|------|------|-----------|
| **Backend Priority** | MCP Dual-Agent 路由规则 + 后端选择决策 | [execution/backend_priority.md](./execution/backend_priority.md) |
| **CLI Tool Layer** | cli-anything-unrealengine 使用场景 + 命令速查 | [execution/cli_tool_layer.md](./execution/cli_tool_layer.md) |

---

## 契约索引

| 契约 | 内容 | 权威源位置 |
|------|------|-----------|
| **Output Contract** | 必需输出字段 + Status 定义 + 验证证据要求 | [contracts/output_contract.md](./contracts/output_contract.md) |

---

## 工作流索引

| 工作流 | 内容 | 权威源位置 |
|--------|------|-----------|
| **Execution Flow** | 标准执行流程 + Hardness 对应 + 工作示例 | [workflows/execution_flow.md](./workflows/execution_flow.md) |
| **Discovery** | 采样策略 + Discovery 命令集 + 输出格式 | [workflows/discovery.md](./workflows/discovery.md) |

---

## Playbooks (详细执行指南)

| Playbook | 用途 |
|----------|------|
| [playbooks/discovery.md](./playbooks/discovery.md) | 项目发现详细步骤 |
| [playbooks/cpp_architecture.md](./playbooks/cpp_architecture.md) | C++ 架构/性能处理 |
| [playbooks/multiplayer_and_net.md](./playbooks/multiplayer_and_net.md) | 多人/复制/GAS 处理 |
| [playbooks/mcp_agent_orchestration.md](./playbooks/mcp_agent_orchestration.md) | MCP Agent 编排 |
| [playbooks/validation.md](./playbooks/validation.md) | 验证流程 |

---

## References (按需加载)

| Reference | 内容 |
|-----------|------|
| [references/mcp_tools_mab.md](./references/mcp_tools_mab.md) | McpAutomationBridge 完整工具集 |
| [references/mcp_tools_umg.md](./references/mcp_tools_umg.md) | UmgMcp 完整工具集 |
| [references/mcp_tools_overlap.md](./references/mcp_tools_overlap.md) | MAB vs UMG 重叠分析 |
| [references/mcp_agent_system.md](./references/mcp_agent_system.md) | Dual-Agent 系统导航索引 |
| [references/mcp_extension_guide.md](./references/mcp_extension_guide.md) | Handler 扩展指南 |
| [references/enhanced_input.md](./references/enhanced_input.md) | Enhanced Input 系统 |
| [references/gameplay_ability_system.md](./references/gameplay_ability_system.md) | GAS 系统 |
| [references/common_pitfalls.md](./references/common_pitfalls.md) | 常见陷阱 |
| [references/cli_validation_report.md](./references/cli_validation_report.md) | CLI 验证报告 |

---

## Agent Prompt Files (子 Agent 初始化)

| Agent | 文件位置 |
|-------|---------|
| **ue-core-agent** | [.trae/agents/ue-core-agent.md](../../agents/ue-core-agent.md) |
| **umg-ui-agent** | [.trae/agents/umg-ui-agent.md](../../agents/umg-ui-agent.md) |
| **unreal-code-review** | [.trae/agents/unreal-code-review-agent.md](../../agents/unreal-code-review-agent.md) |
| **coordination-protocol** | [.trae/agents/coordination-protocol.md](../../agents/coordination-protocol.md) |

---

## 快速参考卡

### Hardness 选择

| 级别 | 触发条件 |
|------|---------|
| **H** | 跨模块重构 / 复制+GAS / 插件不确定 / 安全敏感 |
| **M** | 多文件变更 / 调试评审 / 无 H 触发条件 |
| **L** | 单命令 / 窄范围 / 低风险 Q&A |

### 后端选择

| 场景 | 后端 |
|------|------|
| Widget 创建/编辑 | MCP (umg-ui-agent) |
| Actor 操作 | MCP (ue-core-agent) |
| PIE 测试/截图 | MCP (ue-core-agent) |
| 项目健康检查 | CLI (cli-anything) |
| 离线 UMG 分析 | CLI (cli-anything) |

### 输出契约 (必需字段)

```
Hardness: L|M|H
Delegation Plan: [团队/角色/车道]
Scriptability: scriptable|non-scriptable + [path]
Execution Backend: mcp|cli-anything|native-cli
Verification Evidence: [通过/未验证项]
Status: DONE|BLOCKED
```

---

## 文档结构图

```
.trae/skills/unreal-engine/
├── SKILL.md                    # 本文件 (入口索引)
├── SOURCES.md                  # 来源声明
│
├── frameworks/                 # 核心框架
│   ├── hardness.md             # Hardness 分级规则
│   ├── delegation.md           # 委托与团队生成
│   └── scriptability.md        # 脚本化决策
│
├── execution/                  # 执行层
│   ├── backend_priority.md     # 后端优先级
│   └── cli_tool_layer.md       # CLI 工具层
│
├── contracts/                  # 契约定义
│   └── output_contract.md      # 输出契约
│
├── workflows/                  # 工作流
│   ├── execution_flow.md       # 执行流程
│   └── discovery.md            # 发现工作流
│
├── playbooks/                  # 详细执行指南
│   ├── discovery.md
│   ├── cpp_architecture.md
│   ├── multiplayer_and_net.md
│   ├── mcp_agent_orchestration.md
│   └── validation.md
│
└── references/                 # 参考文档 (按需加载)
    ├── mcp_tools_mab.md        # MAB 工具集
    ├── mcp_tools_umg.md        # UMG 工具集
    ├── mcp_tools_overlap.md    # 重叠分析
    ├── mcp_agent_system.md     # Agent 系统索引
    ├── mcp_extension_guide.md
    ├── enhanced_input.md
    ├── gameplay_ability_system.md
    ├── common_pitfalls.md
    └── cli_validation_report.md
```

---

*文档版本: 2.0 (原子化拆分版)*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
*定位: 入口索引 — 本文件是导航中心，所有实质内容均有唯一权威源*
