# MCP Dual-Agent System — 导航索引

> **版本**: 1.1 (同步于 coordination-protocol v1.1)
> **定位**: 本文件是 **快速导航卡** — 不包含实质规则定义, 仅提供索引和查找
> **权威源映射**: 每个章节下方标注了完整内容的存放位置

---

## 架构总览 (简化版)

```
SoloCoder (编排+验收)
    ├── 分派 ──→ ue-core-agent (McpAutomationBridge / WebSocket)
    │               "引擎的双手" — Actor/材质/骨骼动画/SCS/PIE
    │
    └── 分派 ──→ umg-ui-agent (UmgMcp / TCP Socket→stdio)
                    "UI 的设计师" — Widget/布局/动画/UI材质/BP事件

完成任务 → UnrealCodeReview (4维评审, 强制) → SoloCoder (PASS/FAIL验收)

辅助工具: cli-anything-unrealengine (Tool Layer, 非 Agent) — 见 SKILL.md
```

**完整架构图 (含 Phase R/A 质量门禁)**: → [coordination-protocol.md](../../agents/coordination-protocol.md#L9-L104)

---

## Agent 快速身份卡片

| | **ue-core-agent** | **umg-ui-agent** |
|---|---|---|
| **MCP 服务** | `unreal-engine` (npx) | `umg-mcp` (uv run) |
| **底层插件** | McpAutomationBridge (C++/WS) | UmgMcp (C++/TCP:55557) |
| **角色** | 引擎级操作专家 | UMG Widget 设计专家 |
| **哲学** | "引擎的双手" | "UI 的设计师" |
| **独占能力** | PIE/Screenshot/Actor全生命周期/SCS/骨骼动画/材质MF·MI | apply_layout(HTML→UMG)/Widget Sequencer动画/布局分析/重叠检测 |
| **完整提示词** | [.trae/agents/ue-core-agent.md](../../agents/ue-core-agent.md) | [.trae/agents/umg-ui-agent.md](../../agents/umg-ui-agent.md) |

---

## 职责边界速查

### ue-core-agent 独占 (不可越界)
PIE控制 / Screenshot / 键鼠模拟 / Skeletal Animation全套 / SCS组件树 / 材质MF·MI·Domain属性

### umg-ui-agent 独占 (不可越界)
apply_layout HTML→UMG批量转换 / Widget Sequencer动画全套 / 屏幕空间布局分析 / 视觉重叠检测 / UMG→JSON反向导出

### 共享区 (可协作, 有路由优先级)
BP图形编辑 / Material节点图操作 / Asset列表查询 / Blueprint编译

**完整硬边界定义 + 路由决策树**: → [coordination-protocol.md 任务路由矩阵](../../agents/coordination-protocol.md#L108-L139)

---

## 协作模式索引

| 模式 | 适用场景 | 完整指南 |
|------|---------|---------|
| **A: 顺序流水线** | 先引擎准备→再UI搭建→最后测试 | [coordination-protocol.md Flow A抽象](../../agents/coordination-protocol.md#L145-L187) + [Playbook 1](../playbooks/mcp_agent_orchestration.md#L19-L100) |
| **B: 并行执行** | 材质系统和UI面板可同时开发 | [coordination-protocol.md Flow C抽象](../../agents/coordination-protocol.md#L210-L242) + [Playbook 3](../playbooks/mcp_agent_orchestration.md) |
| **C: 回调绑定** | UI按钮事件→引擎Actor响应 | [coordination-protocol.md 回调约定](../../agents/coordination-protocol.md#L270-L277) |

---

## 关键规范索引

| 规范 | 内容 | 权威源位置 |
|------|------|-----------|
| **通信协议格式** | Agent间上下文传递 JSON + 回调事件表 | [coordination-protocol.md §通信协议](../../agents/coordination-protocol.md#L246-L277) |
| **冲突解决** | 资源锁/功能争抢/降级策略 | [coordination-protocol.md §冲突解决](../../agents/coordination-protocol.md#L281-L318) |
| **质量检查清单** | umg-ui 8项 + ue-core 7项 验收标准 | [coordination-protocol.md §质量清单](../../agents/coordination-protocol.md#L322-L343) |
| **CodeReview 评审** | Phase R: 4维度评分(30/30/25/15) + Verdict规则 | [coordination-protocol.md §UnrealCodeReview](../../agents/coordination-protocol.md#L347-L471) |
| **SoloCoder 验收** | Phase A: 决策矩阵(PASS≥0.85) + 返工A/B/C/D (N≤3) | [coordination-protocol.md §SoloCoder验收](../../agents/coordination-protocol.md#L475-L594) |
| **CLI Tool Layer** | cli-anything-unrealengine 使用场景 + 命令速查 | [SKILL.md §CLI Tool Layer](../SKILL.md#L148-L192) |
| **MCP 工具参考** | MAB 200+工具 + UmgMcp 50+工具 + 重叠矩阵 | [references/mcp_tools.md](./mcp_tools.md) |

---

## 快速参考卡 (精简版)

| 我要... | 找谁 | 核心工具 |
|---------|------|---------|
| 创建/编辑 UMG Widget | 🎨 umg-ui | `create_widget`, `apply_layout` |
| 做 UI 入场/交互动画 | 🎨 umg-ui | `set_animation_data`, `create_animation` |
| UI 材质 (按钮发光/渐变) | 🎨 umg-ui | `material_set_target` + `material_add_node` |
| BP 事件绑定 (Widget) | 🎨 umg-ui | `set_edit_function` + `add_step` |
| 生成/操控 Actor | ⚙️ ue-core | `spawn_actor`, `set_transform` |
| 创建 PBR/MF/MI 材质 | ⚙️ ue-core | `create_material` + 全流程 API |
| 骨骼动画 (AB/BS/SM) | ⚙️ ue-core | `create_animation_bp` + `create_state_machine` |
| PIE 测试 / 截图 | ⚙️ ue-core | `play_in_editor`, `screenshot` |
| SCS 组件树操作 | ⚙️ ue-core | `blueprint_modify_scs` |
| 编译任何 Blueprint | 皆可 | `compile_blueprint()` |
| 项目健康检查 (前置) | 🔧 CLI Tool | `project info`, `plugin list` |
| 离线 UMG 结构分析 | 🔧 CLI Tool | `ui umg-read <path>` |
| 强制质量评审 (必经) | 🔍 CodeReview | 4维度逐项打分 |
| 最终验收决策 | 👑 SoloCoder | PASS → 交付 / FAIL → 返工 |

**完整版 (含 CodeReview/SoloCoder 行)**: → [coordination-protocol.md 快速参考卡](../../agents/coordination-protocol.md#L598-L616)

---

*本文件定位: 导航索引 — 所有实质内容均有唯一权威源，此处不重复定义*
*版本: 1.1 | 同步于 coordination-protocol v1.1*
