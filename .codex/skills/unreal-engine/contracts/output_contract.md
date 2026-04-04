# Output Contract (输出契约)

> **权威源**: 本文件定义所有 UE 任务的必需输出格式
> **版本**: 1.0

---

## 概述

对于每个非平凡的 Unreal 请求，输出必须包含以下字段。缺少任何字段意味着任务不完整 (M/H 级别)。

---

## 必需字段 (所有 Hardness)

| 字段 | 类型 | 说明 |
|------|------|------|
| `Hardness` | `L\|M\|H` | 任务复杂度/风险级别 |
| `Delegation Plan` | string | Agent 团队、角色、车道所有权 |
| `Scriptability` | `scriptable\|non-scriptable` | + chosen execution path |
| `Execution Backend` | `mcp\|cli-anything\|native-cli` | (+ fallback reason if not mcp) |
| `Verification Evidence` | string | 通过了什么，还有什么未验证 |
| `Status` | `DONE\|BLOCKED` | 任务最终状态 |

---

## M/H 级别额外必需字段

| 字段 | 类型 | 说明 |
|------|------|------|
| `Review Agent Findings` | string | 边界清晰度、可读性、效率 |
| `Context Experience` | string | 从本任务中提取的可复用经验 |
| `Skill Optimization Proposals` | string | 改进本 skill 的具体建议 |

---

## 输出模板

### L 级别输出模板

```
Hardness: L
Delegation Plan: none
Scriptability: scriptable|non-scriptable + [chosen path]
Execution Backend: mcp|cli-anything|native-cli
Verification Evidence: [assumption stated, no high-risk path touched]
Status: DONE
```

### M 级别输出模板

```
Hardness: M
Delegation Plan: Team A(explore) + Team B(executor) -> Team C(verifier)
Scriptability: scriptable|non-scriptable + [chosen path]
Execution Backend: mcp|cli-anything|native-cli
Verification Evidence: PIE + standalone checks, residual risk listed
Review Agent Findings: [boundary clarity, readability, efficiency]
Context Experience: [reusable lessons from this task]
Status: DONE
```

### H 级别输出模板

```
Hardness: H
Delegation Plan: planner->architect->executor with parallel [lane A] + [lane B] -> verifier -> code-reviewer
Scriptability: scriptable|non-scriptable + [chosen path]
Execution Backend: mcp|cli-anything|native-cli
Verification Evidence: client/server checks, replication/GAS assertions, unresolved list if any
Review Agent Findings: [boundary clarity, readability, efficiency]
Context Experience: [reusable lessons from this task]
Skill Optimization Proposals: [concrete updates to improve this skill]
Status: DONE|BLOCKED
```

---

## Status 定义

| Status | 含义 | 后续行动 |
|--------|------|---------|
| `DONE` | 任务完成，所有验证通过 | 交付给用户 |
| `BLOCKED` | 任务受阻，无法继续 | 返回未解决风险 + 下一步负责人/行动 |

### BLOCKED 触发条件

- H 级别修复循环超过 3 次尝试或 45 分钟
- 评审车道不可用
- 风险无法在当前 Hardness 下界定
- 关键检查仍然失败
- 重复相同失败签名
- 验证证据缺失两次

---

## 验证证据要求

### L 级别
- 假设已声明
- 无高风险路径被触及

### M 级别
- PIE 测试通过
- 独立检查完成
- 残余风险已列出

### H 级别
- 客户端/服务器检查通过
- 复制/GAS 断言通过
- 未解决列表 (如有)

---

## 示例输出

### 示例 1: L 级别 - 单命令查找

```
Hardness: L
Delegation Plan: none
Scriptability: scriptable + mcp
Execution Backend: mcp (ue-core-agent)
Verification Evidence: assumption stated, no high-risk path touched
Status: DONE
```

### 示例 2: M 级别 - 多文件功能修复

```
Hardness: M
Delegation Plan: Team A(explore) + Team B(executor) -> Team C(verifier)
Scriptability: scriptable + mcp
Execution Backend: mcp (ue-core-agent)
Verification Evidence: PIE + standalone checks passed, no residual risk
Review Agent Findings: Boundary clear, code readable, efficient implementation
Context Experience: When modifying Widget base class, always check BP subclasses first
Status: DONE
```

### 示例 3: H 级别 - 复制 + GAS 关键路径

```
Hardness: H
Delegation Plan: planner->architect->executor with parallel replication lane + GAS lane -> verifier -> code-reviewer
Scriptability: scriptable + mcp
Execution Backend: mcp (ue-core-agent + umg-ui-agent collaboration)
Verification Evidence: client/server checks passed, replication/GAS assertions passed
Review Agent Findings: Clear separation of concerns, well-documented replication logic
Context Experience: GAS ability activation must check net ownership before applying effects
Skill Optimization Proposals: Add explicit net ownership check step to GAS playbook
Status: DONE
```

### 示例 4: H 级别 - BLOCKED

```
Hardness: H
Delegation Plan: planner->architect->executor -> verifier [FAILED] -> code-reviewer [UNAVAILABLE]
Scriptability: scriptable + mcp
Execution Backend: mcp (ue-core-agent)
Verification Evidence: Critical check failing after 3 attempts: replication state not syncing to clients
Review Agent Findings: Implementation correct but verification lane unavailable for independent review
Context Experience: Writer/reviewer separation is critical for H-level tasks
Skill Optimization Proposals: Add fallback reviewer assignment mechanism
Status: BLOCKED

Unresolved Risks:
- Replication state not syncing to clients
- Root cause: possibly network layer configuration

Next Owner/Action:
- Owner: SoloCoder
- Action: Investigate network configuration or escalate to network specialist
```

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
