# Scriptability Framework (脚本化优先机制)

> **权威源**: 本文件定义脚本化决策规则和执行路径选择
> **版本**: 1.0

---

## 概述

在执行任务前，必须先分类工作流为 `scriptable` (可脚本化) 或 `non-scriptable` (不可脚本化)。

**核心原则**: 如果存在可脚本化路径，优先使用自动化后端，避免手动多步执行。

---

## 脚本化决策规则

### Scriptable (可脚本化)

**特征**:
- 可重复的 CLI/构建/打包/配置/会话/日志解析步骤
- 确定性的输入/输出

**示例**:
- 项目构建: `uec build` / `RunUAT BuildCookRun`
- 资产导入: `cli-anything asset import`
- 配置修改: INI 文件编辑
- 日志分析: 正则表达式匹配
- 批量资产操作: 列表遍历 + 统一处理

### Non-Scriptable (不可脚本化)

**特征**:
- 探索性设计决策
- 无法在当前任务窗口中可靠编码为脚本

**示例**:
- UI/UX 设计决策
- 架构权衡分析
- 创意方向选择
- 需要人工判断的边界情况

---

## 执行后端优先级

当任务被判定为 `scriptable` 时，按以下优先级选择执行后端:

```
1. MCP Dual-Agent System (AudiR7S 项目首选)
   ├── ue-core-agent (McpAutomationBridge) — 引擎级操作
   └── umg-ui-agent (UmgMcp) — UMG Widget/UI 操作
   → 详见: execution/backend_priority.md

2. Unreal-engine MCP (通用 MCP 单后端)

3. cli-anything-unrealengine (离线/批量操作)
   → 详见: execution/cli_tool_layer.md

4. Native UE CLI (最终回退)
   ├── Build.bat
   ├── RunUAT.bat
   └── UnrealEditor-Cmd.exe
```

---

## 决策流程

```
任务请求
│
├── Step 1: 评估 Scriptability
│   ├── 可重复 CLI 步骤? → scriptable
│   ├── 确定性输入/输出? → scriptable
│   └── 探索性设计决策? → non-scriptable
│
├── Step 2: 选择执行路径
│   ├── scriptable → 按后端优先级选择
│   └── non-scriptable → 主 Agent 手动处理
│
└── Step 3: 记录选择理由
    └── 在输出中声明: Scriptability: scriptable|non-scriptable + chosen path
```

---

## 后端选择决策矩阵

| 场景 | 用 MCP | 用 CLI | 用 Native CLI | 决策依据 |
|------|--------|--------|--------------|----------|
| Widget 创建/编辑 | ✅ | ❌ | ❌ | 需要实时操作 Editor |
| Actor 操作 | ✅ | ❌ | ❌ | 需要引擎运行时 |
| 材质节点编辑 | ✅ | ❌ | ❌ | 需要编辑器交互 |
| PIE 测试/截图 | ✅ | ❌ | ❌ | 需要引擎运行时 |
| 项目健康检查 | ❌ | ✅ | ❌ | Editor 未启动或仅需元数据 |
| UMG 离线结构分析 | ⚠️ | ✅ | ❌ | 无需运行时数据，快速静态扫描 |
| 插件/模块状态确认 | ❌ | ✅ | ❌ | 纯信息查询 |
| 资产盘点 | ⚠️ | ✅ | ❌ | 批量统计场景 |
| 编译触发 | ❌ | ✅ | ⚠️ | CLI 独有能力 |
| 配置审计 | ❌ | ✅ | ❌ | INI section 列表 |
| 高级打包 | ❌ | ❌ | ✅ | RunUAT 独有能力 |

---

## 输出要求

对于每个非平凡 UE 请求，输出必须包含:

```
Scriptability: scriptable|non-scriptable
Execution Backend: mcp|cli-anything|native-cli (+ fallback reason if not mcp)
```

### 示例输出

**Scriptable 任务**:
```
Scriptability: scriptable
Execution Backend: mcp (ue-core-agent)
Rationale: Actor spawn 操作需要实时 Editor 交互
```

**Non-Scriptable 任务**:
```
Scriptability: non-scriptable
Execution Backend: manual (主 Agent 处理)
Rationale: UI 布局设计需要探索性决策
```

---

## 文档触发机制

当以下情况发生时，必须触发 `document-specialist`:
- API/插件行为对版本敏感
- 本地代码与文档冲突
- 行为在本地代码/注释中未记录且影响正确性

**必需引用载荷**:
- 官方文档链接
- 文档版本/日期 (如可用)
- 提取的规则
- 对当前决策的影响

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
