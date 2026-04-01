#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "UsdCarFactoryBlueprintTypes.generated.h"

class AUsdHierarchicalBuildActor;
class UCarGeneratedAssemblyDataAsset;

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FUsdCarFactoryBuildInputs
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	TObjectPtr<AUsdHierarchicalBuildActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	FFilePath SourceUsdFile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	TObjectPtr<UCarGeneratedAssemblyDataAsset> GeneratedDataAsset = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	TSubclassOf<AStaticMeshActor> StaticMeshProxyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	TSubclassOf<AActor> TransformProxyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	bool bBuildAsync = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	int32 MaxPrimBuildPerTick = 24;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	float ParseResultFrameBudgetMs = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "USD Build")
	int32 MaxParseResultsPerTick = 2;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FUsdCarFactoryBuildResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bRefreshedFromSourceUsd = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bDegradedToCachedMeshes = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 AddedPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 RemovedPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 ChangedMeshPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 ChangedTransformPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 BuiltProxyActorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	FString Message;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FUsdCarFactorySourceRefreshResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bSourceFileFound = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bNeedsRefresh = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bDeltaScanSucceeded = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bDegradedToCachedMeshes = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 AddedPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 RemovedPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 ChangedMeshPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 ChangedTransformPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	FString Message;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FUsdCarFactoryDiffResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 TargetPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 ExistingProxyActorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 UnchangedPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 StalePrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	FString Message;
};

USTRUCT(BlueprintType)
struct USDCARFACTORYPIPELINE_API FUsdCarFactoryApplyResult
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	bool bSuccess = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 TargetPrimCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 RemovedProxyActorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	int32 BuiltProxyActorCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "USD Build")
	FString Message;
};
