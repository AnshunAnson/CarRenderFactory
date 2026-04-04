#include "OTA_QiComponent.h"
#include "Character/OTA_Character.h"
#include "AbilitySystemBlueprintLibrary.h"

UOTA_QiComponent::UOTA_QiComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UOTA_QiComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwnerCharacter)
    {
        OwnerCharacter = Cast<AOTA_Character>(GetOwner());
        if (!OwnerCharacter)
        {
            return;
        }
    }

    if (bIsLooting)
    {
        LootQiDrainAccumulator += DeltaTime;
        if (LootQiDrainAccumulator >= 1.0f)
        {
            ModifyQi(-QI_LOSS_PER_SECOND_LOOTING);
            LootQiDrainAccumulator = 0.0f;
        }
    }
}

EQiLevel UOTA_QiComponent::GetQiLevel() const
{
    return CurrentQiLevel;
}

float UOTA_QiComponent::GetQiValue() const
{
    if (!OwnerCharacter)
    {
        return 0.0f;
    }

    return OwnerCharacter->GetQi();
}

void UOTA_QiComponent::ModifyQi(float Delta)
{
    if (!OwnerCharacter)
    {
        return;
    }

    float OldQi = GetQiValue();
    OwnerCharacter->ModifyQi(Delta);
    float NewQi = GetQiValue();

    EQiLevel OldLevel = CurrentQiLevel;
    EQiLevel NewLevel = CalculateQiLevel(NewQi);

    if (OldLevel != NewLevel)
    {
        CurrentQiLevel = NewLevel;
        UpdateQiLevelEffects(OldLevel, NewLevel);
        OnQiLevelChanged.Broadcast(OldLevel, NewLevel);
    }
}

void UOTA_QiComponent::OnHitDealt(AActor* Target)
{
    ModifyQi(QI_GAIN_ON_HIT);
}

void UOTA_QiComponent::OnHitReceived(AActor* Attacker)
{
    ModifyQi(-QI_LOSS_ON_HIT);
}

void UOTA_QiComponent::OnBlockSuccess(AActor* Attacker)
{
    ModifyQi(QI_GAIN_ON_BLOCK);
}

void UOTA_QiComponent::OnLootStart()
{
    bIsLooting = true;
    LootQiDrainAccumulator = 0.0f;
}

void UOTA_QiComponent::OnLootEnd()
{
    bIsLooting = false;
    LootQiDrainAccumulator = 0.0f;
}

EQiLevel UOTA_QiComponent::CalculateQiLevel(float QiValue) const
{
    if (QiValue >= QI_HIGH_THRESHOLD)
    {
        return EQiLevel::High;
    }
    else if (QiValue >= QI_LOW_THRESHOLD)
    {
        return EQiLevel::Normal;
    }
    else if (QiValue > 0.0f)
    {
        return EQiLevel::Low;
    }
    else
    {
        return EQiLevel::Critical;
    }
}

void UOTA_QiComponent::UpdateQiLevelEffects(EQiLevel OldLevel, EQiLevel NewLevel)
{
}
