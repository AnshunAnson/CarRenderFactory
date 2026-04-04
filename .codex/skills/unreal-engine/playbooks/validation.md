# Validation Playbook

## Goal
Provide independent acceptance evidence by Hardness level.

## Required Roles
- `M`: `verifier` mandatory
- `H`: `verifier` + `code-reviewer` mandatory

Writer and reviewer must be separate lanes.

## Review-Agent Mandate (`M/H`)
Review-agent must evaluate:
- rule boundary clarity (are skill rules unambiguous and enforceable),
- agent readability (is the skill easy for agents to follow quickly),
- execution efficiency (does the skill minimize unnecessary work and retries).

Review-agent must also produce:
- `Context Experience`: reusable lessons from this task context,
- `Skill Optimization Proposals`: concrete edits to improve `unreal-engine` skill quality.

## Validation Checklist by Hardness
- `L`
  - confirm discovery assumptions are stated
  - confirm no high-risk paths were modified
- `M`
  - validate expected behavior in PIE
  - validate standalone behavior where relevant
  - confirm key logs/assertions are clean
  - return explicit unverified items
- `H`
  - complete `M`
  - validate multiplayer client/server scenario for touched paths
  - verify replication/GAS critical path assertions
  - require code-reviewer findings summary (or explicit no-findings statement)

## Harness Change Validation (when harness is modified)
- run targeted harness tests for touched scope
- run one representative CLI command proving the new/updated capability
- confirm backward compatibility for unaffected commands (smoke-level check)
- include `Harness Delta`, `Harness Verification`, and `Compatibility Note` in final output

## Verification Evidence Format
Output must include:
- `Verification Evidence`
  - Environment (`engine version`, `target`, `PIE/Standalone/Client-Server`)
  - Steps executed
  - Passed checks
  - Failed checks (if any)
  - Logs/assertions summary
  - Not covered / residual risk
  - Reviewer identity (`M/H` required)

For `M/H`, also include:
- `Review Agent Findings`
  - Boundary Clarity
  - Agent Readability
  - Execution Efficiency
- `Context Experience`
- `Skill Optimization Proposals`

When harness changed, also include:
- `Harness Delta`
- `Harness Verification`
- `Compatibility Note` (`none|limited|breaking`)

## Completion Gate
- `M/H` without verifier evidence -> incomplete.
- `H` without code-reviewer evidence -> incomplete.
- `M/H` without review-agent findings -> incomplete.
- missing reviewer separation (writer == reviewer) -> `Status: BLOCKED`.
- unresolved critical verification failure -> `Status: BLOCKED` (do not claim done).

## BLOCKED Output Requirements
When blocked, output must include:
- `Status: BLOCKED`
- unresolved risk list
- next owner/action for escalation
