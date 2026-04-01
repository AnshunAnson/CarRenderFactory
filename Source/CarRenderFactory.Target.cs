// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// 导入 UnrealBuildTool 命名空间，提供构建系统的核心类型和功能
// TargetRules 类是所有构建目标配置的基类
using UnrealBuildTool;

// 导入 System.Collections.Generic 命名空间
// 提供 List<T>、Dictionary<K,V> 等泛型集合类型
using System.Collections.Generic;

/// <summary>
/// CarRenderFactory 游戏运行时构建目标配置类
///
/// 此类定义游戏在最终打包和运行时的编译设置。
/// 构建目标 (Target) 决定了：
/// - 编译类型 (游戏/编辑器/服务器/客户端)
/// - 包含的模块
/// - 引擎版本和构建设置
///
/// 文件命名规则: {ProjectName}.Target.cs
/// 类命名规则: {ProjectName}Target
/// </summary>
public class CarRenderFactoryTarget : TargetRules
{
	/// <summary>
	/// 构造函数 - 初始化游戏运行时目标配置
	/// </summary>
	/// <param name="Target">
	/// TargetInfo 对象，包含当前构建目标的环境信息
	/// 包括平台 (Win64/Linux/Console)、配置 (Debug/Development/Shipping) 等
	/// </param>
	public CarRenderFactoryTarget(TargetInfo Target) : base(Target)
	{
		// ============================================================
		// 目标类型设置
		// ============================================================
		// TargetType.Game 表示这是一个独立的游戏运行时目标
		//
		// 可选类型:
		// - Game: 打包后的游戏运行时 (无编辑器功能)
		// - Editor: UE 编辑器 (包含编辑器功能)
		// - Client: 网络客户端 (无服务器逻辑)
		// - Server: 专用服务器 (无客户端渲染)
		// - Program: 独立工具程序
		Type = TargetType.Game;

		// ============================================================
		// 构建设置版本
		// ============================================================
		// BuildSettingsVersion.V6 是 UE5.7 的最新构建设置版本
		//
		// 版本演进:
		// - V2: UE4.27 之前
		// - V3: UE5.0-5.1
		// - V4: UE5.2-5.3
		// - V5: UE5.4-5.5
		// - V6: UE5.6+
		//
		// 不同版本影响:
		// - PCH (预编译头) 策略
		// - Unity Build 设置
		// - 包含路径解析方式
		DefaultBuildSettings = BuildSettingsVersion.V6;

		// ============================================================
		// 引擎头文件包含顺序版本
		// ============================================================
		// EngineIncludeOrderVersion 定义引擎头文件的包含顺序
		// Unreal5_7 对应 UE 5.7 版本的包含顺序
		//
		// 这影响:
		// - 头文件搜索路径顺序
		// - 模块间的依赖解析
		// - 跨版本兼容性
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

		// ============================================================
		// 额外模块名称
		// ============================================================
		// ExtraModuleNames 是一个列表，包含此目标需要链接的游戏模块
		//
		// "CarRenderFactory" 是本项目的主游戏模块
		// 添加后，UBT 会自动:
		// 1. 查找 Source/CarRenderFactory/CarRenderFactory.Build.cs
		// 2. 解析模块依赖关系
		// 3. 链接编译后的模块到目标
		ExtraModuleNames.Add("CarRenderFactory");

		// ============================================================
		// 其他常用设置 (可根据需要添加)
		// ============================================================
		// bUsePCHFiles = true;                    // 启用预编译头
		// bUseUnity = true;                        // 启用 Unity Build
		// bCompileAgainstEngine = true;            // 链接引擎模块
		// bCompileAgainstCoreUObject = true;       // 链接 CoreUObject
		// bBuildEditor = false;                    // 不构建编辑器功能
		// bIsBuildingDedicatedServer = false;      // 非专用服务器
		// TargetPlatform = TargetPlatformDescriptor.Win64;  // 目标平台
	}
}
