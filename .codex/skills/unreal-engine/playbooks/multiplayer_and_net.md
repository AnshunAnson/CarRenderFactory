# Multiplayer and Network Playbook

## Goal
Ensure replication, authority, and GAS behaviors are explicit, testable, and version-aware.

## Executor Scope
- replication (`bReplicates`, `DOREPLIFETIME`, `OnRep`)
- RPC correctness and authority boundaries
- GAS initialization and ability/effect execution paths
- multiplayer data-flow validation

## Core Rules
- Server-authoritative state for competitive or integrity-sensitive behavior.
- Never rely on client-only state mutation for canonical gameplay data.
- Ensure replication wiring is explicit, not implied.
- GAS actor info init must be valid on both server and client lifecycle paths.

## Hardness Expectations
- `L`
  - conceptual guidance only, no deep lane orchestration
- `M`
  - must run `explore + executor + verifier`
  - verify authority and replication assumptions with concrete evidence
- `H`
  - run full pipeline and split at least two lanes (e.g. replication lane + GAS lane)
  - add `document-specialist` if plugin/API ambiguity exists
  - do not complete without `code-reviewer` evidence

## Verification Targets
- replication path correctness
- RPC function decoration and implementation shape
- GAS dependency presence (`GameplayAbilities`, `GameplayTags`, `GameplayTasks`)
- init path completeness for ASC and relevant actor ownership

## Failure Handling
If verifier fails:
1. open fix loop with explicit root cause
2. apply bounded retries
3. if unresolved, block completion and report unresolved risks
