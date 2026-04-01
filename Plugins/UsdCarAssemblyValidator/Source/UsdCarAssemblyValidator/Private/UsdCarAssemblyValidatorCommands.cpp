// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 文件: UsdCarAssemblyValidatorCommands.cpp
// 功能: USD 汽车装配验证器插件的命令注册实现
// 描述: 定义并注册插件的所有 UI 命令
// ============================================================

#include "UsdCarAssemblyValidatorCommands.h"

// 定义本地化文本命名空间
#define LOCTEXT_NAMESPACE "FUsdCarAssemblyValidatorModule"

/**
 * RegisterCommands - 注册所有命令
 *
 * 使用 UI_COMMAND 宏定义命令
 * UI_COMMAND 宏参数说明：
 * - 第1个参数: 命令变量名（成员变量指针）
 * - 第2个参数: 命令的显示名称（本地化文本）
 * - 第3个参数: 命令的描述说明（本地化文本）
 * - 第4个参数: UI 动作类型（Button/CheckBox/ToggleButton等）
 * - 第5个参数: 快捷键绑定（FInputChord() 表示无快捷键）
 */
void FUsdCarAssemblyValidatorCommands::RegisterCommands()
{
	// 注册 "打开插件窗口" 命令
	// - 名称: "UsdCarAssemblyValidator"
	// - 描述: "Bring up UsdCarAssemblyValidator window"（打开 USD 汽车装配验证器窗口）
	// - 类型: Button（按钮类型，点击执行动作）
	// - 快捷键: 无
	UI_COMMAND(OpenPluginWindow, "UsdCarAssemblyValidator", "Bring up UsdCarAssemblyValidator window", EUserInterfaceActionType::Button, FInputChord());
}

// 取消本地化文本命名空间定义
#undef LOCTEXT_NAMESPACE
