// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

// ============================================================
// 模块头文件包含
// ============================================================
#include "CarRenderFactory.h"

// ============================================================
// 模块管理器头文件包含
// ============================================================
#include "Modules/ModuleManager.h"

// ============================================================
// 编辑器模块头文件（仅编辑器构建）
// ============================================================
#if WITH_EDITOR
#include "PropertyEditorModule.h"
#include "OpenUSDActorDetails.h"
#include "OpenUSDActor.h"
#endif

// ============================================================
// 自定义模块类
// ============================================================
// 实现自定义模块行为，包括注册 Details 面板自定义
class FCarRenderFactoryModule : public FDefaultGameModuleImpl
{
public:
    // 模块加载时调用
    virtual void StartupModule() override
    {
#if WITH_EDITOR
        // 注册 OpenUSD Actor 的 Details 面板自定义
        if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
        {
            FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
            PropertyModule.RegisterCustomClassLayout(
                AOpenUSDActor::StaticClass()->GetFName(),
                FOnGetDetailCustomizationInstance::CreateStatic(&FOpenUSDActorDetails::MakeInstance)
            );
        }
#endif
        UE_LOG(LogTemp, Log, TEXT("CarRenderFactory module loaded"));
    }

    // 模块卸载时调用
    virtual void ShutdownModule() override
    {
#if WITH_EDITOR
        // 取消注册 Details 面板自定义
        if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
        {
            FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
            PropertyModule.UnregisterCustomClassLayout(AOpenUSDActor::StaticClass()->GetFName());
        }
#endif
        UE_LOG(LogTemp, Log, TEXT("CarRenderFactory module unloaded"));
    }

    // 是否支持游戏线程刷新
    virtual bool IsGameModule() const override
    {
        return true;  // 主游戏模块返回 true
    }
};

// ============================================================
// 主游戏模块注册宏
// ============================================================
IMPLEMENT_PRIMARY_GAME_MODULE(FCarRenderFactoryModule, CarRenderFactory, "CarRenderFactory");
