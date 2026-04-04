# Unreal Common Pitfalls

## Build / Compile
- Editor still open while rebuilding C++.
- Missing module dependencies in `.Build.cs`.
- Stale generated files:
  - clean `Binaries/`, `Intermediate/`, optionally `Saved/`.

## Input
- Enhanced Input plugin disabled.
- Mapping context never added.
- Action asset path mismatch.
- Wrong trigger event (`Started` vs `Triggered` vs `Completed`).

## GAS
- ASC exists but `InitAbilityActorInfo` not called in all lifecycle paths.
- Attributes not marked/replicated correctly.
- Abilities granted only on client (must be server authoritative).

## Replication
- Missing `bReplicates = true`.
- Missing `DOREPLIFETIME` registration.
- RPC signatures missing `_Implementation`.

## Debug Commands (Unreal In-Game Console)
```text
stat fps
stat unit
stat game
stat gpu
showdebug enhancedinput
showdebug abilitysystem
stat net
```

## Recovery Sequence
1. Save work and close editor.
2. Clean generated folders (`Binaries`, `Intermediate`, optionally `Saved`).
3. Regenerate project files.
4. Build from IDE.
5. Re-open project and inspect logs in `Saved/Logs`.
