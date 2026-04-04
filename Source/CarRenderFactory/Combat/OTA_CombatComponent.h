#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "OTA_CombatTypes.h"
#include "OTA_CombatComponent.generated.h"

class AOTA_Character;

UCLASS(ClassGroup = "Combat", meta = (BlueprintSpawnableComponent))
class CARRENDERFACTORY_API UOTA_CombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UOTA_CombatComponent();

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECounterResult CheckCounterRelationship(EMeleeType AttackerType, EMeleeType DefenderType) const;

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool CanDeflectProjectile(EMeleeType MeleeType, EBulletType BulletType) const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PerformAttackTrace();

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamageToTarget(AActor* Target, float BaseDamage, EMeleeType AttackType);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackRange = 150.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float AttackAngle = 90.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float LightAttackDamage = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float HeavyAttackDamage = 30.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
    float CounterDamageMultiplier = 1.5f;

private:
    AOTA_Character* OwnerCharacter = nullptr;
};
