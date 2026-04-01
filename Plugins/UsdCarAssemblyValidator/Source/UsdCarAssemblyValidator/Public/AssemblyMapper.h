// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"
#include "Containers/Map.h"
#include "UsdStageParser.h"

/**
 * FPartMapping - 部件映射规则结构
 *
 * 定义 USD Prim 到 CCR 蓝图槽位的映射关系
 * 用于将 USD 中的部件正确放置到蓝图实例的对应槽位
 */
struct FPartMapping
{
	FString UsdPartName;       // USD 部件名称，例如: "Body", "Wheel_FL", "Wheel_FR"
	FString CcrSlotName;       // CCR 蓝图槽位名称，对应蓝图中的组件插槽
	FString DefaultMaterial;   // 默认材质路径，UE 资源路径格式

	/**
	 * 默认构造函数
	 * 初始化所有字符串为空
	 */
	FPartMapping()
		: UsdPartName(TEXT(""))
		, CcrSlotName(TEXT(""))
		, DefaultMaterial(TEXT(""))
	{
	}

	/**
	 * 参数化构造函数
	 *
	 * @param InUsdPart USD 部件名称
	 * @param InSlot CCR 槽位名称
	 * @param InMaterial 默认材质路径
	 */
	FPartMapping(const FString& InUsdPart, const FString& InSlot, const FString& InMaterial)
		: UsdPartName(InUsdPart)
		, CcrSlotName(InSlot)
		, DefaultMaterial(InMaterial)
	{
	}
};

/**
 * FMaterialMapping - 材质映射规则结构
 *
 * 定义 USD 变体到 UE 材质的映射关系
 * 允许根据 USD 中的变体选择自动应用对应的材质
 */
struct FMaterialMapping
{
	FString PartType;          // 部件类型，例如: "Body", "Wheel_FL"
	FString VariantName;       // 变体名称，例如: "Red", "Blue", "Carbon"
	FString MaterialPath;      // UE 材质资源路径，例如: "/Game/Materials/Car/Body_Red"

	/**
	 * 默认构造函数
	 */
	FMaterialMapping()
		: PartType(TEXT(""))
		, VariantName(TEXT(""))
		, MaterialPath(TEXT(""))
	{
	}

	/**
	 * 参数化构造函数
	 *
	 * @param InPart 部件类型
	 * @param InVariant 变体名称
	 * @param InMaterial 材质路径
	 */
	FMaterialMapping(const FString& InPart, const FString& InVariant, const FString& InMaterial)
		: PartType(InPart)
		, VariantName(InVariant)
		, MaterialPath(InMaterial)
	{
	}
};

/**
 * FMappingResult - 装配映射结果结构
 *
 * 存储单个部件映射操作的完整结果
 * 包括映射状态、槽位信息、推荐材质以及警告信息
 */
struct FMappingResult
{
	bool bIsValid;                    // 映射是否有效（是否找到对应的槽位）
	FString PartType;                 // 部件类型
	FString CcrSlotName;              // 对应的 CCR 槽位名称
	FString RecommendedMaterial;      // 推荐使用的材质路径
	TArray<FString> Warnings;         // 映射过程中产生的警告信息

	/**
	 * 默认构造函数
	 * bIsValid 默认为 false，表示映射失败
	 */
	FMappingResult()
		: bIsValid(false)
		, PartType(TEXT(""))
		, CcrSlotName(TEXT(""))
		, RecommendedMaterial(TEXT(""))
	{
	}
};

/**
 * FAssemblyMapper - 装配映射引擎
 *
 * 负责将 USD 部件映射到 CCR 蓝图槽位
 *
 * 主要功能:
 * - 维护部件映射表（USD 部件名 -> CCR 槽位名）
 * - 维护材质映射表（部件类型 + 变体名 -> 材质路径）
 * - 提供映射查询接口
 * - 支持自定义映射规则扩展
 *
 * 使用示例:
 * @code
 * FAssemblyMapper Mapper;
 * Mapper.InitializeMappings();
 * FMappingResult Result = Mapper.MapPartToCcr("Wheel_FL");
 * @endcode
 */
class FAssemblyMapper
{
public:
	/**
	 * 构造函数
	 */
	FAssemblyMapper();

	/**
	 * 析构函数
	 */
	~FAssemblyMapper();

	/**
	 * InitializeMappings - 初始化映射规则
	 *
	 * 加载默认的部件映射和材质映射
	 * 必须在使用其他方法前调用
	 */
	void InitializeMappings();

	/**
	 * MapPartToCcr - 查找 USD 部件对应的 CCR 槽位
	 *
	 * 根据部件类型查找映射规则，返回完整的映射结果
	 *
	 * @param PartType USD 部件类型，例如: "Body", "Wheel_FL"
	 * @return FMappingResult 映射结果，包含槽位名称、推荐材质等
	 */
	FMappingResult MapPartToCcr(const FString& PartType);

	/**
	 * MapVariantToMaterial - 查找部件对应的材质
	 *
	 * 根据部件类型和变体名称查找对应的材质路径
	 *
	 * @param PartType 部件类型
	 * @param VariantName 变体名称
	 * @return FString 材质路径，如果未找到返回空字符串
	 */
	FString MapVariantToMaterial(const FString& PartType, const FString& VariantName);

	/**
	 * AddPartMapping - 添加自定义部件映射
	 *
	 * 允许用户扩展或覆盖默认的部件映射规则
	 *
	 * @param Mapping 部件映射规则
	 */
	void AddPartMapping(const FPartMapping& Mapping);

	/**
	 * AddMaterialMapping - 添加自定义材质映射
	 *
	 * 允许用户扩展或覆盖默认的材质映射规则
	 *
	 * @param Mapping 材质映射规则
	 */
	void AddMaterialMapping(const FMaterialMapping& Mapping);

	/**
	 * ValidatePartMapping - 验证映射规则的有效性
	 *
	 * 检查指定部件类型是否存在有效的映射
	 *
	 * @param PartType 部件类型
	 * @return true 存在有效映射
	 * @return false 不存在映射
	 */
	bool ValidatePartMapping(const FString& PartType);

	/**
	 * GetSupportedPartTypes - 获取所有支持的部件类型
	 *
	 * 返回当前映射表中所有已定义的部件类型
	 *
	 * @return TArray<FString> 部件类型列表
	 */
	TArray<FString> GetSupportedPartTypes() const;

private:
	// ===== 映射表数据结构 =====

	/**
	 * PartMappings - 部件映射表
	 *
	 * Key: 部件类型 (PartType)
	 * Value: 完整的部件映射规则 (FPartMapping)
	 *
	 * 示例:
	 * "Body" -> {"Body", "BodySlot", "/Game/Materials/Car/Body_Default"}
	 * "Wheel_FL" -> {"Wheel_FL", "FrontLeftWheelSlot", "/Game/Materials/Car/Wheel_Default"}
	 */
	TMap<FString, FPartMapping> PartMappings;

	/**
	 * MaterialMappings - 材质映射表
	 *
	 * 两级映射:
	 * 第一级 Key: 部件类型 (PartType)
	 * 第二级 Key: 变体名称 (VariantName)
	 * Value: 材质路径 (MaterialPath)
	 *
	 * 示例:
	 * "Body" -> {"Red" -> "/Game/Materials/Car/Body_Red",
	 *            "Blue" -> "/Game/Materials/Car/Body_Blue"}
	 */
	TMap<FString, TMap<FString, FString>> MaterialMappings;

	/**
	 * LoadDefaultPartMappings - 加载默认的部件映射规则
	 *
	 * 初始化标准的汽车部件映射
	 * 包括: Body, Wheel_FL/FR/RL/RR, Caliper_FL, SteeringWheel 等
	 */
	void LoadDefaultPartMappings();

	/**
	 * LoadDefaultMaterialMappings - 加载默认的材质映射规则
	 *
	 * 初始化标准的材质变体映射
	 * 包括各种颜色和材质选项
	 */
	void LoadDefaultMaterialMappings();
};
