// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "Modules/ModuleManager.h"
// 包含模块管理器头文件，用于模块的注册和管理

// 前向声明工具栏和菜单构建器类，减少头文件依赖
class FToolBarBuilder;
class FMenuBuilder;

/**
 * USD 汽车装配验证器模块类
 *
 * 这是插件的主模块类，继承自 IModuleInterface 接口
 * 负责：
 * - 插件的启动和关闭
 * - 注册菜单和工具栏
 * - 管理插件窗口的创建
 */
class FUsdCarAssemblyValidatorModule : public IModuleInterface
{
public:

	/** IModuleInterface 接口实现 */

	/**
	 * StartupModule - 模块启动时调用
	 * 在模块加载到内存后执行，负责：
	 * - 初始化样式资源
	 * - 注册命令
	 * - 设置菜单扩展
	 * - 注册标签页生成器
	 */
	virtual void StartupModule() override;

	/**
	 * ShutdownModule - 模块关闭时调用
	 * 在模块卸载前执行，负责：
	 * - 清理菜单注册
	 * - 注销样式
	 * - 注销命令
	 * - 移除标签页生成器
	 */
	virtual void ShutdownModule() override;

	/**
	 * PluginButtonClicked - 插件按钮点击回调
	 * 当用户点击工具栏按钮或菜单项时调用
	 * 功能：打开插件的主窗口（标签页）
	 */
	void PluginButtonClicked();

private:

	/**
	 * RegisterMenus - 注册菜单项
	 * 将插件命令注册到：
	 * - 主菜单的 Window 菜单下
	 * - 编辑器工具栏中
	 */
	void RegisterMenus();

	/**
	 * OnSpawnPluginTab - 生成插件标签页
	 * 当需要创建插件窗口时调用
	 * @param SpawnTabArgs 标签页生成参数
	 * @return 返回新创建的 SDockTab 控件
	 */
	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	/**
	 * PluginCommands - 插件命令列表
	 * 存储所有与该插件相关的 UI 命令
	 * 用于绑定按钮点击事件和快捷键
	 */
	TSharedPtr<class FUICommandList> PluginCommands;
};
