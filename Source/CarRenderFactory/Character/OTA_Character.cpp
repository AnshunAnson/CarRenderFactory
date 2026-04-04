#include "OTA_Character.h"
#include "AbilitySystemComponent.h"
#include "Core/OTA_AttributeSet.h"
#include "Combat/OTA_CombatComponent.h"
#include "Core/OTA_QiComponent.h"
#include "Net/UnrealNetwork.h"

AOTA_Character::AOTA_Character()
{
    bReplicates = true;

    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UOTA_AttributeSet>(TEXT("AttributeSet"));
    CombatComponent = CreateDefaultSubobject<UOTA_CombatComponent>(TEXT("CombatComponent"));
    QiComponent = CreateDefaultSubobject<UOTA_QiComponent>(TEXT("QiComponent"));

    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
}

UAbilitySystemComponent* AOTA_Character::GetAbilitySystemComponent() const
{
    return ASC;
}

void AOTA_Character::BeginPlay()
{
    Super::BeginPlay();
}

void AOTA_Character::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    if (ASC)
    {
        ASC->InitAbilityActorInfo(NewController, this);
    }
}

void AOTA_Character::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (ASC)
    {
        ASC->InitAbilityActorInfo(GetPlayerState(), this);
    }
}

void AOTA_Character::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AOTA_Character, CurrentMeleeType);
    DOREPLIFETIME(AOTA_Character, CombatState);
}

bool AOTA_Character::IsAlive() const
{
    if (AttributeSet)
    {
        return AttributeSet->GetHealth() > 0.0f;
    }
    return true;
}

void AOTA_Character::SetCurrentMeleeType(EMeleeType NewType)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    EMeleeType OldType = CurrentMeleeType;
    CurrentMeleeType = NewType;

    OnRep_CurrentMeleeType(OldType);
}

void AOTA_Character::SetCombatState(ECombatState NewState)
{
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    ECombatState OldState = CombatState;
    CombatState = NewState;

    OnRep_CombatState(OldState);
}

void AOTA_Character::ApplyDamage(float Damage, AActor* DamageInstigator)
{
    if (!AttributeSet || !ASC)
    {
        return;
    }

    AttributeSet->SetHealth(AttributeSet->GetHealth() - Damage);

    if (AttributeSet->GetHealth() <= 0.0f)
    {
        SetCombatState(ECombatState::Dead);
    }
}

void AOTA_Character::ModifyQi(float Delta)
{
    if (!AttributeSet || !ASC)
    {
        return;
    }

    float NewQi = FMath::Clamp(AttributeSet->GetQi() + Delta, 0.0f, AttributeSet->GetMaxQi());
    AttributeSet->SetQi(NewQi);
}

void AOTA_Character::ModifyGold(int32 Delta)
{
    if (!AttributeSet || !ASC)
    {
        return;
    }

    AttributeSet->SetGold(AttributeSet->GetGold() + Delta);
}

float AOTA_Character::GetQi() const
{
    return AttributeSet ? AttributeSet->GetQi() : 0.0f;
}

float AOTA_Character::GetHealth() const
{
    return AttributeSet ? AttributeSet->GetHealth() : 0.0f;
}

int32 AOTA_Character::GetGold() const
{
    return AttributeSet ? static_cast<int32>(AttributeSet->GetGold()) : 0;
}

void AOTA_Character::OnRep_CurrentMeleeType(EMeleeType OldType)
{
}

void AOTA_Character::OnRep_CombatState(ECombatState OldState)
{
}
