// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "Framework/Commands/Commands.h"  // TCommands 基类定义
#include "UsdCarAssemblyValidatorStyle.h"  // 样式定义，用于命令关联图标

/**
 * FUsdCarAssemblyValidatorCommands - 插件命令定义类
 *
 * 继承自 TCommands 模板类，用于定义插件的所有 UI 命令
 * TCommands 是虚幻引擎的命令系统基类，支持：
 * - 命令的注册和注销
 * - 命令与快捷键绑定
 * - 命令与 UI 元素（按钮、菜单项）关联
 *
 * 模板参数是类自身，这是 CRTP（奇异递归模板模式）
 */
class FUsdCarAssemblyValidatorCommands : public TCommands<FUsdCarAssemblyValidatorCommands>
{
public:

	/**
	 * 构造函数
	 *
	 * 调用基类 TCommands 的构造函数，初始化命令上下文信息
	 *
	 * @param ContextName 命令上下文名称 "UsdCarAssemblyValidator"
	 * @param ContextDesc 上下文的本地化描述文本
	 * @param ParentContextName 父上下文名称（NAME_None 表示无父级）
	 * @param InStyleSetName 关联的样式集名称（用于获取图标等资源）
	 */
	FUsdCarAssemblyValidatorCommands()
		: TCommands<FUsdCarAssemblyValidatorCommands>(
			TEXT("UsdCarAssemblyValidator"),                                          // 命令上下文名称
			NSLOCTEXT("Contexts", "UsdCarAssemblyValidator", "UsdCarAssemblyValidator Plugin"),  // 本地化显示名称
			NAME_None,                                                                 // 无父上下文
			FUsdCarAssemblyValidatorStyle::GetStyleSetName()                          // 关联的样式集
		)
	{
	}

	/**
	 * RegisterCommands - 注册命令
	 *
	 * 继承自 TCommands 接口
	 * 在此函数中使用 UI_COMMAND 宏定义所有命令
	 * 命令定义包括：
	 * - 命令 ID
	 * - 显示名称
	 * - 描述文本
	 * - UI 动作类型（按钮、复选框等）
	 * - 快捷键绑定
	 */
	virtual void RegisterCommands() override;

public:
	/**
	 * OpenPluginWindow - 打开插件窗口命令
	 *
	 * 这是一个 UICommandInfo 智能指针
	 * 代表 "打开插件窗口" 的命令，可被：
	 * - 工具栏按钮使用
	 * - 菜单项使用
	 * - 快捷键触发
	 */
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};