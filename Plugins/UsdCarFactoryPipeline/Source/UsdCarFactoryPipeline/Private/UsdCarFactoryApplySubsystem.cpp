#include "UsdCarFactoryApplySubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Editor.h"
#include "UsdCarFactoryDataAssetSubsystem.h"
#include "UsdCarFactoryDiffSubsystem.h"

FUsdCarFactoryApplyResult UUsdCarFactoryApplySubsystem::ApplyBuildPlan(const FUsdCarFactoryBuildInputs& Inputs)
{
	FUsdCarFactoryApplyResult Result;

#if WITH_EDITOR
	AUsdHierarchicalBuildActor* TargetActor = Inputs.TargetActor.Get();
	if (!TargetActor)
	{
		Result.Message = TEXT("ApplyBuildPlan requires TargetActor.");
		return Result;
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

	UUsdCarFactoryDiffSubsystem* DiffSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryDiffSubsystem>();
	if (!DiffSubsystem)
	{
		Result.Message = TEXT("Failed to resolve UUsdCarFactoryDiffSubsystem.");
		return Result;
	}

	const FUsdCarFactoryDiffResult DiffPreview = DiffSubsystem->BuildDiff(Inputs);
	Result.RemovedProxyActorCount = DiffPreview.StalePrimCount;

	TargetActor->BuildPreviousPartIndexCache();
	TArray<UsdCarFactoryPipelineBuild::FUsdProxyBuildPrimNode> PrimNodes;
	TargetActor->CollectPrimNodeDataFromGeneratedAsset(BuildAsset, PrimNodes);
	Result.TargetPrimCount = PrimNodes.Num();

	TargetActor->StartBuildFromPreparedPrimNodes(BuildAsset, MoveTemp(PrimNodes), false, false);
	if (TargetActor->bBuildAsync)
	{
		Result.bSuccess = true;
		Result.Message = TEXT("Apply build started asynchronously from GeneratedDataAsset.");
	}
	else
	{
		Result.BuiltProxyActorCount = TargetActor->GatherAttachedProxyActors().Num();
		Result.bSuccess = true;
		Result.Message = TEXT("Apply build completed from GeneratedDataAsset.");
	}
#else
	Result.Message = TEXT("Apply is only available in editor.");
#endif

	return Result;
}
