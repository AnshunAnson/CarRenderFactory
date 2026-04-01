// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"
#include "Containers/Array.h"

/**
 * FValidationResultItem - 验证结果条目结构
 *
 * 存储单个部件的验证结果
 * 包含验证状态、部件类型和详细消息
 */
struct FValidationResultItem
{
	/**
	 * EValidationStatus - 验证状态枚举
	 */
	enum EValidationStatus
	{
		Status_Success,    // 成功：部件验证通过
		Status_Warning,    // 警告：部件存在非致命问题
		Status_Error       // 错误：部件存在致命问题
	};

	EValidationStatus Status;    // 验证状态
	FString PartType;            // 部件类型
	FString Message;             // 详细消息说明

	/**
	 * 默认构造函数
	 */
	FValidationResultItem()
		: Status(Status_Success)
		, PartType(TEXT(""))
		, Message(TEXT(""))
	{
	}

	/**
	 * 参数化构造函数
	 *
	 * @param InStatus 验证状态
	 * @param InPart 部件类型
	 * @param InMsg 详细消息
	 */
	FValidationResultItem(EValidationStatus InStatus, const FString& InPart, const FString& InMsg)
		: Status(InStatus)
		, PartType(InPart)
		, Message(InMsg)
	{
	}
};

/**
 * FAssemblyValidationReport - 装配验证报告结构
 *
 * 存储完整的装配验证结果
 * 包括统计数据、详细结果列表和识别的部件
 */
struct FAssemblyValidationReport
{
	bool bIsValid;                              // 整体验证是否通过
	int32 SuccessCount;                         // 验证成功的部件数
	int32 WarningCount;                         // 有警告的部件数
	int32 ErrorCount;                           // 有错误的部件数

	TArray<FValidationResultItem> Results;      // 每个部件的详细验证结果
	TArray<FString> IdentifiedParts;            // 所有识别的部件列表

	/**
	 * 默认构造函数
	 */
	FAssemblyValidationReport()
		: bIsValid(false)
		, SuccessCount(0)
		, WarningCount(0)
		, ErrorCount(0)
	{
	}
};

/**
 * FAssemblyValidator - 装配验证器类
 *
 * 负责验证 USD 装配是否符合规则
 *
 * 验证内容包括:
 * - 必需部件是否存在
 * - 部件网格是否有效
 * - 部件材质是否有效
 * - 部件与变体的兼容性
 */
class FAssemblyValidator
{
public:
	/**
	 * 构造函数
	 */
	FAssemblyValidator();

	/**
	 * 析构函数
	 */
	~FAssemblyValidator();

	/**
	 * ValidateAssembly - 验证 USD 装配是否有效
	 *
	 * 执行完整的装配验证流程
	 *
	 * @param UsdFilePath USD 文件路径
	 * @param SelectedVariant 选中的变体名称
	 * @return FAssemblyValidationReport 验证报告
	 */
	FAssemblyValidationReport ValidateAssembly(const FString& UsdFilePath, const FString& SelectedVariant);

	/**
	 * ValidatePart - 验证单个部件
	 *
	 * 检查指定部件是否存在且有效
	 *
	 * @param PartType 部件类型
	 * @return true 部件有效
	 * @return false 部件无效或不存在
	 */
	bool ValidatePart(const FString& PartType);

	/**
	 * ValidatePartVariantCompatibility - 验证部件与变体的兼容性
	 *
	 * 检查指定部件是否支持指定的变体
	 *
	 * @param PartType 部件类型
	 * @param VariantName 变体名称
	 * @return true 兼容
	 * @return false 不兼容
	 */
	bool ValidatePartVariantCompatibility(const FString& PartType, const FString& VariantName);

	/**
	 * GenerateReportText - 生成验证报告的文本表示
	 *
	 * 将验证报告转换为可读的文本格式
	 *
	 * @param Report 验证报告
	 * @return FString 格式化的报告文本
	 */
	FString GenerateReportText(const FAssemblyValidationReport& Report);

private:
	/**
	 * CheckMissingParts - 检查所有必需部件是否存在
	 *
	 * 比较识别的部件列表与必需部件列表
	 *
	 * @param IdentifiedParts 识别的部件列表
	 * @return TArray<FString> 缺失的部件列表
	 */
	TArray<FString> CheckMissingParts(const TArray<FString>& IdentifiedParts);

	/**
	 * ValidatePartAssets - 验证部件的网格和材质
	 *
	 * 检查部件的网格资源和材质是否有效
	 *
	 * @param PartType 部件类型
	 * @return FValidationResultItem 验证结果
	 */
	FValidationResultItem ValidatePartAssets(const FString& PartType);
};

/**
 * FAssemblyInjector - 装配注入器类
 *
 * 负责将验证通过的装配注入到 CCR 蓝图中
 *
 * 注入流程:
 * 1. 查找目标 CCR 蓝图 Actor
 * 2. 遍历验证通过的部件
 * 3. 为每个部件槽位设置静态网格
 * 4. 为每个部件设置材质
 */
class FAssemblyInjector
{
public:
	/**
	 * 构造函数
	 */
	FAssemblyInjector();

	/**
	 * 析构函数
	 */
	~FAssemblyInjector();

	/**
	 * Initialize - 初始化注入器
	 *
	 * 准备注入所需的资源和引用
	 *
	 * @return true 初始化成功
	 * @return false 初始化失败
	 */
	bool Initialize();

	/**
	 * InjectToBlueprint - 将装配注入到 CCR 蓝图实例
	 *
	 * 执行完整的注入流程
	 *
	 * @param TargetActorPath 目标 Actor 的路径或名称
	 * @param ValidationReport 验证报告（包含部件信息）
	 * @return true 注入成功
	 * @return false 注入失败
	 */
	bool InjectToBlueprint(const FString& TargetActorPath, const FAssemblyValidationReport& ValidationReport);

	/**
	 * SetSlotMesh - 设置部件槽位的静态网格
	 *
	 * 为指定的槽位设置静态网格资源
	 *
	 * @param TargetActor 目标 Actor
	 * @param SlotName 槽位名称
	 * @param MeshPath 网格资源路径
	 * @return true 设置成功
	 * @return false 设置失败
	 */
	bool SetSlotMesh(class AActor* TargetActor, const FString& SlotName, const FString& MeshPath);

	/**
	 * SetSlotMaterial - 设置部件槽位的材质
	 *
	 * 为指定的槽位设置材质
	 *
	 * @param TargetActor 目标 Actor
	 * @param SlotName 槽位名称
	 * @param MaterialPath 材质资源路径
	 * @return true 设置成功
	 * @return false 设置失败
	 */
	bool SetSlotMaterial(class AActor* TargetActor, const FString& SlotName, const FString& MaterialPath);

	/**
	 * GetInjectionLog - 获取注入结果日志
	 *
	 * 返回注入过程中的所有日志消息
	 *
	 * @return TArray<FString> 日志消息数组
	 */
	TArray<FString> GetInjectionLog() const;

private:
	/**
	 * FindTargetActor - 查找目标 CCR 蓝图实例
	 *
	 * 在当前场景中查找指定路径或名称的 Actor
	 *
	 * @param ActorPath Actor 路径或名称
	 * @return AActor* 找到的 Actor 指针，找不到返回 nullptr
	 */
	class AActor* FindTargetActor(const FString& ActorPath);

	/**
	 * LoadMesh - 从网格路径加载网格资源
	 *
	 * 使用同步加载方式加载静态网格
	 *
	 * @param MeshPath 网格资源路径
	 * @return UStaticMesh* 加载的网格指针，加载失败返回 nullptr
	 */
	class UStaticMesh* LoadMesh(const FString& MeshPath);

	/**
	 * LoadMaterial - 从材质路径加载材质资源
	 *
	 * 使用同步加载方式加载材质
	 *
	 * @param MaterialPath 材质资源路径
	 * @return UMaterialInterface* 加载的材质指针，加载失败返回 nullptr
	 */
	class UMaterialInterface* LoadMaterial(const FString& MaterialPath);

	/**
	 * InjectionLog - 注入过程的日志
	 *
	 * 记录注入过程中的所有操作和结果
	 */
	TArray<FString> InjectionLog;
};
