# UmgMcp Tools (UMG/UI 专用工具参考)

> **权威源**: 本文件定义 UmgMcp 完整工具集
> **版本**: 1.0

---

## 概述

UmgMcp 是专注于 UMG (Unreal Motion Graphics) Widget 系统的 Python FastMCP Server，配合 UE Editor 内的 UmgMcp C++ 插件工作。

**插件路径**: `Plugins/UnrealMotionGraphicsMCP-main/`
**Python 入口**: `Plugins/UnrealMotionGraphicsMCP-main/Resources/Python/UmgMcpServer.py`
**通信方式**: TCP Socket (Python → UE Plugin @ 127.0.0.1:55557) → stdio to Claude
**配置文件**: `.trae/mcp.json` → 服务名 `umg-mcp`

---

## 连接信息

| 配置项 | 值 | 说明 |
|--------|-----|------|
| MCP 服务名 | `umg-mcp` | 在 `.trae/mcp.json` 中注册 |
| 运行命令 | `uv run --directory <Python> UmgMcpServer.py` | 通过 uv 执行 |
| UE 插件端口 | TCP 55557 | UmgMcp UE 插件监听端口 |
| Socket 超时 | 5 秒 | 可通过 env `SOCKET_TIMEOUT` 调整 |
| 启动超时 | 30000ms | MCP server 启动等待时间 |

---

## 核心架构: Active Target 工作流

所有 UMG 操作围绕 **Active Target**（当前活动目标资产）展开。

### 目标解析优先级 (4级回退)
1. **显式参数** — 调用时提供 `asset_path`
2. **缓存目标** — 通过 `set_target_umg_asset()` 设置过
3. **编辑器焦点** — UE Editor 当前聚焦的资产
4. **历史记录** — 最近编辑过的资产

---

## 完整工具分类

### Category 1: Introspection (内省)

| Tool | 描述 | 参数 |
|------|------|------|
| `get_widget_schema` | 获取某类 Widget 的可用属性 Schema | `widget_type: str` (如 "Button", "CanvasPanel") |
| `get_creatable_widget_types` | 可创建的 Widget 类型列表 | 无参数 (返回哲学性指南) |

### Category 2: Attention & Context (上下文管理)

| Tool | 描述 | 参数 |
|------|------|------|
| `get_target_umg_asset` | 获取当前 Active Target 路径 | 无参数 |
| `get_last_edited_umg_asset` | 获取最后编辑的 UMG 资产路径 | 无参数 |
| `get_recently_edited_umg_assets` | 最近编辑历史列表 | `max_count: int = 5` |
| `set_target_umg_asset` | 设置 Active Target (不存在则创建) | `asset_path: str` |
| `set_active_widget` | 设置 Active Widget 作用域 (隐式父级) | `widget_name: str` |

### Category 3: Sensing (感知)

| Tool | 描述 | 参数 |
|------|------|------|
| `get_widget_tree` | 获取完整 Widget 层级树 | 无参数 (作用于 Active Target) |
| `query_widget_properties` | 查询指定 Widget 的属性值 | `widget_name: str`, `properties: List[str]` |
| `get_layout_data` | 计算屏幕空间包围盒坐标 | `resolution_width=1920`, `resolution_height=1080` |
| `check_widget_overlap` | 检测 Widget 视觉重叠 | `widget_names: Optional[List[str]]` |

### Category 4: Action (操作)

| Tool | 描述 | 参数 |
|------|------|------|
| `create_widget` | 创建新 Widget (隐式附加到 Active Widget) | `parent_name: str`, `widget_type: str`, `new_widget_name: str` |
| `set_widget_properties` | 统一属性设置 | `widget_name: str`, `properties: Dict[str, Any]` |
| `delete_widget` | 从树中删除 Widget | `widget_name: str` |
| `reparent_widget` | 移动到新父级 | `widget_name: str`, `new_parent_name: str` |
| `save_asset` | 保存资产到磁盘 | 无参数 |

### Category 5: File Transformation (文件转换)

| Tool | 描述 | 参数 |
|------|------|------|
| `export_umg_to_json` | UMG → JSON 反向导出 | `asset_path: str`, `widget_name="Root"` |
| `apply_layout` | ⭐ HTML/JSON → UMG 批量布局转换 | `layout_content: str` (HTML 或 JSON), `widget_name="Root"` |

**apply_layout 是 UmgMcp 最强大的独有能力**, 支持两种格式:

HTML 格式:
```html
<CanvasPanel>
  <VerticalBox name="Panel" anchors="fill" padding="40">
    <TextBlock name="Title" text="Hello" font-size="32" color="#FFFFFF"/>
    <Button name="Btn_OK">OK</Button>
  </VerticalPanel>
</CanvasPanel>
```

JSON 格式:
```json
{"type": "CanvasPanel", "name": "Root", "children": [
  {"type": "Image", "name": "BG", "brush": "/Game/T_BG"},
  {"type": "Button", "name": "Btn_Confirm"}
]}
```

### Category 6: Blueprint (蓝图图形编辑)

| Tool | 描述 | 参数 |
|------|------|------|
| `compile_blueprint` | 编译 Active Target 或指定 BP | `blueprint_name: str?` |
| `add_step` | 添加执行节点 (PC 自动跳转) | `name: str`, `args?: List[Any]`, `comment?: str`, `input_wires?: Dict` |
| `prepare_value` | 放置数据节点 (PC 不移动) | `name: str`, `args?: List[Any]` |
| `connect_data_to_pin` | 连接引脚 (`NodeID:PinName`) | `source: str`, `target: str` |
| `get_function_nodes` | 列出函数中所有节点 | 无参数 |
| `set_edit_function` | 设置当前编辑函数/图 | `function_name: str` (支持 `Widget.Event`) |
| `set_cursor_node` | 手动设置 Program Counter | `node_id: str` |
| `search_function_library` | 搜索可调用函数 | `query?: str`, `class_name?: str` |
| `add_variable` | 添加成员变量 | `name: str`, `type: str`, `subType?: str` |
| `delete_variable` | 删除成员变量 | `name: str` |
| `delete_node` | 删除节点 | `node_id: str` |
| `get_variables` | 列出变量 | 无参数 |

**Program Counter (PC) 机制**:
- `add_step` 后 PC 自动跳转到新节点
- `prepare_value` 不移动 PC (需手动 connect)
- `set_cursor_node` 用于跳转 (如 If 分支)
- `set_edit_function` 首次创建时 PC=Entry，已存在时 PC=最后节点

### Category 7: Animation Sequencer (Widget 动画)

| Tool | 描述 | 参数 |
|------|------|------|
| `set_animation_scope` | 设置当前动画作用域 | `animation_name: str` |
| `set_widget_scope` | 设置动画中的 Widget 作用域 | `widget_name: str` |
| `get_all_animations` | 列出所有动画 | 无参数 |
| `get_animation_keyframes` | 获取关键帧 | `animation_name: str` |
| `get_animated_widgets` | 受影响的 Widget 列表 | `animation_name: str` |
| `get_animation_full_data` | 完整动画数据 | `animation_name: str` |
| `get_widget_animation_data` | 特定 Widget 动画数据 | `animation_name: str`, `widget_name: str` |
| `create_animation` | 创建新动画 | `animation_name: str` |
| `delete_animation` | 删除动画 | `animation_name: str` |
| `set_property_keys` | 设置关键帧 | `property_name: str`, `keys: List[Dict]` |
| `set_animation_data` | ⭐ 批量 JSON 动画定义 | `tracks: List[Dict]` |
| `remove_property_track` | 移除属性轨道 | `property_name: str` |
| `remove_keys` | 删除特定关键帧 | `property_name: str`, `times: List[float]` |

**支持的属性类型**: Float (RenderOpacity), Vector2D (Translation/Scale), LinearColor (ColorAndOpacity)

### Category 8: Material (UI 材质编辑)

| Tool | 描述 | 参数 |
|------|------|------|
| `material_set_target` | 设定目标材质 (自动创建 UI Domain) | `path: str`, `create_if_not_found=True` |
| `material_define_variable` | 定义接口参数 | `name: str`, `type: str` (Scalar/Vector/Texture) |
| `material_add_node` | 放置节点 (标准名称) | `symbol: str`, `handle?: str` |
| `material_delete` | 通过 handle 删除节点 | `handle: str` |
| `material_connect_nodes` | 自然连接 (A→B) | `from_handle: str`, `to_handle: str` |
| `material_connect_pins` | 精确引脚连接 | `source, source_pin, target, target_pin` |
| `material_set_hlsl_node_io` | HLSL Custom 节点 | `handle, code: str, inputs: List[str]` |
| `material_set_node_properties` | 节点内部属性 | `handle: str`, `properties: Dict` |
| `material_compile_asset` | 显式编译材质 | 无参数 |
| `material_get_pins` | 查询引脚 | `handle?: str` (Master=全部) |
| `material_get_graph` | 获取完整图拓扑 | 无参数 |

### Category 9: Editor & Level (辅助)

| Tool | 描述 | 参数 |
|------|------|------|
| `refresh_asset_registry` | 强制刷新资产注册表 | `paths?: List[str]` |
| `list_assets` | 列出资产 | `class_name?, package_path?, max_count=50` |
| `get_actors_in_level` | 关卡 Actor 列表 | 无参数 |
| `spawn_actor` | 生成基础 Actor | `actor_type, name, location?, rotation?` |

---

## 典型工作流程

### 流程 1: 从零构建 UI

```python
# Step 1: 建立目标
set_target_umg_asset("/Game/UI/WBP_CarConfigurator")

# Step 2: 用 HTML 批量构建主体结构
apply_layout("""
<CanvasPanel name="Root">
  <Image name="BG" brush="/Game/UI/Textures/T_Background" anchors="fill"/>
  <VerticalBox name="MainContainer" anchors="fill" padding="40">
    <TextBlock name="Title" text="Audi R7S" font-size="48" color="#C0C0C0"/>
    <UniformGridPanel name="OptionGrid" columns="3"/>
  </VerticalPanel>
</CanvasPanel>
""")

# Step 3: 精细调整
set_active_widget("OptionGrid")
for option in ["Exterior", "Interior", "Performance"]:
    create_widget("UniformGridPanel", f"Card_{option}")
    set_widget_properties(f"Card_{option}", {"Width": 300, "Height": 200})

save_asset()
```

### 流程 2: Widget 动画制作

```python
# Step 1: 创建入场动画
create_animation("Intro_Anim")

# Step 2: 为标题制作滑入+淡入效果
set_widget_scope("Title")
set_animation_data([
    {"property": "RenderTransform.Translation",
     "keys": [{"time": 0, "value": [0, -100]}, {"time": 0.6, "value": [0, 0]}]},
    {"property": "RenderOpacity",
     "keys": [{"time": 0, "value": 0}, {"time": 0.4, "value": 1.0}]}
])
```

---

## See Also

- [McpAutomationBridge Tools](./mcp_tools_mab.md) — 引擎级工具
- [工具重叠对比](./mcp_tools_overlap.md) — MAB vs UMG 重叠分析
- [MCP Agent System](./mcp_agent_system.md)

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
