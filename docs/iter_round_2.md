# Iteration Round 2 Design (Hardening)

## 当前系统核心冗余 / 耦合 / 错误风险点
- `scripts/iter_converge.sh` 仅创建新目录，**未复制上一轮最终产物**，不满足“隔离修改”硬约束。
- diff 生成仅依赖 `git diff`，**无法覆盖未跟踪文件**，导致“产物固化”不完整。
- 轮次目录结构缺乏统一清单文件，后续轮次难以自动承接输入集。

## 本轮删减 & 收敛策略
- 删减“隐式流程假设”：把“上一轮产物复制”显式化为脚本强制步骤。
- 收敛 patch 生成路径：统一由单函数导出“跟踪变更 + 未跟踪文件”到 `iter_round_{N}_diff.patch`。
- 收敛轮次元数据：每轮输出固定 `MANIFEST.md`，减少状态分叉。

## 保留的最小闭环定义
1. 读取 `iterations/LATEST_ROUND` 得到 `N-1`。
2. 创建 `iterations/round_{N}/workspace/` 并复制上一轮关键产物（设计文档、diff、轮次目录）。
3. 若 `docs/iter_round_{N}.md` 不存在则先生成模板（设计先行）。
4. 导出完整 patch 到 `iter_round_{N}_diff.patch`。
5. 写入 `iterations/round_{N}/MANIFEST.md`，更新 `iterations/LATEST_ROUND`。

## 明确删除 / 合并 / 收敛清单
- **删除**：删除 `round_{N}/README.md` 的自由格式输出，改为结构化 `MANIFEST.md`（统一字段）。
- **合并**：将 patch 生成逻辑合并为单入口函数 `write_patch`。
- **收敛**：将“复制上一轮产物”收敛为单入口函数 `copy_previous_artifacts`。

## 迭代后目标结构
- `docs/iter_round_2.md`（本轮设计）
- `scripts/iter_converge.sh`（硬化后的单入口）
- `iterations/round_2/workspace/`（隔离输入副本）
- `iterations/round_2/MANIFEST.md`（结构化产物索引）
- `iter_round_2_diff.patch`（本轮完整补丁）
