#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "OTA_PlayerState.generated.h"

class UAbilitySystemComponent;
class UOTA_AttributeSet;

UCLASS()
class CARRENDERFACTORY_API AOTA_PlayerState : public APlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AOTA_PlayerState();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UFUNCTION(BlueprintPure, Category = "Ability")
    UOTA_AttributeSet* GetAttributeSet() const { return AttributeSet; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
    UAbilitySystemComponent* ASC = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
    UOTA_AttributeSet* AttributeSet = nullptr;

    void InitializeAbilitySystem();
    void GrantDefaultAbilities();
    void ApplyDefaultEffects();
};
