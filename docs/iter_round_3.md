# Iteration Round 3 Design (Execution Convergence)

> 状态：**历史归档（冻结）**
> 说明：本文件仅保留 Round 3 决策上下文；当前流程规则请以 `docs/convergence_workflow.md` 为准。

## 1) 当前系统核心冗余 / 耦合 / 错误风险点
- 收敛脚本入口分散（`scripts/` 与 `tools/` 并存），新一轮执行者难以判断“哪个是权威入口”。
- 轮次状态同时存在 `iterations/LATEST_ROUND` 与 `iterations/state/current_round.txt` 两套标记，存在漂移风险。
- 轮次产物路径有三种风格（根目录 patch、`iterations/round_N`、`tools/convergence/workdirs`），审计时上下文切换成本较高。

## 2) 本轮删减 & 收敛策略
- 保留“**轻量、可追溯、可人工审查**”三原则，不做额外抽象层。
- 将本轮定义为“治理轮”：先补齐 round_3 设计与清单，再把轮次游标统一推进到 3。
- 继续使用现有脚本生成最小产物，避免一次性重构脚本体系导致流程中断。

## 3) 保留的最小闭环定义
1. 明确本轮设计文档（本文件）作为决策源。
2. 生成并保留本轮 diff 快照（`iter_round_3_diff.patch`）。
3. 维护 round 指针（`LATEST_ROUND` 与 `current_round.txt`）与轮次目录一致。
4. 在 `iterations/round_3/` 写入 manifest，保证下一轮可回溯来源。

## 4) 明确删除 / 合并 / 收敛清单
- **本轮不删除脚本文件**，仅收敛流程状态与文档。
- **收敛状态**：
  - `iterations/LATEST_ROUND` -> `3`
  - `iterations/state/current_round.txt` -> `3`
- **新增产物**：
  - `docs/iter_round_3.md`
  - `iterations/round_3/MANIFEST.md`
  - `iter_round_3_diff.patch`

## 5) 迭代后目标结构
- `docs/iter_round_N.md`：设计与收敛策略主记录。
- `iterations/round_N/MANIFEST.md`：每轮来源与产物索引。
- `iterations/LATEST_ROUND` + `iterations/state/current_round.txt`：双游标保持一致。
- `iter_round_N_diff.patch`：跨轮交接的可复用补丁快照。
