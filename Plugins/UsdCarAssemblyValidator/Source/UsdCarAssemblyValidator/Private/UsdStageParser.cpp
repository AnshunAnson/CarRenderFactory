// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#include "UsdStageParser.h"
#include "Misc/Paths.h"
#include "UnrealUSDWrapper.h"
#include "USDTypesConversion.h"

// USD 包装器头文件
#include "UsdWrappers/UsdStage.h"
#include "UsdWrappers/UsdPrim.h"
#include "UsdWrappers/SdfPath.h"

// 定义日志分类
DEFINE_LOG_CATEGORY(LogUsdStageParser);

FUsdStageParser::FUsdStageParser()
    : StageHandle(nullptr)
    , bIsStageOpen(false)
{
}

FUsdStageParser::~FUsdStageParser()
{
    CloseStage();
}

bool FUsdStageParser::OpenStage(const FString& UsdFilePath)
{
    if (bIsStageOpen)
    {
        CloseStage();
    }

    // 使用 UnrealUSDWrapper 打开 Stage
    UE::FUsdStage Stage = UnrealUSDWrapper::OpenStage(*UsdFilePath, EUsdInitialLoadSet::LoadAll);

    if (!Stage)
    {
        UE_LOG(LogUsdStageParser, Warning, TEXT("Failed to open USD stage: %s"), *UsdFilePath);
        return false;
    }

    // 存储 Stage 句柄 (使用 void* 避免头文件依赖)
    StageHandle = new UE::FUsdStage(MoveTemp(Stage));
    bIsStageOpen = true;

    // 缓存 Stage 数据
    CurrentStageData.StagePath = UsdFilePath;

    UE_LOG(LogUsdStageParser, Log, TEXT("Successfully opened USD stage: %s"), *UsdFilePath);
    return true;
}

void FUsdStageParser::CloseStage()
{
    if (StageHandle)
    {
        UE::FUsdStage* StagePtr = static_cast<UE::FUsdStage*>(StageHandle);
        delete StagePtr;
        StageHandle = nullptr;
    }

    bIsStageOpen = false;
    CurrentStageData = FUsdStageData();
}

TArray<FUsdVariantSetInfo> FUsdStageParser::GetVariantSets() const
{
    TArray<FUsdVariantSetInfo> Result;

    if (!StageHandle)
    {
        return Result;
    }

    // TODO: 实现 VariantSets 提取
    // 需要使用 UE::FUsdStage 的 VariantSets API

    return Result;
}

TArray<FUsdPrimInfo> FUsdStageParser::ScanCarAssemblyParts()
{
    TArray<FUsdPrimInfo> Parts;

    if (!StageHandle)
    {
        return Parts;
    }

    UE::FUsdStage* StagePtr = static_cast<UE::FUsdStage*>(StageHandle);
    UE::FUsdStage Stage = *StagePtr;

    // 获取伪根节点并遍历
    UE::FUsdPrim RootPrim = Stage.GetPseudoRoot();
    if (!RootPrim)
    {
        return Parts;
    }

    // 遍历所有子 Prim
    TArray<UE::FUsdPrim> PrimsToProcess;
    PrimsToProcess.Add(RootPrim);

    while (PrimsToProcess.Num() > 0)
    {
        UE::FUsdPrim CurrentPrim = PrimsToProcess.Pop();

        // 获取子 Prim
        TArray<UE::FUsdPrim> Children = CurrentPrim.GetFilteredChildren(false);

        for (const UE::FUsdPrim& Child : Children)
        {
            FString PrimType = Child.GetTypeName().ToString();
            FString PrimPath = Child.GetPrimPath().GetString();
            FString PrimName = Child.GetName().ToString();

            // 只处理 Mesh 类型的 Prim
            if (PrimType == TEXT("Mesh") || PrimType == TEXT("Xform") || PrimType == TEXT("Scope"))
            {
                FUsdPrimInfo Info;
                Info.PrimPath = PrimPath;
                Info.PrimName = PrimName;
                Info.PrimType = PrimType;
                Info.PartType = IdentifyPartType(PrimPath, PrimName);

                // 只将 Mesh 类型添加到部件列表
                if (PrimType == TEXT("Mesh"))
                {
                    Parts.Add(Info);
                }
            }

            // 继续遍历子节点
            PrimsToProcess.Add(Child);
        }
    }

    CurrentStageData.IdentifiedParts = Parts;
    UE_LOG(LogUsdStageParser, Log, TEXT("Scanned %d car assembly parts"), Parts.Num());
    return Parts;
}

bool FUsdStageParser::SelectVariant(const FString& VariantSetName, const FString& VariantName)
{
    if (!StageHandle)
    {
        return false;
    }

    // TODO: 实现变体选择
    // 需要使用 UE::FUsdStage 的 VariantSets API

    UE_LOG(LogUsdStageParser, Log, TEXT("SelectVariant: %s/%s"), *VariantSetName, *VariantName);
    return true;
}

FUsdStageData FUsdStageParser::GetStageData() const
{
    FUsdStageData Data = CurrentStageData;
    Data.VariantSets = GetVariantSets();
    return Data;
}

bool FUsdStageParser::IsStageOpen() const
{
    return bIsStageOpen;
}

FString FUsdStageParser::IdentifyPartType(const FString& PrimPath, const FString& PrimName)
{
    // 基于命名规则识别部件类型
    FString LowerPath = PrimPath.ToLower();
    FString LowerName = PrimName.ToLower();

    // 车身
    if (LowerPath.Contains(TEXT("body")) || LowerName.Contains(TEXT("body")))
    {
        return TEXT("Body");
    }

    // 车轮
    if (LowerPath.Contains(TEXT("wheel")) || LowerName.Contains(TEXT("wheel")))
    {
        if (LowerPath.Contains(TEXT("_fl")) || LowerPath.Contains(TEXT("_lf")))
        {
            return TEXT("Wheel_FL");
        }
        if (LowerPath.Contains(TEXT("_fr")) || LowerPath.Contains(TEXT("_rf")))
        {
            return TEXT("Wheel_FR");
        }
        if (LowerPath.Contains(TEXT("_rl")) || LowerPath.Contains(TEXT("_lr")))
        {
            return TEXT("Wheel_RL");
        }
        if (LowerPath.Contains(TEXT("_rr")) || LowerPath.Contains(TEXT("_rb")))
        {
            return TEXT("Wheel_RR");
        }
        return TEXT("Wheel");
    }

    // 卡钳
    if (LowerPath.Contains(TEXT("caliper")) || LowerName.Contains(TEXT("caliper")))
    {
        if (LowerPath.Contains(TEXT("_fl")) || LowerPath.Contains(TEXT("_lf")))
        {
            return TEXT("Caliper_FL");
        }
        if (LowerPath.Contains(TEXT("_fr")) || LowerPath.Contains(TEXT("_rf")))
        {
            return TEXT("Caliper_FR");
        }
        return TEXT("Caliper");
    }

    // 方向盘
    if (LowerPath.Contains(TEXT("steering")) || LowerName.Contains(TEXT("steering")))
    {
        return TEXT("SteeringWheel");
    }

    // 门
    if (LowerPath.Contains(TEXT("door")) || LowerName.Contains(TEXT("door")))
    {
        return TEXT("Door");
    }

    // 引擎盖
    if (LowerPath.Contains(TEXT("hood")) || LowerName.Contains(TEXT("hood")))
    {
        return TEXT("Hood");
    }

    // 后备箱
    if (LowerPath.Contains(TEXT("trunk")) || LowerName.Contains(TEXT("trunk")))
    {
        return TEXT("Trunk");
    }

    // 车窗
    if (LowerPath.Contains(TEXT("window")) || LowerName.Contains(TEXT("window")))
    {
        return TEXT("Window");
    }

    // 车灯
    if (LowerPath.Contains(TEXT("light")) || LowerName.Contains(TEXT("light")))
    {
        return TEXT("Light");
    }

    // 默认使用 Prim 名称
    return PrimName;
}

void FUsdStageParser::RecursiveScanPrims(const FString& PrimPath, TArray<FUsdPrimInfo>& OutParts)
{
    // 此方法保留用于将来扩展
}
