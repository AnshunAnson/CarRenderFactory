#pragma once

#include "CoreMinimal.h"
#include "Containers/Queue.h"
#include "GameFramework/Actor.h"
#include "HAL/ThreadSafeCounter.h"
#include "UObject/SoftObjectPath.h"
#include "UsdWrappers/UsdStage.h"

#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Data/CarUsdVariantDataAsset.h"
#include "UsdHierarchicalBuildActor.generated.h"

class UMaterialInterface;
class AStaticMeshActor;
class UStaticMesh;
class UStaticMeshComponent;
class UUsdAssetCache3;
class UObject;
struct FPropertyChangedEvent;

namespace UsdCarFactoryPipelineBuild
{
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
}

UCLASS(meta = (PrioritizeCategories = "USD Build"))
class USDCARFACTORYPIPELINE_API AUsdHierarchicalBuildActor : public AActor
{
	GENERATED_BODY()

public:
	AUsdHierarchicalBuildActor();

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build",
		meta = (DisplayName = "USD源文件", ToolTip = "用于比对现有USDdatacache。若源文件有更新，LoadAndBuild会先刷新缓存再构建代理层级。")
	)
	FFilePath SourceUsdFile;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build",
		meta = (DisplayName = "生成数据资产", ToolTip = "用于保存USD构建快照(Parts)。首次可留空，点击构建时会自动创建。")
	)
	TObjectPtr<UCarGeneratedAssemblyDataAsset> GeneratedDataAsset = nullptr;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build",
		meta = (
			DisplayName = "USD资产缓存引用",
			ToolTip = "构建使用的UUsdAssetCache3。可手动指定复用；为空时会自动创建，并与生成数据资产同步。"
		)
	)
	TObjectPtr<UUsdAssetCache3> UsdAssetCacheReference = nullptr;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Variant",
		meta = (
			DisplayName = "变体数据资产",
			ToolTip = "推荐独立资产：用于持久化所有材质/属性变体。请在该资产里切换“活动Variant”菜单，切换后会自动回放到当前Actor。"
		)
	)
	TObjectPtr<UCarUsdVariantDataAsset> VariantDataAsset = nullptr;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Variant",
		meta = (
			DisplayName = "构建时自动创建Variant资产",
			ToolTip = "默认关闭。关闭时LoadAndBuild只使用已有Variant资产，不会新建；点“保存当前缓存到活动Variant”时才会创建。"
		)
	)
	bool bAutoCreateVariantDataAssetOnBuild = false;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Variant",
		meta = (
			GetOptions = "GetActiveVariantNameOptions",
			DisplayName = "活动Variant",
			ToolTip = "在Actor面板直接切换活动Variant。变更会同步到VariantDataAsset并立即回放。"
		)
	)
	FName ActiveVariantName = TEXT("Default");

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Variant",
		meta = (
			DisplayName = "保存Variant名称",
			ToolTip = "点击“保存当前缓存到活动Variant”时优先保存到该名称。为空时保存到当前活动Variant。"
		)
	)
	FName SaveVariantName = NAME_None;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Actor Class",
		meta = (
			DisplayName = "TypeName到Actor类映射",
			ToolTip = "按USD Prim的TypeName指定Actor类（例如 Mesh/Xform）。默认: xform->Actor, mesh->StaticMeshActor。Mesh映射到非AStaticMeshActor会回退。"
		)
	)
	TMap<FString, TSubclassOf<AActor>> TypeNameToActorClassMap;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build",
		meta = (DisplayName = "异步分批构建", ToolTip = "开启后分帧处理，编辑器更流畅；关闭会一次性处理，可能明显卡顿。")
	)
	bool bBuildAsync = true;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build",
		meta = (ClampMin = "1", UIMin = "1", DisplayName = "每帧最大构建数量", ToolTip = "只在异步模式生效。值越小越流畅但构建更慢，值越大越快但更容易卡顿。建议范围：10~40。")
	)
	int32 MaxPrimBuildPerTick = 24;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Performance",
		meta = (
			ClampMin = "2",
			ClampMax = "4",
			UIMin = "2",
			UIMax = "4",
			DisplayName = "解析并发上限",
			ToolTip = "全局USD解析线程池并发上限（2~4）。仅影响OpenStage/遍历/Delta/Hash，不影响导入和应用。"
		)
	)
	int32 MaxConcurrentParseTasks = 2;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Performance",
		meta = (
			ClampMin = "2.0",
			ClampMax = "5.0",
			UIMin = "2.0",
			UIMax = "5.0",
			DisplayName = "解析结果每帧预算(ms)",
			ToolTip = "GameThread每帧消费解析结果队列的时间预算（2~5ms）。"
		)
	)
	float ParseResultFrameBudgetMs = 3.0f;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Performance",
		meta = (
			ClampMin = "1",
			UIMin = "1",
			DisplayName = "每帧最多消费解析结果数",
			ToolTip = "GameThread每帧最多分发多少个解析完成结果。"
		)
	)
	int32 MaxParseResultsPerTick = 2;

	UFUNCTION(
		BlueprintCallable,
		CallInEditor,
		Category = "USD Build",
		meta = (
			DisplayName = "加载并构建USD",
			ToolTip = "优先检查现有USDdatacache；若检测到源USD变化则先更新缓存，再构建/同步代理Actor层级。构建结束自动应用活动Variant。"
		)
	)
	void LoadAndBuild();

	UFUNCTION(
		BlueprintCallable,
		CallInEditor,
		Category = "USD Build",
		meta = (
			DisplayName = "保存当前缓存到活动Variant",
			ToolTip = "第3步：扫描当前Actor下代理对象，把材质和渲染属性写入“变体数据资产/活动Variant”。不会写回Parts快照。"
		)
	)
	void SaveCurrentCacheAsMaterialVariant();

	UFUNCTION()
	TArray<FName> GetActiveVariantNameOptions() const;

	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	bool bIsBuildInProgress = false;
	int32 ActiveBuildRequestId = 0;

	struct FPrimNodeBuildData
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
		FCriticalSection DispatchMutex;
		TQueue<FSourceParseTask, EQueueMode::Mpsc> PendingTasks;
		TQueue<FSourceParseResult, EQueueMode::Mpsc> CompletedResults;
		TAtomic<int32> ActiveWorkerCount{0};
		TAtomic<int32> MaxConcurrentTasks{2};
		FThreadSafeCounter PendingTaskCount;
		FThreadSafeCounter CompletedResultCount;
	};

private:
	TArray<FPrimNodeBuildData> PendingBuildPrimNodes;
	TMap<FString, AActor*> PendingApplyAttachedProxyActors;
	TMap<FString, AActor*> PendingApplyPrimToProxyActor;
	TArray<FCarGeneratedPartRecord> PendingApplyNewRecords;
	TSet<FString> PendingApplyUnchangedPrimPaths;
	TArray<FString> PendingApplyStalePrimPaths;
	TObjectPtr<UCarGeneratedAssemblyDataAsset> PendingApplyBuildAsset = nullptr;
	int32 PendingApplyRequestId = 0;
	int32 PendingApplyIndex = 0;
	int32 PendingApplyRecordCount = 0;
	TMap<FString, AActor*> PendingDiffAttachedProxyActors;
	TArray<FString> PendingDiffAttachedPrimPaths;
	TArray<UsdCarFactoryPipelineBuild::FProxyActorSnapshot> PendingDiffTargetSnapshots;
	TMap<FString, UsdCarFactoryPipelineBuild::FProxyActorSnapshot> PendingDiffExistingSnapshots;
	TObjectPtr<UCarGeneratedAssemblyDataAsset> PendingDiffBuildAsset = nullptr;
	int32 PendingDiffRequestId = 0;
	int32 PendingDiffPrimIndex = 0;
	int32 PendingDiffAttachedIndex = 0;
	bool bPendingDiffRefreshedFromSourceUsd = false;
	TObjectPtr<UCarGeneratedAssemblyDataAsset> PendingSourceBuildAsset = nullptr;
	TArray<FPrimNodeBuildData> PendingSourcePrimNodes;
	TArray<FString> PendingSelectiveImportPrimPaths;
	TMap<FString, TObjectPtr<UStaticMesh>> PendingSelectivePrimToMesh;
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> PendingSelectivePrimToMaterials;
	FString PendingSourceUsdPath;
	FDateTime PendingSourceUsdTimestampUtc;
	int64 PendingSourceUsdFileSizeBytes = -1;
	int32 PendingSourceRequestId = 0;
	int32 PendingSelectiveImportIndex = 0;
	int32 PendingSelectiveImportBatchCount = 0;
	bool bPendingSourceRefreshFromUsd = false;
	bool bPendingSelectiveImportFallbackToFullImport = false;
	bool bParseResultPumpScheduled = false;
	int32 PendingDeltaAddedCount = 0;
	int32 PendingDeltaRemovedCount = 0;
	int32 PendingDeltaChangedMeshCount = 0;
	int32 PendingDeltaChangedXformCount = 0;
	TMap<FString, int32> PreviousPartIndexByPrimPath;
	double ActiveBuildStartSeconds = 0.0;
	double ActiveSourceRefreshSeconds = 0.0;
	double ActiveDeltaScanSeconds = 0.0;
	double ActiveSelectiveImportSeconds = 0.0;
	double ActiveDiffSeconds = 0.0;
	double ActiveApplySeconds = 0.0;
	FDelegateHandle VariantDataAssetChangedHandle;
	TWeakObjectPtr<UCarUsdVariantDataAsset> BoundVariantDataAsset;

	UCarGeneratedAssemblyDataAsset* ResolveOrCreateGeneratedDataAsset();
	UCarUsdVariantDataAsset* ResolveOrCreateVariantDataAsset();
	bool ShouldRefreshCacheFromSourceUsd(
		const UCarGeneratedAssemblyDataAsset* BuildAsset,
		const FString& SourceUsdPath,
		const FDateTime& SourceUsdTimestamp,
		int64 SourceUsdFileSizeBytes
	) const;
	bool CollectPrimNodeDataFromSourceUsd(TArray<FPrimNodeBuildData>& OutData) const;
	static bool RunSourceDeltaScanSync(
		const FString& SourceUsdPath,
		const TMap<FString, FCachePrimSnapshot>& CacheSnapshots,
		FSourceDeltaScanResult& OutDeltaResult
	);
	static bool CollectSourcePrimSnapshots(
		const UE::FUsdStage& InStage,
		TArray<FSourcePrimSnapshot>& OutSnapshots
	);
	static bool IsMeshPrimType(const FString& PrimTypeName);
	static FString ComputeMeshContentHash(const UE::FUsdStage& InStage, const UE::FUsdPrim& Prim);
	static FSourceParseResult MakeParseResult(
		const TWeakObjectPtr<AUsdHierarchicalBuildActor>& TargetActor,
		int32 RequestId,
		const FString& SourceUsdPath,
		const FDateTime& SourceUsdTimestampUtc,
		int64 SourceUsdFileSizeBytes,
		FSourceDeltaScanResult&& DeltaResult,
		bool bDeltaScanCallSucceeded,
		double DeltaScanSeconds,
		const TCHAR* DefaultFailureMessage
	);
	static void UpdateSourceRefreshMetadata(
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		const FString& SourceUsdPath,
		const FDateTime& SourceUsdTimestampUtc,
		int64 SourceUsdFileSizeBytes
	);
	void ConvertSourceSnapshotsToPrimNodes(
		const TArray<FSourcePrimSnapshot>& SourceSnapshots,
		TArray<FPrimNodeBuildData>& OutPrimNodes
	) const;
	void BuildCachePrimSnapshots(
		const UCarGeneratedAssemblyDataAsset* BuildAsset,
		TMap<FString, FCachePrimSnapshot>& OutCacheSnapshots
	) const;
	void BeginAsyncSourceDeltaScan(
		const FString& SourceUsdPath,
		const FDateTime& SourceUsdTimestampUtc,
		int64 SourceUsdFileSizeBytes,
		TMap<FString, FCachePrimSnapshot>&& CacheSnapshots
	);
	void BeginSelectiveImportBatches(
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		TArray<FPrimNodeBuildData>&& PrimNodes,
		TArray<FString>&& ChangedMeshPrimPaths,
		const FString& SourceUsdPath,
		const FDateTime& SourceUsdTimestampUtc,
		int64 SourceUsdFileSizeBytes,
		int32 RequestId,
		bool bRefreshedFromSourceUsd,
		int32 AddedCount,
		int32 RemovedCount,
		int32 ChangedMeshCount,
		int32 ChangedXformCount
	);
	void ProcessSelectiveImportBatch();
	void FinalizeSourceRefreshAndStartBuild();
	bool ImportMeshAssetsForPrimPaths(
		const FString& SourceUsdPath,
		const TArray<FString>& PrimPaths,
		TMap<FString, TObjectPtr<UStaticMesh>>& OutPrimToMesh,
		TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& OutPrimToMaterials
	) const;
	void ApplyImportedAssetsToPrimNodes(
		TArray<FPrimNodeBuildData>& InOutPrimNodes,
		const TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
		const TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials
	) const;
	void StartBuildFromPreparedPrimNodes(
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		TArray<FPrimNodeBuildData>&& PrimNodes,
		bool bRefreshedFromSourceUsd,
		bool bUseExistingRequestId
	);
	void ResetPendingSourceRefreshState();
	void ScheduleParseResultPump();
	void ProcessParseResultPump();
	void HandleParseResult(FSourceParseResult&& ParseResult);
	static FGlobalParseSchedulerState& GetGlobalParseSchedulerState();
	static void ConfigureGlobalParseScheduler(int32 InMaxConcurrentTasks);
	static void EnqueueGlobalParseTask(FSourceParseTask&& Task);
	static void TryDispatchGlobalParseWorkers();
	static void ExecuteGlobalParseTask(FSourceParseTask&& Task);
	static bool DequeueGlobalParseResult(FSourceParseResult& OutResult);
	static bool HasGlobalCompletedParseResults();
	static bool HasGlobalParseWork();
	void EnsureLegacyPartsMigrated(UCarGeneratedAssemblyDataAsset* BuildAsset, UCarUsdVariantDataAsset* VariantAsset);
	void BuildPreviousPartIndexCache();
	void CollectPrimNodeDataFromGeneratedAsset(const UCarGeneratedAssemblyDataAsset* InBuildAsset, TArray<FPrimNodeBuildData>& OutData) const;
	void BuildPrimToImportedMeshMap(
		const TArray<TObjectPtr<UObject>>& ImportedAssets,
		TMap<FString, TObjectPtr<UStaticMesh>>& OutPrimToMesh
	) const;
	void BuildPrimToImportedMaterialMap(
		const TArray<TObjectPtr<UObject>>& ImportedAssets,
		TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& OutPrimToMaterials
	) const;
	bool CollectPrimNodeDataFromImportedStage(
		const UE::FUsdStage& InStage,
		const TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
		const TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials,
		TArray<FPrimNodeBuildData>& OutData
	) const;
	void ApplyBuildResults(
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		const TSet<FString>& UnchangedPrimPaths,
		const TArray<FString>& StalePrimPaths,
		TMap<FString, AActor*>&& AttachedProxyActorsSnapshot
	);
	TMap<FString, AActor*> GatherAttachedProxyActors() const;
	TMap<FString, AActor*> GatherAttachedProxyActors(TArray<AActor*>& OutDuplicateProxyActors) const;
	AActor* ResolveProxyActorByObjectPath(const FString& ObjectPath) const;
	AActor* SpawnProxyActor(const FPrimNodeBuildData& PrimData);
	TSubclassOf<AActor> ResolveMappedActorClass(const FString& PrimTypeName) const;
	void EnsureDefaultTypeNameActorClassMap();
	UsdCarFactoryPipelineBuild::FProxyActorSnapshot BuildTargetSnapshotForDiff(const FPrimNodeBuildData& PrimData) const;
	UsdCarFactoryPipelineBuild::FProxyActorSnapshot BuildExistingSnapshotForDiff(const FString& PrimPath, AActor* Actor) const;
	void BeginAsyncDiffPreparation(
		UCarGeneratedAssemblyDataAsset* BuildAsset,
		TMap<FString, AActor*>&& AttachedProxyActors,
		bool bRefreshedFromSourceUsd
	);
	void ProcessDiffPreparationBatch();
	void LaunchAsyncDiffComputation();
	void ResetPendingDiffState();
	void ProcessApplyBuildBatch();
	void ResetPendingApplyState();
	void ApplyVariantToProxyActor(const FCarUsdProxyActorState& VariantState, AActor* ProxyActor) const;
	const FCarUsdVariantRecord* FindVariantRecord(
		const UCarUsdVariantDataAsset* VariantAsset,
		const FName& VariantName
	) const;
	FCarUsdVariantRecord* FindMutableVariantRecord(
		UCarUsdVariantDataAsset* VariantAsset,
		const FName& VariantName
	) const;
	void BindVariantDataAssetEvents();
	void UnbindVariantDataAssetEvents();
	void HandleVariantDataAssetActiveVariantChanged(UCarUsdVariantDataAsset* ChangedAsset);
	void ApplyActiveVariant();

	void ApplyMaterials(
		const FPrimNodeBuildData& PrimData,
		UStaticMeshComponent* TargetComponent,
		const FCarGeneratedPartRecord* PreviousRecord
	) const;

	const FCarGeneratedPartRecord* FindPreviousRecord(const FString& PrimPath) const;

	static FString GetPrimLeafName(const FString& PrimPath);
	static FString MakeProxyActorLabel(const FString& PrimPath);
	static int32 GetPrimDepth(const FString& PrimPath);
	FName GetProxyActorTag() const;
	FName MakePrimTag(const FString& PrimPath) const;
};
