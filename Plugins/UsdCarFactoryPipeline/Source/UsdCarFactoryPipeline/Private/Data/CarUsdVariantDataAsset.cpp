#include "Data/CarUsdVariantDataAsset.h"

#include "UObject/UnrealType.h"

TArray<FName> UCarUsdVariantDataAsset::GetVariantNameOptions() const
{
	TArray<FName> Result;
	Result.Reserve(Variants.Num());

	for (const FCarUsdVariantRecord& Variant : Variants)
	{
		if (!Variant.VariantName.IsNone())
		{
			Result.AddUnique(Variant.VariantName);
		}
	}

	return Result;
}

const FCarUsdVariantRecord* UCarUsdVariantDataAsset::FindVariant(const FName& VariantName) const
{
	if (VariantName.IsNone())
	{
		return nullptr;
	}

	return Variants.FindByPredicate(
		[&VariantName](const FCarUsdVariantRecord& Variant)
		{
			return Variant.VariantName == VariantName;
		}
	);
}

FCarUsdVariantRecord* UCarUsdVariantDataAsset::FindVariantMutable(const FName& VariantName)
{
	if (VariantName.IsNone())
	{
		return nullptr;
	}

	return Variants.FindByPredicate(
		[&VariantName](const FCarUsdVariantRecord& Variant)
		{
			return Variant.VariantName == VariantName;
		}
	);
}

#if WITH_EDITOR
void UCarUsdVariantDataAsset::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName =
		PropertyChangedEvent.GetPropertyName() != NAME_None
			? PropertyChangedEvent.GetPropertyName()
			: PropertyChangedEvent.GetMemberPropertyName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCarUsdVariantDataAsset, Variants))
	{
		EnsureActiveVariantValid(false);
		OnActiveVariantChanged.Broadcast(this);
		return;
	}

	if (PropertyName == GET_MEMBER_NAME_CHECKED(UCarUsdVariantDataAsset, ActiveVariantName))
	{
		EnsureActiveVariantValid(false);
		OnActiveVariantChanged.Broadcast(this);
	}
}
#endif

void UCarUsdVariantDataAsset::EnsureActiveVariantValid(bool bBroadcastIfChanged)
{
	FName NewActiveName = ActiveVariantName;
	if (Variants.IsEmpty())
	{
		NewActiveName = NAME_None;
	}
	else if (FindVariant(NewActiveName) == nullptr)
	{
		for (const FCarUsdVariantRecord& Variant : Variants)
		{
			if (!Variant.VariantName.IsNone())
			{
				NewActiveName = Variant.VariantName;
				break;
			}
		}

		if (NewActiveName.IsNone())
		{
			NewActiveName = FName(TEXT("Default"));
		}
	}

	if (NewActiveName != ActiveVariantName)
	{
		ActiveVariantName = NewActiveName;
#if WITH_EDITOR
		MarkPackageDirty();
#endif
		if (bBroadcastIfChanged)
		{
			OnActiveVariantChanged.Broadcast(this);
		}
	}
}
