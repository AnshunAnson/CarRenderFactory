#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "UsdCarFactoryBlueprintTypes.h"
#include "UsdCarFactoryDataAssetSubsystem.generated.h"

class UCarGeneratedAssemblyDataAsset;
class UUsdAssetCache3;

UCLASS(BlueprintType)
class USDCARFACTORYPIPELINE_API UUsdCarFactoryDataAssetSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "USD Build")
	bool ResolveOrCreateBuildAssets(
		const FUsdCarFactoryBuildInputs& Inputs,
		UCarGeneratedAssemblyDataAsset*& OutGeneratedDataAsset,
		UUsdAssetCache3*& OutUsdAssetCache,
		FString& OutMessage
	);

private:
	UCarGeneratedAssemblyDataAsset* ResolveOrCreateGeneratedDataAsset(const FUsdCarFactoryBuildInputs& Inputs, FString& OutMessage);
	UUsdAssetCache3* ResolveOrCreateUsdAssetCacheAsset(
		const FUsdCarFactoryBuildInputs& Inputs,
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		FString& OutMessage
	);
};
