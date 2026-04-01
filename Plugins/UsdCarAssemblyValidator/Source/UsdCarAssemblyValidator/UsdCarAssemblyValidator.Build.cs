// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 文件: UsdCarAssemblyValidator.Build.cs
// 功能: USD 汽车装配验证器插件的构建配置
// 描述: 定义模块的依赖关系、包含路径和编译设置
// ============================================================

using UnrealBuildTool;

/**
 * UsdCarAssemblyValidator - 模块构建规则类
 *
 * 继承自 ModuleRules，定义模块的构建配置
 * 包括：
 * - 头文件包含路径
 * - 模块依赖关系
 * - 预编译头设置
 */
public class UsdCarAssemblyValidator : ModuleRules
{
	/**
	 * 构造函数
	 *
	 * @param Target 构建目标信息（平台、配置等）
	 */
	public UsdCarAssemblyValidator(ReadOnlyTargetRules Target) : base(Target)
	{
		// 预编译头使用模式
		// UseExplicitOrSharedPCHs: 使用显式或共享的预编译头
		// 可以加速编译过程
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// ===== 公共包含路径 =====
		// 这些路径对依赖此模块的其他模块可见
		PublicIncludePaths.AddRange(
			new string[] {
				// 公共头文件目录：包含对外公开的头文件
				System.IO.Path.Combine(ModuleDirectory, "Public"),
			}
			);


		// ===== 私有包含路径 =====
		// 这些路径仅对此模块内部可见
		PrivateIncludePaths.AddRange(
			new string[] {
				// 私有头文件目录：包含内部实现的头文件
				System.IO.Path.Combine(ModuleDirectory, "Private"),
			}
			);


		// ===== 公共依赖模块 =====
		// 这些模块的头文件对此模块公开
		// 依赖此模块的其他模块也会自动依赖这些模块
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",          // 核心模块：基础类型、容器、字符串等
			}
			);


		// ===== 私有依赖模块 =====
		// 这些模块仅在此模块内部使用
		// 依赖此模块的其他模块不会自动依赖这些模块
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Projects",        // 项目管理：插件加载、模块管理
				"InputCore",       // 输入核心：输入事件处理
				"EditorFramework", // 编辑器框架：编辑器基础功能
				"UnrealEd",        // 虚幻编辑器：编辑器主要功能
				"ToolMenus",       // 工具菜单：菜单系统
				"CoreUObject",     // 核心 UObject：反射、序列化
				"Engine",          // 引擎核心：Actor、Component 等
				"Slate",           // Slate UI：UI 控件框架
				"SlateCore",       // Slate 核心：基础 UI 类型
				"DesktopPlatform", // 桌面平台：文件对话框等
				// USD 模块
				"UnrealUSDWrapper", // USD SDK 封装
				"USDClasses",       // USD 类定义
				"USDStage",         // USD Stage 相关
				"USDSchemas",       // USD Schemas
			}
			);


		// ===== 动态加载模块 =====
		// 这些模块在运行时按需加载，而非启动时加载
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// 此插件暂无动态加载的模块
				// 如果需要，可以在此添加模块名称
			}
			);
	}
}
