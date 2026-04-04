# Iteration Round 1 Design (Convergence-First)

## 1) Current Redundancy / Coupling / Risk
- The iteration workflow is purely verbal and manual, causing duplicated human steps and high chance of skipped actions.
- Round numbering, workspace isolation, and artifact generation are not enforced by code, creating implicit state and branch divergence risk.
- Patch generation has no standard entrypoint, so results are hard to reproduce.

## 2) This Round's Reduction & Convergence Strategy
- Converge all mandatory iteration mechanics into a single script entrypoint.
- Remove manual state branching by storing a single round counter file.
- Enforce strict directory structure for each round (`iterations/round_N/`) to keep artifacts isolated and traceable.

## 3) Minimum Closed Loop Kept
Minimum loop for each round:
1. Determine round number from one state file.
2. Create isolated round workspace from previous artifact.
3. Require a design document before allowing completion.
4. Emit standardized patch artifact for the round.
5. Advance state to next round.

## 4) Explicit Delete / Merge / Converge List
- **Converge**: scattered manual round operations -> `scripts/iter_converge.sh`.
- **Delete (process-level)**: manual, ad-hoc numbering and output paths.
- **Keep**: only required control files:
  - `docs/iter_round_{N}.md`
  - `iterations/state/current_round.txt`
  - `iterations/round_{N}/` artifacts

## 5) Post-Iteration Target Structure
- `scripts/iter_converge.sh` (single orchestration script)
- `docs/iter_round_1.md` (design source of truth for this round)
- `iterations/state/current_round.txt` (single explicit state)
- `iterations/round_1/workdir/` (isolated copy)
- `iterations/round_1/iter_round_1_diff.patch` (round artifact)
