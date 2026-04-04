# MCP Agent Orchestration Playbook

> **Hardness**: M (medium) ~ H (high) 取决于任务复杂度
> **适用场景**: 需要同时使用 ue-core-agent 和 umg-ui-agent 的混合任务
> **前置条件**: UE Editor 已启动，两个 MCP 服务均可用

---

## 编排前检查清单

- [ ] UE Editor 已启动并加载 AudiR7S 项目
- [ ] McpAutomationBridge 插件已启用 (Edit → Plugins)
- [ ] UmgMcp 插件已启用 (Edit → Plugins)
- [ ] `.trae/mcp.json` 中 `unreal-engine` 和 `umg-mcp` 服务均已配置
- [ ] 确认两个 Agent 的 MCP 连接正常（可执行简单命令测试）

---

## Playbook 1: 车辆配置器完整搭建（H 级）

**目标**: 从零创建一个完整的车辆配置器 UI 系统，包含 3D 预览 + 配置面板 + 动画效果

### Phase 0: 任务分解与规划

```
输入: "创建奥迪 R7S 车辆配置器界面"

输出:
├── Hardness: H
├── Delegation Plan:
│   ├── Phase 1: ue-core-agent — 引擎环境准备 (Actor/Material/Lighting)
│   ├── Phase 2: umg-ui-agent — UI 界面构建 (Widget/Layout/Animation)
│   ├── Phase 3: umg-ui-agent — BP 事件绑定
│   └── Phase 4: ue-core-agent — PIE 测试验证
├── Scriptability: scriptable (所有步骤均可通过 MCP 命令完成)
└── Execution Backend: mcp-dual-agent
```

### Phase 1: 引擎准备 (ue-core-agent)

**委托上下文**:
```json
{
  "task_type": "engine_setup",
  "context": {
    "target_level": "/Game/Maps/Map_Configurator",
    "car_mesh": "/Game/Meshes/SM_R7S_Body",
    "output_dir": "/Game/AUDI_Content/UI/Materials"
  },
  "expected_output": ["actor_names", "material_paths"],
  "callback_action": "ui_construction"
}
```

**执行步骤**:
1. `spawn_actor` — StaticMeshActor (CarBody, SM_R7S_Body), location=[0,0,0]
2. `spawn_actor` — CameraActor (PreviewCam), location=[0,500,200], rotation looking at origin
3. `spawn_actor` — SkyLightActor / DirectionalLight (SceneLighting)
4. `create_material` — M_CarPaintSystem, domain="Surface", shading_model="Subsurface"
5. Material 节点图搭建:
   - `add_texture_sample` T_BaseColor, T_Normal, T_Clearcoat, T_AO, T_Roughness
   - `add_scalar_parameter` Clearcoat(1.0), Metallic(0.9), Roughness(0.3)
   - `add_vector_parameter` BaseColorTint([1,1,1])
   - `add_custom_expression` CAR_PAINT_FRESNEL (HLSL)
   - `connect_nodes` 完成车漆 PBR 节点图
6. 批量创建 MI:
   ```
   FOR color IN [MisanoRed, VegasYellow, GlacierWhite, ChronosGrey, NavarraBlue]:
     create_material_instance(parent=M_CarPaintSystem, name=MI_{color})
     set_vector_parameter_value(MI_{color}, BaseColorTint, color_rgb)
   ```

**验收标准**:
- [ ] Car Preview Actor 在关卡中可见
- [ ] Preview Camera 正确朝向车辆
- [ ] M_CarPaintSystem 编译无错误
- [ ] 5 个 MI 实例创建成功且参数正确

### Phase 2: UI 构建 (umg-ui-agent)

**委托上下文**:
```json
{
  "task_type": "ui_construction",
  "context": {
    "target_asset": "/Game/AUDI_Content/UI/WBP_CarConfigurator",
    "available_materials": [
      "/Game/AUDI_Content/UI/Materials/MI_MisanoRed",
      "/Game/AUDI_Content/UI/Materials/MI_VegasYellow",
      ...
    ],
    "car_preview_actor": "CarBody"
  },
  "expected_output": ["widget_tree_structure", "animation_names"],
  "callback_action": "event_binding"
}
```

**执行步骤**:
1. `set_target_umg_asset("/Game/AUDI_Content/UI/WBP_CarConfigurator")` — 创建或打开
2. `apply_layout(HTML_CONFIGURATOR_LAYOUT)` — 用 HTML 批量构建主体结构:

```html
<CanvasPanel name="Root">
  <Image name="BG" brush="/Game/AUDI_Content/UI/Textures/T_BG_Main" anchors="fill"/>
  <Overlay name="MainOverlay" anchors="fill">
    <!-- 3D 预览区域 -->
    <CanvasPanel name="PreviewArea" size="[900,600]" position="[40,40]">
      <Image name="CarRenderTarget" size="[900,600]"/>
      <Border name="PreviewBorder" padding="[2,2,2,2]"/>
    </CanvasPanel>
    <!-- 右侧配置面板 -->
    <VerticalBox name="ConfigPanel" position="[980,40]" size="[880,600]">
      <TextBlock name="Title" text="Audi R7S Configurator" font-size="36" font="/Game/Fonts/F_AudiType"/>
      <TextBlock name="Subtitle" text="Select Your Configuration" font-size="18" color="#888888"/>
      <!-- 配置选项卡 -->
      <UniformGridPanel name="OptionTabs" columns="3">
        <!-- 由后续 create_widget 添加 -->
      </UniformGridPanel>
      <!-- 颜色选择器 -->
      <HorizontalBox name="ColorRow" padding="[20,20,20,20]">
        <!-- 由后续 create_widget 添加颜色按钮 -->
      </HorizontalBox>
      <!-- 底部操作区 -->
      <HorizontalBox name="ActionBar" padding="[20,20,20,20]">
        <Button name="Btn_Reset">Reset</Button>
        <Button name="Btn_Confirm">Confirm Configuration</Button>
      </HorizontalBox>
    </VerticalBox>
  </Overlay>
</CanvasPanel>
```

3. 精细构建选项卡 Widget:
   ```python
   set_active_widget("OptionTabs")
   options = ["Exterior", "Interior", "Performance", "Technology"]
   for opt in options:
       create_widget("Button", f"Tab_{opt}")
       set_widget_properties(f"Tab_{opt}", {
           "Width": 260, "Height": 60,
           "Brush": {"Widget": "/Game/AUDI_Content/UI/Materials/M_TabInactive"},
           "Font": "/Game/Fonts/F_AudiType"
       })
   ```
4. 构建颜色选择按钮:
   ```python
   set_active_widget("ColorRow")
   colors = [("MisanoRed", MI_MisanoRed), ("VegasYellow", MI_VegasYellow),
            ("GlacierWhite", MI_GlacierWhite), ("ChronosGrey", MI_ChronosGrey)]
   for name, mi_path in colors:
       create_widget("Button", f"Btn_Color_{name}")
       set_widget_properties(f"Btn_Color_{name}", {
           "Size": [80, 80],
           "Brush": {"Material": mi_path}  # 直接用 MI 作为 Button Brush
       })
   ```
5. 创建入场动画:
   ```python
   create_animation("Intro_SlideIn")
   
   # 标题滑入
   set_widget_scope("Title")
   set_animation_data([
       {"property": "RenderTransform.Translation",
        "keys": [{"time": 0, "value": [-400, 0]}, {"time": 0.6, "value": [0, 0]}]},
       {"property": "RenderOpacity",
        "keys": [{"time": 0, "value": 0}, {"time": 0.4, "value": 1.0}]}
   ])
   
   # 配置面板交错入场
   for i, widget in enumerate(["ConfigPanel"]):
       set_widget_scope(widget)
       set_property_keys("RenderTransform.Translation", [
           {"time": 0.3 + i*0.15, "value": [300, 0]},
           {"time": 0.7 + i*0.15, "value": [0, 0]}
       ])
   ```
6. 创建颜色切换过渡动画:
   ```python
   create_animation("ColorTransition")
   # 在 ColorSelected 回调中动态设置 target widget
   ```

**验收标准**:
- [ ] Widget 树深度 ≤ 5 层
- [ ] 所有命名符合规范 (`Btn_*`, `Txt_*`, `Pnl_*`)
- [ ] Anchor 使用合理（非硬编码 Position）
- [ ] Intro_SlideIn 动画关键帧合法
- [ ] `save_asset()` 已调用

### Phase 3: BP 事件绑定 (umg-ui-agent)

**执行步骤**:
1. 绑定 Tab 切换事件:
   ```python
   for tab in ["Exterior", "Interior", "Performance", "Technology"]:
       set_edit_function(f"Tab_{tab}.OnClicked")
       add_step("Call Function", args=["OnTabChanged"], input_wires={"TabIndex": tab})
   ```
2. 绑定颜色选择事件:
   ```python
   for name, mi_path in colors:
       set_edit_function(f"Btn_Color_{name}.OnClicked")
       add_step("Call Function", args=["OnColorSelected"],
                input_wires={"ColorName": name, "MaterialPath": mi_path})
   ```
3. 绑定 Confirm 按钮:
   ```python
   set_edit_function("Btn_Confirm.OnClicked")
   add_step("Call Function", args=["OnConfigurationConfirmed"])
   ```
4. 绑定 Reset 按钮:
   ```python
   set_edit_function("Btn_Reset.OnClicked")
   add_step("Call Function", args=["OnConfigurationReset"])
   ```
5. `compile_blueprint()`
6. `save_asset()`

**验收标准**:
- [ ] 所有 OnClicked 事件有对应处理函数
- [ ] BP 编译 0 error
- [ ] 无 dangling delegate

### Phase 4: 测试验证 (ue-core-agent)

**执行步骤**:
1. `play_in_editor()`
2. 等待 5 秒让 UI 初始化
3. `screenshot(path="/TestResults/configurator_initial.png")`
4. `simulate_input(key="Click", target="Btn_Color_MisanoRed")`
5. 等待动画完成 (约 1s)
6. `screenshot(path="/TestResults/configurator_color_selected.png")`
7. `simulate_input(key="Click", target="Btn_Confirm")`
8. `screenshot(path="/TestResults/configurator_confirmed.png")`
9. `stop_play()`

**交付物**:
- WBP_CarConfigurator (UMG Asset)
- M_CarPaintSystem + 5x MI 实例
- Preview Actors in Level
- 3 张测试截图
- 编译验证报告

---

## Playbook 2: UI 迁移重构（M 级）

**目标**: 将旧版 WBP_OldMenu 迁移到新的 C++ 基类体系

### Phase 分解

| Phase | Agent | 内容 |
|-------|-------|------|
| 1 | umg-ui-agent | 导出旧结构、分析层级、设计新布局 |
| 2 | umg-ui-agent | 应用新布局、设置 C++ 基类属性 |
| 3 | umg-ui-agent | 制作动效、绑定事件 |
| 4 | umg-ui-agent | 编译保存 |

### 关键执行模式: 分析→导出→重建→验证

```python
# Step 1: 导出现有资产
export_umg_to_json("/Game/UI/Old/WBP_Menu_Old")

# Step 2: 分析
get_widget_tree()
layout = get_layout_data(1920, 1080)
overlaps = check_widget_overlap()

# Step 3: 基于分析结果设计优化布局
optimized_json = design_optimized_layout(tree_analysis, layout_data)

# Step 4: 设置新目标 (基于 C++ 基类)
set_target_umg_asset("/Game/AUDI_CONTENT/UI/Panels/WBP_NewMenu")

# Step 5: 应用
apply_layout(optimized_json)

# Step 6: C++ 基类属性绑定
set_widget_properties("Root", {
    "InputMode": "GameOnly",          # UWidgetBase
    "bFocusable": true               # UWidgetBase
})

# Step 7: 子控件使用 C++ 按钮基类
for btn in [Btn_1, Btn_2, Btn_3]:
    set_widget_properties(btn, {
        "ButtonStyle": {              # UMenuButtonBase
            "Normal": { "Color": "#FFFFFF", "Opacity": 1.0 },
            "Hovered": { "Color": "#C0C0C0", "Opacity": 1.0 }
        }
    })

# Step 8: 动画
create_animation("Open_Anim")
set_animation_data(OPEN_ANIM_KEYS)

# Step 9: BP 事件
set_edit_function("Btn_Primary.OnClicked")
add_step("Call Function", args=["HandlePrimaryAction"])

# Step 10: 收尾
compile_blueprint()
save_asset()
```

---

## Playbook 3: 材色+UI 协同搭建（M 级）

**目标**: 创建一套 UI 按钮材质系统，同时生成对应的 UI 面板使用这些材质

### 并行 Lane 设计

```
Lane A [ue-core-agent]:          Lane B [umg-ui-agent]:
┌─────────────────────┐         ┌─────────────────────┐
│ 1. 创建主材质          │         │ 1. 创建按钮容器 Panel │
│    M_UIButtonSystem   │         │    set_target_umg     │
│ 2. 创建 MF             │         │ 2. apply_layout       │
│    MF_ButtonRipple    │         │    (HTML 按钮网格)      │
│ 3. 创建 MI 变体        │         │ 3. 将 MI 绑定到 Brush   │
│    MI_Normal/Hover/    │  ──────►│    set_widget_props   │
│    Pressed/Disabled   │  context│    (Brush.Material)   │
│ 4. 创建边框/发光材质   │         │ 4. Hover 动画          │
│    M_UIGlow/GlowEffect│         │    create_animation     │
└─────────────────────┘         └─────────────────────┘
              │                           │
              └───────────┬───────────────┘
                          ▼
                 Phase 3: 验证 [umg-ui-agent]
                 compile_blueprint + save_asset
                 get_layout_data 验证视觉正确性
```

---

## 错误处理与恢复策略

| 错误类型 | 检测方法 | 恢复动作 | 上报条件 |
|---------|---------|---------|---------|
| MCP 连接超时 | Socket timeout / WebSocket disconnect | 重试 3 次，间隔 5s | 3 次均失败 → 报告 SoloCoder |
| UE Editor 未响应 | 命令无返回 > 30s | 检查 Editor 是否在 Busy/Loading | 持续 60s 无响应 → 报告 |
| 资产编译错误 | compile 返回 errors[] | 分析错误日志，修复后重编译 | 无法自修复 → 报告 |
| Widget 名称冲突 | create_widget 返回 exists=true | 更名后重试 | — |
| 材质节点断连 | material_compile_asset 返回 error | 用 material_get_graph 检查，修复连接 | — |
| Agent 间上下文丢失 | callback 参数缺失 | 从 Knowledge Graph Memory 重建 | 重建失败 → 报告 |

---

## 输出合同模板

每个编排任务完成后，**UnrealCodeReview 先评审**，然后 **SoloCoder 做最终验收**。

### 完整交付流水线

```yaml
# === Agent 执行完成 → 提交交付物 ===
Agent Output:
  task_id: <uuid>
  executor: ue-core-agent | umg-ui-agent
  phase_completed: N
  deliverables:
    assets: [path1, path2, ...]
    compilation: {success: bool, errors: int, warnings: int}
    test_evidence: [screenshot_path]
  self_check: {checklist: completed, issues: []}

# === Phase R: UnrealCodeReview 评审 (强制) ===
Review Report:
  review_id: <uuid>
  verdict: PASS | CONDITIONAL_PASS | FAIL
  overall_score: 0.0 ~ 1.0
  dimension_scores:
    deliverable_completeness: float   # 30%
    code_quality: float               # 30%
    best_practice_compliance: float    # 25%
    goal_achievement: float            # 15%
  findings:
    - id: F-NNN
      severity: Critical|Major|Minor|Info
      description: str
      suggested_fix: str
      agent_responsible: str
  recommendation: APPROVE|REWORK|ESCALATE

# === Phase A: SoloCoder 验收决策 ===
SoloCoder Decision:
  IF verdict == PASS AND score >= 0.85:
    → APPROVE
    Step 1: Knowledge Graph Memory 更新 (Task COMPLETED)
    Step 2: AGENTS.md 文档更新 (Phase 进度 / 新资产记录)
    Step 3: 向用户交付结果摘要

  ELIF verdict == CONDITIONAL_PASS AND score >= 0.70:
    → APPROVE with Notes (标注技术债)
    (同上，附加 Minor findings 列表)

  ELSE:  # FAIL or Critical
    → REJECT + 编排返工
    分析根因 → 选择策略 (A/B/C/D) → 重试 (最多 N=3)
```

### 返工策略速查

| 策略 | 适用场景 | 操作 |
|------|---------|------|
| **A) 同 Agent 重试** | Minor/Major ≤ 2, 简单修复 | findings→修复指令→原 Agent, retry++ |
| **B) 切换 Agent** | 原 Agent 反复同类失败 | 转交给另一 Agent |
| **C) 拆分任务** | 复杂度过高 | 拆为 N 个子任务分别分派 |
| **D) 放弃上报** | N=3 仍 FAIL 或不可修复 | 根因分析报告 → 通知用户 |

---

*关联参考*: [MCP Agent System](./references/mcp_agent_system.md) | [MCP Tools](./references/mcp_tools.md)
