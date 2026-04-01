// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"

/**
 * USD Car Assembly Validator Plugin
 * USD 汽车装配验证器插件
 *
 * 本插件提供验证 USD 汽车装配的 UI 界面，具有以下功能：
 *
 * 布局结构：
 *
 * 顶部区域 (TOP SECTION):
 * - USD 文件选择下拉框
 * - 控制按钮：重新加载 Stage、扫描变体、验证装配
 *
 * 中间区域 (MIDDLE SECTION - 三栏分割):
 * - 左侧面板：变体集 (TreeView 树形视图)
 * - 中间面板：当前选中的变体 (文本显示)
 * - 右侧面板：识别的部件列表 (ListView 列表视图)
 *   包含：Body, Wheel_FL, Wheel_FR, Wheel_RL, Wheel_RR, Caliper_FL, SteeringWheel
 *
 * 底部区域 (BOTTOM SECTION):
 * - 操作按钮："注入到选中的 CCR 蓝图"
 * - 统计信息：成功数 / 警告数 / 失败数
 * - 日志输出：带时间戳的多行只读文本框
 *
 * 主要功能:
 * - 加载和验证 USD stage 文件
 * - 扫描和显示变体集
 * - 显示当前选中的变体
 * - 列出识别的装配部件
 * - 验证装配结构
 * - 将验证通过的装配注入到 CCR 蓝图
 * - 显示带颜色编码的操作日志（成功/警告/错误）
 */

/**
 * FAssemblyValidatorStageData - UI 层使用的 USD Stage 信息数据结构
 *
 * 存储加载的 USD Stage 的所有相关信息
 * 包括文件路径、变体集、识别的部件等
 */
struct FAssemblyValidatorStageData
{
	/**
	 * StageFilePath - Stage 文件路径
	 * 存储 USD 文件的完整路径
	 * 例如: "D:/Assets/Cars/CarModel.usda"
	 */
	FString StageFilePath;

	/**
	 * VariantSets - 变体集名称数组
	 * 存储所有找到的变体集名称
	 * 例如: ["ColorVariant", "WheelVariant", "InteriorVariant"]
	 */
	TArray<FString> VariantSets;

	/**
	 * IdentifiedParts - 识别的部件列表
	 * 存储从 USD Stage 中识别出的所有汽车部件名称
	 * 例如: ["Body", "Wheel_FL", "Wheel_FR", "Wheel_RL", "Wheel_RR"]
	 */
	TArray<FString> IdentifiedParts;

	/**
	 * CurrentVariant - 当前选中的变体
	 * 存储用户当前选择的变体名称
	 */
	FString CurrentVariant;
};

/**
 * FAssemblyValidationResult - 装配验证结果数据结构
 *
 * 存储装配验证的结果信息
 * 包括验证状态、警告/错误数量、验证消息
 */
struct FAssemblyValidationResult
{
	/**
	 * bIsValid - 装配是否有效
	 * true: 装配验证通过
	 * false: 装配存在问题
	 */
	bool bIsValid;

	/**
	 * WarningCount - 警告数量
	 * 非致命问题的数量（如材质缺失但使用默认材质）
	 */
	int32 WarningCount;

	/**
	 * ErrorCount - 错误数量
	 * 致命问题的数量（如必需部件缺失）
	 */
	int32 ErrorCount;

	/**
	 * ValidationMessages - 验证消息列表
	 * 存储所有验证过程中产生的消息
	 * 包括成功、警告、错误三种类型的信息
	 */
	TArray<FString> ValidationMessages;
};
