#include "OTA_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "OTA_AttributeSet.h"

AOTA_PlayerState::AOTA_PlayerState()
{
    ASC = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    AttributeSet = CreateDefaultSubobject<UOTA_AttributeSet>(TEXT("AttributeSet"));

    ASC->SetIsReplicated(true);
    ASC->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

UAbilitySystemComponent* AOTA_PlayerState::GetAbilitySystemComponent() const
{
    return ASC;
}

void AOTA_PlayerState::BeginPlay()
{
    Super::BeginPlay();

    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, GetPawn());
    }
}

void AOTA_PlayerState::Reset()
{
    Super::Reset();

    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, GetPawn());
    }
}

void AOTA_PlayerState::InitializeAbilitySystem()
{
    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, GetPawn());
        GrantDefaultAbilities();
        ApplyDefaultEffects();
    }
}

void AOTA_PlayerState::GrantDefaultAbilities()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // TODO: 从配置中读取默认能力类
}

void AOTA_PlayerState::ApplyDefaultEffects()
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

    // TODO: 从配置中读取默认效果类
}
