// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// 导入 UnrealBuildTool 命名空间
// ModuleRules 是所有模块构建配置的基类
// 提供了定义模块依赖、编译选项、链接设置等功能的属性
using UnrealBuildTool;

/// <summary>
/// CarRenderFactory 模块构建配置类
///
/// 模块 (Module) 是 UE 代码组织的基本单元，每个模块:
/// - 有自己的构建配置 (.Build.cs)
/// - 可以独立编译和链接
/// - 可以定义对其他模块的依赖关系
/// - 可以选择公开或隐藏 API
///
/// 文件命名规则: {ModuleName}.Build.cs
/// 类命名规则: {ModuleName} (继承自 ModuleRules)
///
/// 模块目录结构:
/// Source/CarRenderFactory/
/// ├── CarRenderFactory.Build.cs  ← 此文件
/// ├── CarRenderFactory.h         ← 模块头文件
/// └── CarRenderFactory.cpp       ← 模块实现
/// </summary>
public class CarRenderFactory : ModuleRules
{
	/// <summary>
	/// 构造函数 - 配置模块的编译设置和依赖关系
	/// </summary>
	/// <param name="Target">
	/// ReadOnlyTargetRules 对象，提供只读的目标构建信息
	/// 包括:
	/// - Target.Type (Game/Editor/Server/Client)
	/// - Target.Platform (Win64/Linux/Mac 等)
	/// - Target.Configuration (Debug/Development/Shipping)
	/// - Target.bBuildEditor (是否为编辑器构建)
	///
	/// 使用 Target 参数可以实现条件编译，例如:
	/// if (Target.bBuildEditor) { /* 添加编辑器依赖 */ }
	/// </param>
	public CarRenderFactory(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.AddRange(new string[] {
			System.IO.Path.Combine(ModuleDirectory, "Character"),
			System.IO.Path.Combine(ModuleDirectory, "Combat"),
			System.IO.Path.Combine(ModuleDirectory, "Core"),
			System.IO.Path.Combine(ModuleDirectory, "Input")
		});

		PublicDependencyModuleNames.AddRange(new string[] {
			// --------------------------------------------------------
			// Core - 引擎核心模块
			// --------------------------------------------------------
			// 提供基础功能:
			// - 内存管理 (FMemory, TSharedPtr, TWeakPtr)
			// - 容器类 (TArray, TMap, TSet, FString)
			// - 数学库 (FVector, FRotator, FQuat, FMatrix)
			// - 字符串处理 (FString, FName, FText)
			// - 哈希和加密
			// - 多线程 (FRunnable, FCriticalSection)
			// - 日志系统 (UE_LOG)
			"Core",

			// --------------------------------------------------------
			// CoreUObject - UObject 系统核心
			// --------------------------------------------------------
			// 提供 UE 的对象系统:
			// - UObject 基类 (反射、序列化、垃圾回收)
			// - UClass/UProperty/UFunction 反射系统
			// - 垃圾回收 (GC) 机制
			// - 对象序列化/反序列化
			// - 委托系统 (TDelegate)
			// - 软引用 (TSoftObjectPtr)
			"CoreUObject",

			// --------------------------------------------------------
			// Engine - 引擎核心模块
			// --------------------------------------------------------
			// 提供游戏框架核心:
			// - AActor - 游戏对象基类
			// - UActorComponent - 组件基类
			// - UWorld - 世界/关卡容器
			// - AGameModeBase/APlayerController - 游戏框架
			// - USceneComponent - 场景组件
			// - UPrimitiveComponent - 渲染组件
			// - 物理系统接口
			// - 音频系统基础
			"Engine",

			// --------------------------------------------------------
			// InputCore - 输入系统核心
			// --------------------------------------------------------
			// 提供基础输入功能:
			// - FKey - 输入键定义
			// - EInputEvent - 输入事件类型
			// - UInputComponent - 输入绑定组件
			// - 输入轴和动作映射基础
			// - 键盘/鼠标/手柄输入枚举
			"InputCore",

			// --------------------------------------------------------
			// EnhancedInput - 增强输入系统 (UE5 新系统)
			// --------------------------------------------------------
			// UE5 的现代化输入系统:
			// - UInputAction - 输入动作资产
			// - UInputMappingContext - 输入映射上下文
			// - 输入修饰器 (死区、缩放、平滑)
			// - 输入触发器 (按下、长按、双击)
			// - 运行时重新映射输入
			// - 玩家控制器集成
			//
			// 使用示例:
			// UInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
			"EnhancedInput",

			// --------------------------------------------------------
			// Gameplay Ability System (GAS) - 核心战斗系统
			// --------------------------------------------------------
			// GameplayAbilities - 能力系统核心
			// - UAbilitySystemComponent (ASC)
			// - UGameplayAbility - 能力基类
			// - UGameplayEffect - 效果系统
			// - UAttributeSet - 属性集
			// - 能力激活、冷却、消耗
			"GameplayAbilities",

			// GameplayTags - 游戏标签系统
			// - FGameplayTag - 标签定义
			// - FGameplayTagContainer - 标签容器
			// - 标签查询和匹配
			// - 状态、类型、事件标记
			"GameplayTags",

			// GameplayTasks - 游戏任务系统
			// - UGameplayTask - 任务基类
			// - 异步能力任务
			// - 能力等待和延迟
			"GameplayTasks",

			// --------------------------------------------------------
			// 网络模块
			// --------------------------------------------------------
			// Networking - 网络基础
			// - 网络连接管理
			// - RPC 支持
			"Networking",

			// Sockets - Socket 通信
			// - 底层网络通信
			// - P2P 连接支持
			"Sockets",

			// --------------------------------------------------------
			// 物理模块
			// --------------------------------------------------------
			// PhysicsCore - 物理核心
			// - 碰撞检测
			// - 物理查询
			"PhysicsCore",

			// Chaos - Chaos 物理引擎
			// - 高级物理模拟
			// - 物理交互
			"Chaos",

			// --------------------------------------------------------
			// AI 模块 (用于战斗 AI)
			// --------------------------------------------------------
			// AIModule - AI 系统
			// - AI 控制器
			// - 行为树
			"AIModule",

			// NavigationSystem - 导航系统
			// - 寻路
			// - 导航网格
			"NavigationSystem",

			// StateTreeModule - 状态树系统 (UE5.4+)
			// - UStateTreeAIComponent
			// - 状态机 AI
			"StateTreeModule",

			// GameplayStateTreeModule - 游戏玩法状态树
			// - UStateTreeAIComponent (AI专用组件)
			// - UStateTreeComponent
			// - AI相关状态树任务
			"GameplayStateTreeModule"
		});

		// ============================================================
		// 私有依赖模块
		// ============================================================
		// PrivateDependencyModuleNames 定义模块的私有依赖
		// 私有依赖意味着:
		// - 仅在此模块的 .cpp 文件中可访问
		// - 依赖不会传递给依赖此模块的其他模块
		// - 适合实现细节依赖，不暴露给外部
		PrivateDependencyModuleNames.AddRange(new string[] {
			// Slate UI 框架
			"Slate",
			"SlateCore",

			// Niagara 特效系统
			"Niagara",

			// 音频系统
			"AudioMixer",

			// 调试工具
			"GameplayDebugger"
		});

		// ============================================================
		// Slate UI 依赖 (可选)
		// ============================================================
		// 如果需要使用 Slate UI 系统，取消下面的注释
		// Slate 是 UE 的自定义 UI 框架，用于:
		// - 编辑器工具窗口
		// - 游戏内 UI (HUD、菜单)
		// - 调试可视化界面
		//
		// Slate - Slate 控件和布局系统
		// SlateCore - Slate 核心渲染和输入处理
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// ============================================================
		// 在线子系统依赖 (可选)
		// ============================================================
		// 如果需要使用在线功能，取消下面的注释
		// OnlineSubsystem 提供:
		// - 成就系统
		// - 排行榜
		// - 云存档
		// - 好友列表
		// - 匹配系统
		//
		// 具体平台 (Steam/EOS/Console) 需要额外配置
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// ============================================================
		// Steam 在线子系统说明
		// ============================================================
		// 如果需要使用 Steam 在线功能:
		// 1. 在 .uproject 文件中启用 OnlineSubsystemSteam 插件
		// 2. 配置 DefaultEngine.ini 中的在线子系统设置
		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

		// ============================================================
		// 编辑器专属依赖
		// ============================================================
		// 仅在编辑器构建时包含
		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				// UnrealEd - 虚幻编辑器核心
				// - 编辑器操作和管理
				"UnrealEd",

				// PropertyEditor - 属性编辑器
				// - Details 面板自定义
				// - 属性编辑控件
				"PropertyEditor",

				// EditorFramework - 编辑器框架
				// - 编辑器工具基类
				"EditorFramework"
			});
		}

		// ============================================================
		// 其他常用模块 (按需添加)
		// ============================================================
		// 渲染相关:
		// - "RenderCore" - 渲染核心
		// - "RHI" - 渲染硬件接口
		// - "Renderer" - 渲染器实现
		//
		// 物理相关:
		// - "PhysicsCore" - 物理核心
		// - "Chaos" - Chaos 物理引擎
		//
		// AI 相关:
		// - "AIModule" - AI 系统
		// - "NavigationSystem" - 导航系统
		//
		// 网络相关:
		// - "Networking" - 网络基础
		// - "Sockets" - Socket 通信
		//
		// 媒体相关:
		// - "MediaAssets" - 媒体资产
		// - "MediaUtils" - 媒体工具
	}
}
