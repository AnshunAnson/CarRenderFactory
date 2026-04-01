// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once
// 防止头文件重复包含

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * FAssemblyValidatorUIManager - 装配验证器 UI 管理器
 *
 * 负责管理插件的用户界面，包括：
 * - 创建和管理主面板控件
 * - 显示/隐藏验证器窗口
 * - 协调 UI 与后端逻辑的交互
 *
 * 继承自 TSharedFromThis，允许在类内部安全地获取 this 的共享指针
 */
class FAssemblyValidatorUIManager : public TSharedFromThis<FAssemblyValidatorUIManager>
{
public:
	/**
	 * 构造函数
	 * 初始化成员变量
	 */
	FAssemblyValidatorUIManager();

	/**
	 * 析构函数
	 * 调用 Shutdown() 清理资源
	 */
	virtual ~FAssemblyValidatorUIManager();

	/**
	 * Initialize - 初始化 UI 管理器
	 *
	 * 创建主面板控件实例
	 * 在使用管理器前必须调用
	 */
	void Initialize();

	/**
	 * Shutdown - 关闭 UI 管理器
	 *
	 * 隐藏窗口并清理所有资源
	 * 在销毁管理器前调用
	 */
	void Shutdown();

	/**
	 * GetUIPanel - 获取主面板控件
	 *
	 * @return 返回主面板控件的共享指针
	 */
	TSharedPtr<class SAssemblyValidatorPanel> GetUIPanel() const;

	/**
	 * ShowValidatorWindow - 显示验证器窗口
	 *
	 * 创建并显示独立的验证器窗口
	 * 如果窗口已存在则将其置前
	 */
	void ShowValidatorWindow();

	/**
	 * HideValidatorWindow - 隐藏验证器窗口
	 *
	 * 隐藏并销毁当前的验证器窗口
	 */
	void HideValidatorWindow();

	/**
	 * LoadUsdStage - 加载 USD Stage 文件
	 *
	 * 打开并加载指定的 USD 文件
	 *
	 * @param FilePath USD 文件的完整路径
	 */
	void LoadUsdStage(const FString& FilePath);

	/**
	 * ScanVariants - 扫描变体
	 *
	 * 在当前加载的 Stage 中扫描所有变体集
	 * 更新 UI 显示变体列表
	 */
	void ScanVariants();

	/**
	 * VerifyAssembly - 验证装配
	 *
	 * 验证当前装配的结构是否正确
	 * 检查必需部件是否存在、材质是否有效等
	 */
	void VerifyAssembly();

	/**
	 * InjectToCCRBlueprint - 注入到 CCR 蓝图
	 *
	 * 将验证通过的装配注入到选中的 CCR 蓝图实例
	 * 设置各部件的静态网格和材质
	 */
	void InjectToCCRBlueprint();

	/**
	 * AddLogMessage - 添加日志消息
	 *
	 * 在 UI 的日志输出区域添加一条消息
	 *
	 * @param Message 消息内容
	 * @param bIsError 是否为错误消息
	 * @param bIsWarning 是否为警告消息
	 */
	void AddLogMessage(const FString& Message, bool bIsError = false, bool bIsWarning = false);

	/**
	 * ClearLog - 清空日志
	 *
	 * 清除日志输出区域的所有消息
	 */
	void ClearLog();

private:
	/**
	 * UIPanel - 主 UI 面板
	 *
	 * 存储装配验证器面板控件的共享指针
	 * 面板包含所有的 UI 元素
	 */
	TSharedPtr<class SAssemblyValidatorPanel> UIPanel;

	/**
	 * ValidatorWindow - 验证器窗口
	 *
	 * 弱指针指向独立的验证器窗口
	 * 使用弱指针避免循环引用
	 */
	TWeakPtr<class SWindow> ValidatorWindow;

	/**
	 * CurrentStageData - 当前 USD Stage 数据
	 *
	 * 指向当前加载的 Stage 数据结构的原始指针
	 * 需要手动管理内存
	 */
	struct FAssemblyValidatorStageData* CurrentStageData;
};
