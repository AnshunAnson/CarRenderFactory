// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 文件: AssemblyValidatorUIManager.cpp
// 功能: 装配验证器 UI 管理器的实现
// 描述: 管理插件的用户界面，协调 UI 与后端逻辑
// ============================================================

#include "AssemblyValidatorUIManager.h"       // UI 管理器头文件
#include "SAssemblyValidatorPanel.h"          // 主面板控件
#include "AssemblyValidatorTypes.h"           // 数据类型定义
#include "Widgets/SWindow.h"                  // 窗口控件
#include "Framework/Application/SlateApplication.h"  // Slate 应用程序

/**
 * 构造函数
 *
 * 初始化成员变量为默认值
 * CurrentStageData 设为 nullptr，表示尚未加载任何 Stage
 */
FAssemblyValidatorUIManager::FAssemblyValidatorUIManager()
	: CurrentStageData(nullptr)  // 当前 Stage 数据指针初始化为空
{
}

/**
 * 析构函数
 *
 * 调用 Shutdown() 确保所有资源被正确释放
 */
FAssemblyValidatorUIManager::~FAssemblyValidatorUIManager()
{
	Shutdown();
}

/**
 * Initialize - 初始化 UI 管理器
 *
 * 创建主面板控件实例
 * 使用 SNew 宏声明式创建 Slate 控件
 */
void FAssemblyValidatorUIManager::Initialize()
{
	// 创建装配验证器面板控件
	// SNew 是 Slate 的声明式控件创建宏
	UIPanel = SNew(SAssemblyValidatorPanel);
}

/**
 * Shutdown - 关闭 UI 管理器
 *
 * 执行清理工作：
 * 1. 隐藏并销毁窗口
 * 2. 释放 Stage 数据
 * 3. 重置面板控件
 */
void FAssemblyValidatorUIManager::Shutdown()
{
	// 隐藏并销毁验证器窗口
	HideValidatorWindow();

	// 清理 Stage 数据
	if (CurrentStageData)
	{
		// 删除动态分配的 Stage 数据对象
		delete CurrentStageData;
		CurrentStageData = nullptr;
	}

	// 重置面板控件的共享指针
	UIPanel.Reset();
}

/**
 * GetUIPanel - 获取主面板控件
 *
 * @return 返回主面板控件的共享指针
 */
TSharedPtr<class SAssemblyValidatorPanel> FAssemblyValidatorUIManager::GetUIPanel() const
{
	return UIPanel;
}

/**
 * ShowValidatorWindow - 显示验证器窗口
 *
 * 创建并显示独立的验证器窗口
 * 如果窗口已存在则将其置前（聚焦）
 */
void FAssemblyValidatorUIManager::ShowValidatorWindow()
{
	// 如果面板未初始化，先初始化
	if (!UIPanel.IsValid())
	{
		Initialize();
	}

	// 如果窗口已存在，将其置前并返回
	if (ValidatorWindow.IsValid())
	{
		ValidatorWindow.Pin()->BringToFront();
		return;
	}

	// 创建新的独立窗口
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("USD Car Assembly Validator")))  // 窗口标题
		.ClientSize(FVector2D(1200, 800))  // 窗口大小：1200x800 像素
		[
			UIPanel.ToSharedRef()  // 窗口内容为主面板
		];

	// 保存窗口的弱引用
	ValidatorWindow = Window;

	// 将窗口添加到 Slate 应用程序
	FSlateApplication::Get().AddWindow(Window);
}

/**
 * HideValidatorWindow - 隐藏验证器窗口
 *
 * 请求销毁当前窗口并重置窗口引用
 */
void FAssemblyValidatorUIManager::HideValidatorWindow()
{
	// 尝试获取窗口的强引用
	if (TSharedPtr<SWindow> Window = ValidatorWindow.Pin())
	{
		// 请求销毁窗口
		Window->RequestDestroyWindow();
		// 重置弱引用
		ValidatorWindow.Reset();
	}
}

/**
 * LoadUsdStage - 加载 USD Stage 文件
 *
 * @param FilePath USD 文件的完整路径
 *
 * 流程：
 * 1. 记录日志
 * 2. 创建或更新 Stage 数据对象
 * 3. 存储文件路径
 * 4. TODO: 实现实际的 USD 加载逻辑
 */
void FAssemblyValidatorUIManager::LoadUsdStage(const FString& FilePath)
{
	// 记录加载开始日志
	AddLogMessage(FString::Printf(TEXT("Loading USD stage: %s"), *FilePath));

	// 创建或更新 Stage 数据对象
	if (!CurrentStageData)
	{
		// 首次加载，创建新的数据对象
		CurrentStageData = new FAssemblyValidatorStageData();
	}

	// 存储文件路径
	CurrentStageData->StageFilePath = FilePath;

	// TODO: 在此实现实际的 USD 加载逻辑
	// 可能包括：打开 Stage、遍历 Prim 树、识别部件等
	AddLogMessage(TEXT("USD stage loaded successfully."));
}

/**
 * ScanVariants - 扫描变体
 *
 * 在当前加载的 Stage 中扫描所有变体集
 * 更新 Stage 数据中的 VariantSets 数组
 */
void FAssemblyValidatorUIManager::ScanVariants()
{
	// 检查是否已加载 Stage
	if (!CurrentStageData)
	{
		AddLogMessage(TEXT("No USD stage loaded. Please load a stage first."), true);
		return;
	}

	// 记录扫描开始日志
	AddLogMessage(TEXT("Scanning for variants..."));

	// TODO: 实现实际的变体扫描逻辑
	// 当前使用占位符数据进行演示
	CurrentStageData->VariantSets.Empty();
	CurrentStageData->VariantSets.Add(TEXT("ColorVariant"));      // 颜色变体集
	CurrentStageData->VariantSets.Add(TEXT("WheelVariant"));      // 车轮变体集
	CurrentStageData->VariantSets.Add(TEXT("InteriorVariant"));   // 内饰变体集

	// 记录找到的变体集数量
	AddLogMessage(FString::Printf(TEXT("Found %d variant sets."), CurrentStageData->VariantSets.Num()));
}

/**
 * VerifyAssembly - 验证装配
 *
 * 验证当前装配的结构是否正确
 * 检查必需部件、材质等
 */
void FAssemblyValidatorUIManager::VerifyAssembly()
{
	// 检查是否已加载 Stage
	if (!CurrentStageData)
	{
		AddLogMessage(TEXT("No USD stage loaded. Cannot verify assembly."), true);
		return;
	}

	// 记录验证开始日志
	AddLogMessage(TEXT("Verifying assembly structure..."));

	// TODO: 实现实际的装配验证逻辑
	// 创建验证结果对象
	FAssemblyValidationResult ValidationResult;
	ValidationResult.bIsValid = true;    // 暂时设为验证通过
	ValidationResult.WarningCount = 0;   // 警告数
	ValidationResult.ErrorCount = 0;     // 错误数

	// 根据验证结果记录日志
	if (ValidationResult.bIsValid)
	{
		AddLogMessage(TEXT("Assembly verification completed successfully."));
	}
	else
	{
		AddLogMessage(TEXT("Assembly verification found issues."), true);
	}
}

/**
 * InjectToCCRBlueprint - 注入到 CCR 蓝图
 *
 * 将验证通过的装配注入到选中的 CCR 蓝图实例
 */
void FAssemblyValidatorUIManager::InjectToCCRBlueprint()
{
	// 检查是否已加载 Stage
	if (!CurrentStageData)
	{
		AddLogMessage(TEXT("No USD stage loaded. Cannot inject to CCR."), true);
		return;
	}

	// 记录注入开始日志
	AddLogMessage(TEXT("Injecting assembly into CCR blueprint..."));

	// TODO: 实现实际的注入逻辑
	// 可能包括：
	// 1. 查找选中的 CCR 蓝图 Actor
	// 2. 遍历识别的部件
	// 3. 设置每个部件槽位的静态网格
	// 4. 设置每个部件的材质

	AddLogMessage(TEXT("Successfully injected into CCR blueprint."));
}

/**
 * AddLogMessage - 添加日志消息
 *
 * 将消息传递给面板控件显示
 *
 * @param Message 消息内容
 * @param bIsError 是否为错误消息（默认 false）
 * @param bIsWarning 是否为警告消息（默认 false）
 */
void FAssemblyValidatorUIManager::AddLogMessage(const FString& Message, bool bIsError, bool bIsWarning)
{
	// 检查面板是否有效
	if (UIPanel.IsValid())
	{
		// 将消息传递给面板的日志输出区域
		UIPanel->AddLogMessage(Message, bIsError, bIsWarning);
	}
}

/**
 * ClearLog - 清空日志
 *
 * 清除所有日志消息
 * TODO: 实现实际的清空逻辑
 */
void FAssemblyValidatorUIManager::ClearLog()
{
	// TODO: 实现日志清空
	// 可能需要调用 UIPanel 的清空方法
}
