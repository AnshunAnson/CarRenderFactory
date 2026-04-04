# Discovery Playbook

## Goal
Produce verifiable Unreal project context before any implementation.

## Output Contract
Return these evidence blocks:
- `.uproject` path and `EngineAssociation`
- module list and enabled plugins
- discovered assets relevant to task (Input/GAS/BP/etc.)
- existing code pattern references (at least 1-3 files/functions)
- harness capability coverage when automation is requested (`reuse|extend|fallback`)
- scriptability assessment (`scriptable|non-scriptable`) with rationale
- backend priority decision (`mcp|cli-anything|native-cli`)

## Hardness Execution Matrix
- `L`
  - Find `.uproject`
  - Read version and quick-scan `Source/` + `Content/` for task-related artifacts
- `M`
  - Full module/plugin extraction from `.uproject`
  - Map `Source/`, `Content/`, `Config/`
  - Enumerate task-critical assets and naming conventions
- `H`
  - Complete `M`
  - Add version/API compatibility verification against official docs when uncertain
  - Re-validate critical asset paths and ownership assumptions

## Sampling Policy
- `H`: no truncation on critical artifacts (`.uproject`, module/plugin declarations, task-related assets, touched subsystem files).
- `M`: sampling allowed for exploratory listing, but must explicitly state sampling caveat in output.
- `L`: quick scan is acceptable.

## Document-Specialist Citation Trigger
Trigger `document-specialist` when:
- plugin/API behavior is version-sensitive,
- local code behavior and docs appear conflicting, or
- required behavior is undocumented locally and affects correctness.

Required citation payload in output:
- official documentation link
- documentation version/date (if available)
- extracted rule
- impact on current implementation decision

## PowerShell Commands
```powershell
$uproject = Get-ChildItem -Recurse -Filter *.uproject | Select-Object -First 1
$uproject.FullName
Get-Content -Raw $uproject.FullName
Get-ChildItem .\Source -Recurse -Include *.h,*.cpp -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Get-ChildItem .\Content -Recurse -Filter *.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
Get-ChildItem .\Config -Recurse -File -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
```

## Optional Sampling Commands (`M` only)
```powershell
Get-ChildItem .\Source -Recurse -Include *.h,*.cpp -ErrorAction SilentlyContinue | Select-Object -First 80 -ExpandProperty FullName
Get-ChildItem .\Content -Recurse -Filter *.uasset -ErrorAction SilentlyContinue | Select-Object -First 200 -ExpandProperty FullName
```

## Harness Capability Discovery (when automation is in scope)
```powershell
Set-Location D:\UE\UE_5.7\agent-harness
cli-anything-unrealengine --help
cli-anything-unrealengine --json project info <path-to-uproject>
```

Decision rule:
- `mcp`: Unreal-engine MCP is available and covers required capability (preferred).
- `reuse`: MCP unavailable/insufficient, but harness command coverage exists and output is sufficient.
- `extend`: MCP unavailable/insufficient and harness exists but missing behavior/fields/flags needed by task.
- `fallback`: both MCP and harness are unavailable/unsafe/insufficient; use native UE CLI and record reason.

Priority:
1. Unreal-engine MCP
2. cli-anything-unrealengine
3. native UE CLI

## Scriptability Assessment
Mark as `scriptable` when:
- workflow consists of repeatable CLI/harness/native-UE commands,
- inputs and outputs are deterministic enough for automation,
- reruns are likely needed (build/cook/package/test loops).

Mark as `non-scriptable` when:
- primary work is exploratory reasoning and cannot be reliably scripted in this scope.

If `scriptable`, prefer script/harness path and include chosen command chain in `Delegation Plan`.

## Stop Conditions
Do not proceed to implementation until the evidence blocks are complete for the selected Hardness level.
