# CLI Tool Layer (辅助工具层)

> **定位**: `cli-anything-unrealengine` 是 **Tool (工具)**, 不是 Agent
> 
> 它由 SoloCoder 或执行 Agent 按需调用，无独立身份、无职责边界、无自主决策能力。

---

## 验证状态

| 项目 | 值 |
|------|-----|
| **评分** | ⭐4/5 (85%通过率) |
| **详细报告** | [references/cli_validation_report.md](../references/cli_validation_report.md) |
| **路径** | `D:\UE\UE_5.7\agent-harness` |
| **启动** | `Set-Location D:\UE\UE_5.7\agent-harness; cli-anything-unrealengine --json <command>` |

---

## CLI vs MCP 调用决策规则

| 场景 | 用 CLI | 用 MCP | 决策依据 |
|------|--------|--------|----------|
| **项目健康检查** (任务开始前) | ✅ `project info` / `stats code` | ❌ | Editor 未启动或仅需元数据 |
| **UMG 离线结构分析** (CodeReview 阶段) | ✅ `ui umg-read` (.uasset 解析) | ⚠️ `get_widget_tree`(需Editor在线) | 无需运行时数据, 快速静态扫描 |
| **插件/模块状态确认** | ✅ `plugin list` / `modules` | ❌ | 纯信息查询 |
| **资产盘点** | ✅ `asset list --type Blueprint` | ⚠️ `list_assets`(需Editor) | 批量统计场景 |
| **编译触发** | ✅ `build editor --config Development` | ❌ | CLI 独有能力 |
| **配置审计** | ✅ `config list` | ❌ | INI section 列表 |
| **Widget 创建/编辑** | ❌ | ✅ `create_widget` / `apply_layout` | 需要实时操作 Editor |
| **Actor 操作** | ❌ | ✅ `spawn_actor` / `set_transform` | 需要引擎运行时 |
| **材质节点编辑** | ❌ | ✅ `add_node` / `connect_nodes` | 需要编辑器交互 |
| **PIE 测试/截图** | ❌ | ✅ `play_in_editor` / `screenshot` | 需要引擎运行时 |

---

## 核心命令速查

| 命令 | 用途 | 典型触发时机 |
|------|------|-------------|
| `project info` | 项目元数据 (名称/引擎/模块/插件) | 任务开始前一次性检查 |
| `stats project/code/content/build` | 项目规模全景 (资产数/代码行/空间占用) | CodeReview 前置分析 |
| `ui umg-read <path>` | **离线 UMG .uasset 解析** (named_widgets/generated_classes/references) | CodeReview 离线审查 / 迁移前分析 |
| `plugin list` | 插件启用状态 | 确认 MCP 插件可用性 |
| `build editor --config Development` | 触发 Editor 目标编译 | C++ 代码变更后 |
| `asset list --type <Type>` | 按类型过滤资产列表 | 资产盘点 / 清理 |
| `config list` | INI 配置文件 section 列表 | 配置排查 |
| `clean ddc/binaries/intermediate` | 缓存清理 | 编译问题排查 (需关闭Editor) |

---

## Bootstrap (PowerShell)

```powershell
Set-Location D:\UE\UE_5.7\agent-harness
cli-anything-unrealengine --json project info "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

---

## 典型使用场景

### 场景 1: 任务前置健康检查

```powershell
# 检查项目状态
cli-anything-unrealengine --json project info "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 检查插件状态
cli-anything-unrealengine --json plugin list "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 检查代码规模
cli-anything-unrealengine --json stats code "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

### 场景 2: CodeReview 离线分析

```powershell
# 离线解析 UMG 结构
cli-anything-unrealengine --json ui umg-read "/Game/AUDI_Content/UI/Core/WBP_WidgetBase"

# 输出包含: named_widgets, generated_classes, references
```

### 场景 3: 资产盘点

```powershell
# 列出所有 Blueprint 资产
cli-anything-unrealengine --json asset list --type Blueprint "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"

# 列出所有 Widget Blueprint
cli-anything-unrealengine --json asset list --type WidgetBlueprint "D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject"
```

---

## Fallback 策略

如果 harness 不可用或命令覆盖不足:
1. 使用 Native UE CLI
2. 在 `Verification Evidence` 中声明回退原因

### Native UE CLI 参考

> **验证日期**: 2026-04-02
> **验证环境**: UE 5.7 InstalledBuild (D:\UE\UE_5.7)

#### 编译命令

```powershell
# 编译 Editor 目标 (开发迭代)
cd D:\UE\UE_5.7\Engine\Build\BatchFiles
.\Build.bat <ProjectName>Editor Win64 Development "<ProjectPath>/<ProjectName>.uproject" -waitmutex

# 示例: AudiR7S 项目
.\Build.bat AudiR7SEditor Win64 Development "D:\UE\Project\AudiR7S\AudiR7S\AudiR7S.uproject" -waitmutex
```

#### 编译结果验证 (2026-04-02)

| 目标 | 结果 | 说明 |
|------|------|------|
| **Editor 目标** | ✅ 成功 | 生成 UnrealEditor-AudiR7S.dll |
| **Game 目标** | ❌ 失败 | 链接错误: 缺失 GInternalProjectName 等符号 |

#### InstalledBuild 限制

**Game 目标链接失败原因**:
- 缺失符号: `GInternalProjectName`, `GIsGameAgnosticExe`, `FMemory_Malloc/Free/Realloc`
- 这些符号由源码构建的 Launch 模块提供，InstalledBuild 不包含

**解决方案**:
1. **Editor 目标编译** - 正常工作，满足开发需求
2. **Game 目标打包** - 必须在 UE Editor 中执行 (File → Package Project)
3. **源码构建** - 如需命令行打包，需使用 Source Build 而非 InstalledBuild

#### 前置条件

```powershell
# 1. 关闭 UE Editor (Live Coding 会阻止编译)
Stop-Process -Name "UnrealEditor" -Force -ErrorAction SilentlyContinue

# 2. 等待进程完全退出
Start-Sleep -Seconds 3

# 3. 执行编译
cd D:\UE\UE_5.7\Engine\BatchFiles
.\Build.bat AudiR7SEditor Win64 Development "D:\UE\Project\AudiR7S\AudiR7S\AudiR7S.uproject" -waitmutex
```

---

## 编译时机与状态监控

### 什么时候可以直接编译 (无需关闭 Editor)

| 场景 | 可直接编译 | 说明 |
|------|-----------|------|
| **纯 C++ 代码修改** | ✅ 可以 | 使用 Live Coding (Ctrl+Alt+F11) 热重载 |
| **头文件新增 UPROPERTY/UFUNCTION** | ✅ 可以 | UHT 会在下次启动时处理 |
| **Blueprint 调用的新 C++ 函数** | ✅ 可以 | Live Coding 支持 |

### 什么时候必须关闭 Editor 再编译

| 场景 | 必须关闭 | 原因 |
|------|---------|------|
| **新增 C++ 类** | ❌ 必须关闭 | 需要生成 .generated.h 文件 |
| **修改 UCLASS/USTRUCT 宏** | ❌ 必须关闭 | UHT 需要重新生成反射代码 |
| **修改模块依赖 (Build.cs)** | ❌ 必须关闭 | 需要重新生成 makefile |
| **删除/重命名 C++ 类** | ❌ 必须关闭 | 需要清理旧的生成文件 |
| **命令行 Build.bat 编译** | ❌ 必须关闭 | Live Coding 会锁定 DLL |

### UE Editor 状态监控

#### 方法 1: PowerShell 进程检查

```powershell
# 检查 UE Editor 是否运行
Get-Process | Where-Object {$_.ProcessName -like "*UnrealEditor*"} | Select-Object Id, ProcessName, MainWindowTitle

# 检查 Live Coding 状态 (通过互斥体)
# 如果存在 LiveCoding_* 互斥体，说明 Live Coding 活跃
```

#### 方法 2: 检查 Live Coding 互斥体

```powershell
# Live Coding 互斥体命名规则: Global\LiveCoding_<EnginePath>
# 示例: Global\LiveCoding_D++UE+UE_5.7+Engine+Binaries+Win64+UnrealEditor.exe

# 检查是否存在 Live Coding 锁
$mutexName = "Global\LiveCoding_D++UE+UE_5.7+Engine+Binaries+Win64+UnrealEditor.exe"
try {
    $mutex = [System.Threading.Mutex]::OpenExisting($mutexName)
    Write-Host "Live Coding is ACTIVE - Must close Editor before compiling"
    $mutex.Dispose()
} catch {
    Write-Host "Live Coding is INACTIVE - Safe to compile"
}
```

#### 方法 3: 检查 DLL 锁定状态

```powershell
# 检查项目 DLL 是否被锁定
$dllPath = "D:\UE\Project\AudiR7S\AudiR7S\Binaries\Win64\UnrealEditor-AudiR7S.dll"
if (Test-Path $dllPath) {
    try {
        $file = [System.IO.File]::Open($dllPath, 'Open', 'Write')
        $file.Close()
        Write-Host "DLL is NOT locked - Safe to compile"
    } catch {
        Write-Host "DLL is LOCKED - UE Editor is using it"
    }
}
```

### 编译决策流程图

```
开始编译请求
    │
    ├── 检查 UE Editor 进程
    │   ├── 未运行 → 直接编译 ✅
    │   └── 正在运行 → 检查编译类型
    │       │
    │       ├── 纯 C++ 实现修改 (.cpp)
    │       │   └── 使用 Live Coding (Ctrl+Alt+F11) ✅
    │       │
    │       └── 新增类/修改头文件/Build.cs
    │           │
    │           ├── 关闭 UE Editor
    │           ├── 等待进程退出 (3-5秒)
    │           ├── 执行 Build.bat
    │           └── 重新启动 Editor ✅
```

### 自动化编译脚本示例

```powershell
function Invoke-UECompile {
    param(
        [string]$ProjectPath = "D:\UE\Project\AudiR7S\AudiR7S\AudiR7S.uproject",
        [string]$Target = "AudiR7SEditor",
        [switch]$ForceCloseEditor
    )
    
    # 检查 UE Editor 进程
    $ueProcess = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
    
    if ($ueProcess -and -not $ForceCloseEditor) {
        Write-Warning "UE Editor is running (PID: $($ueProcess.Id))"
        Write-Host "Options:"
        Write-Host "  1. Use Live Coding (Ctrl+Alt+F11 in Editor)"
        Write-Host "  2. Run with -ForceCloseEditor to auto-close"
        return
    }
    
    if ($ueProcess -and $ForceCloseEditor) {
        Write-Host "Closing UE Editor..."
        Stop-Process -Name "UnrealEditor" -Force
        Start-Sleep -Seconds 5
    }
    
    # 执行编译
    Write-Host "Compiling $Target..."
    Set-Location "D:\UE\UE_5.7\Engine\Build\BatchFiles"
    & .\Build.bat $Target Win64 Development $ProjectPath -waitmutex
}

# 使用示例
# Invoke-UECompile -ForceCloseEditor  # 强制关闭并编译
# Invoke-UECompile                     # 检查状态，提示用户选择
```

---

## 防止重复启动项目

### 问题场景

- UE Editor 启动慢 (30-60秒)，用户可能重复点击
- 多个 UE 实例会争抢同一项目的文件锁
- 可能导致资产损坏或配置冲突

### 方法 1: 启动前检查进程

```powershell
function Start-UEProject {
    param(
        [string]$ProjectPath = "D:\UE\Project\AudiR7S\AudiR7S\AudiR7S.uproject",
        [string]$EnginePath = "D:\UE\UE_5.7"
    )
    
    # 检查是否已有 UE Editor 运行
    $existingProcess = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue
    
    if ($existingProcess) {
        Write-Warning "UE Editor is already running (PID: $($existingProcess.Id))"
        Write-Host "Window Title: $($existingProcess.MainWindowTitle)"
        
        # 可选: 激活现有窗口
        # (Add-Type -TypeDefinition "using System; using System.Runtime.InteropServices; public class Win32 { [DllImport(\"user32.dll\")] public static extern bool SetForegroundWindow(IntPtr hWnd); }") | Out-Null
        # [Win32]::SetForegroundWindow($existingProcess.MainWindowHandle)
        
        return $false
    }
    
    # 启动新实例
    Write-Host "Starting UE Editor..."
    $editorExe = "$EnginePath\Engine\Binaries\Win64\UnrealEditor.exe"
    Start-Process $editorExe -ArgumentList $ProjectPath
    
    return $true
}

# 使用示例
# Start-UEProject  # 检查并启动
```

### 方法 2: 使用互斥体 (Mutex) 防止重复启动

```powershell
# 项目级互斥体命名规则: Global\UEProject_<ProjectName>
$projectMutexName = "Global\UEProject_AudiR7S"

function Test-ProjectRunning {
    param([string]$ProjectName)
    
    $mutexName = "Global\UEProject_$ProjectName"
    try {
        $mutex = [System.Threading.Mutex]::OpenExisting($mutexName)
        $mutex.Dispose()
        return $true  # 项目已在运行
    } catch {
        return $false  # 项目未运行
    }
}

# 使用示例
if (Test-ProjectRunning -ProjectName "AudiR7S") {
    Write-Host "AudiR7S is already running!"
} else {
    Write-Host "AudiR7S is not running, safe to start"
}
```

### 方法 3: 检查项目锁定文件

```powershell
function Test-ProjectLocked {
    param([string]$ProjectPath)
    
    # UE 会在运行时创建锁定文件
    $lockFile = Join-Path (Split-Path $ProjectPath) "Saved\Lock"
    
    if (Test-Path $lockFile) {
        $lockContent = Get-Content $lockFile -ErrorAction SilentlyContinue
        Write-Host "Project is locked by: $lockContent"
        return $true
    }
    
    return $false
}
```

### 方法 4: 完整的启动管理脚本

```powershell
function Start-UEProjectSafe {
    param(
        [string]$ProjectPath = "D:\UE\Project\AudiR7S\AudiR7S\AudiR7S.uproject",
        [string]$EnginePath = "D:\UE\UE_5.7",
        [switch]$ForceNew
    )
    
    $projectName = [System.IO.Path]::GetFileNameWithoutExtension($ProjectPath)
    
    # 1. 检查进程
    $existingProcess = Get-Process -Name "UnrealEditor" -ErrorAction SilentlyContinue | 
                       Where-Object { $_.MainWindowTitle -like "*$projectName*" }
    
    if ($existingProcess -and -not $ForceNew) {
        Write-Warning "[$projectName] Already running (PID: $($existingProcess.Id))"
        Write-Host "Use -ForceNew to start a new instance"
        return $existingProcess
    }
    
    # 2. 检查锁定文件
    $lockFile = Join-Path (Split-Path $ProjectPath) "Saved\Lock"
    if ((Test-Path $lockFile) -and -not $ForceNew) {
        Write-Warning "[$projectName] Lock file exists at: $lockFile"
        Write-Host "Another instance may be running or crashed"
        Write-Host "Use -ForceNew to override"
        return $null
    }
    
    # 3. 启动新实例
    Write-Host "[$projectName] Starting UE Editor..."
    $editorExe = "$EnginePath\Engine\Binaries\Win64\UnrealEditor.exe"
    
    $process = Start-Process $editorExe -ArgumentList $ProjectPath -PassThru
    
    Write-Host "[$projectName] Started with PID: $($process.Id)"
    return $process
}

# 使用示例
# Start-UEProjectSafe                    # 安全启动，检查现有实例
# Start-UEProjectSafe -ForceNew          # 强制启动新实例
```

### 最佳实践总结

| 检查项 | 方法 | 优先级 |
|--------|------|--------|
| **进程检查** | `Get-Process -Name "UnrealEditor"` | ⭐⭐⭐ 高 |
| **窗口标题匹配** | `$_.MainWindowTitle -like "*ProjectName*"` | ⭐⭐ 中 |
| **锁定文件检查** | `Saved/Lock` 文件 | ⭐ 低 (可能残留) |
| **互斥体检查** | `Global\UEProject_<Name>` | ⭐⭐ 中 (需自定义实现) |

#### 其他 Native 命令

```powershell
# 生成项目文件 (仅 Source Build 可用)
GenerateProjectFiles.bat -project=D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject

# 打包游戏 (仅 Source Build 可用)
RunUAT.bat BuildCookRun -project=D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject -platform=Win64 -clientconfig=Shipping -cook -build -stage -pak -archive -archivedirectory=D:/Builds/AudiR7S

# 启动 Editor
UnrealEditor.exe D:/UE/Project/AudiR7S/AudiR7S/AudiR7S.uproject
```

---

## 与 Agent 的关系

```
SoloCoder / 执行 Agent
        │
        ├── 需要离线分析? ──→ CLI Tool
        │                       └── 返回静态数据
        │
        └── 需要实时操作? ──→ MCP Agent
                                └── 返回执行结果
```

**关键原则**:
- CLI Tool 是被动工具，由 Agent 按需调用
- CLI Tool 不做决策，只返回数据
- CLI Tool 不替代 MCP，是补充方案

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
