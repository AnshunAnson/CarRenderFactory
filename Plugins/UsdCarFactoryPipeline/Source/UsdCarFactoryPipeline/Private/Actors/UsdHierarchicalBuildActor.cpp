#include "Actors/UsdHierarchicalBuildActor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Async/Async.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Editor.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformTime.h"
#include "Materials/MaterialInterface.h"
#include "Misc/Crc.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/ScopeLock.h"
#include "Misc/SecureHash.h"
#include "TimerManager.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "USDAssetCache3.h"
#include "USDAssetUserData.h"
#include "USDStageImportContext.h"
#include "USDStageImporter.h"
#include "USDStageImportOptions.h"
#include "USDTypesConversion.h"
#include "USDGeomMeshConversion.h"
#include "UnrealUSDWrapper.h"
#include "USDObjectUtils.h"
#include "UsdCarFactoryCoordinatorSubsystem.h"
#include "UsdCarFactoryDataAssetSubsystem.h"

#if USE_USD_SDK
#include "USDIncludesStart.h"
#include "pxr/usd/usdGeom/mesh.h"
#include "pxr/usd/usdShade/materialBindingAPI.h"
#include "pxr/usd/usdGeom/xformCache.h"
#include "pxr/usd/usdGeom/xformable.h"
#include "USDIncludesEnd.h"
#include "UsdWrappers/SdfPath.h"
#include "UsdWrappers/UsdPrim.h"
#include "UsdWrappers/UsdStage.h"
#endif

DEFINE_LOG_CATEGORY_STATIC(LogUsdCarFactoryPipelineBuild, Log, All);

namespace UsdCarFactoryPipelineBuild
{
	static const TCHAR* GeneratedAssetFolder = TEXT("/Game/CarRenderFactory/Generated");
	static const TCHAR* GeneratedAssetBaseName = TEXT("DA_UsdGeneratedAssembly");
	static const TCHAR* AssetCacheBaseName = TEXT("DA_UsdAssetCache");
	static const TCHAR* PrimTagPrefix = TEXT("UsdCarFactoryPipeline.Prim.");
	static const TCHAR* ExactPrimTagPrefix = TEXT("UsdCarFactoryPipeline.PrimPath.");
	static constexpr int32 SelectiveImportBatchSize = 32;
	static constexpr int32 SelectiveImportMaxPrimPaths = 512;
#if USE_USD_SDK
	// Unreal USD resolver access has proven unstable under concurrent background stage opens.
	// Keep delta-scan stage open/traversal serialized while still running off the game thread.
	static FCriticalSection SourceDeltaScanMutex;
#endif

	static USceneComponent* EnsureSceneRoot(AActor* Actor)
	{
		if (!Actor)
		{
			return nullptr;
		}

		if (USceneComponent* ExistingRoot = Actor->GetRootComponent())
		{
			return ExistingRoot;
		}

		USceneComponent* NewRoot = NewObject<USceneComponent>(Actor, TEXT("GeneratedSceneRoot"));
		if (!NewRoot)
		{
			return nullptr;
		}

		NewRoot->SetMobility(EComponentMobility::Movable);
		Actor->AddInstanceComponent(NewRoot);
		Actor->SetRootComponent(NewRoot);
		NewRoot->RegisterComponent();
		return NewRoot;
	}

	static FString GetObjectPath(const UObject* Object)
	{
		return IsValid(Object) ? Object->GetPathName() : FString();
	}

	static void RewritePublishedAssetsIntoCache(UUsdAssetCache3* AssetCache, const TArray<UObject*>& ImportedAssets)
	{
		if (!AssetCache || AssetCache->GetOutermost() == GetTransientPackage())
		{
			return;
		}

		const FString TransientPackagePath = GetTransientPackage()->GetPathName();
		bool bCacheMutated = false;

		for (UObject* ImportedAsset : ImportedAssets)
		{
			if (!IsValid(ImportedAsset) || ImportedAsset->GetOutermost() == GetTransientPackage())
			{
				continue;
			}

			UUsdAssetUserData* AssetUserData = UsdUnreal::ObjectUtils::GetAssetUserData<UUsdAssetUserData>(ImportedAsset);
			if (!AssetUserData || AssetUserData->OriginalHash.IsEmpty())
			{
				continue;
			}

			const FString& Hash = AssetUserData->OriginalHash;
			const FSoftObjectPath ExistingPath = AssetCache->GetCachedAssetPath(Hash);
			if (ExistingPath.IsValid() && ExistingPath.ToString().StartsWith(TransientPackagePath))
			{
				AssetCache->StopTrackingAsset(Hash);
			}

			AssetCache->CacheAsset(Hash, FSoftObjectPath(ImportedAsset));
			bCacheMutated = true;
		}

		if (bCacheMutated)
		{
			AssetCache->MarkPackageDirty();
			if (UPackage* Package = AssetCache->GetOutermost())
			{
				Package->MarkPackageDirty();
			}
		}
	}

	static FString MakeSafePackageSegment(const FString& InName)
	{
		FString SafeName;
		SafeName.Reserve(InName.Len());
		for (const TCHAR Char : InName)
		{
			if (FChar::IsAlnum(Char) || Char == TEXT('_'))
			{
				SafeName.AppendChar(Char);
			}
			else
			{
				SafeName.AppendChar(TEXT('_'));
			}
		}

		while (SafeName.Contains(TEXT("__")))
		{
			SafeName.ReplaceInline(TEXT("__"), TEXT("_"));
		}
		SafeName.TrimCharInline(TEXT('_'), nullptr);
		if (SafeName.IsEmpty())
		{
			SafeName = TEXT("UnknownUsd");
		}
		return SafeName;
	}

	// 从 USD 文件路径提取安全且稳定的标识符（用于目录名/前缀）
	static FString ExtractSourceIdentifier(const FString& UsdFilePath)
	{
		if (UsdFilePath.IsEmpty())
		{
			return TEXT("UnknownUsd_00000000");
		}

		const FString NormalizedPath = FPaths::ConvertRelativePathToFull(UsdFilePath);
		const FString BaseName =
			NormalizedPath.IsEmpty() ? TEXT("UnknownUsd") : FPaths::GetBaseFilename(NormalizedPath, false);
		FString SafeName = MakeSafePackageSegment(BaseName);
		if (SafeName.Len() > 48)
		{
			SafeName = SafeName.Left(48);
		}

		const uint32 PathHash = FCrc::StrCrc32(*NormalizedPath);
		return FString::Printf(TEXT("%s_%08X"), *SafeName, PathHash);
	}

	static bool IsTransformPrimType(const FString& PrimTypeName)
	{
		return PrimTypeName.Equals(TEXT("Xform"), ESearchCase::IgnoreCase);
	}

	static bool IsSupportedProxyPrimType(const FString& PrimTypeName, bool bIsStaticMesh)
	{
		return bIsStaticMesh || IsTransformPrimType(PrimTypeName);
	}

	template <typename MethodType>
	static void ScheduleActorContinuation(const TWeakObjectPtr<AUsdHierarchicalBuildActor>& WeakActor, MethodType Method)
	{
		AsyncTask(
			ENamedThreads::GameThread,
			[WeakActor, Method]()
			{
				if (AUsdHierarchicalBuildActor* StrongActor = WeakActor.Get())
				{
					(StrongActor->*Method)();
				}
			}
		);
	}

	static FString ExtractExactPrimPathFromActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return FString();
		}

		for (const FName& Tag : Actor->Tags)
		{
			const FString TagValue = Tag.ToString();
			if (TagValue.StartsWith(ExactPrimTagPrefix))
			{
				return TagValue.RightChop(FCString::Strlen(ExactPrimTagPrefix));
			}
		}

		return FString();
	}

	static void RemoveTagsByPrefix(AActor* Actor, const TCHAR* Prefix)
	{
		if (!Actor)
		{
			return;
		}

		for (int32 TagIndex = Actor->Tags.Num() - 1; TagIndex >= 0; --TagIndex)
		{
			if (Actor->Tags[TagIndex].ToString().StartsWith(Prefix))
			{
				Actor->Tags.RemoveAt(TagIndex);
			}
		}
	}

	static void ApplyProxyTags(AActor* Actor, const FName& ProxyTag, const FName& PrimTag, const FString& PrimPath)
	{
		if (!Actor)
		{
			return;
		}

		RemoveTagsByPrefix(Actor, PrimTagPrefix);
		RemoveTagsByPrefix(Actor, ExactPrimTagPrefix);

		Actor->Tags.AddUnique(ProxyTag);
		Actor->Tags.AddUnique(PrimTag);
		Actor->Tags.AddUnique(FName(*(FString(ExactPrimTagPrefix) + PrimPath)));
	}

	static bool AreTransformsEquivalent(const FTransform& A, const FTransform& B)
	{
		return A.GetLocation().Equals(B.GetLocation(), 0.01f)
			&& A.GetRotation().Equals(B.GetRotation(), KINDA_SMALL_NUMBER)
			&& A.GetScale3D().Equals(B.GetScale3D(), 0.001f);
	}

	static bool AreSnapshotsEquivalent(const FProxyActorSnapshot& A, const FProxyActorSnapshot& B)
	{
		return A.PrimPath == B.PrimPath
			&& A.ParentPrimPath == B.ParentPrimPath
			&& A.bIsStaticMesh == B.bIsStaticMesh
			&& A.StaticMeshPath == B.StaticMeshPath
			&& A.MaterialPaths == B.MaterialPaths
			&& AreTransformsEquivalent(A.RelativeTransform, B.RelativeTransform);
	}

	static FBuildDiffResult ComputeBuildDiff(
		const TArray<FProxyActorSnapshot>& TargetSnapshots,
		const TMap<FString, FProxyActorSnapshot>& ExistingSnapshots
	)
	{
		FBuildDiffResult Result;
		TSet<FString> TargetPrimPaths;

		for (const FProxyActorSnapshot& TargetSnapshot : TargetSnapshots)
		{
			TargetPrimPaths.Add(TargetSnapshot.PrimPath);
			if (const FProxyActorSnapshot* ExistingSnapshot = ExistingSnapshots.Find(TargetSnapshot.PrimPath))
			{
				if (AreSnapshotsEquivalent(TargetSnapshot, *ExistingSnapshot))
				{
					Result.UnchangedPrimPaths.Add(TargetSnapshot.PrimPath);
				}
			}
		}

		for (const TPair<FString, FProxyActorSnapshot>& ExistingPair : ExistingSnapshots)
		{
			if (!TargetPrimPaths.Contains(ExistingPair.Key))
			{
				Result.StalePrimPaths.Add(ExistingPair.Key);
			}
		}

		return Result;
	}
}

AUsdHierarchicalBuildActor::AUsdHierarchicalBuildActor()
{
	PrimaryActorTick.bCanEverTick = false;
	EnsureDefaultProxyActorClasses();
}

void AUsdHierarchicalBuildActor::PostLoad()
{
	Super::PostLoad();
	EnsureDefaultProxyActorClasses();
	MigrateLegacyTypeNameActorClassMap();
}

void AUsdHierarchicalBuildActor::BeginDestroy()
{
	if (UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem())
	{
		BuildSubsystem->RemoveJobState(*this);
	}

	Super::BeginDestroy();
}

void AUsdHierarchicalBuildActor::PostRegisterAllComponents()
{
	// Pure proxy-build role: do not auto-load any USD stage.
	AActor::PostRegisterAllComponents();
}

void AUsdHierarchicalBuildActor::PostUnregisterAllComponents()
{
	AActor::PostUnregisterAllComponents();
}

#if WITH_EDITOR
void AUsdHierarchicalBuildActor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName =
		PropertyChangedEvent.GetPropertyName() != NAME_None
			? PropertyChangedEvent.GetPropertyName()
			: PropertyChangedEvent.GetMemberPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(AUsdHierarchicalBuildActor, GeneratedDataAsset))
	{
		return;
	}

	if (
		PropertyName == GET_MEMBER_NAME_CHECKED(AUsdHierarchicalBuildActor, ParseResultFrameBudgetMs)
		|| PropertyName == GET_MEMBER_NAME_CHECKED(AUsdHierarchicalBuildActor, MaxParseResultsPerTick)
	)
	{
		ParseResultFrameBudgetMs = FMath::Clamp(ParseResultFrameBudgetMs, 2.0f, 5.0f);
		MaxParseResultsPerTick = FMath::Max(1, MaxParseResultsPerTick);
	}
}
#endif

UsdCarFactoryPipelineBuild::FUsdProxyBuildRequest AUsdHierarchicalBuildActor::MakeBuildRequest() const
{
	UsdCarFactoryPipelineBuild::FUsdProxyBuildRequest Request;
	Request.TargetActor = const_cast<AUsdHierarchicalBuildActor*>(this);
	Request.SourceUsdPath = SourceUsdFile.FilePath;
	Request.GeneratedDataAsset = GeneratedDataAsset;
	Request.StaticMeshProxyActorClass = StaticMeshProxyActorClass;
	Request.TransformProxyActorClass = TransformProxyActorClass;
	Request.bBuildAsync = bBuildAsync;
	Request.MaxPrimBuildPerTick = MaxPrimBuildPerTick;
	Request.ParseResultFrameBudgetMs = ParseResultFrameBudgetMs;
	Request.MaxParseResultsPerTick = MaxParseResultsPerTick;
	return Request;
}

UUsdCarFactoryBuildSubsystem* AUsdHierarchicalBuildActor::GetBuildSubsystem() const
{
	return UUsdCarFactoryBuildSubsystem::Get();
}

void AUsdHierarchicalBuildActor::LoadAndBuild()
{
#if WITH_EDITOR
	FUsdCarFactoryBuildInputs Inputs;
	Inputs.TargetActor = this;
	Inputs.SourceUsdFile = SourceUsdFile;
	Inputs.GeneratedDataAsset = GeneratedDataAsset;
	Inputs.StaticMeshProxyActorClass = StaticMeshProxyActorClass;
	Inputs.TransformProxyActorClass = TransformProxyActorClass;
	Inputs.bBuildAsync = bBuildAsync;
	Inputs.MaxPrimBuildPerTick = MaxPrimBuildPerTick;
	Inputs.ParseResultFrameBudgetMs = ParseResultFrameBudgetMs;
	Inputs.MaxParseResultsPerTick = MaxParseResultsPerTick;

	if (GEditor)
	{
		if (UUsdCarFactoryCoordinatorSubsystem* CoordinatorSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryCoordinatorSubsystem>())
		{
			CoordinatorSubsystem->RunBuild(Inputs);
			return;
		}
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("Failed to resolve UUsdCarFactoryCoordinatorSubsystem."));
#else
	UE_LOG(LogUsdCarFactoryPipelineBuild, Warning, TEXT("LoadAndBuild can only run in editor."));
#endif
}

void AUsdHierarchicalBuildActor::ExecuteBuildPipeline()
{
#if WITH_EDITOR
	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("LoadAndBuild called. SourceUsdFile: '%s'"), *SourceUsdFile.FilePath);
	ActiveBuildStartSeconds = FPlatformTime::Seconds();
	ActiveSourceRefreshSeconds = 0.0;
	ActiveDeltaScanSeconds = 0.0;
	ActiveSelectiveImportSeconds = 0.0;
	ActiveDiffSeconds = 0.0;
	ActiveApplySeconds = 0.0;

	if (bIsBuildInProgress)
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("Previous build request is still running. Latest request will replace it.")
		);
	}

	UCarGeneratedAssemblyDataAsset* BuildAsset = nullptr;
	UUsdAssetCache3* UsdAssetCache = nullptr;
	FString ResolveMessage;
	if (GEditor)
	{
		if (UUsdCarFactoryDataAssetSubsystem* DataAssetSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryDataAssetSubsystem>())
		{
			FUsdCarFactoryBuildInputs Inputs;
			Inputs.TargetActor = this;
			Inputs.SourceUsdFile = SourceUsdFile;
			Inputs.GeneratedDataAsset = GeneratedDataAsset;
			DataAssetSubsystem->ResolveOrCreateBuildAssets(Inputs, BuildAsset, UsdAssetCache, ResolveMessage);
		}
	}
	if (!BuildAsset)
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("Failed to resolve or create GeneratedDataAsset. %s"), *ResolveMessage);
		return;
	}

	ResetPendingApplyState();
	ResetPendingDiffState();
	ResetPendingSourceRefreshState();
	BuildPreviousPartIndexCache();

	if (UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem())
	{
		UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
		JobState.ResetForNextBuild();
		JobState.ActiveRequest = MakeBuildRequest();
		JobState.ActiveBuildStartSeconds = ActiveBuildStartSeconds;
		JobState.Phase = UsdCarFactoryPipelineBuild::EUsdProxyBuildJobPhase::Scanning;
	}

	FString NormalizedSourceUsdPath;
	FDateTime SourceUsdTimestampUtc;
	int64 SourceUsdFileSizeBytes = -1;
	bool bShouldRefreshFromSourceUsd = false;

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("Checking SourceUsdFile. IsEmpty: %d"), SourceUsdFile.FilePath.IsEmpty());

	if (!SourceUsdFile.FilePath.IsEmpty())
	{
		NormalizedSourceUsdPath = FPaths::ConvertRelativePathToFull(SourceUsdFile.FilePath);
		UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("Normalized path: '%s'"), *NormalizedSourceUsdPath);
		if (IFileManager::Get().FileExists(*NormalizedSourceUsdPath))
		{
			SourceUsdTimestampUtc = IFileManager::Get().GetTimeStamp(*NormalizedSourceUsdPath);
			SourceUsdFileSizeBytes = IFileManager::Get().FileSize(*NormalizedSourceUsdPath);
			bShouldRefreshFromSourceUsd =
				ShouldRefreshCacheFromSourceUsd(BuildAsset, NormalizedSourceUsdPath, SourceUsdTimestampUtc, SourceUsdFileSizeBytes);

			if (bShouldRefreshFromSourceUsd)
			{
				++ActiveBuildRequestId;
				bIsBuildInProgress = true;

				TMap<FString, FCachePrimSnapshot> CacheSnapshots;
				BuildCachePrimSnapshots(BuildAsset, CacheSnapshots);
				SourceUsdFile.FilePath = NormalizedSourceUsdPath;

				if (bBuildAsync)
				{
					UE_LOG(
						LogUsdCarFactoryPipelineBuild,
						Warning,
						TEXT("USD source delta scan is forced to run synchronously because UnrealUSD stage access is unstable on background threads.")
					);
				}

				const double DeltaScanStartSeconds = FPlatformTime::Seconds();
				FSourceDeltaScanResult DeltaResult;
				const bool bDeltaScanSucceeded = RunSourceDeltaScanSync(
					NormalizedSourceUsdPath,
					CacheSnapshots,
					DeltaResult
				);
				FSourceParseResult ParseResult = MakeParseResult(
					this,
					ActiveBuildRequestId,
					NormalizedSourceUsdPath,
					SourceUsdTimestampUtc,
					SourceUsdFileSizeBytes,
					MoveTemp(DeltaResult),
					bDeltaScanSucceeded,
					FPlatformTime::Seconds() - DeltaScanStartSeconds,
					TEXT("Synchronous source delta scan failed.")
				);
				HandleParseResult(MoveTemp(ParseResult));
				return;
			}
		}
		else
		{
			UE_LOG(
				LogUsdCarFactoryPipelineBuild,
				Warning,
				TEXT("SourceUsdFile does not exist: '%s'. LoadAndBuild will use existing USDdatacache."),
				*NormalizedSourceUsdPath
			);
		}

		SourceUsdFile.FilePath = NormalizedSourceUsdPath;
	}
	else
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("SourceUsdFile is empty. Building from existing GeneratedDataAsset."));
	}

	TArray<FPrimNodeBuildData> PrimNodes;
	CollectPrimNodeDataFromGeneratedAsset(BuildAsset, PrimNodes);
	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("Collected %d PrimNodes from GeneratedDataAsset. Starting build..."), PrimNodes.Num());
	StartBuildFromPreparedPrimNodes(BuildAsset, MoveTemp(PrimNodes), false, false);
#else
	UE_LOG(LogUsdCarFactoryPipelineBuild, Warning, TEXT("ExecuteBuildPipeline can only run in editor."));
#endif
}

bool AUsdHierarchicalBuildActor::IsMeshPrimType(const FString& PrimTypeName)
{
	return PrimTypeName.Equals(TEXT("Mesh"), ESearchCase::IgnoreCase);
}

FString AUsdHierarchicalBuildActor::ComputeMeshContentHash(const UE::FUsdStage& InStage, const UE::FUsdPrim& Prim)
{
#if USE_USD_SDK
	if (!InStage || !Prim)
	{
		return FString();
	}

	const FString PrimPath = Prim.GetPrimPath().GetString();
	if (PrimPath.IsEmpty())
	{
		return FString();
	}

	const FString GeomHash = UsdUtils::HashGeomMeshPrim(InStage, PrimPath, 0.0);

	FString BindingDigest;
	BindingDigest.Reserve(256);

	const pxr::UsdPrim UsdPrim = static_cast<const pxr::UsdPrim&>(Prim);
	if (UsdPrim)
	{
		pxr::SdfPathVector Targets;
		pxr::UsdRelationship DirectBinding = pxr::UsdShadeMaterialBindingAPI(UsdPrim).GetDirectBindingRel();
		if (DirectBinding)
		{
			DirectBinding.GetTargets(&Targets);
			for (const pxr::SdfPath& Target : Targets)
			{
				BindingDigest += UTF8_TO_TCHAR(Target.GetString().c_str());
				BindingDigest += TEXT("|");
			}
		}

		pxr::UsdGeomMesh UsdMesh(UsdPrim);
		if (UsdMesh)
		{
			pxr::TfToken SubdivisionScheme;
			if (pxr::UsdAttribute SubdivisionSchemeAttr = UsdMesh.GetSubdivisionSchemeAttr(); SubdivisionSchemeAttr)
			{
				SubdivisionSchemeAttr.Get(&SubdivisionScheme, 0.0);
				BindingDigest += UTF8_TO_TCHAR(SubdivisionScheme.GetString().c_str());
			}
		}
	}

	const FString Combined = FString::Printf(TEXT("%s|%s|%s"), *PrimPath, *GeomHash, *BindingDigest);
	FTCHARToUTF8 UTF8(*Combined);

	FSHA1 HashState;
	HashState.Update(reinterpret_cast<const uint8*>(UTF8.Get()), UTF8.Length());
	HashState.Final();

	uint8 Digest[FSHA1::DigestSize];
	HashState.GetHash(Digest);
	return BytesToHex(Digest, FSHA1::DigestSize);
#else
	return FString();
#endif
}

void AUsdHierarchicalBuildActor::BuildCachePrimSnapshots(
	const UCarGeneratedAssemblyDataAsset* BuildAsset,
	TMap<FString, FCachePrimSnapshot>& OutCacheSnapshots
) const
{
	OutCacheSnapshots.Reset();
	if (!BuildAsset)
	{
		return;
	}

	OutCacheSnapshots.Reserve(BuildAsset->Parts.Num());
	for (const FCarGeneratedPartRecord& Record : BuildAsset->Parts)
	{
		if (Record.PrimPath.IsEmpty())
		{
			continue;
		}

		FCachePrimSnapshot Snapshot;
		Snapshot.ParentPrimPath = Record.ParentPrimPath;
		Snapshot.RelativeTransform = Record.RelativeTransform;
		Snapshot.bIsStaticMesh = Record.bIsStaticMesh || (Record.StaticMesh != nullptr);
		Snapshot.MeshContentHash = Record.MeshContentHash;
		OutCacheSnapshots.Add(Record.PrimPath, MoveTemp(Snapshot));
	}
}

bool AUsdHierarchicalBuildActor::CollectSourcePrimSnapshots(
	const UE::FUsdStage& InStage,
	TArray<FSourcePrimSnapshot>& OutSnapshots
)
{
#if USE_USD_SDK
	OutSnapshots.Reset();
	if (!InStage)
	{
		return false;
	}

	const FUsdStageInfo StageInfo(InStage);
	pxr::UsdGeomXformCache XformCache;

	TFunction<void(const UE::FUsdPrim&, const FString&)> VisitPrim;
	VisitPrim = [&OutSnapshots, &VisitPrim, &StageInfo, &XformCache, &InStage](const UE::FUsdPrim& Prim, const FString& ParentPrimPath)
	{
		if (!Prim)
		{
			return;
		}

		FSourcePrimSnapshot Snapshot;
		Snapshot.PrimPath = Prim.GetPrimPath().GetString();
		Snapshot.ParentPrimPath = ParentPrimPath;
		Snapshot.PrimTypeName = Prim.GetTypeName().ToString();
		Snapshot.bIsStaticMesh = IsMeshPrimType(Snapshot.PrimTypeName);

		const pxr::UsdPrim UsdPrim = static_cast<const pxr::UsdPrim&>(Prim);
		if (UsdPrim)
		{
			const pxr::GfMatrix4d LocalToWorld = XformCache.GetLocalToWorldTransform(UsdPrim);
			const FTransform WorldTransform = UsdToUnreal::ConvertMatrix(StageInfo, LocalToWorld);

			if (ParentPrimPath.IsEmpty())
			{
				Snapshot.RelativeTransform = WorldTransform;
			}
			else
			{
				const UE::FUsdPrim ParentPrim = Prim.GetParent();
				const pxr::UsdPrim ParentUsdPrim = ParentPrim ? static_cast<const pxr::UsdPrim&>(ParentPrim) : pxr::UsdPrim();
				if (ParentUsdPrim)
				{
					const pxr::GfMatrix4d ParentLocalToWorld = XformCache.GetLocalToWorldTransform(ParentUsdPrim);
					const pxr::GfMatrix4d RelativeMatrix = LocalToWorld * ParentLocalToWorld.GetInverse();
					Snapshot.RelativeTransform = UsdToUnreal::ConvertMatrix(StageInfo, RelativeMatrix);
				}
				else
				{
					Snapshot.RelativeTransform = WorldTransform;
				}
			}
		}

		if (Snapshot.bIsStaticMesh)
		{
			Snapshot.MeshContentHash = ComputeMeshContentHash(InStage, Prim);
		}

		OutSnapshots.Add(MoveTemp(Snapshot));

		for (UE::FUsdPrim ChildPrim : Prim.GetChildren())
		{
			VisitPrim(ChildPrim, Prim.GetPrimPath().GetString());
		}
	};

	for (UE::FUsdPrim TopLevelPrim : InStage.GetPseudoRoot().GetChildren())
	{
		VisitPrim(TopLevelPrim, FString());
	}

	return true;
#else
	return false;
#endif
}

bool AUsdHierarchicalBuildActor::RunSourceDeltaScanSync(
	const FString& SourceUsdPath,
	const TMap<FString, FCachePrimSnapshot>& CacheSnapshots,
	FSourceDeltaScanResult& OutDeltaResult
)
{
	OutDeltaResult = FSourceDeltaScanResult{};

#if USE_USD_SDK
	FScopeLock SourceDeltaScanLock(&UsdCarFactoryPipelineBuild::SourceDeltaScanMutex);

	UE::FUsdStage Stage = UnrealUSDWrapper::OpenStage(*SourceUsdPath, EUsdInitialLoadSet::LoadAll);
	if (!Stage)
	{
		OutDeltaResult.ErrorMessage = TEXT("Failed to open source USD stage.");
		return false;
	}

	if (!CollectSourcePrimSnapshots(Stage, OutDeltaResult.SourcePrimSnapshots))
	{
		OutDeltaResult.ErrorMessage = TEXT("Failed to collect source prim snapshots.");
		return false;
	}

	TSet<FString> SourcePrimPaths;
	SourcePrimPaths.Reserve(OutDeltaResult.SourcePrimSnapshots.Num());

	for (const FSourcePrimSnapshot& SourceSnapshot : OutDeltaResult.SourcePrimSnapshots)
	{
		SourcePrimPaths.Add(SourceSnapshot.PrimPath);

		const FCachePrimSnapshot* CachedSnapshot = CacheSnapshots.Find(SourceSnapshot.PrimPath);
		if (!CachedSnapshot)
		{
			OutDeltaResult.AddedPrimPaths.Add(SourceSnapshot.PrimPath);
			if (SourceSnapshot.bIsStaticMesh)
			{
				OutDeltaResult.ChangedMeshPrimPaths.Add(SourceSnapshot.PrimPath);
			}
			continue;
		}

		const bool bHierarchyChanged = CachedSnapshot->ParentPrimPath != SourceSnapshot.ParentPrimPath;
		const bool bTransformChanged =
			!UsdCarFactoryPipelineBuild::AreTransformsEquivalent(CachedSnapshot->RelativeTransform, SourceSnapshot.RelativeTransform);
		if (bHierarchyChanged || bTransformChanged || (CachedSnapshot->bIsStaticMesh != SourceSnapshot.bIsStaticMesh))
		{
			OutDeltaResult.HierarchyOrTransformChangedPrimPaths.Add(SourceSnapshot.PrimPath);
		}

		if (SourceSnapshot.bIsStaticMesh)
		{
			if (!CachedSnapshot->bIsStaticMesh || CachedSnapshot->MeshContentHash != SourceSnapshot.MeshContentHash)
			{
				OutDeltaResult.ChangedMeshPrimPaths.Add(SourceSnapshot.PrimPath);
			}
		}
	}

	for (const TPair<FString, FCachePrimSnapshot>& CachedPair : CacheSnapshots)
	{
		if (!SourcePrimPaths.Contains(CachedPair.Key))
		{
			OutDeltaResult.RemovedPrimPaths.Add(CachedPair.Key);
		}
	}

	OutDeltaResult.bSuccess = true;
	return true;
#else
	OutDeltaResult.ErrorMessage = TEXT("USE_USD_SDK is disabled.");
	return false;
#endif
}

AUsdHierarchicalBuildActor::FSourceParseResult AUsdHierarchicalBuildActor::MakeParseResult(
	const TWeakObjectPtr<AUsdHierarchicalBuildActor>& TargetActor,
	int32 RequestId,
	const FString& SourceUsdPath,
	const FDateTime& SourceUsdTimestampUtc,
	int64 SourceUsdFileSizeBytes,
	FSourceDeltaScanResult&& DeltaResult,
	bool bDeltaScanCallSucceeded,
	double DeltaScanSeconds,
	const TCHAR* DefaultFailureMessage
)
{
	FSourceParseResult ParseResult;
	ParseResult.TargetActor = TargetActor;
	ParseResult.RequestId = RequestId;
	ParseResult.SourceUsdPath = SourceUsdPath;
	ParseResult.SourceUsdTimestampUtc = SourceUsdTimestampUtc;
	ParseResult.SourceUsdFileSizeBytes = SourceUsdFileSizeBytes;
	ParseResult.DeltaScanSeconds = DeltaScanSeconds;
	ParseResult.bDeltaScanSucceeded = bDeltaScanCallSucceeded && DeltaResult.bSuccess;
	ParseResult.ErrorMessage = MoveTemp(DeltaResult.ErrorMessage);

	if (ParseResult.bDeltaScanSucceeded)
	{
		ParseResult.SourcePrimSnapshots = MoveTemp(DeltaResult.SourcePrimSnapshots);
		ParseResult.AddedPrimPaths = MoveTemp(DeltaResult.AddedPrimPaths);
		ParseResult.RemovedPrimPaths = MoveTemp(DeltaResult.RemovedPrimPaths);
		ParseResult.HierarchyOrTransformChangedPrimPaths = MoveTemp(DeltaResult.HierarchyOrTransformChangedPrimPaths);
		ParseResult.ChangedMeshPrimPaths = MoveTemp(DeltaResult.ChangedMeshPrimPaths);
	}
	else if (ParseResult.ErrorMessage.IsEmpty() && !bDeltaScanCallSucceeded)
	{
		ParseResult.ErrorMessage = DefaultFailureMessage;
	}

	return ParseResult;
}

void AUsdHierarchicalBuildActor::UpdateSourceRefreshMetadata(
	UCarGeneratedAssemblyDataAsset* BuildAsset,
	const FString& SourceUsdPath,
	const FDateTime& SourceUsdTimestampUtc,
	int64 SourceUsdFileSizeBytes
)
{
	if (!BuildAsset)
	{
		return;
	}

	BuildAsset->Modify();
	BuildAsset->SourceUsdFile.FilePath = SourceUsdPath;
	BuildAsset->SourceUsdTimestampUtc = SourceUsdTimestampUtc;
	BuildAsset->SourceUsdFileSizeBytes = SourceUsdFileSizeBytes;
}

void AUsdHierarchicalBuildActor::ConvertSourceSnapshotsToPrimNodes(
	const TArray<FSourcePrimSnapshot>& SourceSnapshots,
	TArray<FPrimNodeBuildData>& OutPrimNodes
) const
{
	OutPrimNodes.Reset();
	OutPrimNodes.Reserve(SourceSnapshots.Num());

	for (const FSourcePrimSnapshot& SourceSnapshot : SourceSnapshots)
	{
		FPrimNodeBuildData& PrimData = OutPrimNodes.AddDefaulted_GetRef();
		PrimData.PrimPath = SourceSnapshot.PrimPath;
		PrimData.ParentPrimPath = SourceSnapshot.ParentPrimPath;
		PrimData.PrimTypeName = SourceSnapshot.PrimTypeName;
		PrimData.RelativeTransform = SourceSnapshot.RelativeTransform;
		PrimData.WorldTransform = SourceSnapshot.RelativeTransform;
		PrimData.bIsStaticMesh = SourceSnapshot.bIsStaticMesh;
		PrimData.MeshContentHash = SourceSnapshot.MeshContentHash;
	}
}

bool AUsdHierarchicalBuildActor::ImportMeshAssetsForPrimPaths(
	const FString& SourceUsdPath,
	const TArray<FString>& PrimPaths,
	TMap<FString, TObjectPtr<UStaticMesh>>& OutPrimToMesh,
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& OutPrimToMaterials
) const
{
#if WITH_EDITOR
	if (SourceUsdPath.IsEmpty() || PrimPaths.IsEmpty())
	{
		return true;
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("ImportMeshAssetsForPrimPaths: Importing %d prim paths from '%s'"), PrimPaths.Num(), *SourceUsdPath);

	FUsdStageImportContext ImportContext;
	// This path imports directly from a USD file, not from an already-opened stage actor.
	// Reading from stage cache here has been producing incomplete imports (for example 2150 prims -> 1 mesh),
	// so force a fresh stage open for deterministic mesh extraction.
	ImportContext.bReadFromStageCache = false;

	const FString ImportObjectName = FPaths::GetBaseFilename(SourceUsdPath, false);
	const FString SourceIdentifier = UsdCarFactoryPipelineBuild::ExtractSourceIdentifier(SourceUsdPath);
	const FString OutputPath = FString::Printf(TEXT("%s/%s"),
		UsdCarFactoryPipelineBuild::GeneratedAssetFolder, *SourceIdentifier);
	// 按 USD 源文件名隔离输出路径
	if (!ImportContext.Init(
			ImportObjectName.IsEmpty() ? TEXT("UsdBuildSelectiveImport") : ImportObjectName,
			SourceUsdPath,
			*OutputPath,
			RF_Transient,
			true,
			false,
			false
		))
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("ImportMeshAssetsForPrimPaths: Failed to initialize import context for '%s'"), *SourceUsdPath);
		return false;
	}

	UUsdStageImportOptions* ImportOptions =
		ImportContext.ImportOptions ? ImportContext.ImportOptions.Get() : NewObject<UUsdStageImportOptions>();
	ImportContext.ImportOptions = ImportOptions;
	ImportOptions->bImportActors = false;
	ImportOptions->bImportGeometry = true;
	ImportOptions->bImportMaterials = true;
	ImportOptions->bImportOnlyUsedMaterials = true;
	ImportOptions->PrimsToImport = PrimPaths;
	ImportOptions->bUseExistingAssetCache = true;
	// This path needs one mesh asset per requested mesh prim. The USD importer defaults are tuned for full-stage
	// imports and may collapse component/subcomponent subtrees or share identical assets, which causes our leaf-prim
	// selective import path to resolve far fewer meshes than requested.
	ImportOptions->bShareAssetsForIdenticalPrims = false;
	ImportOptions->bUsePrimKindsForCollapsing = false;
	ImportOptions->KindsToCollapse = 0;
	ImportOptions->bMergeIdenticalMaterialSlots = false;
	ImportOptions->bInterpretLODs = false;

	UUsdAssetCache3* ActiveAssetCache = GeneratedDataAsset ? GeneratedDataAsset->UsdAssetCache.Get() : nullptr;
	if (ActiveAssetCache)
	{
		ImportContext.UsdAssetCache = ActiveAssetCache;
		ImportOptions->ExistingAssetCache = FSoftObjectPath(ActiveAssetCache);
	}

	UUsdStageImporter StageImporter;
	StageImporter.ImportFromFile(ImportContext);
	if (ImportContext.ImportedAssets.IsEmpty())
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Warning, TEXT("ImportMeshAssetsForPrimPaths: Import completed without any assets"));
	}
	else
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("ImportMeshAssetsForPrimPaths: Import completed with %d assets for %d requested prims"),
			ImportContext.ImportedAssets.Num(),
			PrimPaths.Num()
		);
	}

	if (ActiveAssetCache)
	{
		UsdCarFactoryPipelineBuild::RewritePublishedAssetsIntoCache(ActiveAssetCache, ImportContext.ImportedAssets);
	}

	TMap<FString, TObjectPtr<UStaticMesh>> BatchPrimToMesh;
	BuildPrimToImportedMeshMap(ImportContext.ImportedAssets, BatchPrimToMesh);
	for (TPair<FString, TObjectPtr<UStaticMesh>>& Pair : BatchPrimToMesh)
	{
		if (!Pair.Key.IsEmpty() && Pair.Value)
		{
			OutPrimToMesh.FindOrAdd(Pair.Key) = Pair.Value;
		}
	}

	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> BatchPrimToMaterials;
	BuildPrimToImportedMaterialMap(ImportContext.ImportedAssets, BatchPrimToMaterials);
	for (TPair<FString, TArray<TObjectPtr<UMaterialInterface>>>& Pair : BatchPrimToMaterials)
	{
		if (Pair.Key.IsEmpty())
		{
			continue;
		}

		TArray<TObjectPtr<UMaterialInterface>>& DestMaterials = OutPrimToMaterials.FindOrAdd(Pair.Key);
		for (UMaterialInterface* Material : Pair.Value)
		{
			DestMaterials.AddUnique(Material);
		}
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("ImportMeshAssetsForPrimPaths: Result - %d meshes, %d material sets"), OutPrimToMesh.Num(), OutPrimToMaterials.Num());

	// Return true if we got any assets, even if stage is null
	return OutPrimToMesh.Num() > 0 || OutPrimToMaterials.Num() > 0;
#else
	return false;
#endif
}

void AUsdHierarchicalBuildActor::ApplyImportedAssetsToPrimNodes(
	TArray<FPrimNodeBuildData>& InOutPrimNodes,
	const TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
	const TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials
) const
{
	for (FPrimNodeBuildData& PrimData : InOutPrimNodes)
	{
		if (!PrimData.bIsStaticMesh)
		{
			continue;
		}

		if (const TObjectPtr<UStaticMesh>* ImportedMesh = PrimToMesh.Find(PrimData.PrimPath))
		{
			PrimData.StaticMesh = *ImportedMesh;
		}
		else if (const FCarGeneratedPartRecord* PreviousRecord = FindPreviousRecord(PrimData.PrimPath))
		{
			PrimData.StaticMesh = PreviousRecord->StaticMesh;
		}

		PrimData.SourceMaterials.Reset();
		PrimData.SourceMaterialSlotNames.Reset();

		if (UStaticMesh* StaticMesh = PrimData.StaticMesh)
		{
			const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->GetStaticMaterials();
			PrimData.SourceMaterials.Reserve(StaticMaterials.Num());
			PrimData.SourceMaterialSlotNames.Reserve(StaticMaterials.Num());
			for (const FStaticMaterial& StaticMaterial : StaticMaterials)
			{
				PrimData.SourceMaterialSlotNames.Add(StaticMaterial.MaterialSlotName);
				PrimData.SourceMaterials.Add(StaticMaterial.MaterialInterface);
			}

			if (const TArray<TObjectPtr<UMaterialInterface>>* ImportedMaterials = PrimToMaterials.Find(PrimData.PrimPath))
			{
				int32 ImportedMaterialIndex = 0;
				for (int32 SlotIndex = 0; SlotIndex < PrimData.SourceMaterials.Num() && ImportedMaterialIndex < ImportedMaterials->Num(); ++SlotIndex)
				{
					if (!PrimData.SourceMaterials[SlotIndex] && ImportedMaterials->IsValidIndex(ImportedMaterialIndex))
					{
						PrimData.SourceMaterials[SlotIndex] = (*ImportedMaterials)[ImportedMaterialIndex++];
					}
				}
			}

			const FCarGeneratedPartRecord* PreviousRecord = FindPreviousRecord(PrimData.PrimPath);
			if (PreviousRecord)
			{
				for (int32 SlotIndex = 0; SlotIndex < PrimData.SourceMaterials.Num(); ++SlotIndex)
				{
					if (!PrimData.SourceMaterials[SlotIndex] && PrimData.SourceMaterialSlotNames.IsValidIndex(SlotIndex))
					{
						const FName& SlotName = PrimData.SourceMaterialSlotNames[SlotIndex];
						if (const TObjectPtr<UMaterialInterface>* OverrideMat = PreviousRecord->MaterialOverrides.Find(SlotName))
						{
							PrimData.SourceMaterials[SlotIndex] = *OverrideMat;
						}
					}
				}
			}
		}
	}
}

void AUsdHierarchicalBuildActor::BeginAsyncSourceDeltaScan(
	const FString& SourceUsdPath,
	const FDateTime& SourceUsdTimestampUtc,
	int64 SourceUsdFileSizeBytes,
	TMap<FString, FCachePrimSnapshot>&& CacheSnapshots
)
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	FSourceParseTask ParseTask;
	ParseTask.TargetActor = this;
	ParseTask.RequestId = ActiveBuildRequestId;
	ParseTask.SourceUsdPath = SourceUsdPath;
	ParseTask.SourceUsdTimestampUtc = SourceUsdTimestampUtc;
	ParseTask.SourceUsdFileSizeBytes = SourceUsdFileSizeBytes;
	ParseTask.CacheSnapshots = MoveTemp(CacheSnapshots);

	BuildSubsystem->EnqueueGlobalParseTask(MoveTemp(ParseTask));
	ScheduleParseResultPump();
#endif
}

void AUsdHierarchicalBuildActor::ScheduleParseResultPump()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	if (JobState.bParseResultPumpScheduled)
	{
		return;
	}

	if (!bIsBuildInProgress && !BuildSubsystem->HasGlobalParseWork())
	{
		return;
	}

	JobState.bParseResultPumpScheduled = true;
	const TWeakObjectPtr<AUsdHierarchicalBuildActor> WeakThis(this);
	AsyncTask(
		ENamedThreads::GameThread,
		[WeakThis]()
		{
			if (AUsdHierarchicalBuildActor* StrongThis = WeakThis.Get())
			{
				StrongThis->ProcessParseResultPump();
			}
		}
	);
#endif
}

void AUsdHierarchicalBuildActor::ProcessParseResultPump()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	JobState.bParseResultPumpScheduled = false;

	const double BudgetSeconds = FMath::Clamp(static_cast<double>(ParseResultFrameBudgetMs), 2.0, 5.0) / 1000.0;
	const int32 MaxResultsToConsume = FMath::Max(1, MaxParseResultsPerTick);
	const double PumpStartSeconds = FPlatformTime::Seconds();

	int32 ConsumedResults = 0;
	while (
		ConsumedResults < MaxResultsToConsume
		&& (FPlatformTime::Seconds() - PumpStartSeconds) < BudgetSeconds
	)
	{
		FSourceParseResult ParseResult;
		if (!BuildSubsystem->DequeueGlobalParseResult(ParseResult))
		{
			break;
		}

		++ConsumedResults;
		if (AUsdHierarchicalBuildActor* TargetActor = ParseResult.TargetActor.Get())
		{
			TargetActor->HandleParseResult(MoveTemp(ParseResult));
		}
	}

	if (BuildSubsystem->HasGlobalCompletedParseResults())
	{
		ScheduleParseResultPump();
	}
#endif
}

void AUsdHierarchicalBuildActor::HandleParseResult(FSourceParseResult&& ParseResult)
{
#if WITH_EDITOR
	if (!bIsBuildInProgress || ActiveBuildRequestId != ParseResult.RequestId)
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("Dropped stale parse result. RequestId=%d ActiveRequestId=%d"),
			ParseResult.RequestId,
			ActiveBuildRequestId
		);
		return;
	}

	UCarGeneratedAssemblyDataAsset* BuildAsset = GeneratedDataAsset.Get();
	if (!BuildAsset)
	{
		UUsdAssetCache3* UsdAssetCache = nullptr;
		FString ResolveMessage;
			if (GEditor)
			{
				if (UUsdCarFactoryDataAssetSubsystem* DataAssetSubsystem = GEditor->GetEditorSubsystem<UUsdCarFactoryDataAssetSubsystem>())
				{
					FUsdCarFactoryBuildInputs Inputs;
					Inputs.TargetActor = this;
					Inputs.SourceUsdFile = SourceUsdFile;
					Inputs.GeneratedDataAsset = GeneratedDataAsset;
					if (!DataAssetSubsystem->ResolveOrCreateBuildAssets(Inputs, BuildAsset, UsdAssetCache, ResolveMessage))
					{
						UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("%s"), *ResolveMessage);
						BuildAsset = nullptr;
					}
				}
			}
		}

	if (!BuildAsset)
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Error,
			TEXT("Failed to resolve GeneratedDataAsset while handling parse result. Source='%s' RequestId=%d"),
			*ParseResult.SourceUsdPath,
			ParseResult.RequestId
		);
		bIsBuildInProgress = false;
		PendingBuildPrimNodes.Reset();
		ResetPendingApplyState();
		ResetPendingDiffState();
		ResetPendingSourceRefreshState();
		return;
	}

	ActiveDeltaScanSeconds = ParseResult.DeltaScanSeconds;

	if (ParseResult.bDeltaScanSucceeded)
	{
		TArray<FPrimNodeBuildData> PrimNodes;
		ConvertSourceSnapshotsToPrimNodes(ParseResult.SourcePrimSnapshots, PrimNodes);
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("Parse result ready. source_prim_count=%d added=%d removed=%d changed_mesh=%d changed_xform=%d"),
			PrimNodes.Num(),
			ParseResult.AddedPrimPaths.Num(),
			ParseResult.RemovedPrimPaths.Num(),
			ParseResult.ChangedMeshPrimPaths.Num(),
			ParseResult.HierarchyOrTransformChangedPrimPaths.Num()
		);

		TArray<FString> ChangedMeshPrimPaths = ParseResult.ChangedMeshPrimPaths.Array();
		ChangedMeshPrimPaths.Sort();

		BeginSelectiveImportBatches(
			BuildAsset,
			MoveTemp(PrimNodes),
			MoveTemp(ChangedMeshPrimPaths),
			ParseResult.SourceUsdPath,
			ParseResult.SourceUsdTimestampUtc,
			ParseResult.SourceUsdFileSizeBytes,
			ParseResult.RequestId,
			true,
			ParseResult.AddedPrimPaths.Num(),
			ParseResult.RemovedPrimPaths.Num(),
			ParseResult.ChangedMeshPrimPaths.Num(),
			ParseResult.HierarchyOrTransformChangedPrimPaths.Num()
		);
		return;
	}

	UE_LOG(
		LogUsdCarFactoryPipelineBuild,
		Warning,
		TEXT("Source delta scan failed for '%s': %s"),
		*ParseResult.SourceUsdPath,
		*ParseResult.ErrorMessage
	);

	TArray<FPrimNodeBuildData> FallbackPrimNodes;
	CollectPrimNodeDataFromGeneratedAsset(BuildAsset, FallbackPrimNodes);
	UE_LOG(
		LogUsdCarFactoryPipelineBuild,
		Warning,
		TEXT("Source delta refresh degraded to cached generated asset data for '%s'. No full cache asset regeneration will be performed."),
		*ParseResult.SourceUsdPath
	);

	ActiveSourceRefreshSeconds = ActiveDeltaScanSeconds;
	StartBuildFromPreparedPrimNodes(BuildAsset, MoveTemp(FallbackPrimNodes), false, true);
#endif
}

void AUsdHierarchicalBuildActor::BeginSelectiveImportBatches(
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
)
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	ResetPendingSourceRefreshState();

	JobState.Phase = UsdCarFactoryPipelineBuild::EUsdProxyBuildJobPhase::Importing;
	JobState.PendingBuildPlan.Reset();
	JobState.PendingBuildPlan.BuildAsset = BuildAsset;
	JobState.PendingBuildPlan.PrimNodes = MoveTemp(PrimNodes);
	JobState.PendingBuildPlan.RequestId = RequestId;
	JobState.PendingBuildPlan.AddedPrimCount = AddedCount;
	JobState.PendingBuildPlan.RemovedPrimCount = RemovedCount;
	JobState.PendingBuildPlan.ChangedMeshPrimCount = ChangedMeshCount;
	JobState.PendingBuildPlan.ChangedTransformPrimCount = ChangedXformCount;
	JobState.PendingBuildPlan.bRefreshedFromSourceUsd = bRefreshedFromSourceUsd;
	JobState.PendingSelectiveImportPrimPaths = MoveTemp(ChangedMeshPrimPaths);
	JobState.ActiveRequest.SourceUsdPath = SourceUsdPath;
	JobState.ActiveRequest.SourceUsdTimestampUtc = SourceUsdTimestampUtc;
	JobState.ActiveRequest.SourceUsdFileSizeBytes = SourceUsdFileSizeBytes;
	JobState.ActiveRequest.GeneratedDataAsset = BuildAsset;
	JobState.PendingSelectiveImportIndex = 0;
	JobState.PendingSelectiveImportBatchCount = 0;
	ActiveSelectiveImportSeconds = FPlatformTime::Seconds();
	JobState.ActiveSelectiveImportSeconds = ActiveSelectiveImportSeconds;

	if (JobState.PendingSelectiveImportPrimPaths.IsEmpty())
	{
		FinalizeSourceRefreshAndStartBuild();
		return;
	}

	if (!bBuildAsync)
	{
		ProcessSelectiveImportBatch();
		return;
	}

	UsdCarFactoryPipelineBuild::ScheduleActorContinuation(
		TWeakObjectPtr<AUsdHierarchicalBuildActor>(this),
		&AUsdHierarchicalBuildActor::ProcessSelectiveImportBatch
	);
#endif
}

void AUsdHierarchicalBuildActor::ProcessSelectiveImportBatch()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	if (
		!bIsBuildInProgress
		|| !JobState.PendingBuildPlan.BuildAsset
		|| JobState.PendingBuildPlan.RequestId != ActiveBuildRequestId
	)
	{
		ResetPendingSourceRefreshState();
		return;
	}

	if (JobState.PendingSelectiveImportIndex >= JobState.PendingSelectiveImportPrimPaths.Num())
	{
		FinalizeSourceRefreshAndStartBuild();
		return;
	}

	const int32 BatchEnd = FMath::Min(
		JobState.PendingSelectiveImportIndex + UsdCarFactoryPipelineBuild::SelectiveImportBatchSize,
		JobState.PendingSelectiveImportPrimPaths.Num()
	);

	TArray<FString> BatchPrimPaths;
	BatchPrimPaths.Reserve(BatchEnd - JobState.PendingSelectiveImportIndex);
	for (int32 PrimPathIndex = JobState.PendingSelectiveImportIndex; PrimPathIndex < BatchEnd; ++PrimPathIndex)
	{
		BatchPrimPaths.Add(JobState.PendingSelectiveImportPrimPaths[PrimPathIndex]);
	}

	TMap<FString, TObjectPtr<UStaticMesh>> BatchPrimToMesh;
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> BatchPrimToMaterials;
	if (!ImportMeshAssetsForPrimPaths(JobState.ActiveRequest.SourceUsdPath, BatchPrimPaths, BatchPrimToMesh, BatchPrimToMaterials))
	{
		JobState.PendingRetryImportPrimPaths = MoveTemp(BatchPrimPaths);
		++JobState.PendingSelectiveImportRetryCount;
		FinalizeSourceRefreshAndStartBuild();
		return;
	}

	for (TPair<FString, TObjectPtr<UStaticMesh>>& Pair : BatchPrimToMesh)
	{
		if (!Pair.Key.IsEmpty() && Pair.Value)
		{
			JobState.PendingSelectivePrimToMesh.FindOrAdd(Pair.Key) = Pair.Value;
			JobState.PendingResolvedSelectiveImportPrimPaths.Add(Pair.Key);
		}
	}

	for (TPair<FString, TArray<TObjectPtr<UMaterialInterface>>>& Pair : BatchPrimToMaterials)
	{
		if (Pair.Key.IsEmpty())
		{
			continue;
		}

		TArray<TObjectPtr<UMaterialInterface>>& ExistingMaterials = JobState.PendingSelectivePrimToMaterials.FindOrAdd(Pair.Key);
		for (UMaterialInterface* Material : Pair.Value)
		{
			ExistingMaterials.AddUnique(Material);
		}
	}

	++JobState.PendingSelectiveImportBatchCount;
	JobState.PendingBuildPlan.ImportedBatchCount = JobState.PendingSelectiveImportBatchCount;
	JobState.PendingBuildPlan.bUsedSelectiveImport = true;
	JobState.PendingSelectiveImportIndex = BatchEnd;

	if (JobState.PendingSelectiveImportIndex >= JobState.PendingSelectiveImportPrimPaths.Num())
	{
		FinalizeSourceRefreshAndStartBuild();
		return;
	}

	if (!bBuildAsync)
	{
		ProcessSelectiveImportBatch();
	}
	else
	{
		UsdCarFactoryPipelineBuild::ScheduleActorContinuation(
			TWeakObjectPtr<AUsdHierarchicalBuildActor>(this),
			&AUsdHierarchicalBuildActor::ProcessSelectiveImportBatch
		);
	}
#endif
}

void AUsdHierarchicalBuildActor::FinalizeSourceRefreshAndStartBuild()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	UCarGeneratedAssemblyDataAsset* BuildAsset = JobState.PendingBuildPlan.BuildAsset.Get();
	const FString SourceUsdPath = JobState.ActiveRequest.SourceUsdPath;
	const int32 RequestId = JobState.PendingBuildPlan.RequestId;
	const bool bRefreshedFromSourceUsd = JobState.PendingBuildPlan.bRefreshedFromSourceUsd;
	const int32 AddedCount = JobState.PendingBuildPlan.AddedPrimCount;
	const int32 RemovedCount = JobState.PendingBuildPlan.RemovedPrimCount;
	const int32 ChangedMeshCount = JobState.PendingBuildPlan.ChangedMeshPrimCount;
	const int32 ChangedXformCount = JobState.PendingBuildPlan.ChangedTransformPrimCount;
	const int32 ImportedBatchCount = JobState.PendingBuildPlan.ImportedBatchCount;
	const TArray<FString> RequestedSelectiveImportPrimPaths = JobState.PendingSelectiveImportPrimPaths;

	TArray<FPrimNodeBuildData> PrimNodes = MoveTemp(JobState.PendingBuildPlan.PrimNodes);
	TMap<FString, TObjectPtr<UStaticMesh>> PrimToMesh = MoveTemp(JobState.PendingSelectivePrimToMesh);
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> PrimToMaterials = MoveTemp(JobState.PendingSelectivePrimToMaterials);
	const TArray<FString> RetryPrimPaths = MoveTemp(JobState.PendingRetryImportPrimPaths);
	UsdCarFactoryPipelineBuild::FUsdProxyBuildPlan BuildPlan = MoveTemp(JobState.PendingBuildPlan);
	const FDateTime SourceUsdTimestampUtc = JobState.ActiveRequest.SourceUsdTimestampUtc;
	const int64 SourceUsdFileSizeBytes = JobState.ActiveRequest.SourceUsdFileSizeBytes;

	ResetPendingSourceRefreshState();

	if (!BuildAsset || !bIsBuildInProgress || RequestId != ActiveBuildRequestId)
	{
		return;
	}

	ActiveSelectiveImportSeconds =
		ActiveSelectiveImportSeconds > 0.0 ? FPlatformTime::Seconds() - ActiveSelectiveImportSeconds : 0.0;

	bool bAppliedFreshSourceData = false;

	if (!RetryPrimPaths.IsEmpty())
	{
		RetryMissingSelectiveImports(
			PrimNodes,
			SourceUsdPath,
			RetryPrimPaths,
			PrimToMesh,
			PrimToMaterials,
			BuildPlan
		);
	}

	bool bMissingSelectiveImports = false;
	for (const FString& RequestedPrimPath : RequestedSelectiveImportPrimPaths)
	{
		if (!RequestedPrimPath.IsEmpty() && !PrimToMesh.Contains(RequestedPrimPath))
		{
			bMissingSelectiveImports = true;
			break;
		}
	}

	if (bMissingSelectiveImports)
	{
		ApplyImportedAssetsToPrimNodes(PrimNodes, PrimToMesh, PrimToMaterials);
		BuildPlan.bDegradedToCachedMeshes = true;
		BuildPlan.DegradedReason =
			TEXT("Selective import did not resolve every changed mesh prim; unresolved prims continue using cached mesh assets and source refresh metadata was not advanced.");
	}
	else
	{
		ApplyImportedAssetsToPrimNodes(PrimNodes, PrimToMesh, PrimToMaterials);
		UpdateSourceRefreshMetadata(BuildAsset, SourceUsdPath, SourceUsdTimestampUtc, SourceUsdFileSizeBytes);
		bAppliedFreshSourceData = true;
	}

	ActiveSourceRefreshSeconds = ActiveDeltaScanSeconds + ActiveSelectiveImportSeconds;

	UE_LOG(
		LogUsdCarFactoryPipelineBuild,
		Log,
		TEXT(
			"Source delta refresh completed. added=%d removed=%d changed_mesh=%d changed_xform=%d imported_batches=%d degraded_to_cached=%s delta_scan=%.3fs selective_import=%.3fs"
		),
		AddedCount,
		RemovedCount,
		ChangedMeshCount,
		ChangedXformCount,
		ImportedBatchCount,
		BuildPlan.bDegradedToCachedMeshes ? TEXT("Yes") : TEXT("No"),
		ActiveDeltaScanSeconds,
		ActiveSelectiveImportSeconds
	);

	if (BuildPlan.bDegradedToCachedMeshes)
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Warning, TEXT("Source refresh degraded to cached meshes: %s"), *BuildPlan.DegradedReason);
	}

	StartBuildFromPreparedPrimNodes(
		BuildAsset,
		MoveTemp(PrimNodes),
		bRefreshedFromSourceUsd && bAppliedFreshSourceData,
		true
	);
#endif
}

void AUsdHierarchicalBuildActor::RetryMissingSelectiveImports(
	TArray<FPrimNodeBuildData>& PrimNodes,
	const FString& SourceUsdPath,
	const TArray<FString>& RequestedPrimPaths,
	TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials,
	UsdCarFactoryPipelineBuild::FUsdProxyBuildPlan& BuildPlan
) const
{
#if WITH_EDITOR
	TArray<FString> MissingPrimPaths;
	for (const FString& RequestedPrimPath : RequestedPrimPaths)
	{
		if (!RequestedPrimPath.IsEmpty() && !PrimToMesh.Contains(RequestedPrimPath))
		{
			MissingPrimPaths.Add(RequestedPrimPath);
		}
	}

	if (MissingPrimPaths.IsEmpty())
	{
		return;
	}

	UE_LOG(
		LogUsdCarFactoryPipelineBuild,
		Warning,
		TEXT("Selective import missed %d mesh prims for '%s'. Retrying the missing prims before degrading to cached mesh assets."),
		MissingPrimPaths.Num(),
		*SourceUsdPath
	);

	TMap<FString, TObjectPtr<UStaticMesh>> RetryPrimToMesh;
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>> RetryPrimToMaterials;
	if (ImportMeshAssetsForPrimPaths(SourceUsdPath, MissingPrimPaths, RetryPrimToMesh, RetryPrimToMaterials))
	{
		for (TPair<FString, TObjectPtr<UStaticMesh>>& Pair : RetryPrimToMesh)
		{
			if (!Pair.Key.IsEmpty() && Pair.Value)
			{
				PrimToMesh.FindOrAdd(Pair.Key) = Pair.Value;
			}
		}

		for (TPair<FString, TArray<TObjectPtr<UMaterialInterface>>>& Pair : RetryPrimToMaterials)
		{
			if (Pair.Key.IsEmpty())
			{
				continue;
			}

			TArray<TObjectPtr<UMaterialInterface>>& ExistingMaterials = PrimToMaterials.FindOrAdd(Pair.Key);
			for (UMaterialInterface* Material : Pair.Value)
			{
				ExistingMaterials.AddUnique(Material);
			}
		}

		BuildPlan.bUsedRetryImport = true;
		ApplyImportedAssetsToPrimNodes(PrimNodes, PrimToMesh, PrimToMaterials);
	}
#endif
}

void AUsdHierarchicalBuildActor::StartBuildFromPreparedPrimNodes(
	UCarGeneratedAssemblyDataAsset* BuildAsset,
	TArray<FPrimNodeBuildData>&& PrimNodes,
	bool bRefreshedFromSourceUsd,
	bool bUseExistingRequestId
)
{
#if WITH_EDITOR
	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("StartBuildFromPreparedPrimNodes: PrimNodes=%d, BuildAsset=%s"), PrimNodes.Num(), BuildAsset ? *BuildAsset->GetName() : TEXT("null"));

	if (!BuildAsset)
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("StartBuildFromPreparedPrimNodes: BuildAsset is null, aborting."));
		bIsBuildInProgress = false;
		PendingBuildPrimNodes.Reset();
		ResetPendingApplyState();
		ResetPendingDiffState();
		return;
	}

	PrimNodes.Sort([](const FPrimNodeBuildData& A, const FPrimNodeBuildData& B)
	{
		return GetPrimDepth(A.PrimPath) < GetPrimDepth(B.PrimPath);
	});

	TMap<FString, FString> ParentPrimPathByPrimPath;
	ParentPrimPathByPrimPath.Reserve(PrimNodes.Num());
	for (const FPrimNodeBuildData& PrimNode : PrimNodes)
	{
		if (!PrimNode.PrimPath.IsEmpty())
		{
			ParentPrimPathByPrimPath.Add(PrimNode.PrimPath, PrimNode.ParentPrimPath);
		}
	}

	TSet<FString> SupportedPrimPaths;
	SupportedPrimPaths.Reserve(PrimNodes.Num());
	TArray<FPrimNodeBuildData> SupportedPrimNodes;
	SupportedPrimNodes.Reserve(PrimNodes.Num());
	int32 FilteredUnsupportedPrimCount = 0;

	for (FPrimNodeBuildData& PrimNode : PrimNodes)
	{
		if (!UsdCarFactoryPipelineBuild::IsSupportedProxyPrimType(PrimNode.PrimTypeName, PrimNode.bIsStaticMesh))
		{
			++FilteredUnsupportedPrimCount;
			continue;
		}

		FString ResolvedParentPrimPath = PrimNode.ParentPrimPath;
		while (!ResolvedParentPrimPath.IsEmpty() && !SupportedPrimPaths.Contains(ResolvedParentPrimPath))
		{
			const FString* ParentOfCurrentParent = ParentPrimPathByPrimPath.Find(ResolvedParentPrimPath);
			if (!ParentOfCurrentParent || *ParentOfCurrentParent == ResolvedParentPrimPath)
			{
				ResolvedParentPrimPath.Reset();
				break;
			}

			ResolvedParentPrimPath = *ParentOfCurrentParent;
		}

		PrimNode.ParentPrimPath = ResolvedParentPrimPath;
		SupportedPrimPaths.Add(PrimNode.PrimPath);
		SupportedPrimNodes.Add(MoveTemp(PrimNode));
	}

	if (FilteredUnsupportedPrimCount > 0)
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("Filtered %d unsupported prim nodes. Only mesh/xform prim types will generate proxy actors."),
			FilteredUnsupportedPrimCount
		);
	}

	PendingBuildPrimNodes = MoveTemp(SupportedPrimNodes);

	if (!bUseExistingRequestId)
	{
		++ActiveBuildRequestId;
	}
	bIsBuildInProgress = true;

	TArray<AActor*> DuplicateAttachedProxyActors;
	TMap<FString, AActor*> AttachedProxyActors = GatherAttachedProxyActors(DuplicateAttachedProxyActors);
	for (AActor* DuplicateActor : DuplicateAttachedProxyActors)
	{
		if (IsValid(DuplicateActor) && DuplicateActor != this)
		{
			DuplicateActor->Destroy();
		}
	}

	if (PendingBuildPrimNodes.IsEmpty())
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Warning,
			TEXT("No prim nodes available for build. cache_refreshed_from_source_usd=%s"),
			bRefreshedFromSourceUsd ? TEXT("Yes") : TEXT("No")
		);
		TArray<FString> AllStalePrimPaths;
		AttachedProxyActors.GetKeys(AllStalePrimPaths);
		ApplyBuildResults(BuildAsset, TSet<FString>(), AllStalePrimPaths, MoveTemp(AttachedProxyActors));
		return;
	}

	if (bBuildAsync)
	{
		const int32 AttachedProxyActorCount = AttachedProxyActors.Num();
		BeginAsyncDiffPreparation(BuildAsset, MoveTemp(AttachedProxyActors), bRefreshedFromSourceUsd);

		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Log,
			TEXT("Async USD build request queued. Prim count: %d, attached proxy count: %d, cache refreshed from source USD: %s"),
			PendingBuildPrimNodes.Num(),
			AttachedProxyActorCount,
			bRefreshedFromSourceUsd ? TEXT("Yes") : TEXT("No")
		);
		return;
	}

	TMap<FString, UsdCarFactoryPipelineBuild::FProxyActorSnapshot> ExistingSnapshots;
	for (const TPair<FString, AActor*>& Pair : AttachedProxyActors)
	{
		if (IsValid(Pair.Value))
		{
			ExistingSnapshots.Add(Pair.Key, BuildExistingSnapshotForDiff(Pair.Key, Pair.Value));
		}
	}

	TArray<UsdCarFactoryPipelineBuild::FProxyActorSnapshot> TargetSnapshots;
	TargetSnapshots.Reserve(PendingBuildPrimNodes.Num());
	for (const FPrimNodeBuildData& PrimData : PendingBuildPrimNodes)
	{
		TargetSnapshots.Add(BuildTargetSnapshotForDiff(PrimData));
	}

	const double DiffStartSeconds = FPlatformTime::Seconds();
	const UsdCarFactoryPipelineBuild::FBuildDiffResult DiffResult =
		UsdCarFactoryPipelineBuild::ComputeBuildDiff(TargetSnapshots, ExistingSnapshots);
	ActiveDiffSeconds = FPlatformTime::Seconds() - DiffStartSeconds;
	ApplyBuildResults(
		BuildAsset,
		DiffResult.UnchangedPrimPaths,
		DiffResult.StalePrimPaths,
		MoveTemp(AttachedProxyActors)
	);
#endif
}

void AUsdHierarchicalBuildActor::ResetPendingSourceRefreshState()
{
	if (UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem())
	{
		BuildSubsystem->GetJobState(*this).ResetSourceRefreshState();
	}
}

bool AUsdHierarchicalBuildActor::ShouldRefreshCacheFromSourceUsd(
	const UCarGeneratedAssemblyDataAsset* BuildAsset,
	const FString& SourceUsdPath,
	const FDateTime& SourceUsdTimestamp,
	int64 SourceUsdFileSizeBytes
) const
{
#if WITH_EDITOR
	if (SourceUsdPath.IsEmpty())
	{
		return false;
	}

	if (!BuildAsset || BuildAsset->Parts.IsEmpty())
	{
		return true;
	}

	const FString CachedSourcePath = FPaths::ConvertRelativePathToFull(BuildAsset->SourceUsdFile.FilePath);
	if (!CachedSourcePath.Equals(SourceUsdPath, ESearchCase::IgnoreCase))
	{
		return true;
	}

	if (SourceUsdTimestamp != BuildAsset->SourceUsdTimestampUtc)
	{
		return true;
	}

	if (SourceUsdFileSizeBytes >= 0 && BuildAsset->SourceUsdFileSizeBytes != SourceUsdFileSizeBytes)
	{
		return true;
	}
#endif
	return false;
}

bool AUsdHierarchicalBuildActor::CollectPrimNodeDataFromSourceUsd(TArray<FPrimNodeBuildData>& OutData) const
{
#if WITH_EDITOR
	if (SourceUsdFile.FilePath.IsEmpty())
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("CollectPrimNodeDataFromSourceUsd: SourceUsdFile.FilePath is empty"));
		return false;
	}

#if USE_USD_SDK
	const FString SourceUsdPath = FPaths::ConvertRelativePathToFull(SourceUsdFile.FilePath);
	if (SourceUsdPath.IsEmpty() || !IFileManager::Get().FileExists(*SourceUsdPath))
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("Source USD does not exist: '%s'"), *SourceUsdPath);
		return false;
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("CollectPrimNodeDataFromSourceUsd: Opening USD stage directly for '%s'"), *SourceUsdPath);

	// Open the USD stage directly using UnrealUSDWrapper (same approach as delta scan)
	UE::FUsdStage Stage = UnrealUSDWrapper::OpenStage(*SourceUsdPath, EUsdInitialLoadSet::LoadAll);
	if (!Stage)
	{
		UE_LOG(LogUsdCarFactoryPipelineBuild, Error, TEXT("CollectPrimNodeDataFromSourceUsd: Failed to open USD stage for '%s'"), *SourceUsdPath);
		return false;
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("CollectPrimNodeDataFromSourceUsd: USD stage opened successfully, collecting prim data..."));

	// Collect prim data directly from the stage
	const FUsdStageInfo StageInfo(Stage);
	pxr::UsdGeomXformCache XformCache;

	TFunction<void(const UE::FUsdPrim&, const FString&)> VisitPrim;
	VisitPrim = [&OutData, &VisitPrim, &XformCache, &StageInfo, &Stage, this](
					const UE::FUsdPrim& Prim,
					const FString& ParentPrimPath
				)
	{
		if (!Prim)
		{
			return;
		}

		const FString PrimPath = Prim.GetPrimPath().GetString();
		FPrimNodeBuildData Data;
		Data.PrimPath = PrimPath;
		Data.ParentPrimPath = ParentPrimPath;
		Data.PrimTypeName = Prim.GetTypeName().ToString();
		Data.RelativeTransform = FTransform::Identity;
		Data.WorldTransform = FTransform::Identity;
		Data.bIsStaticMesh = Data.PrimTypeName.Equals(TEXT("Mesh"), ESearchCase::IgnoreCase);
		if (Data.bIsStaticMesh)
		{
			Data.MeshContentHash = ComputeMeshContentHash(Stage, Prim);
		}

		const pxr::UsdPrim UsdPrim = static_cast<const pxr::UsdPrim&>(Prim);
		if (UsdPrim)
		{
			const pxr::GfMatrix4d LocalToWorld = XformCache.GetLocalToWorldTransform(UsdPrim);
			Data.WorldTransform = UsdToUnreal::ConvertMatrix(StageInfo, LocalToWorld);

			if (ParentPrimPath.IsEmpty())
			{
				Data.RelativeTransform = Data.WorldTransform;
			}
			else
			{
				const UE::FUsdPrim ParentPrim = Prim.GetParent();
				const pxr::UsdPrim ParentUsdPrim = ParentPrim ? static_cast<const pxr::UsdPrim&>(ParentPrim) : pxr::UsdPrim();
				if (ParentUsdPrim)
				{
					const pxr::GfMatrix4d ParentLocalToWorld = XformCache.GetLocalToWorldTransform(ParentUsdPrim);
					const pxr::GfMatrix4d RelativeMatrix = LocalToWorld * ParentLocalToWorld.GetInverse();
					Data.RelativeTransform = UsdToUnreal::ConvertMatrix(StageInfo, RelativeMatrix);
				}
			}
		}

		OutData.Add(MoveTemp(Data));

		// Traverse children
		for (UE::FUsdPrim ChildPrim : Prim.GetChildren())
		{
			VisitPrim(ChildPrim, PrimPath);
		}
	};

	for (UE::FUsdPrim TopLevelPrim : Stage.GetPseudoRoot().GetChildren())
	{
		VisitPrim(TopLevelPrim, FString());
	}

	UE_LOG(LogUsdCarFactoryPipelineBuild, Log, TEXT("CollectPrimNodeDataFromSourceUsd: Collected %d prim nodes from '%s'"), OutData.Num(), *SourceUsdPath);

	return OutData.Num() > 0;
#else
	return false;
#endif
#else
	return false;
#endif
}

void AUsdHierarchicalBuildActor::BuildPrimToImportedMeshMap(
	const TArray<TObjectPtr<UObject>>& ImportedAssets,
	TMap<FString, TObjectPtr<UStaticMesh>>& OutPrimToMesh
) const
{
	OutPrimToMesh.Reset();

	for (UObject* ImportedAsset : ImportedAssets)
	{
		UStaticMesh* StaticMesh = Cast<UStaticMesh>(ImportedAsset);
		if (!StaticMesh)
		{
			continue;
		}

		const UUsdAssetUserData* AssetUserData = Cast<UUsdAssetUserData>(
			UsdUnreal::ObjectUtils::GetAssetUserData(StaticMesh, UUsdAssetUserData::StaticClass())
		);
		if (!AssetUserData)
		{
			continue;
		}

		for (const FString& PrimPath : AssetUserData->PrimPaths)
		{
			if (!PrimPath.IsEmpty() && !OutPrimToMesh.Contains(PrimPath))
			{
				OutPrimToMesh.Add(PrimPath, StaticMesh);
			}
		}
	}
}

void AUsdHierarchicalBuildActor::BuildPrimToImportedMaterialMap(
	const TArray<TObjectPtr<UObject>>& ImportedAssets,
	TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& OutPrimToMaterials
) const
{
	OutPrimToMaterials.Reset();

	for (UObject* ImportedAsset : ImportedAssets)
	{
		UMaterialInterface* Material = Cast<UMaterialInterface>(ImportedAsset);
		if (!Material)
		{
			continue;
		}

		const UUsdAssetUserData* AssetUserData = Cast<UUsdAssetUserData>(
			UsdUnreal::ObjectUtils::GetAssetUserData(Material, UUsdAssetUserData::StaticClass())
		);
		if (!AssetUserData)
		{
			continue;
		}

		for (const FString& PrimPath : AssetUserData->PrimPaths)
		{
			if (PrimPath.IsEmpty())
			{
				continue;
			}

			TArray<TObjectPtr<UMaterialInterface>>& MaterialsForPrim = OutPrimToMaterials.FindOrAdd(PrimPath);
			MaterialsForPrim.AddUnique(Material);
		}
	}
}

bool AUsdHierarchicalBuildActor::CollectPrimNodeDataFromImportedStage(
	const UE::FUsdStage& InStage,
	const TMap<FString, TObjectPtr<UStaticMesh>>& PrimToMesh,
	const TMap<FString, TArray<TObjectPtr<UMaterialInterface>>>& PrimToMaterials,
	TArray<FPrimNodeBuildData>& OutData
) const
{
#if USE_USD_SDK
	if (!InStage)
	{
		return false;
	}

	const FUsdStageInfo StageInfo(InStage);
	pxr::UsdGeomXformCache XformCache;

	TFunction<void(const UE::FUsdPrim&, const FString&)> VisitPrim;
	VisitPrim = [&OutData, &VisitPrim, &PrimToMesh, &PrimToMaterials, &XformCache, &StageInfo, &InStage](
					const UE::FUsdPrim& Prim,
					const FString& ParentPrimPath
				)
	{
		if (!Prim)
		{
			return;
		}

		const FString PrimPath = Prim.GetPrimPath().GetString();
		FPrimNodeBuildData Data;
		Data.PrimPath = PrimPath;
		Data.ParentPrimPath = ParentPrimPath;
		Data.PrimTypeName = Prim.GetTypeName().ToString();
		Data.RelativeTransform = FTransform::Identity;
		Data.WorldTransform = FTransform::Identity;
		Data.bIsStaticMesh = Data.PrimTypeName.Equals(TEXT("Mesh"), ESearchCase::IgnoreCase);
		if (Data.bIsStaticMesh)
		{
			Data.MeshContentHash = ComputeMeshContentHash(InStage, Prim);
		}

		const pxr::UsdPrim UsdPrim = static_cast<const pxr::UsdPrim&>(Prim);
		if (UsdPrim)
		{
			const pxr::GfMatrix4d LocalToWorld = XformCache.GetLocalToWorldTransform(UsdPrim);
			Data.WorldTransform = UsdToUnreal::ConvertMatrix(StageInfo, LocalToWorld);

			if (ParentPrimPath.IsEmpty())
			{
				Data.RelativeTransform = Data.WorldTransform;
			}
			else
			{
				const UE::FUsdPrim ParentPrim = Prim.GetParent();
				const pxr::UsdPrim ParentUsdPrim = ParentPrim ? static_cast<const pxr::UsdPrim&>(ParentPrim) : pxr::UsdPrim();
				if (ParentUsdPrim)
				{
					const pxr::GfMatrix4d ParentLocalToWorld = XformCache.GetLocalToWorldTransform(ParentUsdPrim);
					const pxr::GfMatrix4d RelativeMatrix = LocalToWorld * ParentLocalToWorld.GetInverse();
					Data.RelativeTransform = UsdToUnreal::ConvertMatrix(StageInfo, RelativeMatrix);
				}
				else
				{
					Data.RelativeTransform = Data.WorldTransform;
				}
			}
		}

		if (const TObjectPtr<UStaticMesh>* StaticMeshPtr = PrimToMesh.Find(PrimPath))
		{
			UStaticMesh* StaticMesh = StaticMeshPtr->Get();
			if (StaticMesh)
			{
				Data.bIsStaticMesh = true;
				Data.StaticMesh = StaticMesh;

				const TArray<FStaticMaterial>& StaticMaterials = StaticMesh->GetStaticMaterials();
				Data.SourceMaterials.Reserve(StaticMaterials.Num());
				Data.SourceMaterialSlotNames.Reserve(StaticMaterials.Num());
				for (const FStaticMaterial& StaticMaterial : StaticMaterials)
				{
					Data.SourceMaterialSlotNames.Add(StaticMaterial.MaterialSlotName);
					Data.SourceMaterials.Add(StaticMaterial.MaterialInterface);
				}

				// Fallback for stages where slot defaults are null but material assets were still imported for this prim.
				if (const TArray<TObjectPtr<UMaterialInterface>>* ImportedMaterials = PrimToMaterials.Find(PrimPath))
				{
					int32 ImportedMaterialIndex = 0;
					for (int32 SlotIndex = 0; SlotIndex < Data.SourceMaterials.Num() && ImportedMaterialIndex < ImportedMaterials->Num(); ++SlotIndex)
					{
						if (!Data.SourceMaterials[SlotIndex] && ImportedMaterials->IsValidIndex(ImportedMaterialIndex))
						{
							Data.SourceMaterials[SlotIndex] = (*ImportedMaterials)[ImportedMaterialIndex++];
						}
					}
				}
			}
		}

		OutData.Add(MoveTemp(Data));
		for (UE::FUsdPrim ChildPrim : Prim.GetChildren())
		{
			VisitPrim(ChildPrim, PrimPath);
		}
	};

	for (UE::FUsdPrim TopLevelPrim : InStage.GetPseudoRoot().GetChildren())
	{
		VisitPrim(TopLevelPrim, FString());
	}

	return true;
#else
	return false;
#endif
}

void AUsdHierarchicalBuildActor::BuildPreviousPartIndexCache()
{
	PreviousPartIndexByPrimPath.Reset();

	if (!GeneratedDataAsset)
	{
		return;
	}

	PreviousPartIndexByPrimPath.Reserve(GeneratedDataAsset->Parts.Num());
	for (int32 PartIndex = 0; PartIndex < GeneratedDataAsset->Parts.Num(); ++PartIndex)
	{
		const FString& PrimPath = GeneratedDataAsset->Parts[PartIndex].PrimPath;
		if (!PrimPath.IsEmpty() && !PreviousPartIndexByPrimPath.Contains(PrimPath))
		{
			PreviousPartIndexByPrimPath.Add(PrimPath, PartIndex);
		}
	}
}

void AUsdHierarchicalBuildActor::CollectPrimNodeDataFromGeneratedAsset(
	const UCarGeneratedAssemblyDataAsset* InBuildAsset,
	TArray<FPrimNodeBuildData>& OutData
) const
{
	if (!InBuildAsset)
	{
		return;
	}

	TMap<FString, FPrimNodeBuildData> PrimNodeMap;
	PrimNodeMap.Reserve(InBuildAsset->Parts.Num() * 2);

	auto GetParentPrimPath = [](const FString& PrimPath) -> FString
	{
		if (PrimPath.IsEmpty())
		{
			return FString();
		}

		int32 LastSlashIndex = INDEX_NONE;
		if (!PrimPath.FindLastChar(TEXT('/'), LastSlashIndex) || LastSlashIndex <= 0)
		{
			return FString();
		}

		return PrimPath.Left(LastSlashIndex);
	};

	for (const FCarGeneratedPartRecord& Record : InBuildAsset->Parts)
	{
		if (Record.PrimPath.IsEmpty())
		{
			continue;
		}

		FPrimNodeBuildData Data;
		Data.PrimPath = Record.PrimPath;
		Data.ParentPrimPath = Record.ParentPrimPath;
		const bool bRecordIsStaticMesh = Record.bIsStaticMesh || (Record.StaticMesh != nullptr);
		Data.PrimTypeName = bRecordIsStaticMesh ? TEXT("mesh") : TEXT("xform");
		Data.RelativeTransform = Record.RelativeTransform;
		Data.WorldTransform = Record.RelativeTransform;
		Data.bIsStaticMesh = bRecordIsStaticMesh;
		Data.MeshContentHash = Record.MeshContentHash;
		Data.StaticMesh = Record.StaticMesh;

		if (Data.bIsStaticMesh)
		{
			Data.SourceMaterialSlotNames = Record.MaterialSlotNames;
			Data.SourceMaterials = Record.Materials;
			if (Data.SourceMaterials.IsEmpty() && Record.MaterialOverrides.Num() > 0 && Record.StaticMesh)
			{
				const TArray<FStaticMaterial>& StaticMats = Record.StaticMesh->GetStaticMaterials();
				Data.SourceMaterials.SetNum(StaticMats.Num());
				Data.SourceMaterialSlotNames.Reset(StaticMats.Num());

				for (int32 SlotIndex = 0; SlotIndex < StaticMats.Num(); ++SlotIndex)
				{
					const FName SlotName = StaticMats[SlotIndex].MaterialSlotName;
					Data.SourceMaterialSlotNames.Add(SlotName);
					if (const TObjectPtr<UMaterialInterface>* FoundMat = Record.MaterialOverrides.Find(SlotName))
					{
						Data.SourceMaterials[SlotIndex] = *FoundMat;
					}
				}
			}
		}

		PrimNodeMap.FindOrAdd(Data.PrimPath) = MoveTemp(Data);
	}

	// Rebuild complete parent hierarchy from cached paths.
	// This keeps xform levels even when older cache files only had mesh entries.
	TArray<FString> ExistingPrimPaths;
	PrimNodeMap.GetKeys(ExistingPrimPaths);
	for (const FString& ExistingPrimPath : ExistingPrimPaths)
	{
		FString CurrentPath = ExistingPrimPath;
		while (!CurrentPath.IsEmpty())
		{
			const FString ParentPath = GetParentPrimPath(CurrentPath);
			if (ParentPath.IsEmpty())
			{
				break;
			}

			FPrimNodeBuildData* CurrentNode = PrimNodeMap.Find(CurrentPath);
			if (CurrentNode && CurrentNode->ParentPrimPath.IsEmpty())
			{
				CurrentNode->ParentPrimPath = ParentPath;
			}

			if (!PrimNodeMap.Contains(ParentPath))
			{
				FPrimNodeBuildData ParentNode;
				ParentNode.PrimPath = ParentPath;
				ParentNode.ParentPrimPath = GetParentPrimPath(ParentPath);
				ParentNode.PrimTypeName = TEXT("xform");
				ParentNode.RelativeTransform = FTransform::Identity;
				ParentNode.WorldTransform = FTransform::Identity;
				ParentNode.bIsStaticMesh = false;
				PrimNodeMap.Add(ParentPath, MoveTemp(ParentNode));
			}

			CurrentPath = ParentPath;
		}
	}

	OutData.Reserve(PrimNodeMap.Num());
	for (TPair<FString, FPrimNodeBuildData>& Pair : PrimNodeMap)
	{
		OutData.Add(MoveTemp(Pair.Value));
	}
}

UsdCarFactoryPipelineBuild::FProxyActorSnapshot AUsdHierarchicalBuildActor::BuildTargetSnapshotForDiff(
	const FPrimNodeBuildData& PrimData
) const
{
	UsdCarFactoryPipelineBuild::FProxyActorSnapshot Snapshot;
	Snapshot.PrimPath = PrimData.PrimPath;
	Snapshot.ParentPrimPath = PrimData.ParentPrimPath;
	Snapshot.RelativeTransform = PrimData.RelativeTransform;
	Snapshot.bIsStaticMesh = PrimData.bIsStaticMesh;

	if (!PrimData.bIsStaticMesh)
	{
		return Snapshot;
	}

	const FCarGeneratedPartRecord* PreviousRecord = FindPreviousRecord(PrimData.PrimPath);
	Snapshot.StaticMeshPath = UsdCarFactoryPipelineBuild::GetObjectPath(PrimData.StaticMesh);

	int32 NumSlots = PrimData.SourceMaterials.Num();
	if (PreviousRecord)
	{
		NumSlots = FMath::Max(NumSlots, PreviousRecord->Materials.Num());
	}
	Snapshot.MaterialPaths.SetNum(NumSlots);

	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		if (PreviousRecord && PreviousRecord->Materials.IsValidIndex(SlotIndex) && PreviousRecord->Materials[SlotIndex])
		{
			Snapshot.MaterialPaths[SlotIndex] = UsdCarFactoryPipelineBuild::GetObjectPath(PreviousRecord->Materials[SlotIndex]);
			continue;
		}

		if (PrimData.SourceMaterials.IsValidIndex(SlotIndex) && PrimData.SourceMaterials[SlotIndex])
		{
			Snapshot.MaterialPaths[SlotIndex] = UsdCarFactoryPipelineBuild::GetObjectPath(PrimData.SourceMaterials[SlotIndex]);
		}
	}

	return Snapshot;
}

UsdCarFactoryPipelineBuild::FProxyActorSnapshot AUsdHierarchicalBuildActor::BuildExistingSnapshotForDiff(
	const FString& PrimPath,
	AActor* Actor
) const
{
	UsdCarFactoryPipelineBuild::FProxyActorSnapshot Snapshot;
	Snapshot.PrimPath = PrimPath;

	if (!Actor)
	{
		return Snapshot;
	}

	if (USceneComponent* ActorRootComponent = Actor->GetRootComponent())
	{
		Snapshot.RelativeTransform = ActorRootComponent->GetRelativeTransform();
	}
	else
	{
		Snapshot.RelativeTransform = Actor->GetActorTransform();
	}

	if (AActor* ParentActor = Actor->GetAttachParentActor())
	{
		if (ParentActor != this)
		{
			Snapshot.ParentPrimPath = UsdCarFactoryPipelineBuild::ExtractExactPrimPathFromActor(ParentActor);
		}
	}

	AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(Actor);
	UStaticMeshComponent* StaticMeshComponent = StaticMeshActor ? StaticMeshActor->GetStaticMeshComponent() : nullptr;
	if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
	{
		Snapshot.bIsStaticMesh = true;
		Snapshot.StaticMeshPath = UsdCarFactoryPipelineBuild::GetObjectPath(StaticMeshComponent->GetStaticMesh());

		const int32 NumSlots = StaticMeshComponent->GetNumMaterials();
		Snapshot.MaterialPaths.Reserve(NumSlots);
		for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
		{
			Snapshot.MaterialPaths.Add(UsdCarFactoryPipelineBuild::GetObjectPath(StaticMeshComponent->GetMaterial(SlotIndex)));
		}
	}

	return Snapshot;
}

void AUsdHierarchicalBuildActor::BeginAsyncDiffPreparation(
	UCarGeneratedAssemblyDataAsset* BuildAsset,
	TMap<FString, AActor*>&& AttachedProxyActors,
	bool bRefreshedFromSourceUsd
)
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	ResetPendingDiffState();

	JobState.Phase = UsdCarFactoryPipelineBuild::EUsdProxyBuildJobPhase::Diffing;
	JobState.PendingBuildPlan.BuildAsset = BuildAsset;
	JobState.PendingBuildPlan.bRefreshedFromSourceUsd = bRefreshedFromSourceUsd;
	JobState.PendingBuildPlan.RequestId = ActiveBuildRequestId;
	JobState.PendingDiffAttachedProxyActors = MoveTemp(AttachedProxyActors);
	JobState.PendingDiffAttachedProxyActors.GetKeys(JobState.PendingDiffAttachedPrimPaths);
	JobState.PendingDiffTargetSnapshots.Reserve(PendingBuildPrimNodes.Num());
	JobState.PendingDiffExistingSnapshots.Reserve(JobState.PendingDiffAttachedPrimPaths.Num());
	JobState.PendingDiffPrimIndex = 0;
	JobState.PendingDiffAttachedIndex = 0;

	UsdCarFactoryPipelineBuild::ScheduleActorContinuation(
		TWeakObjectPtr<AUsdHierarchicalBuildActor>(this),
		&AUsdHierarchicalBuildActor::ProcessDiffPreparationBatch
	);
#endif
}

void AUsdHierarchicalBuildActor::ProcessDiffPreparationBatch()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	if (!bIsBuildInProgress || JobState.PendingBuildPlan.RequestId != ActiveBuildRequestId || !JobState.PendingBuildPlan.BuildAsset)
	{
		ResetPendingDiffState();
		return;
	}

	const int32 BaseBatchSize = FMath::Max(1, MaxPrimBuildPerTick);
	const int32 DiffBatchSize = bBuildAsync ? FMath::Max(32, BaseBatchSize * 4) : PendingBuildPrimNodes.Num();
	const double BatchStartSeconds = FPlatformTime::Seconds();
	const double TimeBudgetSeconds = 0.002; // Keep diff-preparation work under ~2ms per frame to avoid editor hitching.

	int32 ProcessedCount = 0;
	while (
		JobState.PendingDiffPrimIndex < PendingBuildPrimNodes.Num()
		&& ProcessedCount < DiffBatchSize
		&& (FPlatformTime::Seconds() - BatchStartSeconds) < TimeBudgetSeconds
	)
	{
		JobState.PendingDiffTargetSnapshots.Add(BuildTargetSnapshotForDiff(PendingBuildPrimNodes[JobState.PendingDiffPrimIndex]));
		++JobState.PendingDiffPrimIndex;
		++ProcessedCount;
	}

	while (
		JobState.PendingDiffAttachedIndex < JobState.PendingDiffAttachedPrimPaths.Num()
		&& ProcessedCount < DiffBatchSize
		&& (FPlatformTime::Seconds() - BatchStartSeconds) < TimeBudgetSeconds
	)
	{
		const FString& PrimPath = JobState.PendingDiffAttachedPrimPaths[JobState.PendingDiffAttachedIndex];
		if (AActor* ExistingActor = JobState.PendingDiffAttachedProxyActors.FindRef(PrimPath); IsValid(ExistingActor))
		{
			JobState.PendingDiffExistingSnapshots.Add(PrimPath, BuildExistingSnapshotForDiff(PrimPath, ExistingActor));
		}
		++JobState.PendingDiffAttachedIndex;
		++ProcessedCount;
	}

	const bool bDone =
		JobState.PendingDiffPrimIndex >= PendingBuildPrimNodes.Num()
		&& JobState.PendingDiffAttachedIndex >= JobState.PendingDiffAttachedPrimPaths.Num();
	if (!bDone)
	{
		UsdCarFactoryPipelineBuild::ScheduleActorContinuation(
			TWeakObjectPtr<AUsdHierarchicalBuildActor>(this),
			&AUsdHierarchicalBuildActor::ProcessDiffPreparationBatch
		);
		return;
	}

	LaunchAsyncDiffComputation();
#endif
}

void AUsdHierarchicalBuildActor::LaunchAsyncDiffComputation()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	if (!bIsBuildInProgress || JobState.PendingBuildPlan.RequestId != ActiveBuildRequestId || !JobState.PendingBuildPlan.BuildAsset)
	{
		ResetPendingDiffState();
		return;
	}

	TWeakObjectPtr<AUsdHierarchicalBuildActor> WeakThis(this);
	TWeakObjectPtr<UCarGeneratedAssemblyDataAsset> WeakBuildAsset(JobState.PendingBuildPlan.BuildAsset);
	const int32 RequestId = JobState.PendingBuildPlan.RequestId;

	TArray<UsdCarFactoryPipelineBuild::FProxyActorSnapshot> TargetSnapshots = MoveTemp(JobState.PendingDiffTargetSnapshots);
	TMap<FString, UsdCarFactoryPipelineBuild::FProxyActorSnapshot> ExistingSnapshots = MoveTemp(JobState.PendingDiffExistingSnapshots);
	TMap<FString, AActor*> AttachedProxyActors = MoveTemp(JobState.PendingDiffAttachedProxyActors);

	ResetPendingDiffState();

	Async(
		EAsyncExecution::ThreadPool,
		[
			WeakThis,
			WeakBuildAsset,
			RequestId,
			TargetSnapshots = MoveTemp(TargetSnapshots),
			ExistingSnapshots = MoveTemp(ExistingSnapshots),
			AttachedProxyActors = MoveTemp(AttachedProxyActors)
		]() mutable
		{
			const double DiffStartSeconds = FPlatformTime::Seconds();
			UsdCarFactoryPipelineBuild::FBuildDiffResult DiffResult =
				UsdCarFactoryPipelineBuild::ComputeBuildDiff(TargetSnapshots, ExistingSnapshots);
			const double DiffSeconds = FPlatformTime::Seconds() - DiffStartSeconds;

			AsyncTask(
				ENamedThreads::GameThread,
				[
					WeakThis,
					WeakBuildAsset,
					RequestId,
					DiffSeconds,
					DiffResult = MoveTemp(DiffResult),
					AttachedProxyActors = MoveTemp(AttachedProxyActors)
				]() mutable
				{
					AUsdHierarchicalBuildActor* StrongThis = WeakThis.Get();
					UCarGeneratedAssemblyDataAsset* StrongBuildAsset = WeakBuildAsset.Get();
					if (!StrongThis || !StrongBuildAsset)
					{
						return;
					}

					if (StrongThis->ActiveBuildRequestId != RequestId)
					{
						UE_LOG(
							LogUsdCarFactoryPipelineBuild,
							Log,
							TEXT("Ignored outdated async build result. RequestId=%d, ActiveRequestId=%d"),
							RequestId,
							StrongThis->ActiveBuildRequestId
						);
						return;
					}

					StrongThis->ActiveDiffSeconds = DiffSeconds;
					StrongThis->ApplyBuildResults(
						StrongBuildAsset,
						DiffResult.UnchangedPrimPaths,
						DiffResult.StalePrimPaths,
						MoveTemp(AttachedProxyActors)
					);
				}
			);
		}
	);
#endif
}

void AUsdHierarchicalBuildActor::ResetPendingDiffState()
{
	if (UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem())
	{
		BuildSubsystem->GetJobState(*this).ResetDiffState();
	}
}

void AUsdHierarchicalBuildActor::ApplyBuildResults(
	UCarGeneratedAssemblyDataAsset* BuildAsset,
	const TSet<FString>& UnchangedPrimPaths,
	const TArray<FString>& StalePrimPaths,
	TMap<FString, AActor*>&& AttachedProxyActorsSnapshot
)
{
#if WITH_EDITOR
	if (!BuildAsset)
	{
		bIsBuildInProgress = false;
		PendingBuildPrimNodes.Reset();
		ResetPendingApplyState();
		return;
	}

	ActiveApplySeconds = FPlatformTime::Seconds();
	TMap<FString, AActor*> AttachedProxyActors = MoveTemp(AttachedProxyActorsSnapshot);
	for (const FString& StalePrimPath : StalePrimPaths)
	{
		if (AActor* StaleActor = AttachedProxyActors.FindRef(StalePrimPath))
		{
			AttachedProxyActors.Remove(StalePrimPath);
			if (IsValid(StaleActor))
			{
				StaleActor->Destroy();
			}
		}
	}

	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		bIsBuildInProgress = false;
		PendingBuildPrimNodes.Reset();
		return;
	}

	BuildAsset->Modify();
	BuildAsset->SourceUsdFile = SourceUsdFile;

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	JobState.Phase = UsdCarFactoryPipelineBuild::EUsdProxyBuildJobPhase::Applying;
	JobState.PendingBuildPlan.BuildAsset = BuildAsset;
	JobState.PendingBuildPlan.RequestId = ActiveBuildRequestId;
	JobState.PendingBuildPlan.UnchangedPrimPaths = UnchangedPrimPaths;
	JobState.PendingBuildPlan.StalePrimPaths = StalePrimPaths;
	JobState.PendingApplyAttachedProxyActors = MoveTemp(AttachedProxyActors);
	JobState.PendingApplyPrimToProxyActor.Empty(PendingBuildPrimNodes.Num());
	JobState.PendingApplyNewRecords.SetNum(PendingBuildPrimNodes.Num());
	JobState.PendingApplyRecordCount = 0;
	JobState.PendingApplyIndex = 0;

	ProcessApplyBuildBatch();
#else
	UE_LOG(LogUsdCarFactoryPipelineBuild, Warning, TEXT("ApplyBuildResults can only run in editor."));
#endif
}

void AUsdHierarchicalBuildActor::ProcessApplyBuildBatch()
{
#if WITH_EDITOR
	UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem();
	if (!BuildSubsystem)
	{
		bIsBuildInProgress = false;
		PendingBuildPrimNodes.Reset();
		return;
	}

	UsdCarFactoryPipelineBuild::FUsdProxyBuildJobState& JobState = BuildSubsystem->GetJobState(*this);
	if (!bIsBuildInProgress || JobState.PendingBuildPlan.RequestId != ActiveBuildRequestId || !JobState.PendingBuildPlan.BuildAsset)
	{
		ResetPendingApplyState();
		return;
	}

	USceneComponent* RootAttachComponent = UsdCarFactoryPipelineBuild::EnsureSceneRoot(this);

	auto ClearProxyMesh = [](UStaticMeshComponent* ProxySMC)
	{
		if (!ProxySMC)
		{
			return;
		}

		const int32 NumMaterials = ProxySMC->GetNumMaterials();
		for (int32 SlotIndex = 0; SlotIndex < NumMaterials; ++SlotIndex)
		{
			ProxySMC->SetMaterial(SlotIndex, nullptr);
		}

		ProxySMC->SetStaticMesh(nullptr);
	};

	const int32 BatchSize =
		bBuildAsync ? FMath::Max(1, MaxPrimBuildPerTick) : FMath::Max(1, PendingBuildPrimNodes.Num());
	struct FPreparedPrimDecision
	{
		const FPrimNodeBuildData* PrimData = nullptr;
		const FCarGeneratedPartRecord* PreviousRecord = nullptr;
		AActor* ExistingProxyActor = nullptr;
		AStaticMeshActor* ExistingProxyStaticActor = nullptr;
		bool bHadExistingProxyActor = false;
		bool bTypeMismatch = false;
	};

	TArray<FPreparedPrimDecision> PreparedPrimDecisions;
	PreparedPrimDecisions.Reserve(BatchSize);

	int32 ProcessedCount = 0;
	while (JobState.PendingApplyIndex < PendingBuildPrimNodes.Num() && ProcessedCount < BatchSize)
	{
		const FPrimNodeBuildData& PrimData = PendingBuildPrimNodes[JobState.PendingApplyIndex++];
		++ProcessedCount;

		FPreparedPrimDecision& Decision = PreparedPrimDecisions.AddDefaulted_GetRef();
		Decision.PrimData = &PrimData;
		Decision.PreviousRecord = FindPreviousRecord(PrimData.PrimPath);
		Decision.ExistingProxyActor = JobState.PendingApplyAttachedProxyActors.FindRef(PrimData.PrimPath);
		if (!Decision.ExistingProxyActor && Decision.PreviousRecord)
		{
			Decision.ExistingProxyActor = ResolveProxyActorByObjectPath(Decision.PreviousRecord->ProxyActorPath);
		}

		if (!IsValid(Decision.ExistingProxyActor))
		{
			Decision.ExistingProxyActor = nullptr;
		}

		Decision.bHadExistingProxyActor = (Decision.ExistingProxyActor != nullptr);
		Decision.ExistingProxyStaticActor = Cast<AStaticMeshActor>(Decision.ExistingProxyActor);
		Decision.bTypeMismatch =
			(Decision.ExistingProxyActor && PrimData.bIsStaticMesh && !Decision.ExistingProxyStaticActor)
			|| (Decision.ExistingProxyStaticActor && !PrimData.bIsStaticMesh);
	}

	for (const FPreparedPrimDecision& Decision : PreparedPrimDecisions)
	{
		if (!Decision.PrimData)
		{
			continue;
		}

		const FPrimNodeBuildData& PrimData = *Decision.PrimData;
		const FCarGeneratedPartRecord* PreviousRecord = Decision.PreviousRecord;
		const bool bHadExistingProxyActor = Decision.bHadExistingProxyActor && !Decision.bTypeMismatch;

		AActor* ProxyActor = Decision.ExistingProxyActor;
		AStaticMeshActor* ProxyStaticActor = Decision.ExistingProxyStaticActor;
		if (Decision.bTypeMismatch && ProxyActor)
		{
			ProxyActor->Destroy();
			ProxyActor = nullptr;
			ProxyStaticActor = nullptr;
		}

		if (!ProxyActor)
		{
			ProxyActor = SpawnProxyActor(PrimData);
			ProxyStaticActor = Cast<AStaticMeshActor>(ProxyActor);
		}

		if (!ProxyActor)
		{
			UE_LOG(
				LogUsdCarFactoryPipelineBuild,
				Warning,
				TEXT("Failed to spawn proxy actor for prim '%s'."),
				*PrimData.PrimPath
			);
			continue;
		}

		UsdCarFactoryPipelineBuild::ApplyProxyTags(ProxyActor, GetProxyActorTag(), MakePrimTag(PrimData.PrimPath), PrimData.PrimPath);
		JobState.PendingApplyAttachedProxyActors.Add(PrimData.PrimPath, ProxyActor);
		JobState.PendingApplyPrimToProxyActor.Add(PrimData.PrimPath, ProxyActor);

		if (PrimData.bIsStaticMesh)
		{
			UStaticMeshComponent* ProxySMC = ProxyStaticActor ? ProxyStaticActor->GetStaticMeshComponent() : nullptr;
			if (!ProxySMC)
			{
				UE_LOG(
					LogUsdCarFactoryPipelineBuild,
					Warning,
					TEXT("Proxy actor '%s' has no static mesh component."),
					*ProxyActor->GetName()
				);
				continue;
			}

			ProxySMC->Modify();
			ProxySMC->SetMobility(EComponentMobility::Movable);

			const UStaticMesh* CurrentStaticMesh = ProxySMC->GetStaticMesh();
			const UStaticMesh* PreviousStaticMesh = PreviousRecord ? PreviousRecord->StaticMesh.Get() : nullptr;

			// Reused proxy actors need an explicit mesh resync when they were previously saved empty
			// or when they still mirror the cached mesh that has since been refreshed from USD.
			bool bShouldSyncStaticMesh = !bHadExistingProxyActor;
			if (!bShouldSyncStaticMesh)
			{
				const bool bCurrentMeshMissingButResolved = (CurrentStaticMesh == nullptr && PrimData.StaticMesh != nullptr);
				const bool bCurrentMeshStillMatchesCachedRecord =
					(CurrentStaticMesh == PreviousStaticMesh) && (CurrentStaticMesh != PrimData.StaticMesh);
				bShouldSyncStaticMesh = bCurrentMeshMissingButResolved || bCurrentMeshStillMatchesCachedRecord;
			}

			if (bShouldSyncStaticMesh)
			{
				ProxySMC->SetStaticMesh(PrimData.StaticMesh);
				ApplyMaterials(PrimData, ProxySMC, PreviousRecord);
			}

			if (!ProxySMC->GetStaticMesh())
			{
				UE_LOG(
					LogUsdCarFactoryPipelineBuild,
					Warning,
					TEXT(
						"Static mesh prim '%s' resolved to proxy actor '%s' without a valid mesh asset. prim_mesh=%s previous_mesh=%s"
					),
					*PrimData.PrimPath,
					*ProxyActor->GetName(),
					*UsdCarFactoryPipelineBuild::GetObjectPath(PrimData.StaticMesh),
					*UsdCarFactoryPipelineBuild::GetObjectPath(PreviousRecord ? PreviousRecord->StaticMesh.Get() : nullptr)
				);
			}
		}
		else if (ProxyStaticActor)
		{
			// Safety fallback: ensure transform-only prim does not keep stale mesh payload.
			ClearProxyMesh(ProxyStaticActor->GetStaticMeshComponent());
		}

		FCarGeneratedPartRecord Record;
		Record.PartKey =
			(PreviousRecord && !PreviousRecord->PartKey.IsNone())
				? PreviousRecord->PartKey
				: FName(*GetPrimLeafName(PrimData.PrimPath));
		Record.PrimPath = PrimData.PrimPath;
		Record.ParentPrimPath = PrimData.ParentPrimPath;
		Record.RelativeTransform = PrimData.RelativeTransform;
		Record.bIsStaticMesh = PrimData.bIsStaticMesh;
		Record.MeshContentHash = PrimData.MeshContentHash;
		Record.CCRComponentName = PreviousRecord ? PreviousRecord->CCRComponentName : NAME_None;
		Record.ProxyActorPath = ProxyActor->GetPathName();

		if (PrimData.bIsStaticMesh)
		{
			if (UStaticMeshComponent* ProxySMC = ProxyStaticActor ? ProxyStaticActor->GetStaticMeshComponent() : nullptr)
			{
				Record.StaticMesh = ProxySMC->GetStaticMesh();

				// Save materials with slot names for mesh-change resilience
				if (UStaticMesh* StaticMesh = ProxySMC->GetStaticMesh())
				{
					const TArray<FStaticMaterial>& StaticMats = StaticMesh->GetStaticMaterials();
					const int32 NumSlots = FMath::Min(StaticMats.Num(), ProxySMC->GetNumMaterials());
					Record.MaterialSlotNames.Reserve(NumSlots);
					Record.Materials.Reserve(NumSlots);
					for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
					{
						const FName SlotName = StaticMats[SlotIndex].MaterialSlotName;
						Record.MaterialSlotNames.Add(SlotName);
						UMaterialInterface* Material = ProxySMC->GetMaterial(SlotIndex);
						Record.Materials.Add(Material);
						if (Material)
						{
							Record.MaterialOverrides.Add(SlotName, Material);
						}
					}
				}
				else
				{
					const int32 NumSlots = ProxySMC->GetNumMaterials();
					Record.Materials.Reserve(NumSlots);
					for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
					{
						Record.Materials.Add(ProxySMC->GetMaterial(SlotIndex));
					}
				}
			}
		}

		JobState.PendingApplyNewRecords[JobState.PendingApplyRecordCount++] = MoveTemp(Record);

		AActor* ParentActor = this;
		USceneComponent* AttachParentComponent = RootAttachComponent;
		if (!PrimData.ParentPrimPath.IsEmpty())
		{
			if (AActor* DirectParent = JobState.PendingApplyPrimToProxyActor.FindRef(PrimData.ParentPrimPath))
			{
				ParentActor = DirectParent;
				AttachParentComponent = UsdCarFactoryPipelineBuild::EnsureSceneRoot(DirectParent);
			}
			else if (AActor* ExistingParent = JobState.PendingApplyAttachedProxyActors.FindRef(PrimData.ParentPrimPath))
			{
				ParentActor = ExistingParent;
				AttachParentComponent = UsdCarFactoryPipelineBuild::EnsureSceneRoot(ExistingParent);
			}
		}

		if (USceneComponent* ChildRoot = UsdCarFactoryPipelineBuild::EnsureSceneRoot(ProxyActor))
		{
			if (AttachParentComponent)
			{
				const bool bNeedsAttach = ChildRoot->GetAttachParent() != AttachParentComponent;
				if (bNeedsAttach)
				{
					ChildRoot->AttachToComponent(AttachParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
				}

				if (bNeedsAttach
					|| !JobState.PendingBuildPlan.UnchangedPrimPaths.Contains(PrimData.PrimPath)
					|| !UsdCarFactoryPipelineBuild::AreTransformsEquivalent(ChildRoot->GetRelativeTransform(), PrimData.RelativeTransform))
				{
					ChildRoot->SetRelativeTransform(PrimData.RelativeTransform);
				}

				continue;
			}
		}

		if (ParentActor && ProxyActor != ParentActor)
		{
			const bool bNeedsAttach = ProxyActor->GetAttachParentActor() != ParentActor;
			if (bNeedsAttach)
			{
				ProxyActor->AttachToActor(ParentActor, FAttachmentTransformRules::KeepWorldTransform);
			}

			if (bNeedsAttach
				|| !JobState.PendingBuildPlan.UnchangedPrimPaths.Contains(PrimData.PrimPath)
				|| !UsdCarFactoryPipelineBuild::AreTransformsEquivalent(ProxyActor->GetActorTransform(), PrimData.WorldTransform))
			{
				ProxyActor->SetActorTransform(PrimData.WorldTransform);
			}
		}
	}

	if (JobState.PendingApplyIndex < PendingBuildPrimNodes.Num())
	{
		UsdCarFactoryPipelineBuild::ScheduleActorContinuation(
			TWeakObjectPtr<AUsdHierarchicalBuildActor>(this),
			&AUsdHierarchicalBuildActor::ProcessApplyBuildBatch
		);
		return;
	}

	JobState.PendingApplyNewRecords.SetNum(JobState.PendingApplyRecordCount, EAllowShrinking::No);
	JobState.PendingBuildPlan.BuildAsset->Parts = MoveTemp(JobState.PendingApplyNewRecords);
	JobState.PendingBuildPlan.BuildAsset->MarkPackageDirty();

	const int32 BuiltPrimCount = JobState.PendingApplyPrimToProxyActor.Num();
	const int32 BuiltMeshCount = JobState.PendingBuildPlan.BuildAsset->Parts.FilterByPredicate(
		[](const FCarGeneratedPartRecord& Record)
		{
			return Record.bIsStaticMesh;
		}
	).Num();
	const int32 BuiltPartRecordCount = JobState.PendingBuildPlan.BuildAsset->Parts.Num();
	const int32 UnchangedCount = JobState.PendingBuildPlan.UnchangedPrimPaths.Num();
	const int32 RemovedCount = JobState.PendingBuildPlan.StalePrimPaths.Num();

	PendingBuildPrimNodes.Reset();
	bIsBuildInProgress = false;
	ResetPendingApplyState();

	UE_LOG(
		LogUsdCarFactoryPipelineBuild,
		Log,
		TEXT("LoadAndBuild completed. Built %d prim nodes, %d total part records (%d mesh records), reused %d unchanged actors, removed %d stale actors. Batch size=%d, source_refresh=%.3fs, diff=%.3fs, apply=%.3fs, total=%.3fs"),
		BuiltPrimCount,
		BuiltPartRecordCount,
		BuiltMeshCount,
		UnchangedCount,
		RemovedCount,
		BatchSize,
		ActiveSourceRefreshSeconds,
		ActiveDiffSeconds,
		FPlatformTime::Seconds() - ActiveApplySeconds,
		FPlatformTime::Seconds() - ActiveBuildStartSeconds
	);
#endif
}

void AUsdHierarchicalBuildActor::ResetPendingApplyState()
{
	if (UUsdCarFactoryBuildSubsystem* BuildSubsystem = GetBuildSubsystem())
	{
		BuildSubsystem->GetJobState(*this).ResetApplyState();
	}
}

TMap<FString, AActor*> AUsdHierarchicalBuildActor::GatherAttachedProxyActors() const
{
	TArray<AActor*> IgnoredDuplicateProxyActors;
	return GatherAttachedProxyActors(IgnoredDuplicateProxyActors);
}

TMap<FString, AActor*> AUsdHierarchicalBuildActor::GatherAttachedProxyActors(TArray<AActor*>& OutDuplicateProxyActors) const
{
	TMap<FString, AActor*> Result;
	OutDuplicateProxyActors.Reset();
	const FName ProxyTag = GetProxyActorTag();
	TMap<FName, FString> PrimTagToPath;
	if (GeneratedDataAsset)
	{
		PrimTagToPath.Reserve(GeneratedDataAsset->Parts.Num());
		for (const FCarGeneratedPartRecord& Record : GeneratedDataAsset->Parts)
		{
			if (!Record.PrimPath.IsEmpty())
			{
				PrimTagToPath.Add(MakePrimTag(Record.PrimPath), Record.PrimPath);
			}
		}
	}

	auto ResolvePrimPath = [&PrimTagToPath](AActor* Actor) -> FString
	{
		if (!Actor)
		{
			return FString();
		}

		FString PrimPath = UsdCarFactoryPipelineBuild::ExtractExactPrimPathFromActor(Actor);
		if (!PrimPath.IsEmpty())
		{
			return PrimPath;
		}

		for (const FName& Tag : Actor->Tags)
		{
			if (const FString* PrimPathFromTag = PrimTagToPath.Find(Tag))
			{
				return *PrimPathFromTag;
			}
		}

		return FString();
	};

	auto CollectFromRoot = [this, &Result, &ResolvePrimPath, &OutDuplicateProxyActors, ProxyTag](const AActor* RootActor)
	{
		if (!RootActor)
		{
			return;
		}

		TArray<AActor*> AttachedActors;
		RootActor->GetAttachedActors(AttachedActors, true, true);
		for (AActor* AttachedActor : AttachedActors)
		{
			if (!IsValid(AttachedActor) || !AttachedActor->ActorHasTag(ProxyTag))
			{
				continue;
			}

			const FString PrimPath = ResolvePrimPath(AttachedActor);
			if (PrimPath.IsEmpty())
			{
				// Proxy actor without resolvable prim path is considered stale/orphaned and should be cleaned.
				OutDuplicateProxyActors.AddUnique(AttachedActor);
				continue;
			}

			if (AActor** ExistingActor = Result.Find(PrimPath))
			{
				if (*ExistingActor != AttachedActor)
				{
					OutDuplicateProxyActors.AddUnique(AttachedActor);
				}
				continue;
			}

			Result.Add(PrimPath, AttachedActor);
		}
	};

	CollectFromRoot(this);

	return Result;
}

AActor* AUsdHierarchicalBuildActor::ResolveProxyActorByObjectPath(const FString& ObjectPath) const
{
	if (ObjectPath.IsEmpty())
	{
		return nullptr;
	}

	return FindObject<AActor>(nullptr, *ObjectPath);
}

AActor* AUsdHierarchicalBuildActor::SpawnProxyActor(const FPrimNodeBuildData& PrimData)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (!UsdCarFactoryPipelineBuild::IsSupportedProxyPrimType(PrimData.PrimTypeName, PrimData.bIsStaticMesh))
	{
		UE_LOG(
			LogUsdCarFactoryPipelineBuild,
			Warning,
			TEXT("Skip spawning proxy actor for unsupported prim type '%s' at '%s'. Only mesh/xform are supported."),
			*PrimData.PrimTypeName,
			*PrimData.PrimPath
		);
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.ObjectFlags |= RF_Transactional;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TSubclassOf<AActor> ProxyActorClass = ResolveProxyActorClass(PrimData);

	AActor* NewActor = nullptr;
	if (PrimData.bIsStaticMesh)
	{
		TSubclassOf<AStaticMeshActor> SpawnMeshClass = AStaticMeshActor::StaticClass();
		if (ProxyActorClass)
		{
			UClass* MappedClass = ProxyActorClass.Get();
			if (MappedClass && MappedClass->IsChildOf(AStaticMeshActor::StaticClass()))
			{
				SpawnMeshClass = MappedClass;
			}
			else
			{
				UE_LOG(
					LogUsdCarFactoryPipelineBuild,
					Warning,
					TEXT("TypeName '%s' mapped class '%s' is not AStaticMeshActor-derived. Fallback to AStaticMeshActor."),
					*PrimData.PrimTypeName,
					MappedClass ? *MappedClass->GetName() : TEXT("None")
				);
			}
		}
		NewActor = World->SpawnActor<AStaticMeshActor>(SpawnMeshClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}
	else
	{
		TSubclassOf<AActor> SpawnNodeClass = ProxyActorClass;
		if (!SpawnNodeClass)
		{
			SpawnNodeClass = AActor::StaticClass();
		}
		NewActor = World->SpawnActor<AActor>(SpawnNodeClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (!NewActor)
	{
		return nullptr;
	}

	if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(NewActor))
	{
		if (UStaticMeshComponent* StaticMeshComponent = StaticMeshActor->GetStaticMeshComponent())
		{
			StaticMeshComponent->SetMobility(EComponentMobility::Movable);
		}
	}

#if WITH_EDITOR
	NewActor->SetActorLabel(MakeProxyActorLabel(PrimData.PrimPath));
#endif

	UsdCarFactoryPipelineBuild::ApplyProxyTags(NewActor, GetProxyActorTag(), MakePrimTag(PrimData.PrimPath), PrimData.PrimPath);

	// Keep newly spawned proxies anchored to this build actor immediately.
	if (USceneComponent* ParentRoot = UsdCarFactoryPipelineBuild::EnsureSceneRoot(this))
	{
		if (USceneComponent* ChildRoot = UsdCarFactoryPipelineBuild::EnsureSceneRoot(NewActor))
		{
			ChildRoot->AttachToComponent(ParentRoot, FAttachmentTransformRules::KeepRelativeTransform);
			ChildRoot->SetRelativeTransform(PrimData.RelativeTransform);
		}
	}

	return NewActor;
}

TSubclassOf<AActor> AUsdHierarchicalBuildActor::ResolveProxyActorClass(const FPrimNodeBuildData& PrimData) const
{
	if (!UsdCarFactoryPipelineBuild::IsSupportedProxyPrimType(PrimData.PrimTypeName, PrimData.bIsStaticMesh))
	{
		return nullptr;
	}

	if (PrimData.bIsStaticMesh)
	{
		if (UClass* MeshProxyClass = StaticMeshProxyActorClass.Get())
		{
			return MeshProxyClass;
		}

		return AStaticMeshActor::StaticClass();
	}

	if (UsdCarFactoryPipelineBuild::IsTransformPrimType(PrimData.PrimTypeName))
	{
		if (UClass* TransformProxyClass = TransformProxyActorClass.Get())
		{
			return TransformProxyClass;
		}

		return AActor::StaticClass();
	}

	return nullptr;
}

void AUsdHierarchicalBuildActor::EnsureDefaultProxyActorClasses()
{
	if (!StaticMeshProxyActorClass)
	{
		StaticMeshProxyActorClass = AStaticMeshActor::StaticClass();
	}

	if (!TransformProxyActorClass)
	{
		TransformProxyActorClass = AActor::StaticClass();
	}
}

void AUsdHierarchicalBuildActor::MigrateLegacyTypeNameActorClassMap()
{
	if (TypeNameToActorClassMap.IsEmpty())
	{
		return;
	}

	bool bMigratedAnyValue = false;
	for (const TPair<FString, TSubclassOf<AActor>>& Pair : TypeNameToActorClassMap)
	{
		if (Pair.Key.Equals(TEXT("mesh"), ESearchCase::IgnoreCase))
		{
			UClass* MappedClass = Pair.Value.Get();
			if (MappedClass && MappedClass->IsChildOf(AStaticMeshActor::StaticClass()))
			{
				StaticMeshProxyActorClass = MappedClass;
				bMigratedAnyValue = true;
			}
			else
			{
				UE_LOG(
					LogUsdCarFactoryPipelineBuild,
					Warning,
					TEXT("Ignoring deprecated mesh mapping '%s' because it is not AStaticMeshActor-derived."),
					MappedClass ? *MappedClass->GetName() : TEXT("None")
				);
			}
		}
		else if (Pair.Key.Equals(TEXT("xform"), ESearchCase::IgnoreCase))
		{
			if (Pair.Value)
			{
				TransformProxyActorClass = Pair.Value;
				bMigratedAnyValue = true;
			}
		}
		else
		{
			UE_LOG(
				LogUsdCarFactoryPipelineBuild,
				Warning,
				TEXT("Ignoring deprecated TypeNameToActorClassMap entry '%s'. Only mesh/xform are supported."),
				*Pair.Key
			);
		}
	}

	if (bMigratedAnyValue)
	{
		Modify();
		TypeNameToActorClassMap.Reset();
	}

	EnsureDefaultProxyActorClasses();
}

void AUsdHierarchicalBuildActor::ApplyMaterials(
	const FPrimNodeBuildData& PrimData,
	UStaticMeshComponent* TargetComponent,
	const FCarGeneratedPartRecord* PreviousRecord
) const
{
	if (!TargetComponent)
	{
		return;
	}

	// Get current mesh's slot names for matching
	TArray<FName> CurrentSlotNames;
	if (UStaticMesh* StaticMesh = TargetComponent->GetStaticMesh())
	{
		const TArray<FStaticMaterial>& StaticMats = StaticMesh->GetStaticMaterials();
		CurrentSlotNames.Reserve(StaticMats.Num());
		for (const FStaticMaterial& Mat : StaticMats)
		{
			CurrentSlotNames.Add(Mat.MaterialSlotName);
		}
	}

	const int32 NumSlots = TargetComponent->GetNumMaterials();
	TArray<TObjectPtr<UMaterialInterface>> FinalMaterials;
	FinalMaterials.SetNum(NumSlots);

	// First, apply source materials from current USD (by slot name if available)
	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		if (PrimData.SourceMaterials.IsValidIndex(SlotIndex))
		{
			FinalMaterials[SlotIndex] = PrimData.SourceMaterials[SlotIndex];
		}
	}

	// Then, override with previous record's materials (by slot name for mesh-change resilience)
	if (PreviousRecord)
	{
		// Build a map from slot name to material from previous record
		TMap<FName, TObjectPtr<UMaterialInterface>> PreviousMaterialsByName;
		for (int32 SavedIndex = 0; SavedIndex < PreviousRecord->MaterialSlotNames.Num() && SavedIndex < PreviousRecord->Materials.Num(); ++SavedIndex)
		{
			if (PreviousRecord->Materials[SavedIndex])
			{
				PreviousMaterialsByName.Add(PreviousRecord->MaterialSlotNames[SavedIndex], PreviousRecord->Materials[SavedIndex]);
			}
		}
		// Also use MaterialOverrides if available (new system)
		for (const auto& Pair : PreviousRecord->MaterialOverrides)
		{
			PreviousMaterialsByName.Add(Pair.Key, Pair.Value);
		}

		// Apply to current slots by name matching
		for (int32 SlotIndex = 0; SlotIndex < CurrentSlotNames.Num() && SlotIndex < NumSlots; ++SlotIndex)
		{
			const FName& SlotName = CurrentSlotNames[SlotIndex];
			if (TObjectPtr<UMaterialInterface>* Found = PreviousMaterialsByName.Find(SlotName))
			{
				FinalMaterials[SlotIndex] = *Found;
			}
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < NumSlots; ++SlotIndex)
	{
		TargetComponent->SetMaterial(SlotIndex, FinalMaterials[SlotIndex]);
	}
}

const FCarGeneratedPartRecord* AUsdHierarchicalBuildActor::FindPreviousRecord(const FString& PrimPath) const
{
	if (!GeneratedDataAsset)
	{
		return nullptr;
	}

	if (const int32* FoundIndex = PreviousPartIndexByPrimPath.Find(PrimPath))
	{
		if (GeneratedDataAsset->Parts.IsValidIndex(*FoundIndex))
		{
			return &GeneratedDataAsset->Parts[*FoundIndex];
		}
	}

	// Fallback for unexpected states where index cache was not prepared yet.
	for (const FCarGeneratedPartRecord& Record : GeneratedDataAsset->Parts)
	{
		if (Record.PrimPath == PrimPath)
		{
			return &Record;
		}
	}

	return nullptr;
}

FString AUsdHierarchicalBuildActor::GetPrimLeafName(const FString& PrimPath)
{
	if (PrimPath.IsEmpty())
	{
		return FString();
	}

	int32 LastSlashIndex = INDEX_NONE;
	if (PrimPath.FindLastChar(TEXT('/'), LastSlashIndex))
	{
		return PrimPath.Mid(LastSlashIndex + 1);
	}

	return PrimPath;
}

FString AUsdHierarchicalBuildActor::MakeProxyActorLabel(const FString& PrimPath)
{
	FString Label = FString::Printf(TEXT("USD_%s"), *GetPrimLeafName(PrimPath));
	Label.ReplaceInline(TEXT("/"), TEXT("_"));
	Label.ReplaceInline(TEXT("."), TEXT("_"));
	Label.ReplaceInline(TEXT(":"), TEXT("_"));
	return Label;
}

int32 AUsdHierarchicalBuildActor::GetPrimDepth(const FString& PrimPath)
{
	int32 Depth = 0;
	for (const TCHAR Char : PrimPath)
	{
		if (Char == TEXT('/'))
		{
			++Depth;
		}
	}
	return Depth;
}

FName AUsdHierarchicalBuildActor::GetProxyActorTag() const
{
	return FName(*FString::Printf(TEXT("UsdCarFactoryPipeline.Proxy.%s"), *GetName()));
}

FName AUsdHierarchicalBuildActor::MakePrimTag(const FString& PrimPath) const
{
	FString Sanitized = PrimPath;
	Sanitized.ReplaceInline(TEXT("/"), TEXT("_"));
	Sanitized.ReplaceInline(TEXT("."), TEXT("_"));
	Sanitized.ReplaceInline(TEXT(":"), TEXT("_"));
	return FName(*FString::Printf(TEXT("UsdCarFactoryPipeline.Prim.%s"), *Sanitized));
}
