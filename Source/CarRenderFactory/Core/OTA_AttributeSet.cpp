#include "OTA_AttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"

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
            // TODO: 触发死亡事件
        }
    }
}

void UOTA_AttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UOTA_AttributeSet, Health);
    DOREPLIFETIME(UOTA_AttributeSet, Qi);
    DOREPLIFETIME(UOTA_AttributeSet, Gold);
}

void UOTA_AttributeSet::OnRep_Health(const FGameplayAttributeData& OldValue)
{
}

void UOTA_AttributeSet::OnRep_Qi(const FGameplayAttributeData& OldValue)
{
}

void UOTA_AttributeSet::OnRep_Gold(const FGameplayAttributeData& OldValue)
{
}
