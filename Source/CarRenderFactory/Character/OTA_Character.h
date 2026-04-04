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

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetAttackPowerMultiplier() const;

    UFUNCTION(BlueprintPure, Category = "Attributes")
    int32 GetKillCount() const;

    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool ActivateQiShield();

    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool ActivateDash();

    UFUNCTION(BlueprintCallable, Category = "Skills")
    bool ActivateTreasureSense();

    UFUNCTION(BlueprintPure, Category = "Skills")
    bool IsSkillOnCooldown() const;

    UFUNCTION(BlueprintPure, Category = "Skills")
    bool IsQiShieldActive() const { return bQiShieldActive; }

    UFUNCTION(BlueprintPure, Category = "Skills")
    bool IsTreasureSenseActive() const { return bTreasureSenseActive; }

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetAttackPowerMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetMoveSpeedMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetQiRegenMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetQiStateAttackMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetGoldAttackMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetQiStateRegenMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Attributes")
    void SetGoldRegenMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintPure, Category = "Attributes")
    float GetQiRegenMultiplier() const { return QiRegenMultiplier; }

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

    void EndQiShield();
    void EndTreasureSense();
    void RefreshFinalMultipliers();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float SkillSharedCooldown = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float QiShieldDuration = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float TreasureSenseDuration = 5.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float DashDistance = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float QiShieldCost = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float DashCost = 15.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float TreasureSenseCost = 10.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Skills")
    float SkillDisabledQiThreshold = 20.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    int32 GoldBuffStep = 50;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    int32 MaxGoldBuffStacks = 3;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    float GoldBuffAttackPerStack = 0.05f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
    float GoldBuffQiRegenPerStack = 0.1f;

    float QiRegenMultiplier = 1.0f;
    float BaseWalkSpeed = 600.0f;
    float QiStateAttackMultiplier = 1.0f;
    float GoldAttackMultiplier = 1.0f;
    float QiStateRegenMultiplier = 1.0f;
    float GoldRegenMultiplier = 1.0f;
    float SkillCooldownEndTime = 0.0f;
    bool bQiShieldActive = false;
    bool bTreasureSenseActive = false;

    FTimerHandle QiShieldTimerHandle;
    FTimerHandle TreasureSenseTimerHandle;
};
