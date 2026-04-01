// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "Styling/SlateStyle.h"  // Slate 样式系统头文件

/**
 * FUsdCarAssemblyValidatorStyle - 插件样式管理类
 *
 * 负责：
 * - 管理插件的视觉样式资源（图标、颜色、画笔等）
 * - 提供样式集的创建、初始化和清理
 * - 支持纹理资源的重新加载
 *
 * 所有方法都是静态的，这是一个单例模式的样式管理器
 */
class FUsdCarAssemblyValidatorStyle
{
public:

	/**
	 * Initialize - 初始化样式系统
	 *
	 * 创建样式集实例并注册到 Slate 样式注册表
	 * 如果样式实例已存在则不做任何操作
	 * 必须在使用样式前调用
	 */
	static void Initialize();

	/**
	 * Shutdown - 关闭样式系统
	 *
	 * 从 Slate 样式注册表注销样式集
	 * 释放样式实例资源
	 * 在模块卸载时调用
	 */
	static void Shutdown();

	/**
	 * ReloadTextures - 重新加载纹理资源
	 *
	 * 通知 Slate 渲染器重新加载所有纹理资源
	 * 在样式资源更新后调用
	 */
	static void ReloadTextures();

	/**
	 * Get - 获取样式集引用
	 *
	 * @return 返回当前样式集的常量引用
	 *         用于访问样式中的画笔、颜色等资源
	 */
	static const ISlateStyle& Get();

	/**
	 * GetStyleSetName - 获取样式集名称
	 *
	 * @return 返回样式集的唯一标识符名称
	 *         用于命令系统关联样式
	 */
	static FName GetStyleSetName();

private:

	/**
	 * Create - 创建样式集
	 *
	 * 内部函数，创建并配置样式集
	 * 设置资源根目录，注册图标画笔等
	 *
	 * @return 返回新创建的样式集共享引用
	 */
	static TSharedRef< class FSlateStyleSet > Create();

private:

	/**
	 * StyleInstance - 样式集实例
	 *
	 * 存储当前的样式集智能指针
	 * 使用 TSharedPtr 管理生命周期
	 */
	static TSharedPtr< class FSlateStyleSet > StyleInstance;
};