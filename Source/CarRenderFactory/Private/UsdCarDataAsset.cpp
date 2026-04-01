// Copyright Epic Games, Inc. All Rights Reserved.
// 版权所有 Epic Games, Inc. 保留所有权利。

#include "UsdCarDataAsset.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Misc/MessageDialog.h"
#include "Misc/DataValidation.h"
#endif

FString UUsdCarDataAsset::GetResolvedUsdPath() const
{
    if (!bUseRelativePath)
    {
        return UsdFilePath;
    }

    // 处理相对路径
    FString BasePath = RelativePathBase;
    if (BasePath.IsEmpty())
    {
        // 默认使用项目内容目录
        BasePath = FPaths::ProjectContentDir();
    }

    return FPaths::Combine(BasePath, UsdFilePath);
}

bool UUsdCarDataAsset::FindPartMappingByPrimPath(const FString& PrimPath, FUsdPartMappingEntry& OutMapping) const
{
    for (const FUsdPartMappingEntry& Entry : PartMappings)
    {
        if (Entry.PrimPath == PrimPath)
        {
            OutMapping = Entry;
            return true;
        }
    }
    return false;
}

bool UUsdCarDataAsset::FindPartMappingByPartType(const FString& PartType, FUsdPartMappingEntry& OutMapping) const
{
    for (const FUsdPartMappingEntry& Entry : PartMappings)
    {
        if (Entry.PartType == PartType)
        {
            OutMapping = Entry;
            return true;
        }
    }
    return false;
}

int32 UUsdCarDataAsset::FindPartMappingIndexByPrimPath(const FString& PrimPath) const
{
    for (int32 i = 0; i < PartMappings.Num(); ++i)
    {
        if (PartMappings[i].PrimPath == PrimPath)
        {
            return i;
        }
    }
    return -1;
}

int32 UUsdCarDataAsset::FindPartMappingIndexByPartType(const FString& PartType) const
{
    for (int32 i = 0; i < PartMappings.Num(); ++i)
    {
        if (PartMappings[i].PartType == PartType)
        {
            return i;
        }
    }
    return -1;
}

UMaterialInterface* UUsdCarDataAsset::GetMaterialForVariant(const FString& VariantSetName, const FString& VariantName, const FString& PartType) const
{
    // 首先尝试查找精确匹配（包含部件类型）
    for (const FUsdVariantMaterialEntry& Entry : VariantMaterialMappings)
    {
        if (Entry.VariantSetName == VariantSetName &&
            Entry.VariantName == VariantName &&
            Entry.PartType == PartType)
        {
            return Entry.Material;
        }
    }

    // 然后查找通用材质（部件类型为空）
    for (const FUsdVariantMaterialEntry& Entry : VariantMaterialMappings)
    {
        if (Entry.VariantSetName == VariantSetName &&
            Entry.VariantName == VariantName &&
            Entry.PartType.IsEmpty())
        {
            return Entry.Material;
        }
    }

    return nullptr;
}

bool UUsdCarDataAsset::ValidateConfiguration(TArray<FString>& OutErrors) const
{
    bool bIsValid = true;

    // 验证 USD 文件路径
    if (UsdFilePath.IsEmpty())
    {
        OutErrors.Add(TEXT("USD file path is empty"));
        bIsValid = false;
    }
    else
    {
        FString ResolvedPath = GetResolvedUsdPath();
        // 注意: 文件存在性检查在某些情况下可能不适用（如远程路径）
        // 仅在绝对路径时检查
        if (FPaths::IsRelative(ResolvedPath) == false)
        {
            if (!FPaths::FileExists(ResolvedPath))
            {
                OutErrors.Add(FString::Printf(TEXT("USD file does not exist: %s"), *ResolvedPath));
                bIsValid = false;
            }
        }
    }

    // 验证部件映射
    TSet<FString> SeenPrimPaths;
    TSet<FString> SeenPartTypes;

    for (int32 i = 0; i < PartMappings.Num(); ++i)
    {
        const FUsdPartMappingEntry& Entry = PartMappings[i];

        if (Entry.PrimPath.IsEmpty() && Entry.PartType.IsEmpty())
        {
            OutErrors.Add(FString::Printf(TEXT("Part mapping[%d] has empty PrimPath and PartType"), i));
            bIsValid = false;
        }

        if (!Entry.PrimPath.IsEmpty() && SeenPrimPaths.Contains(Entry.PrimPath))
        {
            OutErrors.Add(FString::Printf(TEXT("Duplicate PrimPath in part mapping: %s"), *Entry.PrimPath));
            bIsValid = false;
        }
        SeenPrimPaths.Add(Entry.PrimPath);

        if (!Entry.PartType.IsEmpty() && SeenPartTypes.Contains(Entry.PartType))
        {
            OutErrors.Add(FString::Printf(TEXT("Duplicate PartType in part mapping: %s"), *Entry.PartType));
            bIsValid = false;
        }
        SeenPartTypes.Add(Entry.PartType);
    }

    return bIsValid;
}

#if WITH_EDITOR
EDataValidationResult UUsdCarDataAsset::IsDataValid(FDataValidationContext& Context) const
{
    TArray<FString> Errors;
    if (!ValidateConfiguration(Errors))
    {
        for (const FString& Error : Errors)
        {
            Context.AddError(FText::FromString(Error));
        }
        return EDataValidationResult::Invalid;
    }
    return EDataValidationResult::Valid;
}
#endif
