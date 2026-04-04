# C++ Architecture Playbook

## Goal
Deliver maintainable Unreal C++ implementations with explicit ownership, performance discipline, and safe Blueprint interop.

## Executor Scope
- Gameplay frameworks (character/combat/inventory/AI)
- C++ refactors and subsystem boundaries
- Hot-path performance fixes
- Blueprint-facing API shaping

## Implementation Boundaries
- Prefer composition over deep inheritance.
- Make UObject ownership/lifecycle explicit.
- Keep heavy logic out of `Tick` when event/timer patterns are viable.
- Expose minimal stable Blueprint interfaces; keep heavy logic in C++.
- Respect project naming and module boundaries discovered earlier.

## Parallel Split Guidance (`H`)
Minimum two independent lanes:
- Lane A: gameplay/system logic changes
- Lane B: integration and compatibility checks (Blueprint/API/config)

Lane write scopes must be disjoint to prevent merge conflicts.

## Anti-Patterns
- God classes with mixed responsibilities
- repeated world/component lookups in hot paths
- hidden hard references that create load-chain bloat
- implicit replication expectations without explicit setup

## Required Deliverable Notes
- list modified components/systems
- note remaining technical debt or deferred refactor items
- call out performance risk if profiling evidence is missing
