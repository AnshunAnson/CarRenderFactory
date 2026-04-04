# Iteration Round 1 Design

## 1) Current redundancy / coupling / risk points
- The repo has no fixed artifact contract for "iteration round" outputs, so each round can drift in structure and naming.
- There is no explicit, versioned handoff document linking: design doc, workspace snapshot, and diff patch.
- The requested convergence loop exists only in chat text, which creates execution ambiguity and increases human error risk.

## 2) This round's deletion & convergence strategy
- **Delete ambiguity**, not product code: converge round artifacts into a single deterministic folder schema.
- Introduce the minimum required files for a reusable closed loop:
  1. round design (`docs/iter_round_1.md`)
  2. round workspace handoff marker (`iterations/round_1/ROUND_CONTEXT.md`)
  3. round diff patch (`iter_round_1_diff.patch`)
- Avoid adding new runtime modules, scripts, or config branches in this round.

## 3) Minimum closed loop retained
A round is valid only if all three are present and linked:
- Design doc explains what to remove/converge.
- Round context file records input/output boundaries.
- Diff patch captures exact changes for replay.

## 4) Explicit delete / merge / converge list
- **Converge**: adopt `iterations/round_{N}/` as the only round workspace marker path.
- **Converge**: adopt `docs/iter_round_{N}.md` as the only design doc naming.
- **Converge**: adopt `iter_round_{N}_diff.patch` as the only patch naming.
- **Delete (conceptual)**: any future ad-hoc round naming outside this schema.

## 5) Target structure after this round
- `docs/iter_round_1.md`
- `iterations/round_1/ROUND_CONTEXT.md`
- `iter_round_1_diff.patch`
