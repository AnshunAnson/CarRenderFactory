// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// 导入 UnrealBuildTool 命名空间，提供构建系统的核心类型和功能
using UnrealBuildTool;

// 导入 System.Collections.Generic 命名空间
// 虽然当前代码未直接使用，但保留以便后续扩展
using System.Collections.Generic;

/// <summary>
/// CarRenderFactory 编辑器构建目标配置类
///
/// 此类定义 UE 编辑器环境的编译设置。
/// 编辑器目标与游戏目标的主要区别:
///
/// 1. 类型差异:
///    - Game: 只包含运行时必要的代码，体积更小
///    - Editor: 包含编辑器功能、工具、调试代码
///
/// 2. 使用场景:
///    - Game: 最终打包给玩家运行
///    - Editor: 开发者在编辑器中工作
///
/// 3. 功能差异:
///    - Editor 包含蓝图编辑器、材质编辑器、关卡编辑器等
///    - Editor 可以加载和编译蓝图
///    - Editor 可以热重载 C++ 代码
///
/// 文件命名规则: {ProjectName}Editor.Target.cs
/// 类命名规则: {ProjectName}EditorTarget
/// </summary>
public class CarRenderFactoryEditorTarget : TargetRules
{
	/// <summary>
	/// 构造函数 - 初始化编辑器目标配置
	/// </summary>
	/// <param name="Target">
	/// TargetInfo 对象，由 UnrealBuildTool 自动注入
	/// 包含以下信息:
		/// - Target.Platform: 目标平台 (Win64, Linux, Mac 等)
	/// - Target.Configuration: 构建配置 (Debug, Development, Shipping 等)
	/// - Target.Architecture: CPU 架构 (x64, ARM64 等)
	/// - Target.ProjectPath: 项目路径
	/// </param>
	public CarRenderFactoryEditorTarget( TargetInfo Target) : base(Target)
	{
		// ============================================================
		// 目标类型设置
		// ============================================================
		// TargetType.Editor 表示这是一个编辑器目标
		//
		// 编辑器目标的特点:
		// 1. 包含完整的编辑器 UI 系统 (Slate/UMG)
		// 2. 支持蓝图编译和热重载
		// 3. 包含资产导入/导出功能
		// 4. 包含调试和性能分析工具
		// 5. 编译时间通常比 Game 目标长
		// 6. 生成的可执行文件体积更大
		Type = TargetType.Editor;

		// ============================================================
		// 构建设置版本
		// ============================================================
		// 与 Game Target 保持一致，使用 V6 版本
		// 这确保编辑器和游戏使用相同的构建规则
		DefaultBuildSettings = BuildSettingsVersion.V6;

		// ============================================================
		// 引擎头文件包含顺序版本
		// ============================================================
		// 与 Game Target 保持一致，使用 Unreal5_7 版本
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		// ============================================================
		// 额外模块名称
		// ============================================================
		// 编辑器目标同样需要链接游戏模块 CarRenderFactory
		// 这样才能在编辑器中编辑游戏内容、预览游戏效果
		//
		// 注意: 编辑器目标还可以添加额外的编辑器模块
		// 例如: ExtraModuleNames.Add("CarRenderFactoryEditor");
		ExtraModuleNames.Add("CarRenderFactory");

		// ============================================================
		// 编辑器特定设置 (可根据需要添加)
		// ============================================================
		// bBuildEditor = true;                    // 编辑器目标自动为 true
		// bCanHotReloadFromIDE = true;            // 支持 IDE 热重载
		// bUseAdaptiveUnityBuild = true;          // 自适应 Unity Build
		// bWarningsAsErrors = false;              // 警告不作为错误
	}
}
