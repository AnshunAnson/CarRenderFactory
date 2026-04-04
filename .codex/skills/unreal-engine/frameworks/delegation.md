# Delegation Framework (委托与团队生成)

> **权威源**: 本文件定义委托规则和动态团队生成机制
> **版本**: 1.0

---

## 委托门禁 (Delegation Gate)

### 触发委托的场景
- 多文件变更
- 重构
- 调试
- 评审
- 验证
- 研究

### 允许直接处理的场景
- 简单操作
- 短澄清
- 单命令查找

---

## 动态 Agent 团队生成

### 团队构建输入
1. Hardness 级别 (`L/M/H`)
2. 范围大小 (文件/模块/子系统)
3. 风险类型 (复制/GAS/插件不确定性)
4. 所需证据深度 (快速/标准/深度)

### 基线团队形态

#### L 级团队
```
primary agent only
└── optional: document-specialist (API 不确定时)
```

#### M 级团队
```
Phase 1: explore + executor (可并行)
Phase 2: verifier (独立于执行者)
```

#### H 级团队
```
planner -> architect -> executor (至少2条并行车道) -> verifier -> code-reviewer
```

**H 级并行车道示例**:
- 车道 A: 复制逻辑
- 车道 B: GAS 集成

---

## Fallback 策略

### 原则
1. 保持角色覆盖优先
2. 模型选择次之

### 替换规则
- 如果某角色不可用，替换为兼容车道以保持职责分离
- 如果写入者/评审者分离无法保持，返回 `Status: BLOCKED`

---

## 角色定义

| 角色 | 职责 | 适用 Hardness |
|------|------|--------------|
| **planner** | 任务分解、风险评估、团队规划 | H |
| **architect** | 架构设计、技术决策 | H |
| **executor** | 实现代码/资产变更 | M, H |
| **verifier** | 验证实现正确性 | M, H |
| **code-reviewer** | 代码质量评审 | H |
| **explore** | 代码/资产/配置发现 | M, H |
| **document-specialist** | 官方文档查询 | L, M, H (按需) |

---

## 委托决策树

```
任务请求
│
├── Hardness = L?
│   ├── 是 → 主 Agent 直接处理
│   └── 否 → 继续
│
├── Hardness = M?
│   ├── 是 → 启动 explore + executor (并行)
│   │         └── 完成后启动 verifier
│   └── 否 → 继续 (H)
│
└── Hardness = H
    ├── 启动 planner (任务分解)
    ├── 启动 architect (架构设计)
    ├── 启动 executor (至少2条并行车道)
    ├── 启动 verifier (独立验证)
    └── 启动 code-reviewer (最终评审)
```

---

## 团队生成示例

### 示例 1: M 级任务 - 多文件功能修复

```
输入:
- Hardness: M
- 范围: 3个 C++ 文件 + 1个 BP
- 风险: 无复制/GAS
- 证据深度: 标准

生成团队:
Phase 1 (并行):
  - Lane A: explore (发现相关代码/资产)
  - Lane B: executor (起草变更计划)
Phase 2:
  - verifier (验证变更正确性)
```

### 示例 2: H 级任务 - 复制 + GAS 关键路径

```
输入:
- Hardness: H
- 范围: 跨模块 (Gameplay + Network + UI)
- 风险: 复制/GAS/网络权威
- 证据深度: 深度

生成团队:
Phase 1:
  - planner (任务分解 + 风险评估)
Phase 2:
  - architect (架构设计 + 技术决策)
Phase 3 (并行):
  - Lane A: executor (复制逻辑实现)
  - Lane B: executor (GAS 集成实现)
Phase 4:
  - verifier (客户端/服务器验证)
Phase 5:
  - code-reviewer (最终代码评审)
```

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
