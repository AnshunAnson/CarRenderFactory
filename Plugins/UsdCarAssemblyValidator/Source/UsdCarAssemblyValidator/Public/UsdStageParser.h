// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"
#include "Containers/Array.h"

// 前向声明日志分类
DECLARE_LOG_CATEGORY_EXTERN(LogUsdStageParser, Log, All);

/**
 * FUsdPrimInfo - USD Prim 信息结构
 *
 * 存储 USD Stage 中单个 Prim（场景元素）的基本信息
 * 包括路径、名称、类型以及识别出的部件类型
 */
struct USDCARASSEMBLYVALIDATOR_API FUsdPrimInfo
{
	FString PrimPath;          // Prim 的完整路径，例如: "/Car/Body"
	FString PrimName;          // Prim 的名称，例如: "Body"
	FString PrimType;          // Prim 的类型，例如: "Mesh", "Xform", "Scope" 等

	// ===== 部件类型判定 =====
	/**
	 * PartType - 识别出的汽车部件类型
	 * 可能的值:
	 * - "Body"        车身
	 * - "Wheel_FL"    左前轮
	 * - "Wheel_FR"    右前轮
	 * - "Wheel_RL"    左后轮
	 * - "Wheel_RR"    右后轮
	 * - "Caliper_FL"  左前卡钳
	 * - "SteeringWheel" 方向盘
	 */
	FString PartType;

	// ===== 网格信息 =====
	FString MeshPath;          // 关联的网格资源路径（如果存在）

	/**
	 * 默认构造函数
	 * 初始化所有字符串为空
	 */
	FUsdPrimInfo()
		: PrimPath(TEXT(""))
		, PrimName(TEXT(""))
		, PrimType(TEXT(""))
		, PartType(TEXT(""))
		, MeshPath(TEXT(""))
	{
	}
};

/**
 * FUsdVariantInfo - USD Variant 信息结构
 *
 * 存储单个变体的基本信息
 * 变体是 USD 中用于表示同一物体的不同版本（如不同颜色的车身）
 */
struct USDCARASSEMBLYVALIDATOR_API FUsdVariantInfo
{
	FString VariantName;       // 变体名称，例如: "Red", "Blue", "Carbon"
	int32 Index;               // 变体在 VariantSet 中的索引位置

	/**
	 * 默认构造函数
	 */
	FUsdVariantInfo()
		: VariantName(TEXT(""))
		, Index(-1)  // -1 表示无效索引
	{
	}
};

/**
 * FUsdVariantSetInfo - USD VariantSet 信息结构
 *
 * 存储一个 VariantSet（变体集）的完整信息
 * 一个 VariantSet 包含多个变体，例如 "ColorVariant" 包含 "Red", "Blue" 等
 */
struct USDCARASSEMBLYVALIDATOR_API FUsdVariantSetInfo
{
	FString VariantSetName;              // VariantSet 名称，例如: "ColorVariant"
	TArray<FUsdVariantInfo> Variants;    // 该 Set 下的所有变体列表
	int32 CurrentVariantIndex;           // 当前激活的变体索引（-1 表示无激活变体）

	/**
	 * 默认构造函数
	 */
	FUsdVariantSetInfo()
		: VariantSetName(TEXT(""))
		, CurrentVariantIndex(-1)
	{
	}
};

/**
 * FUsdStageData - USD Stage 数据结构
 *
 * 存储整个 USD Stage 的解析结果
 * 包括文件路径、所有变体集、识别的部件等
 *
 * 这是 Stage 解析的主要输出数据结构
 */
struct USDCARASSEMBLYVALIDATOR_API FUsdStageData
{
	FString StagePath;                           // Stage 文件的完整路径
	TArray<FUsdVariantSetInfo> VariantSets;      // Stage 中所有的 VariantSet
	TArray<FUsdPrimInfo> IdentifiedParts;        // 识别出的汽车部件列表

	/**
	 * 默认构造函数
	 */
	FUsdStageData()
		: StagePath(TEXT(""))
	{
	}
};

/**
 * FUsdStageParser - USD Stage 解析器
 *
 * 负责打开、读取和解析 USD 文件
 * 主要功能:
 * - 打开/关闭 USD Stage
 * - 提取 VariantSet 信息
 * - 扫描并识别汽车装配部件
 * - 切换变体选择
 *
 * 使用 void* 存储 USD Stage 指针，避免在头文件中包含 USD 库
 */
class USDCARASSEMBLYVALIDATOR_API FUsdStageParser
{
public:
	/**
	 * 构造函数
	 * 初始化内部状态
	 */
	FUsdStageParser();

	/**
	 * 析构函数
	 * 自动关闭已打开的 Stage
	 */
	~FUsdStageParser();

	/**
	 * OpenStage - 打开 USD Stage 文件
	 *
	 * @param UsdFilePath USD 文件的完整路径（.usda, .usd, .usdc 等）
	 * @return true 打开成功
	 * @return false 打开失败（文件不存在、格式错误等）
	 */
	bool OpenStage(const FString& UsdFilePath);

	/**
	 * CloseStage - 关闭当前 Stage
	 *
	 * 释放 Stage 资源，重置内部状态
	 */
	void CloseStage();

	/**
	 * GetVariantSets - 获取所有 VariantSet
	 *
	 * 返回当前 Stage 中定义的所有 VariantSet 信息
	 *
	 * @return VariantSet 信息数组
	 */
	TArray<FUsdVariantSetInfo> GetVariantSets() const;

	/**
	 * ScanCarAssemblyParts - 扫描识别汽车装配部件
	 *
	 * 遍历 Stage 的 Prim 树，识别并分类汽车部件
	 * 基于命名规则和路径匹配识别部件类型
	 *
	 * @return 识别出的部件信息数组
	 */
	TArray<FUsdPrimInfo> ScanCarAssemblyParts();

	/**
	 * SelectVariant - 选择一个变体
	 *
	 * 在指定的 VariantSet 中切换到指定的 Variant
	 * 这会改变 Stage 的当前状态
	 *
	 * @param VariantSetName VariantSet 名称
	 * @param VariantName 要切换到的 Variant 名称
	 * @return true 切换成功
	 * @return false 切换失败（名称不存在等）
	 */
	bool SelectVariant(const FString& VariantSetName, const FString& VariantName);

	/**
	 * GetStageData - 获取当前 Stage 数据
	 *
	 * 返回包含所有解析结果的数据结构
	 *
	 * @return Stage 数据的副本
	 */
	FUsdStageData GetStageData() const;

	/**
	 * IsStageOpen - 检查 Stage 是否已打开
	 *
	 * @return true Stage 已打开
	 * @return false Stage 未打开
	 */
	bool IsStageOpen() const;

private:
	/**
	 * IdentifyPartType - 从 Prim 路径判定部件类型
	 *
	 * 基于 Prim 的路径和名称，使用命名规则识别汽车部件类型
	 * 例如: "/Car/Wheel_FL" -> "Wheel_FL"
	 *
	 * @param PrimPath Prim 的完整路径
	 * @param PrimName Prim 的名称
	 * @return 识别出的部件类型字符串
	 */
	FString IdentifyPartType(const FString& PrimPath, const FString& PrimName);

	/**
	 * RecursiveScanPrims - 递归遍历 Prim 树扫描部件
	 *
	 * 从指定路径开始，递归遍历所有子 Prim
	 * 对每个 Mesh 类型的 Prim 进行部件识别
	 *
	 * @param PrimPath 起始 Prim 路径
	 * @param OutParts 输出参数，收集识别到的部件
	 */
	void RecursiveScanPrims(const FString& PrimPath, TArray<FUsdPrimInfo>& OutParts);

	// ===== 内部数据成员 =====

	/**
	 * StageHandle - USD Stage 指针
	 *
	 * 使用 void* 类型存储，避免在头文件中包含 USD 库
	 * 实际类型为 pxr::UsdStageRefPtr
	 */
	void* StageHandle;

	/**
	 * CurrentStageData - 当前 Stage 的缓存数据
	 */
	FUsdStageData CurrentStageData;

	/**
	 * bIsStageOpen - Stage 打开状态标志
	 */
	bool bIsStageOpen;
};
