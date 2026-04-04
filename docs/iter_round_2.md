# Iteration Round 2 Design (State-Surface Reduction)

## 1) Current Redundancy / Coupling / Error-Risk Points
- Round initialization currently mixes two intentions: (a) initialize next isolated workspace, (b) capture current repo diff. If misunderstood, users may treat patch as workspace diff rather than repo diff.
- Generated patch files for multiple rounds are tracked by default unless explicitly managed.
- Manual invocation may accidentally reuse an existing round number.

## 2) This Round's Reduction & Convergence Strategy
- Keep the script single-purpose and deterministic: one invocation = one new round scaffold + one patch snapshot.
- Keep runtime artifacts out of versioned state via `.gitignore`, reducing accidental coupling between history and temporary outputs.
- Preserve strict no-overwrite behavior to eliminate silent state corruption.

## 3) Preserved Minimum Closed Loop
1. Allocate N.
2. Copy previous round workspace into `iterative_artifacts/round_N/workspace`.
3. Ensure `docs/iter_round_N.md` exists before round-specific edits.
4. Emit `iter_round_N_diff.patch` as reusable handoff artifact.

## 4) Delete / Merge / Converge List
- **Delete**: implicit assumption that all runtime artifacts should be tracked.
- **Converge**: round state output conventions into fixed path patterns.
- **Keep minimal**: no background daemon, no task queue, no extra config layer.

## 5) Target Structure After Iteration
- `scripts/iter_converge.sh` as single execution entry.
- `docs/iter_round_N.md` as mandatory design-first record.
- `iter_round_N_diff.patch` as patch handoff.
- `iterative_artifacts/round_N/workspace` as isolated mutable workspace.
