# Iteration Round 1 Design (Convergence)

## 1) Current Redundancy / Coupling / Risk
- **No fixed, executable convergence loop**: iteration behavior only exists in prompt text, causing process drift and hidden manual state.
- **No standardized artifact location**: design docs and diff patches are not auto-indexed by round number, increasing traceability risk.
- **No isolation guard**: there is no script-level mechanism forcing round-to-round copy before edits.

## 2) Round-1 Reduction & Convergence Strategy
- Build a **single minimal script** to enforce one-round closed loop execution skeleton:
  1. detect previous round;
  2. create new round directory;
  3. scaffold mandatory design doc;
  4. generate and persist patch file by round number.
- Keep the implementation shell-only and dependency-light to reduce operational surface.

## 3) Minimal Closed Loop Kept
- `docs/iter_round_{N}.md` exists before edits.
- `iterations/round_{N}/` acts as isolated round workspace for artifacts.
- `iter_round_{N}_diff.patch` is generated and persisted locally.

## 4) Explicit Delete / Merge / Converge List
- **Converge**: centralize iterative workflow into one script (`scripts/iter_converge.sh`).
- **Merge**: merge round bookkeeping into single state file (`iterations/LATEST_ROUND`).
- **Delete**: none in round 1 (baseline lacks safe-to-delete, low-risk targets).

## 5) Post-Iteration Target Structure
- `docs/iter_round_1.md` (this design)
- `scripts/iter_converge.sh` (execution entry)
- `iterations/LATEST_ROUND` (round index)
- `iterations/round_1/` (isolated artifact directory)
- `iter_round_1_diff.patch` (round patch artifact)
