# Iteration Round 1 Design (Convergence-First)

## 1) Current Redundancy / Coupling / Error-Risk Points
- The repo has no single executable workflow for “design-first iterative convergence”, so execution relies on manual memory and ad-hoc commands.
- Round artifacts (design + diff + workspace snapshot path) are not normalized, which increases state branching risk across rounds.
- There is no deterministic round number allocation; conflicting filenames can silently overwrite prior outputs.

## 2) This Round's Reduction & Convergence Strategy
- Introduce **one** minimal script to orchestrate a single round lifecycle:
  1. Allocate round number automatically.
  2. Clone previous round workspace into a new isolated workspace directory.
  3. Enforce design-doc-first creation.
  4. Export unified diff patch for this round.
- Avoid introducing plugin/framework dependencies; shell + git only.
- Keep output contract fixed and small to reduce ambiguity.

## 3) Preserved Minimum Closed Loop
Minimum loop retained after this round:
1. Read previous round output.
2. Create current round design doc.
3. Apply modifications in isolated round workspace.
4. Generate `iter_round_{N}_diff.patch`.
5. Emit summary instructions for next round handoff.

## 4) Explicit Delete / Merge / Converge List
- **Converge**: scattered manual iteration steps → `scripts/iter_converge.sh` single entrypoint.
- **Delete by replacement**: implicit round naming and manual patch naming conventions.
- **Not expanded**: no multi-tool orchestration layer, no custom metadata DB, no daemon loop (avoids over-engineering).

## 5) Target Structure After Iteration
- `docs/iter_round_1.md` (this design, as traceable baseline)
- `scripts/iter_converge.sh` (single-round deterministic executor)
- `iterative_artifacts/round_{N}/...` (runtime-generated, local artifacts)

## 6) Round 1 Completion Notes
- Added script-based deterministic round scaffolding.
- Kept loop execution intentionally single-round per invocation to avoid runaway side effects in non-interactive environments while preserving infinite continuation capability via repeated invocation.
