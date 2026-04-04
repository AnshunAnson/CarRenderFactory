#include "OTA_CombatComponent.h"
#include "Character/OTA_Character.h"
#include "DrawDebugHelpers.h"

UOTA_CombatComponent::UOTA_CombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UOTA_CombatComponent::BeginPlay()
{
    Super::BeginPlay();

    OwnerCharacter = Cast<AOTA_Character>(GetOwner());
}

ECounterResult UOTA_CombatComponent::CheckCounterRelationship(EMeleeType AttackerType, EMeleeType DefenderType) const
{
    if (AttackerType == EMeleeType::None || DefenderType == EMeleeType::None)
    {
        return ECounterResult::None;
    }

    if (AttackerType == DefenderType)
    {
        return ECounterResult::Draw;
    }

    if ((AttackerType == EMeleeType::White && DefenderType == EMeleeType::Red) ||
        (AttackerType == EMeleeType::Red && DefenderType == EMeleeType::Blue) ||
        (AttackerType == EMeleeType::Blue && DefenderType == EMeleeType::White))
    {
        return ECounterResult::Counter;
    }

    return ECounterResult::Countered;
}

bool UOTA_CombatComponent::CanDeflectProjectile(EMeleeType MeleeType, EBulletType BulletType) const
{
    if (MeleeType == EMeleeType::White && BulletType == EBulletType::White)
    {
        return true;
    }
    if (MeleeType == EMeleeType::Red && BulletType == EBulletType::Blue)
    {
        return true;
    }
    if (MeleeType == EMeleeType::Blue && BulletType == EBulletType::Red)
    {
        return true;
    }
    return false;
}

void UOTA_CombatComponent::PerformAttackTrace()
{
    if (!OwnerCharacter)
    {
        return;
    }

    FVector StartLocation = OwnerCharacter->GetActorLocation();
    FVector ForwardVector = OwnerCharacter->GetActorForwardVector();
    FVector EndLocation = StartLocation + ForwardVector * AttackRange;

    TArray<FHitResult> HitResults;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(50.0f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerCharacter);

    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_Pawn,
        Sphere,
        QueryParams
    );

#if ENABLE_DRAW_DEBUG
    DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Red, false, 2.0f, 0, 2.0f);
#endif

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (AActor* HitActor = Hit.GetActor())
            {
                EMeleeType CurrentAttackType = OwnerCharacter->GetCurrentMeleeType();
                float Damage = (CurrentAttackType == EMeleeType::Blue) ? HeavyAttackDamage : LightAttackDamage;
                ApplyDamageToTarget(HitActor, Damage, CurrentAttackType);
            }
        }
    }
}

void UOTA_CombatComponent::ApplyDamageToTarget(AActor* Target, float BaseDamage, EMeleeType AttackType)
{
    if (!Target || !OwnerCharacter)
    {
        return;
    }

    AOTA_Character* TargetCharacter = Cast<AOTA_Character>(Target);
    if (!TargetCharacter)
    {
        return;
    }

    ECounterResult CounterResult = CheckCounterRelationship(AttackType, TargetCharacter->GetCurrentMeleeType());

    float FinalDamage = BaseDamage;

    switch (CounterResult)
    {
    case ECounterResult::Counter:
        FinalDamage *= CounterDamageMultiplier;
        break;
    case ECounterResult::Countered:
        FinalDamage = 0.0f;
        OwnerCharacter->SetCombatState(ECombatState::Stunned);
        break;
    case ECounterResult::Draw:
        FinalDamage *= 0.5f;
        break;
    default:
        break;
    }

    if (FinalDamage > 0.0f)
    {
        TargetCharacter->ApplyDamage(FinalDamage, OwnerCharacter);
    }
}
