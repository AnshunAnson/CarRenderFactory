// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 文件: UsdCarAssemblyValidatorStyle.cpp
// 功能: USD 汽车装配验证器插件的样式管理实现
// 描述: 实现样式集的创建、注册和资源管理
// ============================================================

#include "UsdCarAssemblyValidatorStyle.h"           // 样式类头文件
#include "Styling/SlateStyleRegistry.h"            // Slate 样式注册表
#include "Framework/Application/SlateApplication.h" // Slate 应用程序
#include "Slate/SlateGameResources.h"              // Slate 游戏资源
#include "Interfaces/IPluginManager.h"              // 插件管理器接口
#include "Styling/SlateStyleMacros.h"              // Slate 样式宏定义

// 定义一个宏简化路径转换
// RootToContentDir 将相对路径转换为插件内容目录的绝对路径
#define RootToContentDir Style->RootToContentDir

// 静态成员变量初始化：样式集实例指针，初始为空
TSharedPtr<FSlateStyleSet> FUsdCarAssemblyValidatorStyle::StyleInstance = nullptr;

/**
 * Initialize - 初始化样式系统
 *
 * 创建样式集实例并注册到全局样式注册表
 * 使用惰性初始化模式，只在首次调用时创建
 */
void FUsdCarAssemblyValidatorStyle::Initialize()
{
	// 检查样式实例是否已存在
	if (!StyleInstance.IsValid())
	{
		// 创建新的样式集
		StyleInstance = Create();
		// 将样式集注册到 Slate 全局样式注册表
		// 这样所有 Slate 控件都可以访问此样式
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

/**
 * Shutdown - 关闭样式系统
 *
 * 注销样式集并释放资源
 */
void FUsdCarAssemblyValidatorStyle::Shutdown()
{
	// 从全局样式注册表注销样式集
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);

	// 确保样式实例是唯一引用（没有其他地方持有）
	// 如果不是唯一引用，说明有内存泄漏风险
	ensure(StyleInstance.IsUnique());

	// 重置智能指针，释放样式集资源
	StyleInstance.Reset();
}

/**
 * GetStyleSetName - 获取样式集名称
 *
 * @return 返回样式集的唯一标识符 FName
 */
FName FUsdCarAssemblyValidatorStyle::GetStyleSetName()
{
	// 使用静态变量缓存样式集名称
	static FName StyleSetName(TEXT("UsdCarAssemblyValidatorStyle"));
	return StyleSetName;
}

// 定义常用图标尺寸常量
const FVector2D Icon16x16(16.0f, 16.0f);  // 16x16 像素小图标
const FVector2D Icon20x20(20.0f, 20.0f);  // 20x20 像素中等图标

/**
 * Create - 创建样式集
 *
 * 创建新的样式集并配置所有样式资源
 *
 * @return 返回配置完成的样式集
 */
TSharedRef< FSlateStyleSet > FUsdCarAssemblyValidatorStyle::Create()
{
	// 创建新的样式集，指定样式集名称
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("UsdCarAssemblyValidatorStyle"));

	// 设置样式集的内容根目录
	// 获取插件基础目录，然后拼接 Resources 子目录
	// 例如: D:/UE/Project/Plugins/UsdCarAssemblyValidator/Resources/
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("UsdCarAssemblyValidator")->GetBaseDir() / TEXT("Resources"));

	// 注册插件窗口按钮图标
	// IMAGE_BRUSH_SVG 宏创建一个 SVG 格式的图标画笔
	// 参数1: 图标文件名（不含扩展名）
	// 参数2: 图标尺寸
	// 最终路径: Resources/PlaceholderButtonIcon.svg
	Style->Set("UsdCarAssemblyValidator.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("PlaceholderButtonIcon"), Icon20x20));

	return Style;
}

/**
 * ReloadTextures - 重新加载纹理资源
 *
 * 通知 Slate 渲染器重新加载所有纹理资源
 * 用于热重载或资源更新场景
 */
void FUsdCarAssemblyValidatorStyle::ReloadTextures()
{
	// 检查 Slate 应用程序是否已初始化
	if (FSlateApplication::IsInitialized())
	{
		// 获取渲染器并重新加载纹理资源
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

/**
 * Get - 获取样式集引用
 *
 * @return 返回当前样式集的常量引用
 */
const ISlateStyle& FUsdCarAssemblyValidatorStyle::Get()
{
	// 解引用智能指针返回样式集
	return *StyleInstance;
}
