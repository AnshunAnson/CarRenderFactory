# Agent 协作协议 (Coordination Protocol)

> **版本**: 1.1
> **适用项目**: AudiR7S (UE 5.7 Car Configurator UI)
> **参与 Agent**: `ue-core-agent` + `umg-ui-agent` + `unreal-code-review` + `SoloCoder`
> **定位**: 全局规则 (宪法) — 定义系统运转规则、分工协议、评审标准、验收流程

---

## 架构总览 (含评审与验收)

```
┌──────────────────────────────────────────────────────────────────┐
│                    SoloCoder (编排者 + 最终验收)                  │
│  - 任务分解 & 分派                                               │
│  - 接收 CodeReview 汇报                                          │
│  - 验收决策: PASS → 交付 / FAIL → 编排返工                        │
└──────┬─────────────────────────┬────────────────────────────────┘
       │                         │
       │ 分派任务                 │ 接收汇报
       ▼                         │
┌──────────────┐   ┌──────────────┤
│ ue-core-     │   │ umg-ui-      │
│ agent        │   │ agent        │
│              │   │              │
│ McpAuto-     │   │ UmgMcp       │
│ mationBridge │   │ (Python)     │
└──────┬───────┘   └──────┬───────┘
       │ 完成任务           │ 完成任务
       └───────────┬────────┘
                   ▼
       ┌──────────────────────┐
       │  UnrealCodeReview     │ ◄── 质量评估节点 (强制)
       │  (评审者)             │
       │                       │
       │ - 对照任务清单逐项核对  │
       │ - 代码/资产质量检查    │
       │ - 最佳实践合规性验证   │
       │ - 异常情况标记上报     │
       └──────────┬───────────┘
                  │ Review Report
                  ▼
       ┌──────────────────────┐
       │    SoloCoder          │ ◄── 验收节点 (最终决策)
       │  (验收者)              │
       │                        │
       │ PASS: 任务达标 → 交付  │
       │ FAIL: 不达标 → 返工   │
       └──────────────────────┘
```

### 执行流水线（带质量门禁）

```
Phase 0: 任务规划 (SoloCoder)
    │
    ├── 定义任务目标、范围、Hardness
    ├── 生成交付清单 (Deliverables Checklist)
    ├── 确定路由策略 (单 Agent / 协作)
    │
    ├── [可选] CLI 前置健康检查 (Tool Layer, 非 Agent)
    │   ├── `project info` — 确认项目状态/模块/引擎版本
    │   ├── `plugin list` — 确认 McpAutomationBridge + UmgMcp 插件启用状态
    │   └── `stats code` — 确认 C++ 代码行数/文件数 (用于评估变更影响)
    │   (详见 SKILL.md → "CLI Tool Layer" 章节)
    │
    ▼
Phase 1~N: 任务执行 (ue-core-agent / umg-ui-agent)
    │
    ├── 按 Playbook 步骤执行
    ├── 每个 Phase 完成后自检
    ├── 输出交付物清单
    │
    ▼ [🚪 质量门禁 1: 必经]
Phase R: CodeReview 评审 (UnrealCodeReview)  ◄── 强制节点,不可跳过
    │
    ├── 对照 Deliverables Checklist 逐项核对
    ├── 代码质量审查 (命名规范/架构一致性/性能)
    ├── 资产完整性验证 (编译状态/依赖关系)
    ├── 最佳实践合规性 (UE Coding Standards)
    ├── 标记异常项 (Critical/Major/Minor)
    │
    ├── 全部 PASS → 输出 Review Report (PASS)
    └── 存在 FAIL 项 → 输出 Review Report (FAIL) + 建议修复方案
    │
    ▼
Phase A: SoloCoder 验收 (最终决策)
    │
    ├── 审阅 Review Report
    ├── PASS:
    │   ├── 标记任务完成 ✅
    │   ├── 更新 Knowledge Graph Memory
    │   ├── 更新 AGENTS.md 项目文档
    │   └── 向用户交付结果
    │
    └── FAIL:
        ├── 分析失败原因 (Agent 能力不足 / 任务定义不清 / 环境问题)
        ├── 决策返工策略:
        │   ├── A) 同一 Agent 重试 (简单修复)
        │   ├── B) 切换 Agent 执行 (能力不匹配)
        │   ├── C) 拆分任务重新分配 (复杂度过高)
        │   └── D) 放弃并上报 (不可行)
        ├── 编排返工 → 回到 Phase 1 (最多重试 N=3 次)
        └── N 次仍 FAIL → 任务失败, 记录根因分析
```

---

## 任务分类与路由规则

### 规则矩阵：任务 → Agent 路由

| 任务类型                   | 关键词识别                                                                 | 主力 Agent         | 协作 Agent                | 说明                    |
| -------------------------- | -------------------------------------------------------------------------- | ------------------ | ------------------------- | ----------------------- |
| **Widget 创建/编辑** | widget, button, text, panel, canvas, ui, 界面, 控件                        | `umg-ui-agent`   | —                        | UMG 独占                |
| **HTML/JSON 布局**   | layout, html, 批量布局, apply_layout                                       | `umg-ui-agent`   | —                        | UMG 独有能力            |
| **UI 动画**          | widget animation, sequencer, 入场动画, hover anim                          | `umg-ui-agent`   | —                        | Widget Sequencer 归 UMG |
| **UI 材质**          | button material, ui shader, glow, progress bar mat                         | `umg-ui-agent`   | —                        | 轻量材质归 UMG          |
| **Skeletal 动画**    | skeletal, anim bp, blend space, blend tree, state machine, 骨骼动画        | `ue-core-agent`  | —                        | 骨骼动画独占            |
| **材质管线**         | material pipeline, pbr, landscape mat, post-process, mf, material function | `ue-core-agent`  | —                        | 复杂材质归 Core         |
| **Actor 操作**       | spawn actor, transform, component, 关卡对象                                | `ue-core-agent`  | —                        | Actor 独占              |
| **Editor 控制**      | play, pie, stop, screenshot, input, 播放, 截图                             | `ue-core-agent`  | —                        | Editor 控制独占         |
| **BP SCS**           | scs, component tree, add mesh, add camera to bp                            | `ue-core-agent`  | —                        | SCS 独占                |
| **BP 图形编辑**      | blueprint node, graph, compile bp, 变量, 函数                              | **视上下文** | **视上下文**        | 见下方细则              |
| **资产管理**         | asset create, rename, move, list assets                                    | `ue-core-agent`  | `umg-ui-agent`(仅 list) | Core 为主               |
| **混合任务**         | 配置器, 3D预览, car viewer, 完整功能模块                                   | **两者协作** | **两者协作**        | 见下方流程              |

### BP 图形编辑路由细则

```
Blueprint 编辑请求
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

## 标准协作流程

> **注意**: 以下为 **抽象流程定义** (谁→做什么→传什么上下文)。
> 具体任务的逐步执行指南（含参数、代码、验证标准）见:
> → [playbooks/mcp_agent_orchestration.md](../skills/unreal-engine/playbooks/mcp_agent_orchestration.md)

### 流程 A: 顺序流水线（最常见）

**适用场景**: 同时涉及引擎操作 + UI 操作的混合任务（如车辆配置器、3D查看器）

**抽象流程**:

```
Phase 1 [ue-core-agent]: 引擎环境准备
    │   输出: Actor列表 + 材质路径 + 关卡状态
    ↓ 传递上下文 (actor_names, material_paths, level_info)
Phase 2 [umg-ui-agent]: UI 界面构建
    │   输出: Widget名称列表 + 布局结构
    ↓ 传递上下文 (widget_names, event_bindings)
Phase 3 [umg-ui-agent]: UI 动画 + BP事件绑定
    │   输出: 动画名列表 + 编译状态
    ↓ 传递上下文 (animation_names, compiled_bp_path)
Phase 4 [ue-core-agent]: 运行时桥接 + PIE测试验证
        输出: 测试截图 + 运行时验证结果

交付物汇总 → Phase R (CodeReview) → Phase A (SoloCoder验收)
```

**关键规则**:

- 每个 Phase 完成后必须输出标准化的上下文包 (见 §通信协议)
- 上下文传递使用 `callback_action` 字段声明下一个 Agent 的入口动作
- 任一 Phase 失败 → 上报 SoloCoder → 按返工策略处理

**完整执行示例 (车辆配置器 H级)**: → [Playbook 1](../skills/unreal-engine/playbooks/mcp_agent_orchestration.md#L19)

---

### 流程 B: UI 重构（纯 UMG 任务）

**适用场景**: 仅涉及 UMG Widget 的创建/迁移/重构，无需引擎级操作

**抽象流程**:

```
执行者: umg-ui-agent (独立完成, 无需协作)

Step 1: 导出现有结构 (export_umg_to_json)
Step 2: 分析层级深度和性能问题 (get_widget_tree + get_layout_data)
Step 3: 用 apply_layout 重新设计优化版
Step 4: C++ 基类属性绑定 (set_widget_properties)
Step 5: 制作动效 (create_animation + set_animation_data)
Step 6: BP事件绑定 (set_edit_function + add_step)
Step 7: 编译 + 保存 (compile_blueprint + save_asset)

交付物 → Phase R (CodeReview) → Phase A (SoloCoder验收)
```

**完整执行示例 (UI Migration M级)**: → [Playbook 2](../skills/unreal-engine/playbooks/mcp_agent_orchestration.md)

---

### 流程 C: 材质+UI 协作设计（混合但以 Core 为主）

**适用场景**: 材质系统搭建后需要 UI 选择器交互（如车漆颜色选择器）

**抽象流程**:

```
├─ Lane A [ue-core-agent]: 材质系统搭建 (可独立进行)
│   Step 1: 主材质创建 (create_material + 全属性设置)
│   Step 2: MI 批量生成 (create_material_instance + 参数设值)
│   └── 输出: MI路径列表
│
└─ Lane B [umg-ui-agent]: UI 面板构建 (可并行进行)
    Step 1: 创建选择器 Widget 树 (apply_layout / create_widget)
    Step 2: 将 MI 绑定到 UI 控件 Brush (set_widget_properties)
    Step 3: BP 事件绑定 (set_edit_function + add_step)
    └── 输出: Widget BP 路径

合并点: Lane A 的 MI 列表 → 注入 Lane B 的 Step 2 上下文
最终: [ue-core-agent] PIE 验证材质切换效果

交付物汇总 → Phase R → Phase A
```

**完整执行示例 (Material+UI M级)**: → [Playbook 3](../skills/unreal-engine/playbooks/mcp_agent_orchestration.md)

---

## 通信协议

### Agent 间上下文传递格式

当需要委托任务给另一个 Agent 时，使用标准化的上下文包：

```json
{
  "task_id": "unique-task-id",
  "source_agent": "ue-core-agent",
  "target_agent": "umg-ui-agent",
  "task_type": "widget_creation",
  "priority": "high",
  "context": {
    "target_asset": "/Game/UI/WBP_CarConfigurator",
    "parent_widget": "Root_CanvasPanel",
    "available_materials": ["/Game/Materials/MI_Red", "/Game/Materials/MI_Blue"],
    "actor_references": ["CarPreviewMesh_CamTarget"]
  },
  "expected_output": "widget_names_created",
  "callback_action": "bind_widget_events"
}
```

### 回调约定

| 场景                         | 回调触发方    | 回调接收方    | 回调内容                                                                  |
| ---------------------------- | ------------- | ------------- | ------------------------------------------------------------------------- |
| UI 按钮点击需改变 Actor 属性 | umg-ui-agent  | ue-core-agent | `{event: "color_changed", params: {color: "Red"}}`                      |
| Actor 位置变化需更新 UI 显示 | ue-core-agent | umg-ui-agent  | `{event: "transform_updated", position: [x,y,z]}`                       |
| 材质参数变化需同步 UI        | ue-core-agent | umg-ui-agent  | `{event: "material_param_changed", param: "GlowIntensity", value: 0.8}` |
| BP 编译完成通知              | 任一          | SoloCoder     | `{status: "compiled", warnings: [], errors: []}`                        |

---

## 冲突解决机制

### 1. 资源冲突（同一资产被两个 Agent 同时修改）

```
检测: Agent 尝试 save_asset 时发现资产已被外部修改
解决:
  - 方案 A: 最后写入胜出 (简单场景)
  - 方案 B: 加锁机制 — Agent 在编辑前声明 "LOCK:/Game/UI/WBP_X"
  - 方案 C: SoloCoder 仲裁 — 冲突上报给编排者决定
推荐: 方案 B (声明式锁)
```

### 2. 功能重叠区的任务争抢

```
场景: 用户说 "创建一个按钮"
可能路由:
  - umg-ui-agent: create_widget("Button", ...)          # UMG Widget 按钮
  - ue-core-agent: manage_widget_authoring add_button   # 3D Widget 按钮

解决:
  - 默认优先级: umg-ui-agent (UI = UMG 的主场)
  - 如果用户指定 "3D Widget" 或 "运行时" → ue-core-agent
  - 如果用户在 UI Designer 上下文中 → umg-ui-agent
```

### 3. Agent 不可用时的降级

```
umg-ui-agent 不可用时:
  → UE-Core-Agent 可用: 使用 manage_widget_authoring 的具体控件命令替代
  → 两者都不可用: 返回错误, 建议 "请启动 UE Editor 并启用对应插件"

ue-core-agent 不可用时:
  → UMG-UI-Agent 可用: 只能做 UI 部分, Actor/Material 部分排队等待
  → 两者都不可用: 同上
```

---

## 质量检查清单 (CodeReview 基准)

> **唯一权威源**: 以下清单为 CodeReview 评审基准，不得在其他文件中重复定义。

### umg-ui-agent 任务验收标准

- [ ] Widget 层级深度 ≤ 5 层（避免过深嵌套）
- [ ] 无孤儿 Widget（每个 Widget 都有有效父级）
- [ ] 命名规范: 类型前缀 + 功能名 (`Btn_Confirm`, `Txt_Title`, `Pnl_Options`)
- [ ] Anchor 设置合理（避免硬编码 Position 导致的分辨率适配问题）
- [ ] 动画关键帧时间范围合理（无负时间值）
- [ ] BP 事件绑定完整（无 dangling delegate）
- [ ] `save_asset()` 已调用
- [ ] `compile_blueprint()` 已通过（无编译错误）

### ue-core-agent 任务验收标准

- [ ] Actor 生成位置不重叠（除非有意设计）
- [ ] 材质节点图无断连（所有必要输入已连接）
- [ ] Material Instance 参数名与父材质匹配
- [ ] SCS 组件命名唯一
- [ ] Animation Blueprint 状态机转换条件完备
- [ ] 资产路径符合项目规范 (`/Game/AUDI_Content/...`)
- [ ] 无内存泄漏风险（动态创建的对象有清理逻辑）

---

## UnrealCodeReview 评审规范 (Phase R)

> **权威源**: 本章节为摘要，完整定义见 [unreal-code-review-agent.md](./unreal-code-review-agent.md)

### 身份定义

**UnrealCodeReview** 是独立的代码/资产质量评审 Agent，不参与任务执行，仅负责**事后评估**。

**核心原则**: 独立性 — 评审者不偏袒任何执行 Agent，严格对照标准逐项打分。

### 评审触发条件（自动触发，不可跳过）

| 触发场景                           | 评审范围                                  |
| ---------------------------------- | ----------------------------------------- |
| 任意 Agent 完成 Phase 并输出交付物 | 全量评审 (交付清单 + 代码质量 + 最佳实践) |
| 协作任务的每个 Phase 完成时        | 该 Phase 交付物专项评审                   |
| 返工完成后                         | 仅评审 FAIL 项的修复情况                  |

### 评审维度与评分标准 (摘要)

| 维度          | 权重 | 检查重点                                    |
| ------------- | ---- | ------------------------------------------- |
| 交付完整性    | 30%  | 资产存在、编译状态、保存状态、依赖关系      |
| 代码/资产质量 | 30%  | 命名规范、架构一致性、性能合规、内存安全    |
| 最佳实践合规  | 25%  | UE Coding Standards、MCP 工具使用、错误处理 |
| 任务目标达成  | 15%  | 功能完整性、边界条件、可复现性              |

**完整评分细则**: → [unreal-code-review-agent.md §评审维度](./unreal-code-review-agent.md#L75-L130)

### Verdict 判定规则 (摘要)

```
IF (存在 Critical) → verdict = FAIL
ELIF (Major ≥ 2) → verdict = FAIL
ELIF (Major == 1 AND overall_score < 0.7) → verdict = FAIL
ELIF (overall_score ≥ 0.85) → verdict = PASS
ELSE → verdict = CONDITIONAL_PASS (附改进建议)
```

**完整判定规则**: → [unreal-code-review-agent.md §Verdict判定](./unreal-code-review-agent.md#L200-L215)

---

## SoloCoder 验收流程 (Phase A)

### 身份定义

**SoloCoder** 是最终决策者，负责：

1. 审阅 UnrealCodeReview 的评审报告
2. 做出最终 PASS/FAIL 决策
3. FAIL 时编排返工策略
4. PASS 时完成收尾工作（Memory 更新 / 文档更新 / 交付）

**核心原则**: 不跳过 CodeReview，不盲目信任 Agent 自检结果。

### 验收决策矩阵

| Review Verdict             | Score       | SoloCoder 决策                   | 行动                                  |
| -------------------------- | ----------- | -------------------------------- | ------------------------------------- |
| **PASS**             | ≥ 0.85     | ✅**APPROVE**              | 执行收尾流程                          |
| **CONDITIONAL_PASS** | 0.70 ~ 0.84 | ⚠️**APPROVE with Notes** | 记录 Minor 问题，执行收尾，标注技术债 |
| **FAIL**             | < 0.70      | ❌**REJECT**               | 进入返工流程                          |
| **FAIL** (Critical)  | any         | ❌**REJECT (紧急)**        | 立即停止，分析根因，可能切换策略      |

### 收尾流程 (APPROVE 后)

```yaml
Step 1 - Knowledge Graph Memory 更新:
  - 创建/更新 Task 实体:
    observations:
      - status: COMPLETED
      - executor: <agent_name>
      - review_score: <score>
      - deliverables: <asset_list>
      - completed_at: <timestamp>
  - 创建/更新关系:
    - task -> executed_by -> agent
    - task -> produces -> assets
    - task -> belongs_to -> project_phase

Step 2 - 项目文档更新 (AGENTS.md):
  - 更新 Phase 进度标记
  - 记录新增资产到目录结构
  - 更新后续路线图 (如有影响)

Step 3 - 向用户交付:
  - 输出简洁的结果摘要
  - 附带关键文件路径引用
  - 如有 Conditional Pass 的 Minor 项，一并列出
```

### 返工编排流程 (REJECT 后)

```yaml
Step 1 - 分析失败原因 (SoloCoder):
  输入: Review Report findings[]
  分析:
    ├── Agent 能力不足?     → 策略 B: 切换 Agent
    ├── 任务定义不清?       → 策略 C: 拆分重定义
    ├── 环境问题 (UE/插件)?  → 策略 D: 排障后重试
    └── 简单疏漏?           → 策略 A: 同 Agent 重试

Step 2 - 选择返工策略:
  A) 同一 Agent 重试:
     适用: Minor/Major 数量 ≤ 2, 修复简单
     操作: 将 findings[] 转为修复指令, 发回原 Agent
     重试计数: retry_count += 1

  B) 切换 Agent:
     适用: 原 Agent 反复在同一类问题上失败
     操作: 将任务转交给另一个 Agent (如 ue-core→umg-ui 或反之)
     注意: 需确认目标 Agent 有对应能力覆盖

  C) 拆分任务重新分配:
     适用: 单次任务复杂度过高, Agent 无法一次性高质量完成
     操作: 将任务拆分为 N 个子任务, 分别分派
     示例: "创建完整配置器" → [Phase1:材质], [Phase2:UI], [Phase3:动画], [Phase4:事件]

  D) 放弃并上报:
     适用: 经 3 次返工仍 FAIL, 或 Critical 错误无法修复
     操作: 标记任务 FAILED, 记录根因分析报告, 通知用户

Step 3 - 执行返工:
  - 重置相关 Phase 状态
  - 将 Review findings 作为「已知约束」传递给 Agent
  - Agent 完成后 → 再次进入 Phase R (CodeReview)
  - 循环直到 PASS 或 达到最大重试次数 (N=3)

Step 4 - 最终处置 (N=3 仍 FAIL):
  - 生成 Root Cause Analysis 报告:
    {
      task_id, total_retries, failure_pattern,
      root_cause: "能力不足|环境限制|需求冲突|其他",
      recommendation: "需要人工介入 / 需要升级工具链 / 需要简化需求"
    }
  - 更新 Knowledge Graph Memory (FAILED 状态)
  - 通知用户任务失败及原因
```

### 返工上下文传递格式

```json
{
  "rework_id": "<uuid>",
  "original_task_id": "<原任务ID>",
  "retry_count": 2,
  "max_retries": 3,
  "strategy": "A|B|C|D",
  "review_findings_to_fix": [
    {
      "finding_id": "F-003",
      "severity": "Major",
      "description": "Widget 层级深度达到 7 层, 超过 5 层上限",
      "suggested_fix": "拆分为多个子 Panel, 减少嵌套深度"
    }
  ],
  "constraints_from_previous_attempt": [
    "不得使用 CanvasPanel 作为中间容器",
    "必须使用 UniformGrid 替代手动 Position 布局"
  ],
  "target_agent": "umg-ui-agent"
}
```

---

## 快速参考卡

| 我要...              | 找谁                | 工具示例                                           |
| -------------------- | ------------------- | -------------------------------------------------- |
| 创建按钮/面板/文本   | 🎨 UMG-UI           | `create_widget("Button", "Btn_OK")`              |
| 用 HTML 一把梭建 UI  | 🎨 UMG-UI           | `apply_layout("<html>...</html>")`               |
| 做 UI 入场动画       | 🎨 UMG-UI           | `set_animation_data(tracks)`                     |
| 改按钮颜色/发光材质  | 🎨 UMG-UI           | `material_set_target` + `material_add_node`    |
| 生成汽车/角色 Actor  | ⚙️ UE-Core        | `spawn_actor(type, name, loc)`                   |
| 移动/旋转 Actor      | ⚙️ UE-Core        | `control_actor set_transform`                    |
| 创建车漆 PBR 材质    | ⚙️ UE-Core        | `create_material` + `set_shading_model`        |
| 做角色跑跳动画       | ⚙️ UE-Core        | `create_animation_bp` + `create_state_machine` |
| 按 Play 测试         | ⚙️ UE-Core        | `control_editor play_in_editor`                  |
| 给 BP 添加 Mesh 组件 | ⚙️ UE-Core        | `blueprint_modify_scs add_component`             |
| 截图验证效果         | ⚙️ UE-Core        | `manage_ui screenshot`                           |
| 编译任何 Blueprint   | 皆可                | `compile_blueprint()`                            |
| 列出项目纹理资源     | 皆可                | `list_assets(class="Texture2D")`                 |
| 项目健康检查 (前置)  | 🔧 CLI Tool         | `project info`, `plugin list`                  |
| 离线 UMG 结构分析    | 🔧 CLI Tool         | `ui umg-read <path>`                             |
| 评审任务质量 (强制)  | 🔍 UnrealCodeReview | 对照 4 维度标准逐项打分                            |
| 验收/返工决策        | 👑 SoloCoder        | 审阅 Review Report → APPROVE / REJECT             |

---

*文档版本: 1.1*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
*定位: 全局规则 (宪法) — 本文件是系统运转的唯一规则权威源*
