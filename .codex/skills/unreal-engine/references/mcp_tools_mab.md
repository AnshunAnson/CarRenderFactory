# McpAutomationBridge Tools (MAB 工具参考)

> **权威源**: 本文件定义 McpAutomationBridge 完整工具集
> **版本**: 2.0 (完整版)
> **插件版本**: 0.1.3
> **更新日期**: 2026-04-02

---

## 概述

McpAutomationBridge 是 Unreal Engine 编辑器的 WebSocket 自动化桥接插件，集成在 `d:\UE\Project\AudiR7S\AudiR7S\Plugins\McpAutomationBridge`。

该插件通过 WebSocket 协议暴露 **200+** 编辑器操作，覆盖 40 个功能领域。

### 核心特性

| 特性 | 说明 |
|------|------|
| **通信协议** | WebSocket (支持 TLS/SSL) |
| **Handler 数量** | 56 个领域处理器文件 |
| **自动化操作** | 200+ 个 |
| **支持引擎** | UE 5.0 - 5.7 |
| **平台** | Win64, Mac, Linux |
| **线程模型** | WebSocket 线程接收 → 游戏线程执行 |

---

## 连接信息

| 配置项 | 默认值 | 说明 |
|--------|--------|------|
| Host | `localhost` | WebSocket 监听地址 |
| Port | `9876` | WebSocket 监听端口 |
| Protocol | `ws://` | 明文传输 |
| TLS | 可选 | 支持 `wss://` |
| Token | 可选 | Capability Token 认证 |

可通过 `UMcpAutomationBridgeSettings` 配置。

---

## MCP 协议格式

### 请求格式
```json
{
  "action": "action_name",
  "request_id": "uuid-string",
  "payload": {
    "param1": "value1",
    "param2": 123
  }
}
```

### 响应格式
```json
{
  "success": true,
  "request_id": "uuid-string",
  "result": { ... },
  "error": null
}
```

### 错误响应
```json
{
  "success": false,
  "request_id": "uuid-string",
  "result": null,
  "error": {
    "code": "ERROR_CODE",
    "message": "Human readable message"
  }
}
```

### 进度更新 (长时操作)
```json
{
  "type": "progress",
  "request_id": "uuid-string",
  "percent": 50.0,
  "message": "Processing...",
  "still_working": true
}
```

---

## 安全机制

| 机制 | 说明 |
|------|------|
| **Capability Token** | 可选的认证令牌，在握手时验证 |
| **速率限制** | 600 messages/min, 120 automation requests/min |
| **路径清理** | `SanitizeProjectRelativePath()` 防止路径遍历攻击 |
| **命令注入防护** | 分号 sanitization 阻止命令注入 |
| **Loopback 绑定** | 默认仅绑定 localhost，防止远程访问 |
| **握手验证** | 必须完成握手后才能发送自动化请求 |

---

## 线程模型

```
WebSocket Thread          Game Thread
      │                        │
      │ 接收消息                │
      │ 解析 JSON               │
      │ 入队 PendingRequests    │
      │ ──────────────────────►│
      │                        │ 派发到 Handler
      │                        │ 执行 UE 操作
      │                        │ 捕获错误/日志
      │                        │ ───────────────►
      │                        │
      │◄──────────────────────│
      │ 发送响应                │
```

关键点：
- WebSocket 帧处理在独立线程
- Handler 执行在游戏线程 (通过 `AsyncTask`)
- 请求队列处理 pending 请求
- 错误捕获设备捕获引擎级错误

---

## 版本兼容性

| UE 版本 | 兼容性 | 特殊处理 |
|---------|--------|---------|
| 5.0 | ✅ | 部分 API 回退 |
| 5.1 | ✅ | ControlRig 头文件在 Private |
| 5.2 | ✅ | - |
| 5.3 | ✅ | - |
| 5.4 | ✅ | - |
| 5.5 | ✅ | GeometryScript AppendCapsule 兼容 |
| 5.6 | ✅ | - |
| 5.7 | ✅ | SCS 保存、GetProtocolType API、Niagara graph init |

---

## Handler 领域索引

### 1. Core Handlers

核心属性和对象操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `execute_editor_function` | 执行编辑器函数 | `function_name`, `params` |
| `set_property` | 设置对象属性 | `object_path`, `property_name`, `value` |
| `get_property` | 获取对象属性 | `object_path`, `property_name` |
| `list_objects` | 列出对象 | `class_filter`, `path` |

### 2. Container Handlers

容器操作 (Array/Map/Set)。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `array_append` | 数组追加 | `array_property`, `elements` |
| `array_remove` | 数组移除 | `array_property`, `index` |
| `array_insert` | 数组插入 | `array_property`, `index`, `element` |
| `array_get_element` | 获取元素 | `array_property`, `index` |
| `array_set_element` | 设置元素 | `array_property`, `index`, `value` |
| `array_clear` | 清空数组 | `array_property` |
| `map_set_value` | Map 设值 | `map_property`, `key`, `value` |
| `map_get_value` | Map 取值 | `map_property`, `key` |
| `map_remove_key` | Map 移除 | `map_property`, `key` |
| `map_has_key` | Map 检查 | `map_property`, `key` |
| `map_get_keys` | Map 键列表 | `map_property` |
| `map_clear` | 清空 Map | `map_property` |
| `set_add` | Set 添加 | `set_property`, `element` |
| `set_remove` | Set 移除 | `set_property`, `element` |
| `set_contains` | Set 检查 | `set_property`, `element` |
| `set_clear` | 清空 Set | `set_property` |

### 3. Asset Handlers

资产管理操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `import_asset` | 导入资产 | `source_path`, `dest_path` |
| `duplicate_asset` | 复制资产 | `source_asset`, `dest_folder` |
| `rename_asset` | 重命名资产 | `asset_path`, `new_name` |
| `move_asset` | 移动资产 | `asset_path`, `dest_folder` |
| `delete_assets` | 删除资产 | `asset_paths[]` |
| `list_assets` | 列出资产 | `folder`, `filter`, `class` |
| `get_asset` | 获取资产信息 | `asset_path` |
| `asset_exists` | 检查资产存在 | `asset_path` |
| `search_assets` | 搜索资产 | `search_text`, `filter` |
| `get_dependencies` | 获取依赖 | `asset_path` |
| `get_references` | 获取引用 | `asset_path` |
| `set_tags` | 设置标签 | `asset_path`, `tags[]` |
| `set_metadata` | 设置元数据 | `asset_path`, `key`, `value` |
| `get_metadata` | 获取元数据 | `asset_path` |
| `export_asset` | 导出资产 | `asset_path`, `export_path` |
| `generate_thumbnail` | 生成缩略图 | `asset_path` |
| `fixup_redirectors` | 修复重定向器 | `package_paths` |
| `bulk_rename_assets` | 批量重命名 | `operations[]` |
| `bulk_delete_assets` | 批量删除 | `asset_paths[]` |
| `nanite_rebuild_mesh` | Nanite 重建 | `static_mesh_path` |

### 4. Source Control Handlers

源控制操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `source_control_checkout` | 检出 | `asset_paths[]` |
| `source_control_submit` | 提交 | `asset_paths[]`, `message` |
| `source_control_get_state` | 获取状态 | `asset_path` |
| `source_control_enable` | 启用源控制 | `provider` |

### 5. Actor Handlers

Actor 生命周期和属性控制。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `spawn_actor` | 生成 Actor | `asset_path`, `location`, `rotation`, `scale` |
| `spawn_blueprint` | 从蓝图生成 | `blueprint_path`, `location`, `rotation` |
| `delete_actor` | 删除 Actor | `actor_name` 或 `actor_path` |
| `duplicate_actor` | 复制 Actor | `source_actor`, `offset` |
| `set_actor_transform` | 设置变换 | `actor_name`, `location`, `rotation`, `scale` |
| `get_actor_transform` | 获取变换 | `actor_name` |
| `attach_actor` | 挂载 Actor | `child_actor`, `parent_actor`, `socket_name` |
| `detach_actor` | 分离 Actor | `actor_name` |
| `find_actor_by_tag` | 按标签查找 | `tag` |
| `find_actor_by_name` | 按名称查找 | `name` |
| `find_actor_by_class` | 按类查找 | `class_name` |
| `add_tag` | 添加标签 | `actor_name`, `tag` |
| `remove_tag` | 移除标签 | `actor_name`, `tag` |
| `set_visibility` | 设置可见性 | `actor_name`, `bVisible` |
| `set_collision` | 设置碰撞 | `actor_name`, `collision_type` |
| `apply_force` | 应用物理力 | `actor_name`, `force`, `location` |
| `add_component` | 添加组件 | `actor_name`, `component_class`, `attach_to` |
| `remove_component` | 移除组件 | `actor_name`, `component_name` |
| `get_components` | 获取组件列表 | `actor_name` |
| `set_component_property` | 设置组件属性 | `actor_name`, `component_name`, `property`, `value` |
| `get_component_property` | 获取组件属性 | `actor_name`, `component_name`, `property` |
| `set_blueprint_variables` | 设置 BP 变量 | `actor_name`, `variables{}` |
| `call_function` | 调用函数 | `actor_name`, `function_name`, `params` |
| `create_snapshot` | 创建快照 | `actor_name` |
| `restore_snapshot` | 恢复快照 | `actor_name` |
| `export_actor` | 导出 Actor | `actor_name`, `export_path` |
| `get_bounding_box` | 获取包围盒 | `actor_name` |
| `get_actor_metadata` | 获取元数据 | `actor_name` |
| `list_actors` | 列出所有 Actor | `filter` |
| `get_actor` | 获取 Actor 详情 | `actor_name` |

### 6. Editor Handlers

编辑器状态和 PIE 控制。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `play_in_editor` | 启动 PIE | `bImmersive`, `WorldType` |
| `stop_play` | 停止 PIE | - |
| `pause_pie` | 暂停 PIE | - |
| `resume_pie` | 恢复 PIE | - |
| `eject_pie` | 弹出 PIE | - |
| `possess_pie` | 占据 PIE | - |
| `step_frame` | 单帧步进 | - |
| `set_camera_view` | 设置视口相机 | `location`, `rotation`, `FOV` |
| `focus_viewport_on_actor` | 聚焦 Actor | `actor_name` |
| `get_viewport_screenshot` | 获取截图 | `bInGameView` |
| `capture_screenshot` | 捕获截图 | `filename` |
| `simulate_input` | 模拟输入 | `input_type`, `key`, `delta` |
| `execute_editor_command` | 执行编辑器命令 | `command` |
| `console_command` | 控制台命令 | `command` |
| `open_asset` | 打开资产 | `asset_path` |
| `close_asset` | 关闭资产 | `asset_path` |
| `save_all` | 保存全部 | - |
| `undo` | 撤销 | - |
| `redo` | 重做 | - |
| `set_view_mode` | 设置视图模式 | `view_mode` |
| `set_editor_mode` | 设置编辑器模式 | `editor_mode` |
| `set_preferences` | 设置偏好 | `preferences{}` |
| `set_viewport_realtime` | 设置实时 | `bRealtime` |
| `set_game_view` | 设置游戏视图 | `bGameView` |
| `set_immersive_mode` | 设置沉浸模式 | `bImmersive` |
| `set_fixed_delta_time` | 设置固定帧时间 | `delta_time` |
| `create_bookmark` | 创建书签 | `bookmark_name`, `location`, `rotation` |
| `jump_to_bookmark` | 跳转书签 | `bookmark_name` |
| `start_recording` | 开始录制 | - |
| `stop_recording` | 停止录制 | - |
| `show_stats` | 显示统计 | `stat_name` |
| `hide_stats` | 隐藏统计 | `stat_name` |
| `open_level` | 打开关卡 | `level_path` |

### 7. Level Handlers

关卡管理操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `open_level` | 打开关卡 | `level_path`, `bLoadAsTempWorld` |
| `save_level` | 保存关卡 | `level_path` |
| `level_exists` | 检查关卡存在 | `level_path` |
| `get_current_level` | 获取当前关卡 | - |
| `get_levels` | 获取关卡列表 | - |
| `create_folder` | 创建文件夹 | `folder_path` |

### 8. Level Structure Handlers

关卡结构和 World Partition。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_sublevel` | 创建子关卡 | `sublevel_name`, `parent_level` |
| `add_external_actor` | 添加外部 Actor | `actor_path`, `level_path` |
| `create_streaming_reference` | 创建流式引用 | `external_actor_package` |
| `get_level_structure` | 获取关卡结构 | `level_path` |

### 9. Blueprint Handlers

蓝图创建和编辑。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_blueprint` | 创建蓝图 | `asset_name`, `parent_class`, `package_path` |
| `spawn_from_blueprint` | 从蓝图生成 | `blueprint_path`, `location`, `rotation` |
| `get_blueprint_variables` | 获取变量 | `blueprint_path` |
| `set_blueprint_variable` | 设置变量 | `actor_name`, `variable_name`, `value` |
| `add_variable` | 添加变量 | `blueprint_path`, `var_name`, `var_type` |
| `delete_variable` | 删除变量 | `blueprint_path`, `var_name` |
| `get_variables` | 列出变量 | `blueprint_path` |
| `compile_blueprint` | 编译蓝图 | `blueprint_path` |
| `list_blueprints` | 列出蓝图 | `filter` |

### 10. Blueprint Graph Handlers

蓝图图形编辑。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `set_edit_function` | 设置编辑函数 | `function_name` |
| `add_step` | 添加执行节点 | `node_type`, `args`, `comment` |
| `prepare_value` | 放置数据节点 | `node_type`, `args` |
| `connect_data_to_pin` | 连接引脚 | `source`, `target` |
| `get_function_nodes` | 列出节点 | `function_name` |
| `set_cursor_node` | 设置光标 | `node_id` |
| `search_function_library` | 搜索函数 | `query`, `class` |
| `delete_node` | 删除节点 | `node_id` |

### 11. SCS Handlers

Blueprint SCS 组件树操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `get_blueprint_scs` | 获取组件树 | `blueprint_path` |
| `add_scs_component` | 添加组件 | `blueprint_path`, `component_class`, `name`, `mesh_path`, `material_path` |
| `remove_scs_component` | 移除组件 | `blueprint_path`, `component_name` |
| `reparent_scs_component` | 重父级 | `blueprint_path`, `component_name`, `new_parent` |
| `set_scs_component_transform` | 设置变换 | `blueprint_path`, `component_name`, `transform` |

### 12. Sequence Handlers

Level Sequence 操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `sequence_create` | 创建序列 | `asset_name`, `package_path` |
| `sequence_open` | 打开序列 | `sequence_path` |
| `sequence_save` | 保存序列 | `sequence_path` |
| `sequence_close` | 关闭序列 | - |
| `sequence_play` | 播放序列 | `sequence_path` |
| `sequence_pause` | 暂停序列 | - |
| `sequence_stop` | 停止序列 | - |
| `sequence_set_time` | 设置时间 | `time` |
| `sequence_set_playback_speed` | 设置速度 | `speed` |
| `sequence_set_display_rate` | 设置帧率 | `frame_rate` |
| `sequence_set_tick_resolution` | 设置分辨率 | `resolution` |
| `sequence_set_view_range` | 设置视图范围 | `start`, `end` |
| `sequence_add_actor` | 添加 Actor | `actor_name` |
| `sequence_add_actors` | 批量添加 Actor | `actor_names[]` |
| `sequence_add_spawnable` | 添加可生成 | `asset_path` |
| `sequence_remove_actors` | 移除 Actor | `actor_names[]` |
| `sequence_get_bindings` | 获取绑定 | - |
| `sequence_get_properties` | 获取属性 | - |
| `sequence_get_metadata` | 获取元数据 | - |
| `sequence_add_camera` | 添加相机 | `camera_name` |
| `sequence_add_track` | 添加轨道 | `track_type`, `object_binding` |
| `sequence_remove_track` | 移除轨道 | `track_name` |
| `sequence_set_track_muted` | 静音轨道 | `track_name`, `bMuted` |
| `sequence_set_track_solo` | 独奏轨道 | `track_name`, `bSolo` |
| `sequence_set_track_locked` | 锁定轨道 | `track_name`, `bLocked` |
| `sequence_add_keyframe` | 添加关键帧 | `track_name`, `time`, `value` |
| `sequence_add_section` | 添加分段 | `section_name` |
| `sequence_duplicate` | 复制序列 | `dest_path` |
| `sequence_rename` | 重命名 | `new_name` |
| `sequence_delete` | 删除序列 | - |
| `sequence_list` | 列出序列 | - |

### 13. Animation Handlers

骨骼动画系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_animation_bp` | 创建动画蓝图 | `asset_name`, `skeleton_path`, `package_path` |
| `create_blend_space` | 创建混合空间 | `asset_name`, `skeleton_path`, `dimensions` |
| `create_blend_tree` | 创建混合树 | `anim_bp_path`, `blend_tree_name` |
| `create_state_machine` | 创建状态机 | `anim_bp_path`, `state_machine_name` |
| `create_procedural_anim` | 程序化动画 | `skeleton_path`, `keyframes` |
| `play_anim_montage` | 播放蒙太奇 | `actor_name`, `montage_path` |
| `setup_ragdoll` | 设置布娃娃 | `actor_name` |
| `activate_ragdoll` | 激活布娃娃 | `actor_name` |
| `cleanup_anim_refs` | 清理动画引用 | `asset_path` |

### 14. Skeleton Handlers

骨骼、Socket、物理资产。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `list_bones` | 列出骨骼 | `skeleton_path` |
| `list_sockets` | 列出 Socket | `skeleton_path` |
| `create_socket` | 创建 Socket | `skeleton_path`, `bone_name`, `socket_name` |
| `configure_socket` | 配置 Socket | `socket_path`, `transform` |
| `delete_socket` | 删除 Socket | `socket_path` |
| `rename_bone` | 重命名骨骼 | `skeleton_path`, `old_name`, `new_name` |
| `set_bone_transform` | 设置骨骼变换 | `skeleton_path`, `bone_name`, `transform` |
| `get_bone_transform` | 获取骨骼变换 | `skeleton_path`, `bone_name` |
| `create_virtual_bone` | 创建虚拟骨骼 | `skeleton_path`, `virtual_bone_name` |
| `list_virtual_bones` | 列出虚拟骨骼 | `skeleton_path` |
| `delete_virtual_bone` | 删除虚拟骨骼 | `skeleton_path`, `virtual_bone_name` |
| `create_physics_asset` | 创建物理资产 | `skeleton_path`, `asset_name` |
| `get_physics_asset_info` | 获取物理资产信息 | `physics_asset_path` |
| `list_physics_bodies` | 列出物理体 | `physics_asset_path` |
| `add_physics_body` | 添加物理体 | `physics_asset_path`, `bone_name` |
| `configure_physics_body` | 配置物理体 | `physics_asset_path`, `body_name`, `config` |
| `remove_physics_body` | 移除物理体 | `physics_asset_path`, `body_name` |
| `add_physics_constraint` | 添加约束 | `physics_asset_path`, `constraint_config` |
| `configure_constraint_limits` | 配置约束限制 | `constraint_path`, `limits` |
| `create_morph_target` | 创建变形目标 | `skeletal_mesh_path`, `morph_name` |
| `set_morph_target_deltas` | 设置变形增量 | `morph_target_path`, `deltas` |
| `import_morph_targets` | 导入变形目标 | `skeletal_mesh_path`, `source_path` |
| `list_morph_targets` | 列出变形目标 | `skeletal_mesh_path` |
| `set_morph_target_value` | 设置变形值 | `actor_name`, `morph_name`, `value` |
| `delete_morph_target` | 删除变形目标 | `skeletal_mesh_path`, `morph_name` |
| `normalize_weights` | 规范化权重 | `skeletal_mesh_path` |
| `prune_weights` | 修剪权重 | `skeletal_mesh_path`, `threshold` |
| `bind_cloth_to_skeletal_mesh` | 绑定布料 | `skeletal_mesh_path`, `cloth_asset` |
| `assign_cloth_asset` | 分配布料资产 | `skeletal_mesh_path`, `cloth_asset` |
| `set_physics_asset` | 设置物理资产 | `skeletal_mesh_path`, `physics_asset` |

### 15. Material Handlers

材质创建和编辑。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_material` | 创建材质 | `material_name`, `package_path`, `domain` |
| `create_material_instance` | 创建 MI | `parent_material`, `inst_name` |
| `set_blend_mode` | 设置混合模式 | `material_path`, `blend_mode` |
| `set_shading_model` | 设置着色模型 | `material_path`, `shading_model` |
| `set_material_domain` | 设置材质域 | `material_path`, `domain` |
| `add_scalar_parameter` | 添加标量参数 | `material_path`, `param_name`, `default` |
| `add_vector_parameter` | 添加向量参数 | `material_path`, `param_name`, `default` |
| `add_texture_parameter` | 添加纹理参数 | `material_path`, `param_name` |
| `add_static_switch_parameter` | 添加静态开关 | `material_path`, `param_name`, `default` |
| `set_scalar_parameter_value` | 设置 MI 标量 | `mi_path`, `param_name`, `value` |
| `set_vector_parameter_value` | 设置 MI 向量 | `mi_path`, `param_name`, `value` |
| `set_texture_parameter_value` | 设置 MI 纹理 | `mi_path`, `param_name`, `texture_path` |
| `create_material_function` | 创建 MF | `function_name`, `package_path` |
| `add_function_input` | 添加 MF 输入 | `mf_path`, `input_name`, `input_type` |
| `add_function_output` | 添加 MF 输出 | `mf_path`, `output_name` |
| `use_material_function` | 使用 MF | `material_path`, `mf_path` |
| `create_landscape_material` | 创建地形材质 | `material_name`, `package_path` |
| `create_decal_material` | 创建贴花材质 | `material_name`, `package_path` |
| `create_post_process_material` | 创建后处理材质 | `material_name`, `package_path` |
| `list_material_instances` | 列出 MI | `parent_material` |
| `reset_instance_parameters` | 重置 MI 参数 | `mi_path` |
| `get_material_stats` | 获取材质统计 | `material_path` |
| `rebuild_material` | 重建材质 | `material_path` |

### 16. Material Graph Handlers

材质节点图操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `add_material_node` | 添加节点 | `material_path`, `node_type`, `position` |
| `add_texture_sample` | 添加纹理采样 | `material_path`, `texture_path`, `position` |
| `add_material_expression` | 添加表达式 | `material_path`, `expression_type`, `position` |
| `connect_material_pins` | 连接引脚 | `material_path`, `output_pin`, `input_pin` |
| `disconnect_material_pins` | 断开引脚 | `material_path`, `pin` |
| `remove_material_node` | 移除节点 | `material_path`, `node_guid` |
| `break_material_connections` | 断开连接 | `material_path`, `node_guid` |
| `get_material_node_details` | 获取节点详情 | `material_path`, `node_guid` |
| `batch_create_material_nodes` | 批量创建 | `material_path`, `nodes[]` |
| `add_custom_expression` | 添加 HLSL | `material_path`, `code`, `inputs` |

### 17. Texture Handlers

纹理操作。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `import_texture` | 导入纹理 | `source_path`, `dest_path` |
| `set_texture_property` | 设置属性 | `texture_path`, `property`, `value` |
| `compress_texture` | 压缩纹理 | `texture_path`, `compression_settings` |
| `generate_mips` | 生成 Mips | `texture_path` |

### 18. Niagara Handlers

Niagara VFX 系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_niagara_system` | 创建系统 | `system_name`, `template_path` |
| `create_niagara_emitter` | 创建发射器 | `emitter_name`, `system_path` |
| `create_niagara_ribbon` | 创建飘带 | `system_name`, `ribbon_name` |
| `spawn_niagara_actor` | 生成 Actor | `system_path`, `location` |
| `modify_niagara_parameter` | 修改参数 | `actor_name`, `parameter_name`, `value` |

### 19. Niagara Graph Handlers

Niagara 图形编辑。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `niagara_graph_add_node` | 添加节点 | `system_path`, `node_type`, `module` |
| `niagara_graph_connect` | 连接模块 | `system_path`, `from_module`, `to_module` |

### 20. Audio Handlers

音频系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_sound` | 创建声音 | `sound_name`, `package_path` |
| `create_dialogue_voice` | 创建对话声音 | `voice_name`, `speaker` |
| `create_dialogue_wave` | 创建对话波形 | `wave_name`, `dialogue_voice` |
| `set_dialogue_context` | 设置对话上下文 | `dialogue_wave`, `context` |
| `create_reverb_effect` | 创建混响 | `effect_name`, `settings` |
| `create_source_effect_chain` | 创建源效果链 | `chain_name` |
| `add_source_effect` | 添加源效果 | `chain_path`, `effect_type` |
| `create_submix_effect` | 创建子混音效果 | `submix_path`, `effect_type` |

### 21. Lighting Handlers

光照系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `setup_lighting` | 设置光照 | `light_type`, `intensity`, `color` |
| `bake_lightmap` | 烘焙光照 | `actor_path`, `settings` |
| `set_light_property` | 设置灯光属性 | `actor_name`, `property`, `value` |

### 22. Environment Handlers

地形和植被。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_landscape` | 创建地形 | `config` |
| `create_landscape_grass_type` | 创建草地类型 | `landscape_path`, `grass_type` |
| `edit_landscape` | 编辑地形 | `operation`, `params` |
| `modify_heightmap` | 修改高度图 | `landscape_path`, `brush`, `strength` |
| `paint_landscape_layer` | 绘制地形层 | `landscape_path`, `layer_name`, `brush` |
| `sculpt_landscape` | 雕刻地形 | `landscape_path`, `brush`, `strength` |
| `set_landscape_material` | 设置地形材质 | `landscape_path`, `material_path` |
| `paint_foliage` | 绘制植被 | `foliage_type`, `density` |
| `add_foliage_instances` | 添加植被实例 | `foliage_type`, `locations[]` |
| `get_foliage_instances` | 获取植被实例 | `foliage_type` |
| `remove_foliage` | 移除植被 | `foliage_type`, `filter` |
| `add_foliage_type` | 添加植被类型 | `foliage_type`, `mesh_path` |
| `create_procedural_terrain` | 程序化地形 | `params` |
| `create_procedural_foliage` | 程序化植被 | `params` |
| `generate_lods` | 生成 LOD | `static_mesh_path`, `lod_count` |

### 23. AI Handlers

AI 系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_ai_controller` | 创建 AI 控制器 | `controller_name` |
| `setup_behavior_tree` | 设置行为树 | `ai_controller`, `behavior_tree_path` |
| `create_behavior_tree` | 创建行为树 | `tree_name`, `package_path` |
| `configure_blackboard` | 配置黑板 | `behavior_tree_path`, `keys` |

### 24. GAS Handlers

Gameplay Ability System。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_gameplay_ability` | 创建能力 | `ability_name`, `package_path` |
| `create_gameplay_effect` | 创建效果 | `effect_name`, `package_path` |
| `create_attribute_set` | 创建属性集 | `attribute_set_name` |
| `setup_ability_system` | 设置能力系统 | `actor_name`, `ability_system_config` |

### 25. Character Handlers

角色系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `configure_character_movement` | 配置角色移动 | `actor_name`, `movement_config` |
| `set_character_state` | 设置角色状态 | `actor_name`, `state` |

### 26. Combat Handlers

战斗系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_weapon` | 创建武器 | `weapon_name`, `weapon_config` |
| `setup_damage_system` | 设置伤害系统 | `actor_name`, `damage_config` |

### 27. Inventory Handlers

物品系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_inventory_item` | 创建物品 | `item_name`, `item_data` |
| `setup_inventory` | 设置物品栏 | `actor_name`, `inventory_config` |

### 28. Interaction Handlers

交互系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_interaction_component` | 创建交互组件 | `actor_name`, `component_config` |
| `configure_interaction_trace` | 配置交互追踪 | `actor_name`, `trace_config` |
| `configure_interaction_widget` | 配置交互 Widget | `actor_name`, `widget_config` |
| `create_door_actor` | 创建门 Actor | `door_name`, `door_config` |
| `create_switch_actor` | 创建开关 Actor | `switch_name`, `switch_config` |
| `create_chest_actor` | 创建宝箱 Actor | `chest_name`, `chest_config` |

### 29. Input Handlers

Enhanced Input。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_input_action` | 创建输入动作 | `action_name`, `action_type` |
| `create_input_mapping_context` | 创建映射上下文 | `context_name` |
| `setup_enhanced_input` | 设置增强输入 | `actor_name`, `input_config` |

### 30. UI Handlers

UI 和 Widget。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_widget` | 创建 Widget | `widget_type`, `widget_name`, `parent` |
| `set_widget_property` | 设置属性 | `widget_name`, `property`, `value` |
| `bind_widget_text` | 绑定文本 | `widget_name`, `text_binding` |
| `bind_widget_color` | 绑定颜色 | `widget_name`, `color_binding` |
| `bind_on_clicked` | 绑定点击 | `widget_name`, `callback` |
| `bind_on_hovered` | 绑定悬停 | `widget_name`, `callback` |
| `bind_on_visibility` | 绑定可见性 | `widget_name`, `visibility_binding` |
| `bind_on_enabled` | 绑定启用 | `widget_name`, `enabled_binding` |

### 31. Widget Authoring Handlers

Widget 创作工具。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `add_button` | 添加按钮 | `parent`, `button_name`, `text` |
| `add_text_block` | 添加文本 | `parent`, `text_name`, `text` |
| `add_image` | 添加图片 | `parent`, `image_name`, `brush` |
| `add_panel` | 添加面板 | `parent`, `panel_name`, `panel_type` |

### 32. Networking Handlers

多人游戏。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `configure_replication` | 配置复制 | `actor_name`, `replication_config` |
| `setup_networking` | 设置网络 | `network_config` |

### 33. GameFramework Handlers

游戏框架。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_game_mode` | 创建游戏模式 | `game_mode_name`, `package_path` |
| `create_player_controller` | 创建玩家控制器 | `controller_name` |
| `create_hud` | 创建 HUD | `hud_name` |
| `setup_game_framework` | 设置游戏框架 | `framework_config` |

### 34. Sessions Handlers

会话和本地多人。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_session` | 创建会话 | `session_config` |
| `join_session` | 加入会话 | `session_id` |
| `setup_local_multiplayer` | 设置本地多人 | `player_count` |

### 35. Volume Handlers

体积和区域。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_trigger_volume` | 创建触发体积 | `volume_name`, `transform` |
| `create_physics_volume` | 创建物理体积 | `volume_name`, `physics_config` |
| `configure_volume` | 配置体积 | `volume_name`, `volume_config` |

### 36. Navigation Handlers

导航系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `rebuild_nav_mesh` | 重建导航网格 | `nav_mesh_path` |
| `configure_navigation` | 配置导航 | `navigation_config` |
| `move_ai_to_location` | AI 移动到位置 | `ai_controller`, `location` |

### 37. Spline Handlers

样条系统。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `create_spline_component` | 创建样条组件 | `actor_name`, `spline_name` |
| `add_spline_point` | 添加样条点 | `spline_path`, `point` |
| `configure_spline` | 配置样条 | `spline_path`, `spline_config` |

### 38. Geometry Handlers

Geometry Script。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `geometry_append_box` | 添加立方体 | `mesh_path`, `dimensions` |
| `geometry_append_sphere` | 添加球体 | `mesh_path`, `radius` |
| `geometry_append_capsule` | 添加胶囊体 | `mesh_path`, `radius`, `half_height` |
| `geometry_boolean` | 布尔运算 | `mesh_a`, `mesh_b`, `operation` |

### 39. World Partition Handlers

World Partition。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `get_world_partition_info` | 获取 WP 信息 | `level_path` |
| `configure_world_partition` | 配置 WP | `level_path`, `wp_config` |

### 40. Performance Handlers

性能分析。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `get_performance_stats` | 获取性能统计 | `stat_type` |
| `start_profiling` | 开始分析 | - |
| `stop_profiling` | 停止分析 | - |

### 41. Render Handlers

渲染设置。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `set_render_setting` | 设置渲染设置 | `setting_name`, `value` |
| `capture_render` | 捕获渲染 | `capture_config` |

### 42. Console Command Handlers

控制台命令。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `execute_console_command` | 执行命令 | `command` |
| `execute_console_commands_batch` | 批量执行 | `commands[]` |

### 43. System Control Handlers

系统控制。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `system_shutdown` | 关闭系统 | - |
| `system_restart` | 重启系统 | - |

### 44. Debug Handlers

调试可视化。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `draw_debug_line` | 绘制调试线 | `start`, `end`, `color` |
| `draw_debug_box` | 绘制调试盒 | `center`, `extent`, `color` |
| `draw_debug_sphere` | 绘制调试球 | `center`, `radius`, `color` |
| `clear_debug_drawings` | 清除调试绘制 | - |

### 45. Log Handlers

日志捕获。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `get_logs` | 获取日志 | `filter`, `count` |
| `capture_automation_log` | 捕获日志 | - |
| `clear_logs` | 清除日志 | - |

### 46. Test Handlers

自动化测试。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `run_test` | 运行测试 | `test_name` |
| `run_all_tests` | 运行所有测试 | `filter` |
| `get_test_results` | 获取测试结果 | `test_id` |

### 47. Pipeline Handlers

构建和打包。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `build_project` | 构建项目 | `target`, `configuration` |
| `cook_content` | 烹饪内容 | `cook_config` |
| `package_project` | 打包项目 | `package_config` |

### 48. Insights Handlers

性能洞察。

| Action | 描述 | 关键参数 |
|--------|------|----------|
| `start_insights_trace` | 开始追踪 | `trace_config` |
| `stop_insights_trace` | 停止追踪 | - |
| `analyze_trace` | 分析追踪 | `trace_path` |

---

## 错误码参考

| 错误码 | 说明 |
|--------|------|
| `INVALID_ACTION` | 未知的 action 名称 |
| `INVALID_PAYLOAD` | JSON 解析失败或缺少必需参数 |
| `ASSET_NOT_FOUND` | 指定的资产路径不存在 |
| `ACTOR_NOT_FOUND` | 指定的 Actor 不存在 |
| `BLUEPRINT_ERROR` | 蓝图编译失败 |
| `MATERIAL_ERROR` | 材质编译失败 |
| `PERMISSION_DENIED` | Token 验证失败或权限不足 |
| `RATE_LIMITED` | 超过速率限制 |
| `INTERNAL_ERROR` | 内部引擎错误 |
| `LANDSCAPE_NOT_FOUND` | 地形未找到 |
| `INVALID_PATH` | 路径遍历攻击检测 |

---

## 最佳实践

### 1. 安全保存资产
```cpp
// 使用安全保存助手 (UE 5.7+)
McpSafeAssetSave(Asset);  // 而非 UPackage::SavePackage()
```

### 2. SCS 组件创建
```cpp
// 正确的 SCS 所有权
SCS->CreateNode(ComponentClass);  // 组件模板由 SCS_Node 拥有
```

### 3. 路径安全
```cpp
// 始终使用项目相对路径
FString SafePath = SanitizeProjectRelativePath(UserPath);
```

### 4. 错误处理
```cpp
// 检查 Handler 返回值
if (!Handler.Execute(RequestId, Action, Payload, Socket)) {
    SendAutomationError(Socket, RequestId, "Handler failed", "HANDLER_ERROR");
}
```

---

## See Also

- [UmgMcp Tools](./mcp_tools_umg.md) — UMG/UI 专用工具
- [工具重叠对比](./mcp_tools_overlap.md) — MAB vs UMG 重叠分析
- [Handler Extension Guide](./mcp_extension_guide.md)
- [MCP Agent System](./mcp_agent_system.md)
- [Backend Priority](../execution/backend_priority.md)

---

*文档版本: 2.0 (完整版)*
*插件版本: 0.1.3*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
