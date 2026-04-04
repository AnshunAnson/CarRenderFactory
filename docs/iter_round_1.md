# Iteration Round 1 Design

## 1) Current Redundancy / Coupling / Risk Points
- **No explicit convergence loop artifact**: the repository lacks a standardized, round-indexed design record for iterative simplification, which increases process drift risk.
- **Patch output path is implicit**: without a fixed naming convention and persisted patch artifact, each round cannot be reproducibly replayed.
- **Round handoff is under-specified**: there is no stable definition of "previous round final artifact" as concrete files.

## 2) Round-1 Deletion & Convergence Strategy
- **Converge process to minimum artifact set**: use one mandatory per-round design doc plus one patch file.
- **Delete process ambiguity**: remove implicit workflow assumptions by writing an explicit minimal loop contract in this document.
- **Do not expand runtime code surface**: this round changes only process documentation/output artifacts to reduce execution risk.

## 3) Minimal Closed Loop to Keep
The minimum valid loop for each round N is defined as:
1. Load previous round artifacts (`docs/iter_round_{N-1}.md` + `iter_round_{N-1}_diff.patch`).
2. Create current round design doc (`docs/iter_round_{N}.md`).
3. Apply only design-mapped edits.
4. Export current diff artifact (`iter_round_{N}_diff.patch`).
5. Record concise round summary.

## 4) Explicit Delete / Merge / Converge File List
- **Converge into new mandatory file**: `docs/iter_round_1.md` (this file).
- **Converge into new mandatory artifact**: `iter_round_1_diff.patch` (generated after edits).
- **No code module merge in Round 1**: to minimize risk, scope is process hardening only.

## 5) Post-Iteration Target Structure
- `docs/iter_round_1.md` exists as the authoritative design source for Round 1.
- `iter_round_1_diff.patch` exists at repository root as reusable patch artifact.
- Future rounds append the same structure with strictly incremented indices.
