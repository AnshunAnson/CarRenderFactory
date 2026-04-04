# CarRenderFactory 项目架构细分（执行版）

> 更新时间：2026-04-04
> 目标：将当前“单模块多子域”结构细分为可并行开发、可验证、可迁移的架构边界。

---

## 1. 当前代码域切分（按职责）

当前 `Source/CarRenderFactory` 已形成 6 个主要域：

1. **Core 域**（对局状态与规则骨架）
   - 代表文件：
     - `Core/OTA_GameMode.*`
     - `Core/OTA_PlayerState.*`
     - `Core/OTA_PlayerController.*`
     - `Core/OTA_AttributeSet.*`
     - `Core/OTA_QiComponent.*`
2. **Character 域**（角色实体与入口）
   - 代表文件：
     - `Character/OTA_Character.*`
3. **Combat 域**（战斗判定与伤害）
   - 代表文件：
     - `Combat/OTA_CombatComponent.*`
     - `Combat/OTA_CombatTypes.h`
4. **Input/Data 域**（输入映射与配置行结构）
   - 代表文件：
     - `Input/OTA_InputConfig.h`
     - `Data/OTA_DataTableRows.h`
5. **Template Variants 域**（玩法模板样例）
   - 代表目录：
     - `TP_FirstPerson/Variant_Shooter/**`
     - `TP_FirstPerson/Variant_Horror/**`
6. **Module Root 域**（模块生命周期）
   - 代表文件：
     - `CarRenderFactory.Build.cs`
     - `CarRenderFactory.h/.cpp`

---

## 2. 分层架构（建议作为长期稳定目标）

```text
L0 Entry Layer
  - PlayerController / Character 输入入口

L1 Gameplay Domain Layer
  - Combat / Skill / Inventory(待落地) / Qi

L2 Match Rule Layer
  - GameMode / ScoreRule / MatchState

L3 Data & Config Layer
  - DataTable Row Struct / Runtime Config Cache

L4 Infrastructure Layer
  - Replication / RPC / AbilitySystem / Engine Services
```

### 关键约束

- L0 不直接写最终属性（Health/Qi/Gold），必须经 L1/L2。
- L1 只依赖抽象规则，不直接硬编码比分规则；比分规则落在 L2。
- L2 的可调参数必须来自 L3（DataTable/Config），禁止散落 magic number。
- L4 仅提供能力，不承载业务规则。

---

## 3. 子域边界（细分到可开发任务）

### A. Combat 子域
- 输入：攻击请求、命中目标、角色倍率快照。
- 输出：伤害事件、受击反馈事件。
- 禁止：直接修改排名或结算。

### B. Qi 子域
- 输入：击中/受击/状态切换事件。
- 输出：Qi 数值、Qi 档位变化事件。
- 禁止：直接结算伤害。

### C. MatchRule 子域
- 输入：击杀、金币、生存时间、局内事件。
- 输出：玩家分数、排名、胜负状态。
- 禁止：直接处理输入或动画反馈。

### D. Inventory/Weapon 子域（下一阶段）
- 输入：拾取/丢弃/切换请求。
- 输出：装备槽状态、可用武器状态。
- 禁止：直接写 PlayerState 最终统计（通过 MatchRule 汇总）。

---

## 4. 目录细分落地方案（不破坏现有代码）

建议在 `Source/CarRenderFactory` 下逐步收敛为：

```text
CarRenderFactory/
  Core/
    Match/
    Player/
    Attributes/
  Gameplay/
    Combat/
    Qi/
    Inventory/      # 待落地
    Weapons/        # 待落地
  Input/
  Data/
  UI/
  Net/
  Templates/
    TP_FirstPerson/
```

> 注意：本轮先做架构文档细分，不进行大规模迁移；采用“新代码按新结构放置，旧代码逐步迁移”的低风险策略。

---

## 5. 依赖方向（必须单向）

```text
Input -> Gameplay -> MatchRule
Data  -> Gameplay -> MatchRule
Net   -> Gameplay / MatchRule (能力注入)
UI    <- Gameplay / MatchRule (只读展示)
```

- `UI` 不反向依赖 `Gameplay` 具体实现（通过事件或只读接口）。
- `Templates` 仅作为示例实现层，不反向污染核心规则域。

---

## 6. 迭代拆分（3 个执行里程碑）

### Milestone 1（当前轮）
- 完成架构细分文档。
- 固化目录与依赖方向约束。

### Milestone 2（开发轮）
- 新增 `Gameplay/Inventory`、`Gameplay/Weapons` 目录并接入主循环。
- 将比分规则由 `GameMode` 内联逻辑抽离为独立规则函数（可 DataTable 驱动）。

### Milestone 3（稳定轮）
- 完成模板域与核心域解耦，减少 `TP_FirstPerson` 对核心玩法层的耦合。
- 补齐联机权威验证清单（PIE 多人 + 关键 RPC）。

---

## 7. 文档联动与维护规则

- 架构变更时同时更新：
  1. `Source/ARCHITECTURE.md`（总览）
  2. `PROJECT_PROGRESS.md`（阶段进度）
  3. 本文档（细分边界与里程碑）
- 新增核心子域时，必须补充“输入/输出/禁止项”三元说明。

