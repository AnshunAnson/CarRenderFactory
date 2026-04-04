#include "OTA_Character.h"
#include "AbilitySystemComponent.h"
#include "OTA_AttributeSet.h"
#include "OTA_CombatComponent.h"
#include "OTA_QiComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"

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

    if (GetCharacterMovement())
    {
        BaseWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
    }
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
        ASC->InitAbilityActorInfo(Cast<AActor>(GetPlayerState()), this);
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
    if (GetLocalRole() != ROLE_Authority || !AttributeSet || !ASC || Damage <= 0.0f || !IsAlive())
    {
        return;
    }

    if (bQiShieldActive)
    {
        EndQiShield();
        return;
    }

    AttributeSet->SetHealth(AttributeSet->GetHealth() - Damage);

    if (AttributeSet->GetHealth() <= 0.0f)
    {
        SetCombatState(ECombatState::Dead);

        if (AOTA_Character* KillerCharacter = Cast<AOTA_Character>(DamageInstigator))
        {
            if (KillerCharacter->AttributeSet)
            {
                const float NewKillCount = KillerCharacter->AttributeSet->GetKillCount() + 1.0f;
                KillerCharacter->AttributeSet->SetKillCount(NewKillCount);
            }
        }
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

    const int32 NewGold = FMath::Max(0, static_cast<int32>(AttributeSet->GetGold()) + Delta);
    AttributeSet->SetGold(static_cast<float>(NewGold));

    const int32 SafeGoldBuffStep = FMath::Max(1, GoldBuffStep);
    const int32 GoldStacks = FMath::Clamp(NewGold / SafeGoldBuffStep, 0, MaxGoldBuffStacks);
    const float AttackMultiplierFromGold = 1.0f + GoldStacks * GoldBuffAttackPerStack;
    const float QiRegenMultiplierFromGold = 1.0f + GoldStacks * GoldBuffQiRegenPerStack;

    SetGoldAttackMultiplier(AttackMultiplierFromGold);
    SetGoldRegenMultiplier(QiRegenMultiplierFromGold);
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

float AOTA_Character::GetAttackPowerMultiplier() const
{
    return AttributeSet ? AttributeSet->GetAttackPowerMultiplier() : 1.0f;
}

int32 AOTA_Character::GetKillCount() const
{
    return AttributeSet ? static_cast<int32>(AttributeSet->GetKillCount()) : 0;
}

bool AOTA_Character::ActivateQiShield()
{
    UWorld* World = GetWorld();
    if (GetLocalRole() != ROLE_Authority || !World || !AttributeSet || IsSkillOnCooldown() || GetQi() < QiShieldCost || GetQi() < SkillDisabledQiThreshold)
    {
        return false;
    }

    ModifyQi(-QiShieldCost);
    bQiShieldActive = true;
    SkillCooldownEndTime = World->GetTimeSeconds() + SkillSharedCooldown;

    World->GetTimerManager().ClearTimer(QiShieldTimerHandle);
    World->GetTimerManager().SetTimer(QiShieldTimerHandle, this, &AOTA_Character::EndQiShield, QiShieldDuration, false);
    return true;
}

bool AOTA_Character::ActivateDash()
{
    UWorld* World = GetWorld();
    if (GetLocalRole() != ROLE_Authority || !World || !AttributeSet || IsSkillOnCooldown() || GetQi() < DashCost || GetQi() < SkillDisabledQiThreshold)
    {
        return false;
    }

    ModifyQi(-DashCost);
    SkillCooldownEndTime = World->GetTimeSeconds() + SkillSharedCooldown;

    FVector DashTarget = GetActorLocation() + GetActorForwardVector() * DashDistance;
    SetActorLocation(DashTarget, true);
    return true;
}

bool AOTA_Character::ActivateTreasureSense()
{
    UWorld* World = GetWorld();
    if (GetLocalRole() != ROLE_Authority || !World || !AttributeSet || IsSkillOnCooldown() || GetQi() < TreasureSenseCost || GetQi() < SkillDisabledQiThreshold)
    {
        return false;
    }

    ModifyQi(-TreasureSenseCost);
    bTreasureSenseActive = true;
    SkillCooldownEndTime = World->GetTimeSeconds() + SkillSharedCooldown;

    World->GetTimerManager().ClearTimer(TreasureSenseTimerHandle);
    World->GetTimerManager().SetTimer(TreasureSenseTimerHandle, this, &AOTA_Character::EndTreasureSense, TreasureSenseDuration, false);
    return true;
}

bool AOTA_Character::IsSkillOnCooldown() const
{
    const UWorld* World = GetWorld();
    return World && World->GetTimeSeconds() < SkillCooldownEndTime;
}

void AOTA_Character::SetAttackPowerMultiplier(float NewMultiplier)
{
    SetQiStateAttackMultiplier(NewMultiplier);
}

void AOTA_Character::SetMoveSpeedMultiplier(float NewMultiplier)
{
    if (AttributeSet)
    {
        const float ClampedMultiplier = FMath::Max(0.1f, NewMultiplier);
        AttributeSet->SetMoveSpeedMultiplier(ClampedMultiplier);

        if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
        {
            MoveComp->MaxWalkSpeed = BaseWalkSpeed * ClampedMultiplier;
        }
    }
}

void AOTA_Character::SetQiRegenMultiplier(float NewMultiplier)
{
    SetQiStateRegenMultiplier(NewMultiplier);
}

void AOTA_Character::SetQiStateAttackMultiplier(float NewMultiplier)
{
    QiStateAttackMultiplier = FMath::Max(0.1f, NewMultiplier);
    RefreshFinalMultipliers();
}

void AOTA_Character::SetGoldAttackMultiplier(float NewMultiplier)
{
    GoldAttackMultiplier = FMath::Max(0.1f, NewMultiplier);
    RefreshFinalMultipliers();
}

void AOTA_Character::SetQiStateRegenMultiplier(float NewMultiplier)
{
    QiStateRegenMultiplier = FMath::Max(0.1f, NewMultiplier);
    RefreshFinalMultipliers();
}

void AOTA_Character::SetGoldRegenMultiplier(float NewMultiplier)
{
    GoldRegenMultiplier = FMath::Max(0.1f, NewMultiplier);
    RefreshFinalMultipliers();
}

void AOTA_Character::RefreshFinalMultipliers()
{
    if (AttributeSet)
    {
        AttributeSet->SetAttackPowerMultiplier(QiStateAttackMultiplier * GoldAttackMultiplier);
    }

    QiRegenMultiplier = QiStateRegenMultiplier * GoldRegenMultiplier;
}

void AOTA_Character::EndQiShield()
{
    bQiShieldActive = false;
}

void AOTA_Character::EndTreasureSense()
{
    bTreasureSenseActive = false;
}

void AOTA_Character::OnRep_CurrentMeleeType(EMeleeType OldType)
{
}

void AOTA_Character::OnRep_CombatState(ECombatState OldState)
{
}

float AOTA_Character::GetQi() const
{
    if (AttributeSet)
    {
        return AttributeSet->GetQi();
    }
    return 0.0f;
}

float AOTA_Character::GetHealth() const
{
    if (AttributeSet)
    {
        return AttributeSet->GetHealth();
    }
    return 0.0f;
}

int32 AOTA_Character::GetGold() const
{
    if (AttributeSet)
    {
        return AttributeSet->GetGold();
    }
    return 0;
}

float AOTA_Character::GetAttackPowerMultiplier() const
{
    if (AttributeSet)
    {
        return AttributeSet->GetAttackPowerMultiplier();
    }
    return 1.0f;
}

int32 AOTA_Character::GetKillCount() const
{
    if (AttributeSet)
    {
        return FMath::FloorToInt(AttributeSet->GetKillCount());
    }
    return 0;
}

bool AOTA_Character::ActivateQiShield()
{
    if (!AttributeSet || !ASC || bQiShieldActive)
    {
        return false;
    }

    if (AttributeSet->GetQi() < QiShieldCost)
    {
        return false;
    }

    ModifyQi(-QiShieldCost);
    bQiShieldActive = true;

    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &AOTA_Character::EndQiShield);
    GetWorldTimerManager().SetTimer(QiShieldTimerHandle, TimerDelegate, QiShieldDuration, false);

    return true;
}

bool AOTA_Character::ActivateDash()
{
    if (!AttributeSet || !ASC)
    {
        return false;
    }

    if (AttributeSet->GetQi() < DashCost)
    {
        return false;
    }

    ModifyQi(-DashCost);

    FVector DashDirection = GetActorForwardVector();
    FVector NewLocation = GetActorLocation() + DashDirection * DashDistance;
    SetActorLocation(NewLocation, true);

    return true;
}

bool AOTA_Character::ActivateTreasureSense()
{
    if (!AttributeSet || !ASC || bTreasureSenseActive)
    {
        return false;
    }

    if (AttributeSet->GetQi() < TreasureSenseCost)
    {
        return false;
    }

    ModifyQi(-TreasureSenseCost);
    bTreasureSenseActive = true;

    FTimerDelegate TimerDelegate;
    TimerDelegate.BindUObject(this, &AOTA_Character::EndTreasureSense);
    GetWorldTimerManager().SetTimer(TreasureSenseTimerHandle, TimerDelegate, TreasureSenseDuration, false);

    return true;
}

bool AOTA_Character::IsSkillOnCooldown() const
{
    if (!AttributeSet)
    {
        return false;
    }

    return AttributeSet->GetQi() < SkillDisabledQiThreshold;
}

void AOTA_Character::SetAttackPowerMultiplier(float NewMultiplier)
{
    if (AttributeSet)
    {
        AttributeSet->SetAttackPowerMultiplier(NewMultiplier);
        RefreshFinalMultipliers();
    }
}

void AOTA_Character::SetMoveSpeedMultiplier(float NewMultiplier)
{
    if (AttributeSet && GetCharacterMovement())
    {
        AttributeSet->SetMoveSpeedMultiplier(NewMultiplier);
        GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed * NewMultiplier;
    }
}

void AOTA_Character::SetQiRegenMultiplier(float NewMultiplier)
{
    QiRegenMultiplier = NewMultiplier;
    RefreshFinalMultipliers();
}

void AOTA_Character::SetQiStateAttackMultiplier(float NewMultiplier)
{
    QiStateAttackMultiplier = NewMultiplier;
    RefreshFinalMultipliers();
}

void AOTA_Character::SetGoldAttackMultiplier(float NewMultiplier)
{
    GoldAttackMultiplier = NewMultiplier;
    RefreshFinalMultipliers();
}

void AOTA_Character::SetQiStateRegenMultiplier(float NewMultiplier)
{
    QiStateRegenMultiplier = NewMultiplier;
    RefreshFinalMultipliers();
}

void AOTA_Character::SetGoldRegenMultiplier(float NewMultiplier)
{
    GoldRegenMultiplier = NewMultiplier;
    RefreshFinalMultipliers();
}

void AOTA_Character::InitializeAbilitySystem()
{
    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, this);
    }
}

void AOTA_Character::OnDeath()
{
    SetCombatState(ECombatState::Dead);
}

void AOTA_Character::EndQiShield()
{
    bQiShieldActive = false;
    GetWorldTimerManager().ClearTimer(QiShieldTimerHandle);
}

void AOTA_Character::EndTreasureSense()
{
    bTreasureSenseActive = false;
    GetWorldTimerManager().ClearTimer(TreasureSenseTimerHandle);
}

void AOTA_Character::RefreshFinalMultipliers()
{
    if (AttributeSet)
    {
        float FinalAttack = QiStateAttackMultiplier * GoldAttackMultiplier;
        AttributeSet->SetAttackPowerMultiplier(FinalAttack);
    }
}
