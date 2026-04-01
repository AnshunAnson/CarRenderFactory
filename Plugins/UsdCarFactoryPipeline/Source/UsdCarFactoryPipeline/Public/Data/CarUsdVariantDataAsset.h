#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"

#include "CarUsdVariantDataAsset.generated.h"

class UMaterialInterface;
class URuntimeVirtualTexture;
class UStaticMesh;
struct FPropertyChangedEvent;

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FCarUsdProxyActorState
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
struct USDCARFACTORYPIPELINE_API FCarUsdVariantRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FName VariantName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FString SourceUsdPath;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	FDateTime SavedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD|Variant")
	TArray<FCarUsdProxyActorState> ProxyActorStates;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCarUsdVariantDataAssetActiveVariantChanged, class UCarUsdVariantDataAsset*);

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UCarUsdVariantDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Variant", meta = (TitleProperty = "VariantName"))
	TArray<FCarUsdVariantRecord> Variants;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Variant",
		meta = (
			GetOptions = "GetVariantNameOptions",
			DisplayName = "活动Variant",
			ToolTip = "设计师常用入口：这里切换后会自动通知绑定该资产的UsdHierarchicalBuildActor，并立即应用对应变体。"
		)
	)
	FName ActiveVariantName = TEXT("Default");

	FOnCarUsdVariantDataAssetActiveVariantChanged OnActiveVariantChanged;

	UFUNCTION()
	TArray<FName> GetVariantNameOptions() const;

	const FCarUsdVariantRecord* FindVariant(const FName& VariantName) const;
	FCarUsdVariantRecord* FindVariantMutable(const FName& VariantName);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void EnsureActiveVariantValid(bool bBroadcastIfChanged);
};
