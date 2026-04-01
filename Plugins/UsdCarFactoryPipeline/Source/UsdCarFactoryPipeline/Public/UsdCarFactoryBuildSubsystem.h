#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "EditorSubsystem.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "HAL/ThreadSafeCounter.h"
#include "UObject/ObjectKey.h"

#include "UsdCarFactoryBuildSubsystem.generated.h"

class AUsdHierarchicalBuildActor;
class UCarGeneratedAssemblyDataAsset;
class UMaterialInterface;
class UStaticMesh;

namespace UsdCarFactoryPipelineBuild
{
	enum class EUsdProxyBuildJobPhase : uint8
	{
		Idle,
		Scanning,
		Importing,
		Diffing,
		Applying
	};

	struct FUsdProxyBuildPrimNode
	{
		FString PrimPath;
		FString ParentPrimPath;
		FString PrimTypeName;
		FTransform RelativeTransform = FTransform::Identity;
		FTransform WorldTransform = FTransform::Identity;
		bool bIsStaticMesh = false;
		FString MeshContentHash;
		TObjectPtr<UStaticMesh> StaticMesh = nullptr;
		TArray<TObjectPtr<UMaterialInterface>> SourceMaterials;
		TArray<FName> SourceMaterialSlotNames;
	};

	struct FUsdProxyBuildRequest
	{
		TWeakObjectPtr<AUsdHierarchicalBuildActor> TargetActor;
		FString SourceUsdPath;
		FDateTime SourceUsdTimestampUtc;
		int64 SourceUsdFileSizeBytes = -1;
		TObjectPtr<UCarGeneratedAssemblyDataAsset> GeneratedDataAsset = nullptr;
		TSubclassOf<AStaticMeshActor> StaticMeshProxyActorClass;
		TSubclassOf<AActor> TransformProxyActorClass;
		bool bBuildAsync = true;
		int32 MaxPrimBuildPerTick = 24;
		float ParseResultFrameBudgetMs = 3.0f;
		int32 MaxParseResultsPerTick = 2;
	};

	struct FUsdProxyBuildPlan
	{
		TObjectPtr<UCarGeneratedAssemblyDataAsset> BuildAsset = nullptr;
		TArray<FUsdProxyBuildPrimNode> PrimNodes;
		TSet<FString> UnchangedPrimPaths;
		TArray<FString> StalePrimPaths;
		int32 RequestId = 0;
		int32 AddedPrimCount = 0;
		int32 RemovedPrimCount = 0;
		int32 ChangedMeshPrimCount = 0;
		int32 ChangedTransformPrimCount = 0;
		int32 ImportedBatchCount = 0;
		bool bRefreshedFromSourceUsd = false;
		bool bUsedSelectiveImport = false;
		bool bUsedRetryImport = false;
		bool bDegradedToCachedMeshes = false;
		FString DegradedReason;

		void Reset()
		{
			BuildAsset = nullptr;
			PrimNodes.Reset();
			UnchangedPrimPaths.Reset();
			StalePrimPaths.Reset();
			RequestId = 0;
			AddedPrimCount = 0;
			RemovedPrimCount = 0;
			ChangedMeshPrimCount = 0;
			ChangedTransformPrimCount = 0;
			ImportedBatchCount = 0;
			bRefreshedFromSourceUsd = false;
			bUsedSelectiveImport = false;
			bUsedRetryImport = false;
			bDegradedToCachedMeshes = false;
			DegradedReason.Reset();
		}
	};

	struct FProxyActorSnapshot
	{
		FString PrimPath;
		FString ParentPrimPath;
		FTransform RelativeTransform = FTransform::Identity;
		bool bIsStaticMesh = false;
		FString StaticMeshPath;
		TArray<FString> MaterialPaths;
	};

	struct FBuildDiffResult
	{
		TSet<FString> UnchangedPrimPaths;
		TArray<FString> StalePrimPaths;
	};

	struct FCachePrimSnapshot
	{
		FString ParentPrimPath;
		FTransform RelativeTransform = FTransform::Identity;
		bool bIsStaticMesh = false;
		FString MeshContentHash;
	};

	struct FSourcePrimSnapshot
	{
		FString PrimPath;
		FString ParentPrimPath;
		FString PrimTypeName;
		FTransform RelativeTransform = FTransform::Identity;
		bool bIsStaticMesh = false;
		FString MeshContentHash;
	};

	struct FSourceDeltaScanResult
	{
		bool bSuccess = false;
		FString ErrorMessage;
		TArray<FSourcePrimSnapshot> SourcePrimSnapshots;
		TSet<FString> AddedPrimPaths;
		TSet<FString> RemovedPrimPaths;
		TSet<FString> HierarchyOrTransformChangedPrimPaths;
		TSet<FString> ChangedMeshPrimPaths;
	};

	struct FSourceParseTask
	{
		TWeakObjectPtr<AUsdHierarchicalBuildActor> TargetActor;
		int32 RequestId = 0;
		FString SourceUsdPath;
		FDateTime SourceUsdTimestampUtc;
		int64 SourceUsdFileSizeBytes = -1;
		TMap<FString, FCachePrimSnapshot> CacheSnapshots;
	};

	struct FSourceParseResult
	{
		TWeakObjectPtr<AUsdHierarchicalBuildActor> TargetActor;
		int32 RequestId = 0;
		FString SourceUsdPath;
		FDateTime SourceUsdTimestampUtc;
		int64 SourceUsdFileSizeBytes = -1;
		bool bDeltaScanSucceeded = false;
		double DeltaScanSeconds = 0.0;
		FString ErrorMessage;
		TArray<FSourcePrimSnapshot> SourcePrimSnapshots;
		TSet<FString> AddedPrimPaths;
		TSet<FString> RemovedPrimPaths;
		TSet<FString> HierarchyOrTransformChangedPrimPaths;
		TSet<FString> ChangedMeshPrimPaths;
	};

	struct FGlobalParseSchedulerState
	{
		TQueue<FSourceParseResult, EQueueMode::Mpsc> CompletedResults;
		FThreadSafeCounter CompletedResultCount;
	};

	struct FUsdProxyBuildJobState
	{
		EUsdProxyBuildJobPhase Phase = EUsdProxyBuildJobPhase::Idle;
		bool bParseResultPumpScheduled = false;
		int32 PendingSelectiveImportRetryCount = 0;
		FUsdProxyBuildRequest ActiveRequest;
		FUsdProxyBuildPlan PendingBuildPlan;
		TMap<FString, AActor*> PendingApplyAttachedProxyActors;
		TMap<FString, AActor*> PendingApplyPrimToProxyActor;
		TArray<FCarGeneratedPartRecord> PendingApplyNewRecords;
		TMap<FString, AActor*> PendingDiffAttachedProxyActors;
		TArray<FString> PendingDiffAttachedPrimPaths;
		TArray<FProxyActorSnapshot> PendingDiffTargetSnapshots;
		TMap<FString, FProxyActorSnapshot> PendingDiffExistingSnapshots;
		TArray<FString> PendingSelectiveImportPrimPaths;
		TMap<FString, TObjectPtr<UStaticMesh>> PendingSelectivePrimToMesh;
		TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> PendingSelectivePrimToMaterials;
		TSet<FString> PendingResolvedSelectiveImportPrimPaths;
		TArray<FString> PendingRetryImportPrimPaths;
		int32 PendingApplyIndex = 0;
		int32 PendingApplyRecordCount = 0;
		int32 PendingDiffPrimIndex = 0;
		int32 PendingDiffAttachedIndex = 0;
		int32 PendingSelectiveImportIndex = 0;
		int32 PendingSelectiveImportBatchCount = 0;
		double ActiveBuildStartSeconds = 0.0;
		double ActiveSourceRefreshSeconds = 0.0;
		double ActiveDeltaScanSeconds = 0.0;
		double ActiveSelectiveImportSeconds = 0.0;
		double ActiveDiffSeconds = 0.0;
		double ActiveApplySeconds = 0.0;

		void ResetForNextBuild()
		{
			Phase = EUsdProxyBuildJobPhase::Idle;
			bParseResultPumpScheduled = false;
			PendingSelectiveImportRetryCount = 0;
			ActiveRequest = FUsdProxyBuildRequest{};
			PendingBuildPlan.Reset();
			PendingApplyAttachedProxyActors.Reset();
			PendingApplyPrimToProxyActor.Reset();
			PendingApplyNewRecords.Reset();
			PendingDiffAttachedProxyActors.Reset();
			PendingDiffAttachedPrimPaths.Reset();
			PendingDiffTargetSnapshots.Reset();
			PendingDiffExistingSnapshots.Reset();
			PendingSelectiveImportPrimPaths.Reset();
			PendingSelectivePrimToMesh.Reset();
			PendingSelectivePrimToMaterials.Reset();
			PendingResolvedSelectiveImportPrimPaths.Reset();
			PendingRetryImportPrimPaths.Reset();
			PendingApplyIndex = 0;
			PendingApplyRecordCount = 0;
			PendingDiffPrimIndex = 0;
			PendingDiffAttachedIndex = 0;
			PendingSelectiveImportIndex = 0;
			PendingSelectiveImportBatchCount = 0;
			ActiveBuildStartSeconds = 0.0;
			ActiveSourceRefreshSeconds = 0.0;
			ActiveDeltaScanSeconds = 0.0;
			ActiveSelectiveImportSeconds = 0.0;
			ActiveDiffSeconds = 0.0;
			ActiveApplySeconds = 0.0;
		}

		void ResetSourceRefreshState()
		{
			Phase = EUsdProxyBuildJobPhase::Idle;
			PendingSelectiveImportRetryCount = 0;
			PendingSelectiveImportPrimPaths.Reset();
			PendingSelectivePrimToMesh.Reset();
			PendingSelectivePrimToMaterials.Reset();
			PendingResolvedSelectiveImportPrimPaths.Reset();
			PendingRetryImportPrimPaths.Reset();
			PendingSelectiveImportIndex = 0;
			PendingSelectiveImportBatchCount = 0;
			ActiveSourceRefreshSeconds = 0.0;
			ActiveDeltaScanSeconds = 0.0;
			ActiveSelectiveImportSeconds = 0.0;
		}

		void ResetDiffState()
		{
			if (Phase == EUsdProxyBuildJobPhase::Diffing)
			{
				Phase = EUsdProxyBuildJobPhase::Idle;
			}

			PendingDiffAttachedProxyActors.Reset();
			PendingDiffAttachedPrimPaths.Reset();
			PendingDiffTargetSnapshots.Reset();
			PendingDiffExistingSnapshots.Reset();
			PendingDiffPrimIndex = 0;
			PendingDiffAttachedIndex = 0;
			ActiveDiffSeconds = 0.0;
		}

		void ResetApplyState()
		{
			if (Phase == EUsdProxyBuildJobPhase::Applying)
			{
				Phase = EUsdProxyBuildJobPhase::Idle;
			}

			PendingApplyAttachedProxyActors.Reset();
			PendingApplyPrimToProxyActor.Reset();
			PendingApplyNewRecords.Reset();
			PendingApplyIndex = 0;
			PendingApplyRecordCount = 0;
			ActiveApplySeconds = 0.0;
			PendingBuildPlan.Reset();
		}
	};
}

UCLASS()
class USDCARFACTORYPIPELINE_API UUsdCarFactoryBuildSubsystem : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	static UUsdCarFactoryBuildSubsystem* Get();

	virtual void Deinitialize() override;

	void EnqueueGlobalParseTask(UsdCarFactoryPipelineBuild::FSourceParseTask&& Task);
	bool DequeueGlobalParseResult(UsdCarFactoryPipelineBuild::FSourceParseResult& OutResult);
	bool HasGlobalCompletedParseResults() const;
	bool HasGlobalParseWork() const;

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& GetJobState(AUsdHierarchicalBuildActor& Actor);
	void ResetJobState(AUsdHierarchicalBuildActor& Actor);
	void RemoveJobState(const AUsdHierarchicalBuildActor& Actor);

private:
	static void ExecuteGlobalParseTask(
		TWeakObjectPtr<UUsdCarFactoryBuildSubsystem> WeakSubsystem,
		UsdCarFactoryPipelineBuild::FSourceParseTask&& Task
	);

	TMap<TObjectKey<AUsdHierarchicalBuildActor>, UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState> JobStates;
	UsdCarFactoryPipelineBuild::FGlobalParseSchedulerState GlobalParseSchedulerState;
};
