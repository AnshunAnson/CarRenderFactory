#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPath.h"
#include "UsdWrappers/ForwardDeclarations.h"

#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "UsdCarFactoryBuildSubsystem.h"
#include "UsdHierarchicalBuildActor.generated.h"

class UMaterialInterface;
class AStaticMeshActor;
class UStaticMesh;
class UStaticMeshComponent;
class UObject;
struct FPropertyChangedEvent;
class UUsdCarFactoryApplySubsystem;
class UUsdCarFactoryCoordinatorSubsystem;
class UUsdCarFactoryDataAssetSubsystem;
class UUsdCarFactoryDiffSubsystem;
class UUsdCarFactorySourceRefreshSubsystem;

namespace UE
{
	class FUsdPrim;
}

UCLASS(meta = (PrioritizeCategories = "USD Build"))
class USDCARFACTORYPIPELINE_API AUsdHierarchicalBuildActor : public AActor
{
	GENERATED_BODY()

public:
	AUsdHierarchicalBuildActor();

#if WITH_EDITORONLY_DATA
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
		Category = "USD Build|Actor Class",
		meta = (
			DisplayName = "静态网格代理Actor类",
			ToolTip = "仅用于 Mesh Prim 的代理生成，必须是 AStaticMeshActor 子类。默认使用 AStaticMeshActor。"
		)
	)
	TSubclassOf<AStaticMeshActor> StaticMeshProxyActorClass;

	UPROPERTY(
		EditAnywhere,
		BlueprintReadOnly,
		Category = "USD Build|Actor Class",
		meta = (
			DisplayName = "变换节点代理Actor类",
			ToolTip = "仅用于 Xform Prim 的层级节点代理。默认使用 AActor。"
		)
	)
	TSubclassOf<AActor> TransformProxyActorClass = AActor::StaticClass();

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
#endif

	UFUNCTION(
		BlueprintCallable,
		CallInEditor,
		Category = "USD Build",
		meta = (
			DisplayName = "加载并构建USD",
			ToolTip = "优先检查现有USDdatacache；若检测到源USD变化则先更新缓存，再构建/同步代理Actor层级。"
		)
	)
	void LoadAndBuild();

	virtual void PostLoad() override;
	virtual void BeginDestroy() override;
	virtual void PostRegisterAllComponents() override;
	virtual void PostUnregisterAllComponents() override;
	virtual bool IsEditorOnly() const override
	{
		return true;
	}

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	friend class UUsdCarFactoryBuildSubsystem;
	friend class UUsdCarFactoryApplySubsystem;
	friend class UUsdCarFactoryCoordinatorSubsystem;
	friend class UUsdCarFactoryDataAssetSubsystem;
	friend class UUsdCarFactoryDiffSubsystem;
	friend class UUsdCarFactorySourceRefreshSubsystem;

	bool bIsBuildInProgress = false;
	int32 ActiveBuildRequestId = 0;

	using FPrimNodeBuildData = UsdCarFactoryPipelineBuild::FUsdProxyBuildPrimNode;
	using FCachePrimSnapshot = UsdCarFactoryPipelineBuild::FCachePrimSnapshot;
	using FSourcePrimSnapshot = UsdCarFactoryPipelineBuild::FSourcePrimSnapshot;
	using FSourceDeltaScanResult = UsdCarFactoryPipelineBuild::FSourceDeltaScanResult;
	using FSourceParseTask = UsdCarFactoryPipelineBuild::FSourceParseTask;
	using FSourceParseResult = UsdCarFactoryPipelineBuild::FSourceParseResult;

private:
	TArray<FPrimNodeBuildData> PendingBuildPrimNodes;
	TMap<FString, int32> PreviousPartIndexByPrimPath;
	double ActiveBuildStartSeconds = 0.0;
	double ActiveSourceRefreshSeconds = 0.0;
	double ActiveDeltaScanSeconds = 0.0;
	double ActiveSelectiveImportSeconds = 0.0;
	double ActiveDiffSeconds = 0.0;
	double ActiveApplySeconds = 0.0;
	UPROPERTY()
	TMap<FString, TSubclassOf<AActor>> TypeNameToActorClassMap;

	UsdCarFactoryPipelineBuild::FUsdProxyBuildRequest MakeBuildRequest() const;
	UUsdCarFactoryBuildSubsystem* GetBuildSubsystem() const;
	void ExecuteBuildPipeline();
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
	void RetryMissingSelectiveImports(
		TArray<FPrimNodeBuildData>& PrimNodes,
		const FString& SourceUsdPath,
		const TArray<FString>& RequestedPrimPaths,
		TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
		TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials,
		UsdCarFactoryPipelineBuild::FUsdProxyBuildPlan& BuildPlan
	) const;
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
	TSubclassOf<AActor> ResolveProxyActorClass(const FPrimNodeBuildData& PrimData) const;
	void EnsureDefaultProxyActorClasses();
	void MigrateLegacyTypeNameActorClassMap();
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
