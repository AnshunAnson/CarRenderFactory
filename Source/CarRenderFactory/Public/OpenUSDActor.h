// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UsdCarDataAsset.h"
#include "UsdStageParser.h"  // 引用 UsdCarAssemblyValidator 插件中的 USD 解析器
#include "OpenUSDActor.generated.h"

// 前向声明
class FUsdStageParser;
class UStaticMeshComponent;
class AStaticMeshActor;

/**
 * EUsdLoadState - USD 加载状态
 */
UENUM(BlueprintType)
enum class EUsdLoadState : uint8
{
    NotLoaded,      // 未加载
    Loading,        // 正在加载
    Loaded,         // 已加载
    Failed          // 加载失败
};

/**
 * FSpawnedMeshActorInfo - 生成的网格 Actor 信息
 */
USTRUCT(BlueprintType)
struct CARRENDERFACTORY_API FSpawnedMeshActorInfo
{
    GENERATED_BODY()

    /** 生成的 Actor 实例 */
    UPROPERTY(BlueprintReadOnly, Category = "Spawned Actor")
    TObjectPtr<AStaticMeshActor> Actor = nullptr;

    /** 对应的 USD Prim 路径 */
    UPROPERTY(BlueprintReadOnly, Category = "Spawned Actor")
    FString PrimPath;

    /** 部件类型 */
    UPROPERTY(BlueprintReadOnly, Category = "Spawned Actor")
    FString PartType;

    /** 是否是瞬态生成的 */
    UPROPERTY(BlueprintReadOnly, Category = "Spawned Actor")
    bool bIsTransient = true;
};

/**
 * FUsdLoadResult - USD 加载结果
 */
USTRUCT(BlueprintType)
struct CARRENDERFACTORY_API FUsdLoadResult
{
    GENERATED_BODY()

    /** 加载是否成功 */
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    bool bSuccess = false;

    /** 当前加载状态 */
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    EUsdLoadState LoadState = EUsdLoadState::NotLoaded;

    /** 生成的 Actor 数量 */
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    int32 SpawnedActorCount = 0;

    /** 错误消息列表 */
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    TArray<FString> ErrorMessages;

    /** 警告消息列表 */
    UPROPERTY(BlueprintReadOnly, Category = "Result")
    TArray<FString> WarningMessages;
};

// 声明委托类型
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUsdLoadComplete, const FUsdLoadResult&, Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUsdLoadFailed, const FString&, ErrorMessage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUsdUnloaded);

/**
 * AOpenUSDActor - OpenUSD 加载 Actor
 *
 * 负责加载 USD 文件并根据 Prim 结构生成 StaticMeshActor 实例
 * 支持瞬态加载模式，生成的 Actor 不会保存到关卡中
 *
 * 使用方式:
 * 1. 在编辑器中设置 UsdDataAsset 属性
 * 2. 调用 Load() 或在编辑器中点击 "Load USD" 按钮加载
 * 3. 使用 Unload() 卸载已加载的内容
 */
UCLASS(BlueprintType, meta = (DisplayName = "OpenUSD Actor"))
class CARRENDERFACTORY_API AOpenUSDActor : public AActor
{
    GENERATED_BODY()

public:
    AOpenUSDActor();
    virtual ~AOpenUSDActor();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    // ===== 配置属性 =====

    /** USD 配置数据资产 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Configuration", meta = (DisplayName = "USD Data Asset"))
    TObjectPtr<UUsdCarDataAsset> UsdDataAsset;

    /** 是否在 BeginPlay 时自动加载 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Configuration")
    bool bAutoLoadOnBeginPlay = false;

    /** 是否在编辑器中显示调试信息 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Configuration")
    bool bShowDebugInfo = false;

    // ===== 运行时状态（Transient） =====

    /** 当前加载状态 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "USD State")
    EUsdLoadState LoadState = EUsdLoadState::NotLoaded;

    /** 最后一次加载的结果 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "USD State")
    FUsdLoadResult LastLoadResult;

    /** 生成的网格 Actor 列表 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category = "USD State")
    TArray<FSpawnedMeshActorInfo> SpawnedMeshActors;

    // ===== 事件委托 =====

    /** USD 加载完成事件 */
    UPROPERTY(BlueprintAssignable, Category = "USD Events")
    FOnUsdLoadComplete OnLoadComplete;

    /** USD 加载失败事件 */
    UPROPERTY(BlueprintAssignable, Category = "USD Events")
    FOnUsdLoadFailed OnLoadFailed;

    /** USD 卸载完成事件 */
    UPROPERTY(BlueprintAssignable, Category = "USD Events")
    FOnUsdUnloaded OnUnloaded;

    // ===== 公共方法 =====

    /**
     * Load - 加载 USD 文件并生成 Actor
     *
     * 这是主要的加载入口点，公开到编辑器和蓝图
     *
     * @return 加载结果
     */
    UFUNCTION(BlueprintCallable, Category = "USD", meta = (CallInEditor = "true"))
    FUsdLoadResult Load();

    /**
     * Unload - 卸载已加载的 USD 内容
     *
     * 销毁所有生成的瞬态 Actor 并关闭 Stage
     */
    UFUNCTION(BlueprintCallable, Category = "USD", meta = (CallInEditor = "true"))
    void Unload();

    /**
     * Reload - 重新加载 USD 文件
     *
     * 先卸载再重新加载
     *
     * @return 加载结果
     */
    UFUNCTION(BlueprintCallable, Category = "USD", meta = (CallInEditor = "true"))
    FUsdLoadResult Reload();

    /**
     * IsLoaded - 检查是否已加载
     *
     * @return 是否已成功加载
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD")
    bool IsLoaded() const { return LoadState == EUsdLoadState::Loaded; }

    /**
     * GetSpawnedActors - 获取所有生成的 Actor
     *
     * @return 生成的 Actor 数组
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD")
    TArray<AStaticMeshActor*> GetSpawnedActors() const;

    /**
     * SelectVariant - 选择变体
     *
     * 在指定变体集中切换变体
     *
     * @param VariantSetName 变体集名称
     * @param VariantName 变体名称
     * @return 是否成功
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    bool SelectVariant(const FString& VariantSetName, const FString& VariantName);

    /**
     * GetAvailableVariantSets - 获取可用的变体集
     *
     * @return 变体集名称数组
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "USD")
    TArray<FString> GetAvailableVariantSets() const;

    /**
     * GetVariantsInSet - 获取变体集中的所有变体
     *
     * @param VariantSetName 变体集名称
     * @return 变体名称数组
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    TArray<FString> GetVariantsInSet(const FString& VariantSetName) const;

protected:
    // ===== 内部方法 =====

    /**
     * OpenUsdStage - 打开 USD Stage
     * 使用 UnrealUSDWrapper API
     */
    bool OpenUsdStage();

    /**
     * CloseUsdStage - 关闭 USD Stage
     */
    void CloseUsdStage();

    /**
     * ParseAndSpawnActors - 解析 Stage 并生成 Actor
     */
    bool ParseAndSpawnActors();

    /**
     * SpawnMeshActorForPrim - 为指定 Prim 生成 StaticMeshActor
     *
     * @param PrimPath Prim 路径
     * @param PartType 部件类型
     * @param Mapping 部件映射配置
     * @return 生成的 Actor
     */
    AStaticMeshActor* SpawnMeshActorForPrim(const FString& PrimPath, const FString& PartType, const FUsdPartMappingEntry* Mapping);

    /**
     * ApplyMaterialsToActor - 为 Actor 应用材质
     *
     * @param Actor 目标 Actor
     * @param PartType 部件类型
     */
    void ApplyMaterialsToActor(AStaticMeshActor* Actor, const FString& PartType);

    /**
     * CleanupSpawnedActors - 清理所有生成的 Actor
     */
    void CleanupSpawnedActors();

    /**
     * LogLoadResult - 记录加载结果
     */
    void LogLoadResult();

#if WITH_EDITOR
    /** 编辑器中显示调试信息 */
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
    /** USD Stage 解析器实例 */
    TSharedPtr<FUsdStageParser> StageParser;

    /** USD Stage 句柄（使用 void* 避免头文件依赖 USD 库） */
    void* StageHandle = nullptr;

    /** 缓存的 Stage 数据 */
    FUsdStageData CachedStageData;
};
