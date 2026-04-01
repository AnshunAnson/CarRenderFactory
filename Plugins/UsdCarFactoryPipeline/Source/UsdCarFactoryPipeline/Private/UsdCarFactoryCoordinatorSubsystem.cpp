#include "UsdCarFactoryCoordinatorSubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "Editor.h"
#include "UsdCarFactoryDataAssetSubsystem.h"
#include "UsdCarFactorySourceRefreshSubsystem.h"

FUsdCarFactoryBuildResult UUsdCarFactoryCoordinatorSubsystem::RunBuild(const FUsdCarFactoryBuildInputs& Inputs)
{
	FUsdCarFactoryBuildResult Result;

#if WITH_EDITOR
	AUsdHierarchicalBuildActor* TargetActor = Inputs.TargetActor.Get();
	if (!TargetActor)
	{
		Result.Message = TEXT("RunBuild requires TargetActor.");
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

	UUsdCarFactorySourceRefreshSubsystem* SourceRefreshSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactorySourceRefreshSubsystem>();
	if (!SourceRefreshSubsystem)
	{
		Result.Message = TEXT("Failed to resolve UUsdCarFactorySourceRefreshSubsystem.");
		return Result;
	}

	const FUsdCarFactorySourceRefreshResult RefreshPreview = SourceRefreshSubsystem->RefreshFromSourceIfNeeded(Inputs);

	TargetActor->ExecuteBuildPipeline();

	Result.bSuccess = true;
	Result.bRefreshedFromSourceUsd = RefreshPreview.bNeedsRefresh && RefreshPreview.bSourceFileFound;
	Result.bDegradedToCachedMeshes = RefreshPreview.bDegradedToCachedMeshes;
	Result.AddedPrimCount = RefreshPreview.AddedPrimCount;
	Result.RemovedPrimCount = RefreshPreview.RemovedPrimCount;
	Result.ChangedMeshPrimCount = RefreshPreview.ChangedMeshPrimCount;
	Result.ChangedTransformPrimCount = RefreshPreview.ChangedTransformPrimCount;
	if (!TargetActor->bBuildAsync)
	{
		Result.BuiltProxyActorCount = TargetActor->GatherAttachedProxyActors().Num();
		Result.Message = TEXT("Build completed.");
	}
	else
	{
		Result.Message = TEXT("Build started asynchronously.");
	}
#else
	Result.Message = TEXT("Coordinator is only available in editor.");
#endif

	return Result;
}
