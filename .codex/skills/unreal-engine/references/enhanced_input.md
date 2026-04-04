# Enhanced Input Quick Reference

## Setup Checklist
- Enable plugin: `Enhanced Input`.
- Add dependencies in module `Build.cs`:
  - `EnhancedInput`
  - `InputCore`
- Set default input classes in project settings when needed.

## Typical Runtime Flow
1. Get `APlayerController`.
2. Get `ULocalPlayer`.
3. Get `UEnhancedInputLocalPlayerSubsystem`.
4. Add mapping context (`AddMappingContext`).
5. Bind `UInputAction` on `UEnhancedInputComponent`.

## Binding Pattern
```cpp
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
        EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
        EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
    }
}
```

## Common Issues
- Input never fires:
  - Missing mapping context.
  - Wrong pawn possessed.
  - Using legacy input component.
- Wrong value types:
  - `FInputActionValue` must match action type (Bool/Axis1D/Axis2D/Axis3D).
- Context priority conflict:
  - Higher priority mapping overrides lower contexts.
