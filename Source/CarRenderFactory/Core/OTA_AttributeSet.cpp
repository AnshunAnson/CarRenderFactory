#include "OTA_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Character/OTA_Character.h"
#include "Combat/OTA_CombatTypes.h"

UOTA_AttributeSet::UOTA_AttributeSet()
{
    InitHealth(100.0f);
    InitMaxHealth(100.0f);
    InitQi(50.0f);
    InitMaxQi(100.0f);
    InitGold(0.0f);
    InitAttackPowerMultiplier(1.0f);
    InitMoveSpeedMultiplier(1.0f);
    InitKillCount(0.0f);
}

void UOTA_AttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
    }
    else if (Attribute == GetQiAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxQi());
    }
}

void UOTA_AttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        if (GetHealth() <= 0.0f)
        {
            if (AOTA_Character* OwnerCharacter = Cast<AOTA_Character>(GetOwningActor()))
            {
                OwnerCharacter->SetCombatState(ECombatState::Dead);
            }
        }
    }
}

void UOTA_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION_NOTIFY(UOTA_AttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UOTA_AttributeSet, Qi, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UOTA_AttributeSet, Gold, COND_None, REPNOTIFY_Always);
}

void UOTA_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UOTA_AttributeSet, Health, OldValue);
}

void UOTA_AttributeSet::OnRep_Qi(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UOTA_AttributeSet, Qi, OldValue);
}

void UOTA_AttributeSet::OnRep_Gold(const FGameplayAttributeData& OldValue)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UOTA_AttributeSet, Gold, OldValue);
}
