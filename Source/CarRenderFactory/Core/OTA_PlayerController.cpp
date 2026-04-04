#include "OTA_PlayerController.h"
#include "OTA_Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/Character.h"

AOTA_PlayerController::AOTA_PlayerController()
{
}

UAbilitySystemComponent* AOTA_PlayerController::GetAbilitySystemComponent() const
{
    return ASC;
}

void AOTA_PlayerController::BeginPlay()
{
    Super::BeginPlay();

    InitializeInput();
}

void AOTA_PlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (MoveAction)
        {
            EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AOTA_PlayerController::Move);
        }

        if (LookAction)
        {
            EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AOTA_PlayerController::Look);
        }

        if (JumpAction)
        {
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &AOTA_PlayerController::Jump);
            EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &AOTA_PlayerController::StopJumping);
        }

        if (LightAttackAction)
        {
            EnhancedInput->BindAction(LightAttackAction, ETriggerEvent::Started, this, &AOTA_PlayerController::LightAttack);
        }

        if (ParryAction)
        {
            EnhancedInput->BindAction(ParryAction, ETriggerEvent::Started, this, &AOTA_PlayerController::Parry);
        }

        if (HeavyAttackAction)
        {
            EnhancedInput->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &AOTA_PlayerController::HeavyAttack);
        }

        if (Skill1Action)
        {
            EnhancedInput->BindAction(Skill1Action, ETriggerEvent::Started, this, &AOTA_PlayerController::Skill1);
        }

        if (Skill2Action)
        {
            EnhancedInput->BindAction(Skill2Action, ETriggerEvent::Started, this, &AOTA_PlayerController::Skill2);
        }

        if (Skill3Action)
        {
            EnhancedInput->BindAction(Skill3Action, ETriggerEvent::Started, this, &AOTA_PlayerController::Skill3);
        }
    }
}

void AOTA_PlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    InitializeAbilitySystem();
}

void AOTA_PlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    InitializeAbilitySystem();
}

void AOTA_PlayerController::SetInputMappingContext(UInputMappingContext* NewContext, int32 Priority)
{
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            Subsystem->ClearAllMappings();
            if (NewContext)
            {
                Subsystem->AddMappingContext(NewContext, Priority);
            }
        }
    }
}

void AOTA_PlayerController::AddInputMappingContext(UInputMappingContext* NewContext, int32 Priority)
{
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (NewContext)
            {
                Subsystem->AddMappingContext(NewContext, Priority);
            }
        }
    }
}

void AOTA_PlayerController::RemoveInputMappingContext(UInputMappingContext* ContextToRemove)
{
    if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
        {
            if (ContextToRemove)
            {
                Subsystem->RemoveMappingContext(ContextToRemove);
            }
        }
    }
}

void AOTA_PlayerController::InitializeInput()
{
    if (bInputInitialized)
    {
        return;
    }

    if (DefaultInputContext)
    {
        AddInputMappingContext(DefaultInputContext, 0);
    }

    if (CombatInputContext)
    {
        AddInputMappingContext(CombatInputContext, 1);
    }

    bInputInitialized = true;
}

void AOTA_PlayerController::InitializeAbilitySystem()
{
    if (AOTA_Character* OTAChar = Cast<AOTA_Character>(GetPawn()))
    {
        ASC = OTAChar->GetAbilitySystemComponent();
    }
}

void AOTA_PlayerController::Move(const FInputActionValue& Value)
{
    const FVector2D MovementVector = Value.Get<FVector2D>();

    if (APawn* ControlledPawn = GetPawn())
    {
        const FRotator Rotation = GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        ControlledPawn->AddMovementInput(ForwardDirection, MovementVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, MovementVector.X);
    }
}

void AOTA_PlayerController::Look(const FInputActionValue& Value)
{
    const FVector2D LookAxisVector = Value.Get<FVector2D>();

    AddYawInput(LookAxisVector.X);
    AddPitchInput(LookAxisVector.Y);
}

void AOTA_PlayerController::Jump()
{
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        ControlledCharacter->Jump();
    }
}

void AOTA_PlayerController::StopJumping()
{
    if (ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn()))
    {
        ControlledCharacter->StopJumping();
    }
}

void AOTA_PlayerController::LightAttack()
{
    if (ASC)
    {
        FGameplayTag LightAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.LightAttack"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(LightAttackTag));
    }
}

void AOTA_PlayerController::Parry()
{
    if (ASC)
    {
        FGameplayTag ParryTag = FGameplayTag::RequestGameplayTag(FName("Ability.Parry"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(ParryTag));
    }
}

void AOTA_PlayerController::HeavyAttack()
{
    if (ASC)
    {
        FGameplayTag HeavyAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.HeavyAttack"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(HeavyAttackTag));
    }
}

void AOTA_PlayerController::Skill1()
{
    if (AOTA_Character* OTAChar = Cast<AOTA_Character>(GetPawn()))
    {
        if (OTAChar->ActivateQiShield())
        {
            return;
        }
    }

    if (ASC)
    {
        FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.QiShield"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));
    }
}

void AOTA_PlayerController::Skill2()
{
    if (AOTA_Character* OTAChar = Cast<AOTA_Character>(GetPawn()))
    {
        if (OTAChar->ActivateDash())
        {
            return;
        }
    }

    if (ASC)
    {
        FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Dash"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));
    }
}

void AOTA_PlayerController::Skill3()
{
    if (AOTA_Character* OTAChar = Cast<AOTA_Character>(GetPawn()))
    {
        if (OTAChar->ActivateTreasureSense())
        {
            return;
        }
    }

    if (ASC)
    {
        FGameplayTag SkillTag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.TreasureSense"));
        ASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SkillTag));
    }
}
