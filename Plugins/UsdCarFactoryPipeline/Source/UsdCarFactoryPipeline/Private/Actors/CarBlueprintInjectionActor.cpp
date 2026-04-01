#include "Actors/CarBlueprintInjectionActor.h"

#include "Components/StaticMeshComponent.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Materials/MaterialInterface.h"

DEFINE_LOG_CATEGORY_STATIC(LogUsdCarFactoryPipelineInject, Log, All);

ACarBlueprintInjectionActor::ACarBlueprintInjectionActor()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ACarBlueprintInjectionActor::InjectToSelectedCCR()
{
#if WITH_EDITOR
	if (!GeneratedDataAsset)
	{
		UE_LOG(LogUsdCarFactoryPipelineInject, Error, TEXT("GeneratedDataAsset is null."));
		return;
	}

	if (!GEditor)
	{
		UE_LOG(LogUsdCarFactoryPipelineInject, Error, TEXT("GEditor is null."));
		return;
	}

	USelection* Selection = GEditor->GetSelectedActors();
	if (!Selection || Selection->Num() == 0)
	{
		UE_LOG(LogUsdCarFactoryPipelineInject, Error, TEXT("No actor selected. Select one CCR actor instance first."));
		return;
	}

	AActor* TargetCCRActor = nullptr;
	for (FSelectionIterator It(*Selection); It; ++It)
	{
		TargetCCRActor = Cast<AActor>(*It);
		if (TargetCCRActor)
		{
			break;
		}
	}

	if (!TargetCCRActor)
	{
		UE_LOG(LogUsdCarFactoryPipelineInject, Error, TEXT("Selected object is not a valid actor."));
		return;
	}

	if (ExpectedCCRClass && !TargetCCRActor->IsA(ExpectedCCRClass))
	{
		UE_LOG(
			LogUsdCarFactoryPipelineInject,
			Error,
			TEXT("Selected actor '%s' does not match ExpectedCCRClass '%s'."),
			*TargetCCRActor->GetName(),
			*ExpectedCCRClass->GetName()
		);
		return;
	}

	TInlineComponentArray<UStaticMeshComponent*> MeshComponents(TargetCCRActor);
	TMap<FName, UStaticMeshComponent*> NameToComponent;
	for (UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent)
		{
			NameToComponent.Add(MeshComponent->GetFName(), MeshComponent);
		}
	}

	int32 InjectedCount = 0;
	TMap<FString, const FCarUsdProxyActorState*> VariantOverrides;
	TArray<FCarUsdProxyActorState> LegacyConvertedStates;
	if (VariantDataAsset)
	{
		const FCarUsdVariantRecord* ActiveVariant = VariantDataAsset->FindVariant(VariantDataAsset->ActiveVariantName);
		if (ActiveVariant)
		{
			for (const FCarUsdProxyActorState& ProxyVariant : ActiveVariant->ProxyActorStates)
			{
				VariantOverrides.Add(ProxyVariant.PrimPath, &ProxyVariant);
			}
		}
	}
	else if (const FCarGeneratedMaterialVariantRecord* LegacyActiveVariant = GeneratedDataAsset->MaterialVariants.FindByPredicate(
				 [this](const FCarGeneratedMaterialVariantRecord& Variant)
				 {
					 return Variant.VariantName == GeneratedDataAsset->ActiveVariantName;
				 }
			 ))
	{
		LegacyConvertedStates.Reserve(LegacyActiveVariant->ProxyActorConfigs.Num());
		for (const FCarGeneratedProxyActorVariantRecord& LegacyProxyVariant : LegacyActiveVariant->ProxyActorConfigs)
		{
			FCarUsdProxyActorState TempState;
			TempState.PrimPath = LegacyProxyVariant.PrimPath;
			TempState.StaticMesh = LegacyProxyVariant.StaticMesh;
			TempState.Materials = LegacyProxyVariant.Materials;
			LegacyConvertedStates.Add(MoveTemp(TempState));
		}
		for (const FCarUsdProxyActorState& LegacyState : LegacyConvertedStates)
		{
			VariantOverrides.Add(LegacyState.PrimPath, &LegacyState);
		}
	}

	for (const FCarGeneratedPartRecord& PartRecord : GeneratedDataAsset->Parts)
	{
		if (PartRecord.CCRComponentName.IsNone())
		{
			UE_LOG(LogUsdCarFactoryPipelineInject, Warning, TEXT("Part '%s' has empty CCRComponentName. Skipped."), *PartRecord.PrimPath);
			continue;
		}

		UStaticMeshComponent* TargetComponent = NameToComponent.FindRef(PartRecord.CCRComponentName);
		if (!TargetComponent)
		{
			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (MeshComponent && MeshComponent->GetName().Equals(PartRecord.CCRComponentName.ToString(), ESearchCase::IgnoreCase))
				{
					TargetComponent = MeshComponent;
					break;
				}
			}
		}

		if (!TargetComponent)
		{
			UE_LOG(
				LogUsdCarFactoryPipelineInject,
				Warning,
				TEXT("CCR component '%s' not found on actor '%s'."),
				*PartRecord.CCRComponentName.ToString(),
				*TargetCCRActor->GetName()
			);
			continue;
		}

		TargetComponent->Modify();

		UStaticMesh* MeshToInject = PartRecord.StaticMesh;
		TArray<TObjectPtr<UMaterialInterface>> MaterialsToInject = PartRecord.Materials;
		if (const FCarUsdProxyActorState* VariantOverride = VariantOverrides.FindRef(PartRecord.PrimPath))
		{
			if (VariantOverride->StaticMesh)
			{
				MeshToInject = VariantOverride->StaticMesh;
			}
			if (!VariantOverride->Materials.IsEmpty())
			{
				MaterialsToInject = VariantOverride->Materials;
			}
		}

		if (MeshToInject)
		{
			TargetComponent->SetStaticMesh(MeshToInject);
		}
		else
		{
			UE_LOG(LogUsdCarFactoryPipelineInject, Warning, TEXT("Part '%s' has null mesh. Materials may still be applied."), *PartRecord.PrimPath);
		}

		const int32 NumMaterials = MaterialsToInject.Num();
		for (int32 SlotIndex = 0; SlotIndex < NumMaterials; ++SlotIndex)
		{
			if (MaterialsToInject[SlotIndex])
			{
				TargetComponent->SetMaterial(SlotIndex, MaterialsToInject[SlotIndex]);
			}
		}

		++InjectedCount;
	}

	UE_LOG(
		LogUsdCarFactoryPipelineInject,
		Log,
		TEXT("InjectToSelectedCCR completed. Injected %d part records into actor '%s'."),
		InjectedCount,
		*TargetCCRActor->GetName()
	);
#else
	UE_LOG(LogUsdCarFactoryPipelineInject, Warning, TEXT("InjectToSelectedCCR can only run in editor."));
#endif
}
