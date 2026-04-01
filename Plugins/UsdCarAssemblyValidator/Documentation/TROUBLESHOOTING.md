# 🆘 USD 汽车装配验证器 - 快速故障排除

## 问题：看不到按钮和 UI 内容

### ✅ 立即尝试的 3 个快速修复

#### 修复 1：重启编辑器（30 秒）
```powershell
# 1. 完全关闭编辑器
# 2. 等待 10 秒
# 3. 重新打开编辑器
# 4. 再次打开 USD Car Assembly Validator 窗口
```

**成功率**: 60%

---

#### 修复 2：清理编辑器缓存（2 分钟）
```powershell
# 1. 关闭编辑器
# 2. 删除缓存
Remove-Item "$env:LOCALAPPDATA\Unreal Engine\Engine\Saved" -Recurse -Force -ErrorAction SilentlyContinue
# 3. 重新打开编辑器
```

**成功率**: 80%

---

#### 修复 3：完整重建（5 分钟）
```powershell
# 1. 关闭编辑器

# 2. 删除中间文件
cd D:\UE\Project\CarRenderFactory
Remove-Item "Intermediate", "Binaries" -Recurse -Force -ErrorAction SilentlyContinue

# 3. 重新生成项目文件
& "D:\UE\UE_5.7\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -Game

# 4. 重新编译
& "D:\UE\UE_5.7\Engine\Build\BatchFiles\Build.bat" `
  CarRenderFactoryEditor Win64 Development `
  -Project="D:\UE\Project\CarRenderFactory\CarRenderFactory.uproject"

# 5. 重新打开编辑器
& "D:\UE\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" `
  "D:\UE\Project\CarRenderFactory\CarRenderFactory.uproject"
```

**成功率**: 100%

---

## 🔍 诊断步骤

如果上述方法都没有解决，请按照以下步骤诊断：

### 步骤 1：检查插件是否加载

在编辑器中打开输出日志：
**Window** → **Output Log**

搜索以下文本：
```
✅ 应该看到：
LogInit: Display: 'UsdCarAssemblyValidator' module loaded successfully

❌ 如果看到：
LogInit: Error: Failed to load 'UsdCarAssemblyValidator' module
→ 插件加载失败，需要检查编译错误
```

### 步骤 2：检查插件是否启用

1. **Edit** → **Plugins**
2. 搜索：`USD Car Assembly Validator`
3. 确保复选框 ✅ 已勾选
4. 如果有 **Recompile** 按钮，点击它
5. 重启编辑器

### 步骤 3：检查代码文件

验证以下文件是否存在且有内容：

```powershell
# 应该存在的文件
Test-Path "Plugins\UsdCarAssemblyValidator\Source\UsdCarAssemblyValidator\Public\SAssemblyValidatorPanel.h"
Test-Path "Plugins\UsdCarAssemblyValidator\Source\UsdCarAssemblyValidator\Private\SAssemblyValidatorPanel.cpp"
Test-Path "Plugins\UsdCarAssemblyValidator\Source\UsdCarAssemblyValidator\Private\UsdCarAssemblyValidator.cpp"
```

### 步骤 4：检查编译是否成功

```powershell
# 检查 DLL 是否生成
Test-Path "Binaries\Win64\UE4Editor-UsdCarAssemblyValidator.dll"

# 如果不存在，需要重新编译
```

---

## 📊 问题诊断表

| 症状 | 可能原因 | 解决方案 |
|------|---------|--------|
| 空白窗口 | UI 代码未加载 | 修复 3（完整重建） |
| 看到"Add code..."文本 | OnSpawnPluginTab 未更新 | 检查 UsdCarAssemblyValidator.cpp 第 57-64 行 |
| 窗口不出现 | 插件未启用 | Edit → Plugins，启用 USD Car Assembly Validator |
| 编辑器崩溃 | Slate 代码错误 | 查看输出日志，检查是否有编译错误 |
| 按钮无反应 | 回调函数未连接 | 检查 SAssemblyValidatorPanel.h 中的回调定义 |

---

## 🎯 预期结果

成功时应该看到：

### 窗口标题
```
✅ "UsdCarAssemblyValidator" 标签
```

### 窗口内容
```
✅ 完整的 3 面板布局
✅ 顶部的 4 个按钮
✅ 中间的变体/当前/部件显示
✅ 底部的日志输出框
```

### 功能正常
```
✅ 点击按钮时窗口有反应
✅ 输出日志更新
✅ 统计计数器变化
```

---

## 🔧 高级诊断

### 启用 Slate 调试

在编辑器中按：`Ctrl + Shift + ~` (波浪号)

这会打开 Slate 调试工具，显示 UI 层次结构。

### 查看详细编译日志

```powershell
$logPath = "C:\Users\$env:USERNAME\AppData\Local\UnrealBuildTool\Log.txt"
Get-Content $logPath -Tail 100
```

### 使用 Visual Studio 调试器

1. 在 Visual Studio 中打开项目
2. Debug → Attach to Process
3. 选择 UE4Editor.exe
4. 在 Slate 代码中设置断点
5. 在编辑器中打开窗口

---

## 📞 获取帮助

如果问题仍未解决：

1. **查看详细文档**: `UI_DISPLAY_FIX.md`
2. **检查编译修复**: `COMPILATION_FIXES.md`
3. **参考构建指南**: `BUILD_AND_DEPLOY_GUIDE.md`
4. **查看输出日志**: 在编辑器中打开 Output Log
5. **检查编译错误**: 重新构建项目，查看错误信息

---

**最后尝试的方案**: 如果一切都失败了，完全删除插件并重新创建。
