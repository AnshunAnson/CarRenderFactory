// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"      // 复合控件基类
#include "Widgets/Views/STreeView.h"      // 树形视图控件
#include "Widgets/Views/SListView.h"      // 列表视图控件

// 前向声明，减少头文件依赖
class SButton;                    // 按钮控件
class SComboButton;               // 下拉按钮控件
class STextBlock;                 // 文本显示控件
class SMultiLineEditableTextBox;  // 多行可编辑文本框控件

/**
 * SAssemblyValidatorPanel - 装配验证器主面板控件
 *
 * 这是插件的主要 UI 面板，继承自 SCompoundWidget
 *
 * 布局结构:
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 顶部区域: USD 文件选择 + 控制按钮                              │
 * ├─────────────┬─────────────────┬─────────────────────────────┤
 * │ 左侧面板    │ 中间面板        │ 右侧面板                     │
 * │ 变体集树    │ 当前变体显示    │ 识别的部件列表               │
 * ├─────────────┴─────────────────┴─────────────────────────────┤
 * │ 底部区域: 注入按钮 + 统计信息 + 日志输出                       │
 * └─────────────────────────────────────────────────────────────┘
 */
class SAssemblyValidatorPanel : public SCompoundWidget
{
public:
	/**
	 * SLATE_BEGIN_ARGS / SLATE_END_ARGS 宏
	 * 用于定义 Slate 控件的参数结构
	 * 允许使用声明式语法创建控件
	 */
	SLATE_BEGIN_ARGS(SAssemblyValidatorPanel) {}
	SLATE_END_ARGS()

	/**
	 * Construct - 构造函数
	 *
	 * 初始化控件，创建所有子控件并设置布局
	 * Slate 控件的标准初始化方式
	 *
	 * @param InArgs 控件参数（通过 SLATE_BEGIN_ARGS 定义）
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * AddLogMessage - 添加日志消息
	 *
	 * 公共方法，供 UI 管理器调用
	 * 在日志输出区域添加一条消息
	 *
	 * @param Message 消息内容
	 * @param bIsError 是否为错误消息（红色显示）
	 * @param bIsWarning 是否为警告消息（黄色显示）
	 */
	void AddLogMessage(const FString& Message, bool bIsError = false, bool bIsWarning = false);

private:
	// ============================================================
	// UI 结构成员变量
	// ============================================================

	/**
	 * 顶部区域 - USD 文件选择
	 * UsdFileButton: 下拉按钮，用于选择 USD 文件
	 */
	TSharedPtr<SComboButton> UsdFileButton;

	/**
	 * 顶部区域 - 控制按钮
	 */
	TSharedPtr<SButton> ReloadStageButton;    // 重新加载 Stage 按钮
	TSharedPtr<SButton> ScanVariantsButton;   // 扫描变体按钮
	TSharedPtr<SButton> VerifyAssemblyButton; // 验证装配按钮

	/**
	 * 中间左侧区域 - 变体集列表
	 * VariantSetsTreeView: 树形视图，显示所有变体集
	 * VariantSetItems: 树形视图的数据源
	 */
	TSharedPtr<STreeView<TSharedPtr<FString>>> VariantSetsTreeView;
	TArray<TSharedPtr<FString>> VariantSetItems;

	/**
	 * 中间区域 - 当前选中的变体
	 * CurrentVariantTextBlock: 文本块，显示当前选中的变体名称
	 */
	TSharedPtr<STextBlock> CurrentVariantTextBlock;

	/**
	 * 中间右侧区域 - 识别的部件列表
	 * IdentifiedPartsListView: 列表视图，显示所有识别的部件
	 * IdentifiedParts: 列表视图的数据源
	 */
	TSharedPtr<SListView<TSharedPtr<FString>>> IdentifiedPartsListView;
	TArray<TSharedPtr<FString>> IdentifiedParts;

	/**
	 * 底部区域 - 操作按钮和日志
	 */
	TSharedPtr<SButton> InjectToCCRButton;                  // 注入到 CCR 蓝图按钮
	TSharedPtr<SMultiLineEditableTextBox> LogOutputTextBox; // 日志输出文本框

	/**
	 * 底部区域 - 统计信息显示
	 */
	TSharedPtr<STextBlock> SuccessStatTextBlock;  // 成功计数文本
	TSharedPtr<STextBlock> WarningStatTextBlock;  // 警告计数文本
	TSharedPtr<STextBlock> FailedStatTextBlock;   // 失败计数文本

	// ============================================================
	// 数据成员变量
	// ============================================================

	/**
	 * CurrentUsdFile - 当前加载的 USD 文件路径
	 */
	FString CurrentUsdFile;

	/**
	 * CurrentVariant - 当前选中的变体名称
	 */
	FString CurrentVariant;

	/**
	 * 统计计数器
	 */
	int32 SuccessCount = 0;   // 成功数量
	int32 WarningCount = 0;   // 警告数量
	int32 FailureCount = 0;   // 失败数量

	// ============================================================
	// 回调函数
	// ============================================================

	/**
	 * OnBrowseUsdFile - 浏览 USD 文件回调
	 * 打开文件对话框，让用户选择 USD 文件
	 * @return 返回 FReply::Handled() 表示事件已处理
	 */
	FReply OnBrowseUsdFile();

	/**
	 * OnReloadStage - 重新加载 Stage 回调
	 * 重新加载当前选中的 USD Stage
	 * @return 返回 FReply::Handled() 表示事件已处理
	 */
	FReply OnReloadStage();

	/**
	 * OnScanVariants - 扫描变体回调
	 * 扫描当前 Stage 中的所有变体集
	 * @return 返回 FReply::Handled() 表示事件已处理
	 */
	FReply OnScanVariants();

	/**
	 * OnVerifyAssembly - 验证装配回调
	 * 验证当前装配的结构是否正确
	 * @return 返回 FReply::Handled() 表示事件已处理
	 */
	FReply OnVerifyAssembly();

	/**
	 * OnInjectToCCR - 注入到 CCR 回调
	 * 将验证通过的装配注入到选中的 CCR 蓝图
	 * @return 返回 FReply::Handled() 表示事件已处理
	 */
	FReply OnInjectToCCR();

	/**
	 * GenerateVariantSetRow - 生成变体集列表行
	 *
	 * 为 TreeView 生成单行控件
	 *
	 * @param InItem 行数据（变体集名称）
	 * @param OwnerTable 所属的表格视图
	 * @return 返回生成的表格行控件
	 */
	TSharedRef<ITableRow> GenerateVariantSetRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * GeneratePartRow - 生成部件列表行
	 *
	 * 为 ListView 生成单行控件
	 *
	 * @param InItem 行数据（部件名称）
	 * @param OwnerTable 所属的表格视图
	 * @return 返回生成的表格行控件
	 */
	TSharedRef<ITableRow> GeneratePartRow(TSharedPtr<FString> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	/**
	 * UpdateStatistics - 更新统计信息
	 *
	 * 刷新界面上的成功/警告/失败计数显示
	 */
	void UpdateStatistics();
};
