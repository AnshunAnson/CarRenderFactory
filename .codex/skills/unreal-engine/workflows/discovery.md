# Discovery Workflow (发现工作流)

> **权威源**: 本文件定义项目发现策略和命令
> **版本**: 1.0

---

## 概述

Discovery 是任务执行的第一步，用于了解项目结构、资产分布、代码组织等基础信息。

---

## 采样策略 (按 Hardness)

### H 级别 - 完整扫描

**规则**:
- 禁止 `First N` 截断关键工件
- 必须完整扫描所有相关文件

**必须完整扫描的工件**:
- `.uproject` 文件
- 模块和插件配置
- 任务相关资产
- 涉及的子系统

### M 级别 - 采样扫描

**规则**:
- 允许采样列表
- 必须显式声明采样警告

**示例声明**:
```
Discovery Warning: Sampled first 50 assets from Content/ directory.
Full scan required for H-level verification.
```

### L 级别 - 快速扫描

**规则**:
- 允许快速扫描
- 无需完整枚举

---

## Discovery 命令集

### 项目结构发现

```powershell
# 查找项目文件
Get-ChildItem -Recurse -Filter *.uproject | Select-Object -ExpandProperty FullName

# 查找 Source 目录结构
Get-ChildItem .\Source -Recurse -Directory | Select-Object -ExpandProperty FullName

# 查找 Build.cs 文件 (模块依赖)
Get-ChildItem .\Source -Recurse -Filter *.Build.cs | Select-Object -ExpandProperty FullName
```

### 源代码发现

```powershell
# 查找所有头文件
Get-ChildItem .\Source -Recurse -Filter *.h -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找所有实现文件
Get-ChildItem .\Source -Recurse -Filter *.cpp -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 按类型统计代码文件
Get-ChildItem .\Source -Recurse -Include *.h,*.cpp -ErrorAction SilentlyContinue | Group-Object Extension
```

### 资产发现

```powershell
# 查找所有资产
Get-ChildItem .\Content -Recurse -Filter *.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 按前缀分类资产
Get-ChildItem .\Content -Recurse -Filter *.uasset -ErrorAction SilentlyContinue | ForEach-Object {
    $name = $_.Name
    if ($name -match '^(BP_|WBP_|M_|MI_|T_|A_)') {
        $matches[1]
    }
} | Group-Object

# 查找 Widget Blueprint
Get-ChildItem .\Content -Recurse -Filter WBP_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找 Actor Blueprint
Get-ChildItem .\Content -Recurse -Filter BP_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找材质
Get-ChildItem .\Content -Recurse -Filter M_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找材质实例
Get-ChildItem .\Content -Recurse -Filter MI_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找纹理
Get-ChildItem .\Content -Recurse -Filter T_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
```

### Input 系统发现

```powershell
# 查找 Input Action
Get-ChildItem .\Content -Recurse -Filter IA_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找 Input Mapping Context
Get-ChildItem .\Content -Recurse -Filter IMC_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
```

### 配置发现

```powershell
# 查找配置文件
Get-ChildItem .\Config -Filter *.ini | Select-Object -ExpandProperty FullName

# 查找插件配置
Get-ChildItem .\Plugins -Recurse -Filter *.uplugin | Select-Object -ExpandProperty FullName
```

---

## CLI 辅助 Discovery

使用 `cli-anything-unrealengine` 进行更详细的项目分析:

```powershell
Set-Location D:\UE\UE_5.7\agent-harness

# 项目元数据
cli-anything-unrealengine --json project info "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 项目统计
cli-anything-unrealengine --json stats project "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
cli-anything-unrealengine --json stats code "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
cli-anything-unrealengine --json stats content "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 插件列表
cli-anything-unrealengine --json plugin list "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 模块列表
cli-anything-unrealengine --json modules "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

---

## Discovery 输出格式

### 项目结构报告

```markdown
## Project Structure Discovery

### Project Info
- Name: AudiR7S
- Engine: 5.7
- Modules: AudiR7S (Runtime)

### Source Structure
- Public Headers: 12 files
- Private Implementation: 15 files
- Build Scripts: 3 files

### Content Structure
- Widget Blueprints: 11
- Actor Blueprints: 5
- Materials: 8
- Material Instances: 12
- Textures: 22

### Input System
- Input Actions: 3
- Mapping Contexts: 2

### Plugins
- McpAutomationBridge: Enabled
- UmgMcp: Enabled
```

---

## 与 Playbook 的关系

Discovery 结果用于驱动后续 Playbook 选择:

| Discovery 结果 | 推荐 Playbook |
|---------------|--------------|
| C++ 架构相关 | playbooks/cpp_architecture.md |
| 多人/复制/GAS | playbooks/multiplayer_and_net.md |
| MCP Agent 编排 | playbooks/mcp_agent_orchestration.md |
| 验证流程 | playbooks/validation.md |

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
