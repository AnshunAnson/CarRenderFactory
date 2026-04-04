#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AbilitySystemInterface.h"
#include "OTA_PlayerController.generated.h"

class UAbilitySystemComponent;
class UOTA_InputComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class CARRENDERFACTORY_API AOTA_PlayerController : public APlayerController, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AOTA_PlayerController();

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnRep_PlayerState() override;

    UFUNCTION(BlueprintCallable, Category = "Input")
    void SetInputMappingContext(UInputMappingContext* NewContext, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void AddInputMappingContext(UInputMappingContext* NewContext, int32 Priority = 0);

    UFUNCTION(BlueprintCallable, Category = "Input")
    void RemoveInputMappingContext(UInputMappingContext* ContextToRemove);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Ability")
    UAbilitySystemComponent* ASC = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* DefaultInputContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* CombatInputContext = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* MoveAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* LookAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* JumpAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* LightAttackAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* ParryAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* HeavyAttackAction = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* Skill1Action = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* Skill2Action = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    UInputAction* Skill3Action = nullptr;

    bool bInputInitialized = false;

    void InitializeInput();
    void InitializeAbilitySystem();

    void Move(const struct FInputActionValue& Value);
    void Look(const FInputActionValue& Value);
    void Jump();
    void StopJumping();
    void LightAttack();
    void Parry();
    void HeavyAttack();
    void Skill1();
    void Skill2();
    void Skill3();
};
