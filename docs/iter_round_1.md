# Iteration Round 1 Design

## 当前系统核心冗余 / 耦合 / 错误风险点
1. 迭代流程仅存在于口头指令，缺少可执行入口，导致每轮执行步骤可能漂移。
2. 轮次产物（设计文档、diff、工作目录）命名规则未固化，容易出现人工命名错误。
3. “复制上轮产物到新目录”没有标准化脚本，容易误改原目录。

## 本轮删减 & 收敛策略
1. **删减人工步骤**：将轮次目录初始化、产物路径计算、命名规范固化到单一脚本中。
2. **收敛入口**：统一使用 `scripts/iterative_convergence.sh` 作为唯一流程入口，避免多入口分叉。
3. **收敛状态表达**：采用轮次目录 `iterations/round_N/` 承载每轮文档与补丁，减少隐式状态。

## 保留的最小闭环定义
最小可用闭环仅包含：
1. 轮次目录创建；
2. 设计文档模板落地；
3. 基于 Git 工作区生成标准 diff 补丁；
4. 输出下一轮建议命令。

## 明确删除 / 合并 / 收敛的模块 / 文件清单
- 新增并收敛到单入口：`scripts/iterative_convergence.sh`
- 新增并固化本轮设计：`docs/iter_round_1.md`
- 新增并固化本轮补丁：`iter_round_1_diff.patch`

> 本轮未执行业务代码删除，优先先收敛流程入口，避免对现有 Unreal 工程产生非必要风险。

## 迭代后目标结构
- `docs/iter_round_1.md`：本轮第一性原理设计依据
- `scripts/iterative_convergence.sh`：轮次迭代统一入口
- `iterations/round_1/`：可回溯目录（工作副本与元数据）
- `iter_round_1_diff.patch`：本轮统一 diff 产物
