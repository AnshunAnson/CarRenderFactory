# 文档导航（Docs Index）

> 目的：统一入口、减少重复、明确每份文档的唯一职责。

## 核心文档一览

| 文档 | 作用 | 何时更新 |
|---|---|---|
| `PROJECT_PROGRESS.md` | 项目阶段、风险、里程碑、当轮进展 | 每轮执行后 |
| `PROJECT_FRAMEWORK.md` | 玩法与系统框架总图（战略视角） | 玩法框架变化时 |
| `docs/project_architecture_breakdown.md` | 架构细分边界与依赖约束（执行视角） | 模块边界/依赖变化时 |
| `docs/convergence_workflow.md` | 收敛流程与脚本执行规则 | 流程脚本变化时 |

## 迭代文档（历史记录）

- `docs/iter_round_1.md`
- `docs/iter_round_1_summary.md`
- `docs/iter_round_2.md`
- `docs/iter_round_3.md`

> 说明：迭代文档用于保留轮次决策上下文，默认按“历史记录”维护，不作为当前规则单一来源。

## 维护规则（清理后）

1. 任何信息只在**一个主文档**维护，其他文档仅做链接，不复制大段内容。
2. 架构或流程变更时，必须同步 `PROJECT_PROGRESS.md` 中的“变更摘要 + 待验证项”。
3. 新增文档前先判断是否可并入现有主文档，避免文档数量无序增长。

## 推荐阅读顺序

`PROJECT_PROGRESS.md` → `PROJECT_FRAMEWORK.md` → `docs/project_architecture_breakdown.md` → `docs/convergence_workflow.md`
