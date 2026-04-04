# Convergence Workflow（统一收敛流程）

> 相关导航：`docs/README.md`

## 目标

提供一个可重复、可审计、低耦合的迭代收敛流程，避免轮次状态和产物分散。

## 推荐入口

- `scripts/iter_converge.sh`

## 执行模式

### 1) 演练（不推进轮次状态）

```bash
scripts/iter_converge.sh --dry-run
```

- 生成/刷新本轮 workdir、patch、manifest。
- 不会修改：
  - `iterations/state/current_round.txt`
  - `iterations/LATEST_ROUND`

### 2) 正式执行（推进状态）

```bash
scripts/iter_converge.sh
```

- 生成/刷新：
  - `iterations/round_N/workdir`
  - `iterations/round_N/iter_round_N_diff.patch`
  - `iterations/round_N/MANIFEST.md`
- 更新状态：
  - `iterations/LATEST_ROUND` = `N`
  - `iterations/state/current_round.txt` = `N+1`

## 前置条件

- 必须存在设计文档：`docs/iter_round_N.md`
- `N` 取自 `iterations/state/current_round.txt`

## 产物定义

- `workdir`：隔离工作副本，用于下一轮参考。
- `iter_round_N_diff.patch`：当前仓库相对迭代目录排除后的 diff 快照。
- `MANIFEST.md`：本轮产物索引和来源映射。

## 维护建议

- 每轮结束后在 `PROJECT_PROGRESS.md` 记录“本轮目标 / 执行结果 / 待验证项”。
- 在 UE 环境可用时补充可运行证据（UBT 编译结果、PIE 截图、回归清单）。
