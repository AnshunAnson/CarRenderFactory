#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Combat/OTA_CombatTypes.h"
#include "OTA_Character.generated.h"

class UAbilitySystemComponent;
class UOTA_AttributeSet;
class UOTA_CombatComponent;
class UOTA_QiComponent;

UCLASS()
class CARRENDERFACTORY_API AOTA_Character : public ACharacter, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AOTA_Character();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category = "Combat")
    EMeleeType GetCurrentMeleeType() const { return CurrentMeleeType; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    ECombatState GetCombatState() const { return CombatState; }

    UFUNCTION(BlueprintPure, Category = "Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCurrentMeleeType(EMeleeType NewType);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void SetCombatState(ECombatState NewState);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyDamage(float Damage, AActor* DamageInstigator);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ModifyQi(float Delta);

    UFUNCTION(BlueprintCallable, Category = "Loot")
    void ModifyGold(int32 Delta);

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetQi() const;

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetHealth() const;

    UFUNCTION(BlueprintPure, Category = "Attributes")
    int32 GetGold() const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAbilitySystemComponent* ASC = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UOTA_AttributeSet* AttributeSet = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UOTA_CombatComponent* CombatComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UOTA_QiComponent* QiComponent = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentMeleeType, BlueprintReadOnly, Category = "Combat")
    EMeleeType CurrentMeleeType = EMeleeType::None;

    UPROPERTY(ReplicatedUsing = OnRep_CombatState, BlueprintReadOnly, Category = "Combat")
    ECombatState CombatState = ECombatState::Idle;

    UFUNCTION()
    void OnRep_CurrentMeleeType(EMeleeType OldType);

    UFUNCTION()
    void OnRep_CombatState(ECombatState OldState);

    void InitializeAbilitySystem();
    void OnDeath();
};
