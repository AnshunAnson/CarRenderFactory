#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Components/RuntimeVirtualTextureComponent.h"
#include "CarUsdVariantDataAsset.generated.h"

class UStaticMesh;
class UMaterialInterface;

USTRUCT(BlueprintType)
struct FCarUsdProxyActorState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FString PrimPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FString ProxyActorPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TEnumAsByte<EComponentMobility::Type> Mobility = TEnumAsByte<EComponentMobility::Type>(EComponentMobility::Movable);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bActorHiddenInGame = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TEnumAsByte<ECollisionEnabled::Type> CollisionEnabled = TEnumAsByte<ECollisionEnabled::Type>(ECollisionEnabled::QueryAndPhysics);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bComponentVisible = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bCastShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bIsStaticMeshActor = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TObjectPtr<UStaticMesh> StaticMesh = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TArray<TObjectPtr<UMaterialInterface>> Materials;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TArray<FName> MaterialSlotNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TMap<FName, TObjectPtr<UMaterialInterface>> MaterialOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bRenderCustomDepth = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	int32 CustomDepthStencilValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	int32 TranslucencySortPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bVisibleInReflectionCaptures = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	bool bCastHiddenShadow = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FLightingChannels LightingChannels;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TArray<TObjectPtr<URuntimeVirtualTexture>> RuntimeVirtualTextures;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	uint8 VirtualTextureRenderPassType = 0;
};

USTRUCT(BlueprintType)
struct FCarUsdVariantRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FName VariantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FString SourceUsdPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	FDateTime SavedAtUtc;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Variant")
	TArray<FCarUsdProxyActorState> ProxyActorStates;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActiveVariantChanged, UCarUsdVariantDataAsset*, ChangedAsset);

/**
 * Stores named snapshots of proxy actor states (materials, visibility, transforms, etc.)
 * for quick variant switching during USD car factory workflows.
 */
UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UCarUsdVariantDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	/** All saved variants. Each variant stores per-actor property snapshots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variants")
	TArray<FCarUsdVariantRecord> Variants;

	/** Currently active variant name. Changing this triggers OnActiveVariantChanged. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variants")
	FName ActiveVariantName;

	/** Broadcast when ActiveVariantName changes (via editor property change or programmatic set). */
	UPROPERTY(BlueprintAssignable, Category = "Variants")
	FOnActiveVariantChanged OnActiveVariantChanged;

	/** Returns a deduplicated list of available variant names. */
	UFUNCTION(BlueprintCallable, Category = "USD Variant")
	TArray<FName> GetVariantNameOptions() const;

	/** Find a variant record by name (const). */
	const FCarUsdVariantRecord* FindVariant(const FName& VariantName) const;

	/** Find a variant record by name (mutable). */
	FCarUsdVariantRecord* FindVariantMutable(const FName& VariantName);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void EnsureActiveVariantValid(bool bBroadcastIfChanged);
};
