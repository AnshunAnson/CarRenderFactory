# Gameplay Ability System (GAS) Quick Reference

## Required Modules
- `GameplayAbilities`
- `GameplayTags`
- `GameplayTasks`

## Core Types
- `UAbilitySystemComponent` (ASC): owns and executes abilities/effects.
- `UAttributeSet`: replicated gameplay attributes.
- `UGameplayAbility`: executable ability logic.
- `UGameplayEffect`: attribute modifiers, durations, and tags.

## Initialization Pattern
1. Create/attach ASC.
2. Register attribute set.
3. Call `InitAbilityActorInfo(OwnerActor, AvatarActor)` on startup and possession transitions.
4. Grant default abilities on authority path.

## Replication Notes
- Use replicated attributes with `OnRep_` handlers.
- Ensure ASC replication mode is suitable for game scale.
- For multiplayer, verify server-authoritative activation and prediction strategy.

## Ability Grant Pattern
```cpp
if (HasAuthority() && AbilitySystemComponent)
{
    AbilitySystemComponent->GiveAbility(
        FGameplayAbilitySpec(DefaultAbilityClass, 1, INDEX_NONE, this)
    );
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
}
```

## Common Failure Points
- Ability does not activate:
  - Missing required tags or costs.
  - ASC not initialized with valid actor info.
  - Input binding to ability not configured.
- Attributes not updating UI:
  - Not replicated.
  - Missing delegate binding.
- Effects not applying:
  - Wrong target data.
  - Execution policy mismatch (server/client).
