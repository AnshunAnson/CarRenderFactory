#include "UsdCarFactorySourceRefreshSubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Editor.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "UsdCarFactoryDataAssetSubsystem.h"

FUsdCarFactorySourceRefreshResult UUsdCarFactorySourceRefreshSubsystem::RefreshFromSourceIfNeeded(const FUsdCarFactoryBuildInputs& Inputs)
{
	FUsdCarFactorySourceRefreshResult Result;

#if WITH_EDITOR
	AUsdHierarchicalBuildActor* TargetActor = Inputs.TargetActor.Get();
	if (!TargetActor)
	{
		Result.Message = TEXT("RefreshFromSourceIfNeeded requires TargetActor.");
		return Result;
	}

	if (!Inputs.SourceUsdFile.FilePath.IsEmpty())
	{
		TargetActor->SourceUsdFile = Inputs.SourceUsdFile;
	}
	if (Inputs.GeneratedDataAsset)
	{
		TargetActor->GeneratedDataAsset = Inputs.GeneratedDataAsset;
	}
	if (Inputs.StaticMeshProxyActorClass)
	{
		TargetActor->StaticMeshProxyActorClass = Inputs.StaticMeshProxyActorClass;
	}
	if (Inputs.TransformProxyActorClass)
	{
		TargetActor->TransformProxyActorClass = Inputs.TransformProxyActorClass;
	}
	TargetActor->bBuildAsync = Inputs.bBuildAsync;
	TargetActor->MaxPrimBuildPerTick = Inputs.MaxPrimBuildPerTick;
	TargetActor->ParseResultFrameBudgetMs = Inputs.ParseResultFrameBudgetMs;
	TargetActor->MaxParseResultsPerTick = Inputs.MaxParseResultsPerTick;

	UCarGeneratedAssemblyDataAsset* BuildAsset = nullptr;
	UUsdAssetCache3* AssetCache = nullptr;
	FString ResolveMessage;
	if (!GEditor)
	{
		Result.Message = TEXT("Editor subsystem access is unavailable.");
		return Result;
	}

	UUsdCarFactoryDataAssetSubsystem* DataAssetSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryDataAssetSubsystem>();
	if (!DataAssetSubsystem)
	{
		Result.Message = TEXT("Failed to resolve UUsdCarFactoryDataAssetSubsystem.");
		return Result;
	}

	if (!DataAssetSubsystem->ResolveOrCreateBuildAssets(Inputs, BuildAsset, AssetCache, ResolveMessage))
	{
		Result.Message = ResolveMessage;
		return Result;
	}

	if (TargetActor->SourceUsdFile.FilePath.IsEmpty())
	{
		Result.bSuccess = true;
		Result.Message = TEXT("SourceUsdFile is empty. Refresh is not required.");
		return Result;
	}

	const FString NormalizedSourceUsdPath = FPaths::ConvertRelativePathToFull(TargetActor->SourceUsdFile.FilePath);
	if (!IFileManager::Get().FileExists(*NormalizedSourceUsdPath))
	{
		Result.bSuccess = true;
		Result.bSourceFileFound = false;
		Result.Message = FString::Printf(TEXT("Source USD does not exist: %s"), *NormalizedSourceUsdPath);
		return Result;
	}

	Result.bSourceFileFound = true;
	const FDateTime SourceUsdTimestampUtc = IFileManager::Get().GetTimeStamp(*NormalizedSourceUsdPath);
	const int64 SourceUsdFileSizeBytes = IFileManager::Get().FileSize(*NormalizedSourceUsdPath);
	Result.bNeedsRefresh = TargetActor->ShouldRefreshCacheFromSourceUsd(
		BuildAsset,
		NormalizedSourceUsdPath,
		SourceUsdTimestampUtc,
		SourceUsdFileSizeBytes
	);

	if (!Result.bNeedsRefresh)
	{
		Result.bSuccess = true;
		Result.Message = TEXT("Source metadata matches GeneratedDataAsset. Refresh is not required.");
		return Result;
	}

	TMap<FString, UsdCarFactoryPipelineBuild::FCachePrimSnapshot> CacheSnapshots;
	TargetActor->BuildCachePrimSnapshots(BuildAsset, CacheSnapshots);

	UsdCarFactoryPipelineBuild::FSourceDeltaScanResult DeltaResult;
	Result.bDeltaScanSucceeded = AUsdHierarchicalBuildActor::RunSourceDeltaScanSync(
		NormalizedSourceUsdPath,
		CacheSnapshots,
		DeltaResult
	);

	Result.AddedPrimCount = DeltaResult.AddedPrimPaths.Num();
	Result.RemovedPrimCount = DeltaResult.RemovedPrimPaths.Num();
	Result.ChangedMeshPrimCount = DeltaResult.ChangedMeshPrimPaths.Num();
	Result.ChangedTransformPrimCount = DeltaResult.HierarchyOrTransformChangedPrimPaths.Num();
	Result.bSuccess = true;

	if (!Result.bDeltaScanSucceeded)
	{
		Result.bDegradedToCachedMeshes = true;
		Result.Message = DeltaResult.ErrorMessage.IsEmpty()
			? TEXT("Source delta scan failed. A full build would degrade to cached data.")
			: DeltaResult.ErrorMessage;
	}
	else
	{
		Result.Message = TEXT("Source delta scan completed.");
	}
#else
	Result.Message = TEXT("Source refresh is only available in editor.");
#endif

	return Result;
}
