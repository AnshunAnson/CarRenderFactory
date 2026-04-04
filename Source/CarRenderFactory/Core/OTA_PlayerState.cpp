#include "OTA_PlayerState.h"
#include "AbilitySystemComponent.h"
#include "OTA_AttributeSet.h"
#include "GameplayAbilitySpec.h"
#include "GameplayEffect.h"

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
    InitializeAbilitySystem();
}

void AOTA_PlayerState::Reset()
{
    Super::Reset();
    InitializeAbilitySystem();
}

void AOTA_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AOTA_PlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AOTA_PlayerState::InitializeAbilitySystem()
{
    if (ASC)
    {
        ASC->InitAbilityActorInfo(this, GetPawn());
        if (!bDefaultsApplied)
        {
            GrantDefaultAbilities();
            ApplyDefaultEffects();
            bDefaultsApplied = true;
        }
    }
}

void AOTA_PlayerState::GrantDefaultAbilities()
{
    if (!HasAuthority() || !ASC)
    {
        return;
    }

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : DefaultAbilities)
    {
        if (AbilityClass)
        {
            ASC->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        }
    }
}

void AOTA_PlayerState::ApplyDefaultEffects()
{
    if (!HasAuthority() || !ASC)
    {
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(this);

    for (const TSubclassOf<UGameplayEffect>& EffectClass : DefaultEffects)
    {
        if (EffectClass)
        {
            const FGameplayEffectSpecHandle EffectSpec = ASC->MakeOutgoingSpec(EffectClass, 1.0f, EffectContext);
            if (EffectSpec.IsValid())
            {
                ASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
            }
        }
    }
}
