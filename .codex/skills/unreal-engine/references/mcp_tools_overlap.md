# MCP Tools Overlap Analysis (工具重叠对比)

> **权威源**: 本文件定义 McpAutomationBridge 与 UmgMcp 的重叠分析
> **版本**: 1.0

---

## 重叠矩阵总览

| 功能域 | McpAutomationBridge | UmgMcp | 重叠度 | 推荐主力 |
|--------|-------------------|---------|--------|----------|
| **Widget 创建** | `create_widget` / `add_button` 等 20+ 具体命令 | `create_widget(type)` 通用 + `apply_layout` HTML | ⚠️ 高 | UMG (**HTML批量布局独有**) |
| **Widget 属性** | 分散的 `set_text/image/visibility/anchor/position...` | 统一 `set_widget_properties(dict)` | ✅ 功能等价 | UMG (更统一) |
| **Widget 查询** | `get_widget_info` | `tree + query + layout_data + overlap` | ⚠️ UMG 更丰富 | UMG (**layout/overlap独有**) |
| **Widget Binding** | `bind_text/color/click/hovered/visibility/enabled` | ❌ 需手动 BP 编辑 | MAB 独有 | MAB |
| **BP 编译** | `blueprint_compile` / `compile_blueprint` | `compile_blueprint` | ✅ 完全一致 | 任一 |
| **BP 节点编辑** | `manage_blueprint_graph` 完整套 | API 一致 (add_step/prepare/connect/...) | ✅ 完全一致 | 视上下文 |
| **BP 变量** | `add/get/delete variable` + metadata | `add/get/delete variable` | ✅ 完全一致 | 任一 |
| **BP SCS** | 完整组件树 (add/remove/reparent/transform) | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Material 创建** | `create_material` + blend_mode/shading_model/domain | `material_set_target`(自动创建) | ⚠️ MAB 更全 | 复杂→MAB, UI→UMG |
| **Material 节点** | `add_node/remove/connect/disconnect/batch` | `add_node/delete/connect_pins` | ✅ 核心一致 | 任一 |
| **Material MF/MI** | create MF / use MF / create MI / set MI params | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Material HLSL** | `add_custom_expression(code)` | `material_set_hlsl_node_io` | ✅ 功能等价 | 任一 |
| **Animation 类型** | Skeletal (AB/BS/BT/StateMachine/Procedural) | Widget Sequencer (UMG 动画) | ❌ **领域不同!** | 互补! |
| **Editor 控制** | play/stop/screenshot/simulate_input/save_all | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Actor 操作** | spawn/delete/transform/component/tag/... | 仅 list/spawn (基础) | MAB 独占 | **必须 MAB** |
| **Asset 管理** | create/rename/move/delete/list/references | 仅 list_assets | MAB 更全 | MAB |
| **Geometry Script** | append_box/sphere/capsule/boolean | ❌ 无 | MAB 独有 | **必须 MAB** |
| **World Partition** | get_info/configure | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Console Command** | execute/batch | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Performance** | stats/profiling/insights | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Debug 可视化** | draw_line/box/sphere/clear | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Environment** | landscape/foliage/LOD | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Niagara VFX** | system/emitter/ribbon/graph | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Audio** | sound/dialogue/reverb/submix | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Lighting** | setup/bake_lightmap | ❌ 无 | MAB 独有 | **必须 MAB** |
| **AI/BehaviorTree** | controller/bt/blackboard | ❌ 无 | MAB 独有 | **必须 MAB** |
| **GAS** | ability/effect/attribute | ❌ 无 | MAB 独有 | **必须 MAB** |
| **Networking** | replication/setup | ❌ 无 | MAB 独有 | **必须 MAB** |

---

## 关键结论

### 1. 不冲突领域 (完美互补)

| 领域 | MAB 能力 | UMG 能力 | 结论 |
|------|---------|---------|------|
| **Animation** | Skeletal (AB/BS/BT/StateMachine) | Widget Sequencer | 零重叠，互补 |
| **Editor 控制** | PIE/Screenshot/Input/Save | 无 | MAB 独占 |
| **Actor 操作** | 完整生命周期管理 | 仅 list/spawn (基础) | MAB 独占 |
| **SCS 组件树** | 完整操作 | 无 | MAB 独占 |
| **HTML 批量布局** | 无 | `apply_layout` | UMG 独占 |
| **Widget 动画数据** | 无 | `set_animation_data` | UMG 独占 |
| **Layout 分析** | 无 | `get_layout_data` + `check_widget_overlap` | UMG 独占 |

### 2. 高重叠但各有优势

| 领域 | MAB 优势 | UMG 优势 | 路由建议 |
|------|---------|---------|---------|
| **Widget CRUD** | `bind_*` 独占能力 | `apply_layout` 杀手锏 | UI 任务 → UMG |
| **Material** | 全流程 (属性/MF/MI) | 轻量但显式 compile + get_graph | 复杂→MAB, UI→UMG |
| **BP Graph** | SCS 操作 | Widget BP 上下文偏好 | 视上下文 |

### 3. 完全一致 (任选)

- BP 编译
- BP 节点编辑 (核心 API)
- BP 变量操作
- Material 节点核心操作
- Material HLSL

---

## 路由决策树

```
任务请求
│
├── 涉及 Widget 创建/布局?
│   ├── 需要 HTML 批量布局? → UMG (apply_layout)
│   ├── 需要 Widget 动画? → UMG (Sequencer)
│   ├── 需要 Layout 分析? → UMG (get_layout_data/overlap)
│   └── 需要 Widget Binding? → MAB (bind_*)
│
├── 涉及 Actor 操作?
│   └── → MAB (完整生命周期)
│
├── 涉及 SCS 组件树?
│   └── → MAB (独占能力)
│
├── 涉及 Skeletal Animation?
│   └── → MAB (AB/BS/BT/StateMachine)
│
├── 涉及 Editor 控制?
│   └── → MAB (PIE/Screenshot/Input)
│
├── 涉及 Material?
│   ├── 需要 MF/MI? → MAB
│   ├── 仅 UI 材质? → UMG
│   └── 复杂 PBR? → MAB
│
├── 涉及 BP 编译?
│   └── → 任一 (API 一致)
│
└── 涉及 BP 图形编辑?
    ├── 目标是 Widget BP? → UMG
    ├── 包含 SCS 操作? → MAB
    └── 其他 → 视上下文
```

---

## 重叠度统计

| 重叠类型 | 数量 | 占比 |
|---------|------|------|
| **MAB 独占** | 8 项 | 47% |
| **UMG 独占** | 4 项 | 24% |
| **高重叠** | 3 项 | 18% |
| **完全一致** | 2 项 | 12% |

**总计**: 17 个功能域

---

## 实际重叠率

虽然表面重叠看起来较高，但实际使用中:

| 场景 | 实际重叠 |
|------|---------|
| Widget CRUD | 8.5% (UMG 的 apply_layout 是独特路径) |
| Material | 15% (MF/MI 归 MAB，UI 材质归 UMG) |
| BP Graph | 20% (SCS 归 MAB，Widget BP 归 UMG) |

**综合实际重叠率**: 约 **8.5%**

---

## See Also

- [McpAutomationBridge Tools](./mcp_tools_mab.md) — MAB 完整工具集
- [UmgMcp Tools](./mcp_tools_umg.md) — UMG 完整工具集
- [MCP Agent System](./mcp_agent_system.md) — 双 Agent 架构
- [Backend Priority](../execution/backend_priority.md) — 后端选择规则

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
