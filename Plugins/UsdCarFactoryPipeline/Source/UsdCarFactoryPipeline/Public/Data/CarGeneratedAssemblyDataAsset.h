#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "UObject/SoftObjectPath.h"

#include "CarGeneratedAssemblyDataAsset.generated.h"

class UMaterialInterface;
class URuntimeVirtualTexture;
class UStaticMesh;
class UUsdAssetCache3;

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FCarGeneratedPartRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FName PartKey = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FString PrimPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FString ParentPrimPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	// Legacy: indexed material array, kept for backward compatibility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	// Material slot names from the static mesh, used to match materials when mesh changes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TArray<FName> MaterialSlotNames;

	// Material overrides keyed by slot name - survives mesh topology changes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TMap<FName, TObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FString MeshContentHash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FName CCRComponentName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FString ProxyActorPath;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FCarGeneratedProxyActorVariantRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FString PrimPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bIsStaticMeshActor = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bActorHiddenInGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TEnumAsByte<EComponentMobility::Type> Mobility = EComponentMobility::Movable;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = ECollisionEnabled::QueryAndPhysics;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bComponentVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bCastShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bRenderCustomDepth = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	int32 CustomDepthStencilValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	int32 TranslucencySortPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bVisibleInReflectionCaptures = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bCastHiddenShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FLightingChannels LightingChannels;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TArray<TObjectPtr<URuntimeVirtualTexture>> RuntimeVirtualTextures;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	ERuntimeVirtualTextureMainPassType VirtualTextureRenderPassType =
		ERuntimeVirtualTextureMainPassType::Exclusive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	// Legacy: indexed material array, kept for backward compatibility
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	// Material slot names from the static mesh, used to match materials when mesh changes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TArray<FName> MaterialSlotNames;

	// Material overrides keyed by slot name - survives mesh topology changes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TMap<FName, TObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FString ProxyActorPath;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FCarGeneratedMaterialVariantRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FName VariantName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FString SourceUsdPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FDateTime SavedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TArray<FCarGeneratedProxyActorVariantRecord> ProxyActorConfigs;
};

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UCarGeneratedAssemblyDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FFilePath SourceUsdFile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	FDateTime SourceUsdTimestampUtc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	int64 SourceUsdFileSizeBytes = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TObjectPtr<UUsdAssetCache3> UsdAssetCache = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TArray<FCarGeneratedPartRecord> Parts;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD|Legacy Variant",
		meta = (AdvancedDisplay, DisplayName = "LegacyVariants(兼容迁移用)")
	)
	TArray<FCarGeneratedMaterialVariantRecord> MaterialVariants;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD|Legacy Variant",
		meta = (AdvancedDisplay, DisplayName = "LegacyActiveVariantName(兼容迁移用)")
	)
	FName ActiveVariantName = TEXT("Default");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	bool bHasAutoMigratedLegacyPartsToVariants = false;
};
