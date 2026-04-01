// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#include "OpenUSDActor.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"
#include "UsdCarDataAsset.h"

#if WITH_EDITOR
#include "Editor.h"
#endif

// 定义日志分类
DEFINE_LOG_CATEGORY_STATIC(LogOpenUSDActor, Log, All);

// ============================================================
// 构造函数 / 析构函数
// ============================================================

AOpenUSDActor::AOpenUSDActor()
{
    // 设置为可生成在关卡中
    PrimaryActorTick.bCanEverTick = false;

    // 默认不自动加载
    bAutoLoadOnBeginPlay = false;
}

AOpenUSDActor::~AOpenUSDActor()
{
    // 确保清理资源
    CloseUsdStage();
}

// ============================================================
// Actor 生命周期
// ============================================================

void AOpenUSDActor::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoLoadOnBeginPlay && UsdDataAsset)
    {
        Load();
    }
}

void AOpenUSDActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 在 PIE 结束时自动卸载（如果配置了）
    if (EndPlayReason == EEndPlayReason::EndPlayInEditor)
    {
        if (UsdDataAsset && UsdDataAsset->LoadSettings.bUnloadOnPIEEnd)
        {
            Unload();
        }
    }

    Super::EndPlay(EndPlayReason);
}

// ============================================================
// 公共方法
// ============================================================

FUsdLoadResult AOpenUSDActor::Load()
{
    // 重置结果
    LastLoadResult = FUsdLoadResult();

    // 验证配置
    if (!UsdDataAsset)
    {
        LastLoadResult.bSuccess = false;
        LastLoadResult.LoadState = EUsdLoadState::Failed;
        LastLoadResult.ErrorMessages.Add(TEXT("UsdDataAsset is not set"));
        OnLoadFailed.Broadcast(TEXT("UsdDataAsset is not set"));
        UE_LOG(LogOpenUSDActor, Error, TEXT("Load failed: UsdDataAsset is not set"));
        return LastLoadResult;
    }

    // 验证数据资产配置
    TArray<FString> ValidationErrors;
    if (!UsdDataAsset->ValidateConfiguration(ValidationErrors))
    {
        LastLoadResult.bSuccess = false;
        LastLoadResult.LoadState = EUsdLoadState::Failed;
        LastLoadResult.ErrorMessages = ValidationErrors;
        OnLoadFailed.Broadcast(FString::Join(ValidationErrors, TEXT("\n")));
        UE_LOG(LogOpenUSDActor, Error, TEXT("Load failed: Configuration validation failed"));
        return LastLoadResult;
    }

    // 如果已加载，先卸载
    if (IsLoaded())
    {
        Unload();
    }

    LoadState = EUsdLoadState::Loading;

    // 1. 打开 USD Stage
    if (!OpenUsdStage())
    {
        LastLoadResult.bSuccess = false;
        LastLoadResult.LoadState = EUsdLoadState::Failed;
        LastLoadResult.ErrorMessages.Add(FString::Printf(TEXT("Failed to open USD stage: %s"), *UsdDataAsset->GetResolvedUsdPath()));
        LoadState = EUsdLoadState::Failed;
        OnLoadFailed.Broadcast(LastLoadResult.ErrorMessages.Last());
        UE_LOG(LogOpenUSDActor, Error, TEXT("Failed to open USD stage: %s"), *UsdDataAsset->GetResolvedUsdPath());
        return LastLoadResult;
    }

    // 2. 应用初始变体选择
    if (UsdDataAsset->LoadSettings.bAutoApplyVariants)
    {
        for (const TPair<FString, FString>& Pair : UsdDataAsset->LoadSettings.DefaultVariantSelection)
        {
            SelectVariant(Pair.Key, Pair.Value);
        }
    }

    // 3. 解析 Stage 并生成 Actor
    if (!ParseAndSpawnActors())
    {
        CloseUsdStage();
        LastLoadResult.bSuccess = false;
        LastLoadResult.LoadState = EUsdLoadState::Failed;
        LastLoadResult.ErrorMessages.Add(TEXT("Failed to parse and spawn actors"));
        LoadState = EUsdLoadState::Failed;
        OnLoadFailed.Broadcast(TEXT("Failed to parse and spawn actors"));
        UE_LOG(LogOpenUSDActor, Error, TEXT("Failed to parse and spawn actors"));
        return LastLoadResult;
    }

    // 成功
    LoadState = EUsdLoadState::Loaded;
    LastLoadResult.bSuccess = true;
    LastLoadResult.LoadState = EUsdLoadState::Loaded;
    LastLoadResult.SpawnedActorCount = SpawnedMeshActors.Num();

    LogLoadResult();
    OnLoadComplete.Broadcast(LastLoadResult);

    UE_LOG(LogOpenUSDActor, Log, TEXT("Successfully loaded USD: %s with %d actors"),
           *UsdDataAsset->GetResolvedUsdPath(), SpawnedMeshActors.Num());

    return LastLoadResult;
}

void AOpenUSDActor::Unload()
{
    // 清理生成的 Actor
    CleanupSpawnedActors();

    // 关闭 Stage
    CloseUsdStage();

    // 重置状态
    LoadState = EUsdLoadState::NotLoaded;
    LastLoadResult = FUsdLoadResult();

    OnUnloaded.Broadcast();
    UE_LOG(LogOpenUSDActor, Log, TEXT("Unloaded USD"));
}

FUsdLoadResult AOpenUSDActor::Reload()
{
    Unload();
    return Load();
}

TArray<AStaticMeshActor*> AOpenUSDActor::GetSpawnedActors() const
{
    TArray<AStaticMeshActor*> Result;
    Result.Reserve(SpawnedMeshActors.Num());

    for (const FSpawnedMeshActorInfo& Info : SpawnedMeshActors)
    {
        if (Info.Actor)
        {
            Result.Add(Info.Actor);
        }
    }

    return Result;
}

bool AOpenUSDActor::SelectVariant(const FString& VariantSetName, const FString& VariantName)
{
    if (!StageParser.IsValid())
    {
        UE_LOG(LogOpenUSDActor, Warning, TEXT("SelectVariant failed: StageParser is not valid"));
        return false;
    }

    bool bSuccess = StageParser->SelectVariant(VariantSetName, VariantName);

    if (bSuccess)
    {
        UE_LOG(LogOpenUSDActor, Log, TEXT("Selected variant: %s/%s"), *VariantSetName, *VariantName);

        // 更新材质
        for (const FSpawnedMeshActorInfo& Info : SpawnedMeshActors)
        {
            if (Info.Actor)
            {
                ApplyMaterialsToActor(Info.Actor, Info.PartType);
            }
        }
    }
    else
    {
        UE_LOG(LogOpenUSDActor, Warning, TEXT("Failed to select variant: %s/%s"), *VariantSetName, *VariantName);
    }

    return bSuccess;
}

TArray<FString> AOpenUSDActor::GetAvailableVariantSets() const
{
    TArray<FString> Result;

    if (StageParser.IsValid())
    {
        TArray<FUsdVariantSetInfo> VariantSets = StageParser->GetVariantSets();
        Result.Reserve(VariantSets.Num());

        for (const FUsdVariantSetInfo& Info : VariantSets)
        {
            Result.Add(Info.VariantSetName);
        }
    }

    return Result;
}

TArray<FString> AOpenUSDActor::GetVariantsInSet(const FString& VariantSetName) const
{
    TArray<FString> Result;

    if (StageParser.IsValid())
    {
        TArray<FUsdVariantSetInfo> VariantSets = StageParser->GetVariantSets();

        for (const FUsdVariantSetInfo& SetInfo : VariantSets)
        {
            if (SetInfo.VariantSetName == VariantSetName)
            {
                Result.Reserve(SetInfo.Variants.Num());

                for (const FUsdVariantInfo& VariantInfo : SetInfo.Variants)
                {
                    Result.Add(VariantInfo.VariantName);
                }

                break;
            }
        }
    }

    return Result;
}

// ============================================================
// 内部方法
// ============================================================

bool AOpenUSDActor::OpenUsdStage()
{
    // 创建解析器实例
    if (!StageParser.IsValid())
    {
        StageParser = MakeShareable(new FUsdStageParser());
    }

    // 获取解析后的 USD 路径
    FString ResolvedPath = UsdDataAsset->GetResolvedUsdPath();

    // 使用 StageParser 打开 Stage
    if (!StageParser->OpenStage(ResolvedPath))
    {
        UE_LOG(LogOpenUSDActor, Error, TEXT("StageParser failed to open: %s"), *ResolvedPath);
        return false;
    }

    // 缓存 Stage 数据
    CachedStageData = StageParser->GetStageData();

    UE_LOG(LogOpenUSDActor, Log, TEXT("Successfully opened USD stage: %s"), *ResolvedPath);
    return true;
}

void AOpenUSDActor::CloseUsdStage()
{
    if (StageParser.IsValid())
    {
        StageParser->CloseStage();
    }

    CachedStageData = FUsdStageData();
}

bool AOpenUSDActor::ParseAndSpawnActors()
{
    if (!StageParser.IsValid() || !UsdDataAsset)
    {
        return false;
    }

    // 获取识别的部件
    TArray<FUsdPrimInfo> IdentifiedParts;

    if (UsdDataAsset->bAutoIdentifyParts)
    {
        // 使用 StageParser 自动识别
        IdentifiedParts = StageParser->ScanCarAssemblyParts();
    }
    else
    {
        // 仅使用 DataAsset 中配置的映射
        for (const FUsdPartMappingEntry& Entry : UsdDataAsset->PartMappings)
        {
            FUsdPrimInfo PrimInfo;
            PrimInfo.PrimPath = Entry.PrimPath;
            PrimInfo.PrimName = FPaths::GetCleanFilename(Entry.PrimPath);
            PrimInfo.PartType = Entry.PartType;
            IdentifiedParts.Add(PrimInfo);
        }
    }

    UE_LOG(LogOpenUSDActor, Log, TEXT("Identified %d parts to spawn"), IdentifiedParts.Num());

    // 为每个部件生成 Actor
    for (const FUsdPrimInfo& Part : IdentifiedParts)
    {
        FUsdPartMappingEntry MappingEntry;
        bool bHasMapping = UsdDataAsset->FindPartMappingByPrimPath(Part.PrimPath, MappingEntry);
        if (!bHasMapping)
        {
            bHasMapping = UsdDataAsset->FindPartMappingByPartType(Part.PartType, MappingEntry);
        }

        const FUsdPartMappingEntry* Mapping = bHasMapping ? &MappingEntry : nullptr;

        AStaticMeshActor* SpawnedActor = SpawnMeshActorForPrim(Part.PrimPath, Part.PartType, Mapping);

        if (SpawnedActor)
        {
            FSpawnedMeshActorInfo Info;
            Info.Actor = SpawnedActor;
            Info.PrimPath = Part.PrimPath;
            Info.PartType = Part.PartType;
            Info.bIsTransient = UsdDataAsset->LoadSettings.bTransientLoad;
            SpawnedMeshActors.Add(Info);
        }
    }

    return SpawnedMeshActors.Num() > 0;
}

AStaticMeshActor* AOpenUSDActor::SpawnMeshActorForPrim(const FString& PrimPath, const FString& PartType, const FUsdPartMappingEntry* Mapping)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogOpenUSDActor, Warning, TEXT("SpawnMeshActorForPrim: World is null"));
        return nullptr;
    }

    // 生成参数
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // 设置为 Transient（不保存）
    if (UsdDataAsset && UsdDataAsset->LoadSettings.bTransientLoad)
    {
        SpawnParams.ObjectFlags |= RF_Transient;
    }

    // 生成名称
    FString ActorName = FString::Printf(TEXT("USD_%s_%s"), *PartType, *FGuid::NewGuid().ToString().Left(8));

    // 计算 Transform
    FTransform SpawnTransform = FTransform::Identity;
    if (Mapping)
    {
        SpawnTransform = Mapping->RelativeTransform;
    }

    // 生成 Actor
    AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnTransform, SpawnParams);

    if (!SpawnedActor)
    {
        UE_LOG(LogOpenUSDActor, Warning, TEXT("Failed to spawn actor for Prim: %s"), *PrimPath);
        return nullptr;
    }

    // 设置标签
    SpawnedActor->SetActorLabel(ActorName);
    SpawnedActor->SetActorHiddenInGame(false);

    // 设置网格
    UStaticMeshComponent* MeshComp = SpawnedActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        // 使用映射中的网格（如果有）
        if (Mapping && Mapping->OverrideMesh)
        {
            MeshComp->SetStaticMesh(Mapping->OverrideMesh);
            UE_LOG(LogOpenUSDActor, Log, TEXT("Set override mesh for %s"), *PartType);
        }
        else
        {
            // TODO: 从 USD Prim 转换网格
            // 这需要使用 UsdToMesh 或 UsdMeshConversion API
            UE_LOG(LogOpenUSDActor, Warning, TEXT("No override mesh for Prim: %s, USD-to-Mesh conversion not implemented"), *PrimPath);
        }

        // 设置碰撞
        if (Mapping)
        {
            MeshComp->SetCollisionEnabled(Mapping->bGenerateCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
        }

        // 应用材质
        ApplyMaterialsToActor(SpawnedActor, PartType);
    }

    return SpawnedActor;
}

void AOpenUSDActor::ApplyMaterialsToActor(AStaticMeshActor* Actor, const FString& PartType)
{
    if (!Actor || !UsdDataAsset)
    {
        return;
    }

    UStaticMeshComponent* MeshComp = Actor->GetStaticMeshComponent();
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
        return;
    }

    // 查找部件映射获取默认材质
    FUsdPartMappingEntry MappingEntry;
    bool bHasMapping = UsdDataAsset->FindPartMappingByPartType(PartType, MappingEntry);

    UMaterialInterface* MaterialToApply = nullptr;

    // 优先使用映射中的默认材质
    if (bHasMapping && MappingEntry.DefaultMaterial)
    {
        MaterialToApply = MappingEntry.DefaultMaterial;
    }

    // 应用当前变体的材质
    for (const FUsdVariantSetInfo& SetInfo : CachedStageData.VariantSets)
    {
        if (SetInfo.CurrentVariantIndex >= 0 && SetInfo.CurrentVariantIndex < SetInfo.Variants.Num())
        {
            FString CurrentVariant = SetInfo.Variants[SetInfo.CurrentVariantIndex].VariantName;
            UMaterialInterface* VariantMaterial = UsdDataAsset->GetMaterialForVariant(SetInfo.VariantSetName, CurrentVariant, PartType);

            if (VariantMaterial)
            {
                MaterialToApply = VariantMaterial;
                break;
            }
        }
    }

    // 应用材质
    if (MaterialToApply)
    {
        MeshComp->SetMaterial(0, MaterialToApply);
        UE_LOG(LogOpenUSDActor, Verbose, TEXT("Applied material to %s"), *PartType);
    }
}

void AOpenUSDActor::CleanupSpawnedActors()
{
    UWorld* World = GetWorld();

    for (const FSpawnedMeshActorInfo& Info : SpawnedMeshActors)
    {
        if (Info.Actor && World)
        {
            // 销毁瞬态 Actor
            if (Info.bIsTransient)
            {
                World->DestroyActor(Info.Actor);
            }
        }
    }

    SpawnedMeshActors.Empty();
}

void AOpenUSDActor::LogLoadResult()
{
    if (bShowDebugInfo)
    {
        UE_LOG(LogOpenUSDActor, Log, TEXT("=== USD Load Result ==="));
        UE_LOG(LogOpenUSDActor, Log, TEXT("Success: %s"), LastLoadResult.bSuccess ? TEXT("true") : TEXT("false"));
        UE_LOG(LogOpenUSDActor, Log, TEXT("State: %d"), (int32)LastLoadResult.LoadState);
        UE_LOG(LogOpenUSDActor, Log, TEXT("Spawned Actors: %d"), LastLoadResult.SpawnedActorCount);

        for (const FString& Error : LastLoadResult.ErrorMessages)
        {
            UE_LOG(LogOpenUSDActor, Error, TEXT("Error: %s"), *Error);
        }

        for (const FString& Warning : LastLoadResult.WarningMessages)
        {
            UE_LOG(LogOpenUSDActor, Warning, TEXT("Warning: %s"), *Warning);
        }
    }
}

#if WITH_EDITOR
void AOpenUSDActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    // 当 UsdDataAsset 变化时，可以选择自动重新加载
    if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AOpenUSDActor, UsdDataAsset))
    {
        if (IsLoaded())
        {
            // 可以选择自动重新加载或提示用户
            UE_LOG(LogOpenUSDActor, Log, TEXT("UsdDataAsset changed, consider reloading"));
        }
    }
}
#endif
