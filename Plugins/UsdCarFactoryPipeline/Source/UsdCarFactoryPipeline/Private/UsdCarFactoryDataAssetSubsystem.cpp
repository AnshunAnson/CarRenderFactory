#include "UsdCarFactoryDataAssetSubsystem.h"

#include "Actors/UsdHierarchicalBuildActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Data/CarGeneratedAssemblyDataAsset.h"
#include "Misc/Crc.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "USDAssetCache3.h"

namespace
{
	static const TCHAR* GeneratedAssetFolder = TEXT("/Game/CarRenderFactory/Generated");
	static const TCHAR* GeneratedAssetBaseName = TEXT("DA_UsdGeneratedAssembly");
	static const TCHAR* AssetCacheBaseName = TEXT("DA_UsdAssetCache");

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
		return SafeName.IsEmpty() ? TEXT("UnknownUsd") : SafeName;
	}

	static FString ExtractSourceIdentifier(const FString& UsdFilePath)
	{
		if (UsdFilePath.IsEmpty())
		{
			return TEXT("UnknownUsd_00000000");
		}

		const FString NormalizedPath = FPaths::ConvertRelativePathToFull(UsdFilePath);
		const FString BaseName = NormalizedPath.IsEmpty() ? TEXT("UnknownUsd") : FPaths::GetBaseFilename(NormalizedPath, false);
		FString SafeName = MakeSafePackageSegment(BaseName);
		if (SafeName.Len() > 48)
		{
			SafeName = SafeName.Left(48);
		}

		return FString::Printf(TEXT("%s_%08X"), *SafeName, FCrc::StrCrc32(*NormalizedPath));
	}

	static AUsdHierarchicalBuildActor* ResolveTargetActor(const FUsdCarFactoryBuildInputs& Inputs)
	{
		return Inputs.TargetActor.Get();
	}

	static FString ResolveSourcePath(const FUsdCarFactoryBuildInputs& Inputs, const AUsdHierarchicalBuildActor* Actor)
	{
		if (Actor && !Actor->SourceUsdFile.FilePath.IsEmpty())
		{
			return FPaths::ConvertRelativePathToFull(Actor->SourceUsdFile.FilePath);
		}

		if (!Inputs.SourceUsdFile.FilePath.IsEmpty())
		{
			return FPaths::ConvertRelativePathToFull(Inputs.SourceUsdFile.FilePath);
		}

		return FString();
	}

	static bool ShouldResetStandaloneAssetDirectory(const UUsdAssetCache3* AssetCache, const FString& DesiredAssetDirectory)
	{
		if (!AssetCache)
		{
			return false;
		}

		const FString& CurrentAssetDirectory = AssetCache->AssetDirectory.Path;
		if (CurrentAssetDirectory.IsEmpty())
		{
			return true;
		}

		if (CurrentAssetDirectory == GetTransientPackage()->GetPathName())
		{
			return true;
		}

		if (!FPackageName::IsValidLongPackageName(CurrentAssetDirectory))
		{
			return true;
		}

		const FString DefaultUsdAssetsDirectory = FPaths::Combine(DesiredAssetDirectory, TEXT("UsdAssets"));
		return CurrentAssetDirectory == DefaultUsdAssetsDirectory;
	}

	static void NormalizeStandaloneUsdAssetCache(UUsdAssetCache3* AssetCache)
	{
		if (!AssetCache || AssetCache->GetOutermost() == GetTransientPackage())
		{
			return;
		}

		const FString DesiredAssetDirectory = FPackageName::GetLongPackagePath(AssetCache->GetOutermost()->GetName());
		if (!FPackageName::IsValidLongPackageName(DesiredAssetDirectory))
		{
			return;
		}

		bool bCacheMutated = false;
		if (ShouldResetStandaloneAssetDirectory(AssetCache, DesiredAssetDirectory))
		{
			AssetCache->Modify();
			AssetCache->AssetDirectory.Path = DesiredAssetDirectory;
			bCacheMutated = true;
		}

		const FString TransientPackagePath = GetTransientPackage()->GetPathName();
		const TMap<FString, FSoftObjectPath> TrackedAssets = AssetCache->GetAllTrackedAssets();
		for (const TPair<FString, FSoftObjectPath>& Pair : TrackedAssets)
		{
			const FString AssetPathString = Pair.Value.ToString();
			if (!AssetPathString.StartsWith(TransientPackagePath))
			{
				continue;
			}

			AssetCache->StopTrackingAsset(Pair.Key);
			bCacheMutated = true;
		}

		AssetCache->RescanAssetDirectory();

		if (bCacheMutated)
		{
			AssetCache->MarkPackageDirty();
			if (UPackage* Package = AssetCache->GetOutermost())
			{
				Package->MarkPackageDirty();
			}
		}
	}
}

bool UUsdCarFactoryDataAssetSubsystem::ResolveOrCreateBuildAssets(
	const FUsdCarFactoryBuildInputs& Inputs,
	UCarGeneratedAssemblyDataAsset*& OutGeneratedDataAsset,
	UUsdAssetCache3*& OutUsdAssetCache,
	FString& OutMessage
)
{
#if WITH_EDITOR
	OutGeneratedDataAsset = ResolveOrCreateGeneratedDataAsset(Inputs, OutMessage);
	if (!OutGeneratedDataAsset)
	{
		OutUsdAssetCache = nullptr;
		return false;
	}

	OutUsdAssetCache = ResolveOrCreateUsdAssetCacheAsset(Inputs, OutGeneratedDataAsset, OutMessage);
	if (!OutUsdAssetCache)
	{
		return false;
	}

	if (OutGeneratedDataAsset->UsdAssetCache != OutUsdAssetCache)
	{
		OutGeneratedDataAsset->Modify();
		OutGeneratedDataAsset->UsdAssetCache = OutUsdAssetCache;
		OutGeneratedDataAsset->MarkPackageDirty();
	}

	if (AUsdHierarchicalBuildActor* TargetActor = ResolveTargetActor(Inputs))
	{
		TargetActor->GeneratedDataAsset = OutGeneratedDataAsset;
	}

	OutMessage = TEXT("Resolved GeneratedDataAsset and standalone UUsdAssetCache3.");
	return true;
#else
	OutGeneratedDataAsset = nullptr;
	OutUsdAssetCache = nullptr;
	OutMessage = TEXT("Build assets can only be resolved in editor.");
	return false;
#endif
}

UCarGeneratedAssemblyDataAsset* UUsdCarFactoryDataAssetSubsystem::ResolveOrCreateGeneratedDataAsset(
	const FUsdCarFactoryBuildInputs& Inputs,
	FString& OutMessage
)
{
#if WITH_EDITOR
	AUsdHierarchicalBuildActor* TargetActor = ResolveTargetActor(Inputs);
	if (TargetActor && TargetActor->GeneratedDataAsset)
	{
		return TargetActor->GeneratedDataAsset;
	}

	if (Inputs.GeneratedDataAsset)
	{
		if (TargetActor)
		{
			TargetActor->GeneratedDataAsset = Inputs.GeneratedDataAsset;
		}
		return Inputs.GeneratedDataAsset;
	}

	const FString NormalizedSourceUsdPath = ResolveSourcePath(Inputs, TargetActor);
	const FString SourceIdentifier = ExtractSourceIdentifier(NormalizedSourceUsdPath);
	const FString AssetFolderPath = FString::Printf(TEXT("%s/%s"), GeneratedAssetFolder, *SourceIdentifier);
	if (!FPackageName::IsValidLongPackageName(AssetFolderPath))
	{
		OutMessage = FString::Printf(TEXT("Invalid generated asset folder: %s"), *AssetFolderPath);
		return nullptr;
	}

	const FString PackagePath = FString::Printf(TEXT("%s/%s"), *AssetFolderPath, GeneratedAssetBaseName);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, GeneratedAssetBaseName);

	if (UCarGeneratedAssemblyDataAsset* ExistingAsset = LoadObject<UCarGeneratedAssemblyDataAsset>(nullptr, *ObjectPath))
	{
		if (TargetActor)
		{
			TargetActor->GeneratedDataAsset = ExistingAsset;
		}
		return ExistingAsset;
	}

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		OutMessage = FString::Printf(TEXT("Failed to create GeneratedDataAsset package: %s"), *PackagePath);
		return nullptr;
	}

	UCarGeneratedAssemblyDataAsset* BuildAsset = NewObject<UCarGeneratedAssemblyDataAsset>(
		Package,
		UCarGeneratedAssemblyDataAsset::StaticClass(),
		*FString(GeneratedAssetBaseName),
		RF_Public | RF_Standalone | RF_Transactional
	);
	if (!BuildAsset)
	{
		OutMessage = TEXT("Failed to create GeneratedDataAsset object.");
		return nullptr;
	}

	FAssetRegistryModule::AssetCreated(BuildAsset);
	BuildAsset->MarkPackageDirty();
	Package->MarkPackageDirty();

	if (TargetActor)
	{
		TargetActor->GeneratedDataAsset = BuildAsset;
	}

	return BuildAsset;
#else
	OutMessage = TEXT("GeneratedDataAsset can only be created in editor.");
	return nullptr;
#endif
}

UUsdAssetCache3* UUsdCarFactoryDataAssetSubsystem::ResolveOrCreateUsdAssetCacheAsset(
	const FUsdCarFactoryBuildInputs& Inputs,
	UCarGeneratedAssemblyDataAsset* BuildAsset,
	FString& OutMessage
)
{
#if WITH_EDITOR
	if (!BuildAsset)
	{
		OutMessage = TEXT("BuildAsset is null.");
		return nullptr;
	}

	if (UUsdAssetCache3* ExistingCache = BuildAsset->UsdAssetCache.Get())
	{
		const bool bIsStandaloneAsset =
			ExistingCache->HasAnyFlags(RF_Public | RF_Standalone)
			&& ExistingCache->GetOutermost() != BuildAsset->GetOutermost();
		if (bIsStandaloneAsset)
		{
			return ExistingCache;
		}
	}

	FString AssetFolderPath;
	const FString NormalizedSourceUsdPath = ResolveSourcePath(Inputs, ResolveTargetActor(Inputs));
	if (!NormalizedSourceUsdPath.IsEmpty())
	{
		AssetFolderPath = FString::Printf(TEXT("%s/%s"), GeneratedAssetFolder, *ExtractSourceIdentifier(NormalizedSourceUsdPath));
	}
	else
	{
		AssetFolderPath = FPackageName::GetLongPackagePath(BuildAsset->GetOutermost()->GetName());
	}

	if (!FPackageName::IsValidLongPackageName(AssetFolderPath))
	{
		OutMessage = FString::Printf(TEXT("Invalid asset cache folder: %s"), *AssetFolderPath);
		return nullptr;
	}

	const FString PackagePath = FString::Printf(TEXT("%s/%s"), *AssetFolderPath, AssetCacheBaseName);
	const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackagePath, AssetCacheBaseName);
	if (UUsdAssetCache3* ExistingAssetCache = LoadObject<UUsdAssetCache3>(nullptr, *ObjectPath))
	{
		NormalizeStandaloneUsdAssetCache(ExistingAssetCache);
		return ExistingAssetCache;
	}

	UPackage* Package = CreatePackage(*PackagePath);
	if (!Package)
	{
		OutMessage = FString::Printf(TEXT("Failed to create asset cache package: %s"), *PackagePath);
		return nullptr;
	}

	UUsdAssetCache3* SourceCache = BuildAsset->UsdAssetCache.Get();
	UUsdAssetCache3* AssetCache = nullptr;
	if (SourceCache)
	{
		AssetCache = DuplicateObject<UUsdAssetCache3>(SourceCache, Package, *FString(AssetCacheBaseName));
		if (AssetCache)
		{
			AssetCache->SetFlags(RF_Public | RF_Standalone | RF_Transactional);
		}
	}
	else
	{
		AssetCache = NewObject<UUsdAssetCache3>(
			Package,
			UUsdAssetCache3::StaticClass(),
			*FString(AssetCacheBaseName),
			RF_Public | RF_Standalone | RF_Transactional
		);
	}

	if (!AssetCache)
	{
		OutMessage = TEXT("Failed to create standalone UUsdAssetCache3 asset.");
		return nullptr;
	}

	NormalizeStandaloneUsdAssetCache(AssetCache);

	FAssetRegistryModule::AssetCreated(AssetCache);
	AssetCache->MarkPackageDirty();
	Package->MarkPackageDirty();
	return AssetCache;
#else
	OutMessage = TEXT("Asset cache can only be created in editor.");
	return nullptr;
#endif
}
