# 文档导航（Docs Index）

> 更新时间：2026-04-04
> 目的：统一文档入口，减少重复维护与口径冲突。

## 1. 核心文档（优先阅读）

1. `PROJECT_PROGRESS.md`（仓库根目录）
   - 项目阶段、风险、里程碑、当轮执行进展。
2. `PROJECT_FRAMEWORK.md`（仓库根目录）
   - 玩法/系统框架总图（战略视角）。
3. `docs/project_architecture_breakdown.md`
   - 架构细分执行版（模块边界、依赖方向、迁移里程碑）。
4. `docs/convergence_workflow.md`
   - 迭代收敛流程（脚本入口、产物、状态推进规则）。

## 2. 迭代文档（按轮次）

- `docs/iter_round_1.md`
- `docs/iter_round_1_summary.md`
- `docs/iter_round_2.md`
- `docs/iter_round_3.md`

用途：记录每轮“问题识别 -> 收敛策略 -> 产物定义”。

## 3. 文档职责边界（避免重复）

- **进度与优先级**：只在 `PROJECT_PROGRESS.md` 维护。
- **架构规则与依赖约束**：只在 `docs/project_architecture_breakdown.md` 维护。
- **流程脚本与执行规范**：只在 `docs/convergence_workflow.md` 维护。
- **框架总图与玩法蓝图**：只在 `PROJECT_FRAMEWORK.md` 维护。

## 4. 更新约定

每次涉及架构或流程变更时，至少同步以下两项：

1. 修改对应“单一责任文档”（例如流程变更优先更新 `docs/convergence_workflow.md`）。
2. 在 `PROJECT_PROGRESS.md` 中追加一条“变更摘要 + 待验证项”。

## 5. 推荐阅读顺序（新成员）

`PROJECT_PROGRESS.md` -> `PROJECT_FRAMEWORK.md` -> `docs/project_architecture_breakdown.md` -> `docs/convergence_workflow.md`
