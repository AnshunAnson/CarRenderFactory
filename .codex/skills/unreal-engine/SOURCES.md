# Unreal Engine Skill Sources

Updated on: 2026-03-31

## Upstream Sources
1. Unreal Engine core skill
- Repo: https://github.com/DSTN2000/claude-unreal-engine-skill
- Base: `SKILL.md`
- Reference docs mirrored under `references/`

2. Unreal C++ architecture practices
- Repo: https://github.com/sickn33/antigravity-awesome-skills
- Path: `skills/unreal-engine-cpp-pro/SKILL.md`

3. Game engineering governance patterns (UE-relevant subset only)
- Repo: https://github.com/404kidwiz/claude-supercode-skills
- Path: `game-developer-skill/SKILL.md`

## Integration Notes
- This skill is now single-entry and includes Hardness (`H/M/L`) plus subagent orchestration.
- Cross-engine guidance was intentionally removed; only Unreal-relevant material is retained.
- Local harness integration is enabled:
  - Path: `D:\UE\UE_5.7\agent-harness`
  - Docs: `UE5.md`, `AGENTS.md`
  - CLI entry: `cli-anything-unrealengine`
- Legacy directories removed in this consolidation:
  - `unreal-engine-cpp-pro`
  - `game-developer`
  - `unreal-engine-suite`
