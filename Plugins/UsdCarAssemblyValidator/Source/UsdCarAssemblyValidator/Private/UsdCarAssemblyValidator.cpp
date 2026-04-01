// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 文件: UsdCarAssemblyValidator.cpp
// 功能: USD 汽车装配验证器插件的主模块实现
// 描述: 实现插件的启动、关闭、菜单注册和窗口创建功能
// ============================================================

#include "UsdCarAssemblyValidator.h"           // 主模块头文件
#include "UsdCarAssemblyValidatorStyle.h"      // 样式管理器头文件
#include "UsdCarAssemblyValidatorCommands.h"   // 命令定义头文件
#include "SAssemblyValidatorPanel.h"           // 主界面面板控件
#include "LevelEditor.h"                       // 关卡编辑器相关
#include "Widgets/Docking/SDockTab.h"          // 可停靠标签页控件
#include "Widgets/Layout/SBox.h"               // 盒子布局控件
#include "Widgets/Text/STextBlock.h"           // 文本显示控件
#include "ToolMenus.h"                         // 工具菜单系统

// 定义插件的标签页唯一标识符名称
static const FName UsdCarAssemblyValidatorTabName("UsdCarAssemblyValidator");

// 定义本地化文本命名空间，用于多语言支持
#define LOCTEXT_NAMESPACE "FUsdCarAssemblyValidatorModule"

/**
 * StartupModule - 模块启动函数
 *
 * 当模块被加载到内存后调用此函数
 * 执行顺序：
 * 1. 初始化样式资源（图标、颜色等）
 * 2. 重新加载纹理资源
 * 3. 注册 UI 命令
 * 4. 创建命令列表并绑定动作
 * 5. 注册菜单启动回调
 * 6. 注册标签页生成器
 */
void FUsdCarAssemblyValidatorModule::StartupModule()
{
	// 模块加载后执行此代码，具体时机在 .uplugin 文件中配置

	// 初始化样式系统，加载插件图标等资源
	FUsdCarAssemblyValidatorStyle::Initialize();

	// 重新加载纹理资源，确保 Slate 渲染器使用最新纹理
	FUsdCarAssemblyValidatorStyle::ReloadTextures();

	// 注册 UI 命令，将命令与样式关联
	FUsdCarAssemblyValidatorCommands::Register();

	// 创建共享的命令列表对象，用于存储和管理命令绑定
	PluginCommands = MakeShareable(new FUICommandList);

	// 将 "打开插件窗口" 命令与点击回调绑定
	// MapAction 参数说明：
	// - 第一个参数：要绑定的命令
	// - 第二个参数：执行动作（点击时调用的函数）
	// - 第三个参数：可执行条件（默认总是可执行）
	PluginCommands->MapAction(
		FUsdCarAssemblyValidatorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FUsdCarAssemblyValidatorModule::PluginButtonClicked),
		FCanExecuteAction());

	// 注册工具菜单的启动回调，在编辑器启动时注册菜单项
	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FUsdCarAssemblyValidatorModule::RegisterMenus));

	// 注册 Nomad 标签页生成器（可停靠的面板）
	// Nomad 标签页可以停靠在编辑器的任意位置
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(UsdCarAssemblyValidatorTabName, FOnSpawnTab::CreateRaw(this, &FUsdCarAssemblyValidatorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FUsdCarAssemblyValidatorTabTitle", "UsdCarAssemblyValidator"))  // 设置显示名称
		.SetMenuType(ETabSpawnerMenuType::Hidden);  // 隐藏菜单类型，不显示在窗口菜单中
}

/**
 * ShutdownModule - 模块关闭函数
 *
 * 在模块卸载前调用，执行清理工作
 * 注意：支持动态重载的模块需要在此函数中清理所有资源
 */
void FUsdCarAssemblyValidatorModule::ShutdownModule()
{
	// 此函数在模块卸载期间调用，用于清理资源
	// 支持动态重载的模块需要在卸载前执行清理

	// 取消注册菜单启动回调
	UToolMenus::UnRegisterStartupCallback(this);

	// 取消注册所有由此模块拥有的菜单项
	UToolMenus::UnregisterOwner(this);

	// 关闭样式系统，释放样式资源
	FUsdCarAssemblyValidatorStyle::Shutdown();

	// 取消注册 UI 命令
	FUsdCarAssemblyValidatorCommands::Unregister();

	// 取消注册 Nomad 标签页生成器
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(UsdCarAssemblyValidatorTabName);
}

/**
 * OnSpawnPluginTab - 生成插件标签页
 *
 * 当需要创建插件窗口时调用
 * 创建一个可停靠的标签页，内容为装配验证面板
 *
 * @param SpawnTabArgs 标签页生成参数（包含标签页 ID 等信息）
 * @return 返回新创建的可停靠标签页控件
 */
TSharedRef<SDockTab> FUsdCarAssemblyValidatorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	// 使用 SNew 宏创建 SDockTab 控件
	// TabRole 设置为 NomadTab 表示这是一个可自由停靠的面板
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// 标签页内容为装配验证面板控件
			SNew(SAssemblyValidatorPanel)
		];
}

/**
 * PluginButtonClicked - 插件按钮点击处理
 *
 * 当用户点击工具栏按钮或菜单项时调用
 * 功能：打开或聚焦插件的主窗口标签页
 */
void FUsdCarAssemblyValidatorModule::PluginButtonClicked()
{
	// TryInvokeTab 尝试打开指定的标签页
	// 如果标签页已存在则聚焦，不存在则创建
	FGlobalTabmanager::Get()->TryInvokeTab(UsdCarAssemblyValidatorTabName);
}

/**
 * RegisterMenus - 注册菜单项
 *
 * 将插件入口添加到编辑器的菜单和工具栏中
 * 注册到两个位置：
 * 1. 主菜单 -> Window 菜单 -> WindowLayout 区域
 * 2. 关卡编辑器工具栏 -> PlayToolBar -> PluginTools 区域
 */
void FUsdCarAssemblyValidatorModule::RegisterMenus()
{
	// 创建菜单所有者作用域
	// OwnerScoped 对象析构时会自动注册所有者，用于后续清理
	FToolMenuOwnerScoped OwnerScoped(this);

	// ===== 注册到主菜单的 Window 菜单 =====
	{
		// 扩展关卡编辑器的主菜单
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			// 查找或创建 WindowLayout 区域
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			// 添加菜单项，绑定到命令列表
			Section.AddMenuEntryWithCommandList(FUsdCarAssemblyValidatorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	// ===== 注册到关卡编辑器工具栏 =====
	{
		// 扩展关卡编辑器的播放工具栏
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			// 查找或创建 PluginTools 区域
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				// 添加工具栏按钮
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FUsdCarAssemblyValidatorCommands::Get().OpenPluginWindow));
				// 设置命令列表
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

// 取消本地化文本命名空间定义
#undef LOCTEXT_NAMESPACE

// 实现模块导出
// 这会创建模块的导出函数，使引擎能够加载此模块
// 参数1: 模块类名
// 参数2: 模块名称（字符串）
IMPLEMENT_MODULE(FUsdCarAssemblyValidatorModule, UsdCarAssemblyValidator)