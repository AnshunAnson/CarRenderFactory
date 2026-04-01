#include "UsdCarFactoryDiffSubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Editor.h"
#include "UsdCarFactoryDataAssetSubsystem.h"

namespace
{
	static bool AreTransformsEquivalent(const FTransform& A, const FTransform& B)
	{
		return A.GetLocation().Equals(B.GetLocation(), 0.01f)
			&& A.GetRotation().Equals(B.GetRotation(), KINDA_SMALL_NUMBER)
			&& A.GetScale3D().Equals(B.GetScale3D(), 0.001f);
	}

	static bool AreSnapshotsEquivalent(
		const UsdCarFactoryPipelineBuild::FProxyActorSnapshot& A,
		const UsdCarFactoryPipelineBuild::FProxyActorSnapshot& B
	)
	{
		return A.PrimPath == B.PrimPath
			&& A.ParentPrimPath == B.ParentPrimPath
			&& A.bIsStaticMesh == B.bIsStaticMesh
			&& A.StaticMeshPath == B.StaticMeshPath
			&& A.MaterialPaths == B.MaterialPaths
			&& AreTransformsEquivalent(A.RelativeTransform, B.RelativeTransform);
	}
}

FUsdCarFactoryDiffResult UUsdCarFactoryDiffSubsystem::BuildDiff(const FUsdCarFactoryBuildInputs& Inputs)
{
	FUsdCarFactoryDiffResult Result;

#if WITH_EDITOR
	AUsdHierarchicalBuildActor* TargetActor = Inputs.TargetActor.Get();
	if (!TargetActor)
	{
		Result.Message = TEXT("BuildDiff requires TargetActor.");
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

	TargetActor->BuildPreviousPartIndexCache();

	TArray<UsdCarFactoryPipelineBuild::FUsdProxyBuildPrimNode> PrimNodes;
	TargetActor->CollectPrimNodeDataFromGeneratedAsset(BuildAsset, PrimNodes);
	Result.TargetPrimCount = PrimNodes.Num();

	TArray<UsdCarFactoryPipelineBuild::FProxyActorSnapshot> TargetSnapshots;
	TargetSnapshots.Reserve(PrimNodes.Num());
	for (const UsdCarFactoryPipelineBuild::FUsdProxyBuildPrimNode& PrimNode : PrimNodes)
	{
		TargetSnapshots.Add(TargetActor->BuildTargetSnapshotForDiff(PrimNode));
	}

	TArray<AActor*> DuplicateProxyActors;
	TMap<FString, AActor*> AttachedProxyActors = TargetActor->GatherAttachedProxyActors(DuplicateProxyActors);
	Result.ExistingProxyActorCount = AttachedProxyActors.Num();

	TMap<FString, UsdCarFactoryPipelineBuild::FProxyActorSnapshot> ExistingSnapshots;
	for (const TPair<FString, AActor*>& Pair : AttachedProxyActors)
	{
		ExistingSnapshots.Add(Pair.Key, TargetActor->BuildExistingSnapshotForDiff(Pair.Key, Pair.Value));
	}

	TSet<FString> TargetPrimPaths;
	for (const UsdCarFactoryPipelineBuild::FProxyActorSnapshot& TargetSnapshot : TargetSnapshots)
	{
		TargetPrimPaths.Add(TargetSnapshot.PrimPath);
		if (const UsdCarFactoryPipelineBuild::FProxyActorSnapshot* ExistingSnapshot = ExistingSnapshots.Find(TargetSnapshot.PrimPath))
		{
			if (AreSnapshotsEquivalent(TargetSnapshot, *ExistingSnapshot))
			{
				++Result.UnchangedPrimCount;
			}
		}
	}

	for (const TPair<FString, UsdCarFactoryPipelineBuild::FProxyActorSnapshot>& ExistingPair : ExistingSnapshots)
	{
		if (!TargetPrimPaths.Contains(ExistingPair.Key))
		{
			++Result.StalePrimCount;
		}
	}

	Result.bSuccess = true;
	Result.Message = TEXT("Build diff computed from GeneratedDataAsset and current proxy actors.");
#else
	Result.Message = TEXT("Diff is only available in editor.");
#endif

	return Result;
}
