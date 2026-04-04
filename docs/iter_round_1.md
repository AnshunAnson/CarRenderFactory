# Iteration Round 1 Design (First-Principles Convergence)

## 1) 当前系统核心冗余 / 耦合 / 错误风险点

1. **流程定义分散且隐式**：仓库中缺少统一、可执行的“收敛迭代”入口，容易导致不同执行者按各自理解运行。
2. **轮次产物缺少固定结构**：没有统一位置保存“设计文档 + diff 补丁 + 上轮快照”，回溯成本高。
3. **迭代步骤依赖手工记忆**：复制目录、写文档、导出补丁等步骤若手动执行，极易漏项或顺序错误。

## 2) 本轮删减 & 收敛策略

- **删减分叉执行路径**：将“收敛迭代”收敛为单一脚本入口 `scripts/converge_iter.sh`，避免同一任务多种做法。
- **删减隐式命名规则**：脚本强制使用 `iterations/round_N`、`docs/iter_round_N.md`、`iter_round_N_diff.patch` 的固定命名。
- **收敛最小闭环**：仅保留四步必需链路：准备轮次目录 → 复制上轮产物 → 生成设计模板 → 生成补丁文件。

## 3) 保留的最小闭环定义

最小可用闭环（MVP）定义：

1. 可从仓库根目录一条命令初始化新轮次；
2. 新轮次目录中必须有上一轮快照目录；
3. 必须生成本轮设计文档模板；
4. 必须生成本轮 diff 补丁文件。

## 4) 明确删除 / 合并 / 收敛的模块 / 文件清单

### 收敛（新增并替代分散人工步骤）
- `scripts/converge_iter.sh`：统一执行入口，替代分散人工操作。
- `docs/iter_round_1.md`：本轮设计文档，作为“先设计后改动”的强约束。
- `iterations/round_1/iter_round_1_diff.patch`：本轮补丁固化产物。

### 删除
- 本轮**不直接删除业务代码**，优先先收敛执行流程，降低后续改动错误面。

## 5) 迭代后目标结构

```text
repo/
├── docs/
│   └── iter_round_1.md
├── scripts/
│   └── converge_iter.sh
└── iterations/
    └── round_1/
        ├── workdir_from_round_0/
        └── iter_round_1_diff.patch
```

## 6) 设计与改动对应关系（强绑定）

- 设计中的“单入口收敛” → 代码实现：`scripts/converge_iter.sh`
- 设计中的“先设计后改动” → 文档实现：`docs/iter_round_1.md`
- 设计中的“产物固化” → 产物实现：`iterations/round_1/iter_round_1_diff.patch`
