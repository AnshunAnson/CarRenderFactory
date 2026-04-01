// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UsdCarDataAsset.generated.h"

class UStaticMesh;
class UMaterialInterface;

/**
 * FUsdPartMappingEntry - 单个部件映射条目
 * 定义 USD Prim 到 UE 静态网格的映射关系
 */
USTRUCT(BlueprintType)
struct CARRENDERFACTORY_API FUsdPartMappingEntry
{
    GENERATED_BODY()

    /** USD Prim 路径，例如: "/Car/Body", "/Car/Wheel_FL" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    FString PrimPath;

    /** 部件类型标识，例如: "Body", "Wheel_FL", "Wheel_FR" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    FString PartType;

    /** 目标静态网格资产（可选，若为空则从 USD 转换） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    TObjectPtr<UStaticMesh> OverrideMesh;

    /** 默认材质（可选） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    TObjectPtr<UMaterialInterface> DefaultMaterial;

    /** 是否在加载时自动生成碰撞 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    bool bGenerateCollision = true;

    /** 生成的 Actor 相对于父 Actor 的偏移 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Mapping")
    FTransform RelativeTransform;

    FUsdPartMappingEntry()
        : bGenerateCollision(true)
        , RelativeTransform(FTransform::Identity)
    {
    }
};

/**
 * FUsdVariantMaterialEntry - 变体材质映射条目
 * 定义 USD 变体到 UE 材质的映射关系
 */
USTRUCT(BlueprintType)
struct CARRENDERFACTORY_API FUsdVariantMaterialEntry
{
    GENERATED_BODY()

    /** 变体集名称，例如: "ColorVariant" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant Material")
    FString VariantSetName;

    /** 变体名称，例如: "Red", "Blue", "Carbon" */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant Material")
    FString VariantName;

    /** 应用的部件类型（空表示应用到所有部件） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant Material")
    FString PartType;

    /** 对应的材质资产 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant Material")
    TObjectPtr<UMaterialInterface> Material;
};

/**
 * FUsdLoadSettings - USD 加载设置
 * 控制 USD Stage 的加载行为
 */
USTRUCT(BlueprintType)
struct CARRENDERFACTORY_API FUsdLoadSettings
{
    GENERATED_BODY();

    /** 是否使用瞬态加载（不保存到磁盘） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    bool bTransientLoad = true;

    /** 初始时间码 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    float InitialTimeCode = 0.0f;

    /** 是否在加载时展开所有 Prim */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    bool bExpandPrimsOnLoad = true;

    /** 根 Prim 路径（空则使用 Stage 根） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    FString RootPrimPath;

    /** 是否在 PIE 结束后卸载 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    bool bUnloadOnPIEEnd = false;

    /** 是否在加载时自动应用变体 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    bool bAutoApplyVariants = false;

    /** 默认变体集选择 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    TMap<FString, FString> DefaultVariantSelection;
};

/**
 * UUsdCarDataAsset - USD 汽车配置数据资产
 *
 * 存储 USD 文件引用和配置信息
 * 作为 AOpenUSDActor 的配置源
 */
UCLASS(BlueprintType, ClassGroup = "USD", meta = (DisplayName = "USD Car Data Asset"))
class CARRENDERFACTORY_API UUsdCarDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // ===== USD 文件配置 =====

    /** USD 文件路径（支持 .usda, .usd, .usdc 格式） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Source", meta = (FilePathFilter = "usd;usda;usdc"))
    FString UsdFilePath;

    /** 是否使用相对路径 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Source")
    bool bUseRelativePath = false;

    /** 相对路径的基础目录 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Source", meta = (EditCondition = "bUseRelativePath"))
    FString RelativePathBase;

    // ===== 加载设置 =====

    /** USD 加载设置 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loading")
    FUsdLoadSettings LoadSettings;

    // ===== 部件映射 =====

    /** 部件映射规则列表 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Mapping")
    TArray<FUsdPartMappingEntry> PartMappings;

    /** 是否自动识别部件（基于命名规则） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Mapping")
    bool bAutoIdentifyParts = true;

    /** 自动识别的命名模式 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Part Mapping")
    TArray<FString> AutoIdentifyPatterns = {TEXT("Body"), TEXT("Wheel_"), TEXT("Caliper"), TEXT("SteeringWheel")};

    // ===== 变体材质映射 =====

    /** 变体到材质的映射 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variant Materials")
    TArray<FUsdVariantMaterialEntry> VariantMaterialMappings;

    // ===== 输出设置 =====

    /** 生成的静态网格保存目录 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output", meta = (ContentDir))
    FString MeshOutputDirectory = TEXT("/Game/USD/GeneratedMeshes");

    /** 是否覆盖已存在的网格 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Output")
    bool bOverwriteExistingMeshes = false;

public:
    /**
     * GetResolvedUsdPath - 获取解析后的 USD 文件完整路径
     * 处理相对路径转换
     * @return 完整的 USD 文件路径
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    FString GetResolvedUsdPath() const;

    /**
     * FindPartMappingByPrimPath - 根据 Prim 路径查找部件映射
     * @param PrimPath Prim 路径
     * @param OutMapping 输出的部件映射
     * @return 是否找到
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    bool FindPartMappingByPrimPath(const FString& PrimPath, FUsdPartMappingEntry& OutMapping) const;

    /**
     * FindPartMappingByPartType - 根据部件类型查找部件映射
     * @param PartType 部件类型
     * @param OutMapping 输出的部件映射
     * @return 是否找到
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    bool FindPartMappingByPartType(const FString& PartType, FUsdPartMappingEntry& OutMapping) const;

    /**
     * FindPartMappingIndexByPrimPath - 根据 Prim 路径查找部件映射索引
     * @param PrimPath Prim 路径
     * @return 索引，未找到返回 -1
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    int32 FindPartMappingIndexByPrimPath(const FString& PrimPath) const;

    /**
     * FindPartMappingIndexByPartType - 根据部件类型查找部件映射索引
     * @param PartType 部件类型
     * @return 索引，未找到返回 -1
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    int32 FindPartMappingIndexByPartType(const FString& PartType) const;

    /**
     * GetMaterialForVariant - 获取指定变体的材质
     * @param VariantSetName 变体集名称
     * @param VariantName 变体名称
     * @param PartType 部件类型（可选过滤）
     * @return 材质接口指针，未找到返回 nullptr
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    UMaterialInterface* GetMaterialForVariant(const FString& VariantSetName, const FString& VariantName, const FString& PartType) const;

    /**
     * ValidateConfiguration - 验证配置是否有效
     * @param OutErrors 输出的错误信息
     * @return 配置是否有效
     */
    UFUNCTION(BlueprintCallable, Category = "USD")
    bool ValidateConfiguration(TArray<FString>& OutErrors) const;

#if WITH_EDITOR
    /** 在编辑器中验证资产 */
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
