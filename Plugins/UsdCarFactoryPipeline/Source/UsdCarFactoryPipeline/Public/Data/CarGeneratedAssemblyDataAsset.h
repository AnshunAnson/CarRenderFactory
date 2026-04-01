#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "UObject/SoftObjectPath.h"

#include "CarGeneratedAssemblyDataAsset.generated.h"

class UMaterialInterface;
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

	// Persist the intended generated proxy type. This must not depend on whether
	// the current build managed to resolve a UStaticMesh asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	bool bIsStaticMesh = false;

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

	// Reference to the standalone USD asset cache asset used by this build snapshot.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TObjectPtr<UUsdAssetCache3> UsdAssetCache = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD")
	TArray<FCarGeneratedPartRecord> Parts;
};
