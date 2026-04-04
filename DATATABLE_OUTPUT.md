# OTA DataTable 输出

以下是可直接导入 Unreal `DataTable` 的 CSV 数据（与当前 C++ 默认参数对齐）。

## 1) 技能表 `DT_OTA_Skills.csv`

| Name | SkillId | QiCost | DurationSeconds | SharedCooldownSeconds | ExtraValue |
|---|---|---:|---:|---:|---:|
| QiShield | QiShield | 20 | 2 | 8 | 1 |
| Dash | Dash | 15 | 0 | 8 | 300 |
| TreasureSense | TreasureSense | 10 | 5 | 8 | 0 |

## 2) 摸金成长表 `DT_OTA_GoldGrowth.csv`

| Name | Stack | GoldRequired | AttackMultiplier | QiRegenMultiplier |
|---|---:|---:|---:|---:|
| Stack0 | 0 | 0 | 1.00 | 1.00 |
| Stack1 | 1 | 50 | 1.05 | 1.10 |
| Stack2 | 2 | 100 | 1.10 | 1.20 |
| Stack3 | 3 | 150 | 1.15 | 1.30 |

## 3) 气势档位表 `DT_OTA_QiLevels.csv`

| Name | MinQiInclusive | MaxQiInclusive | AttackMultiplier | MoveSpeedMultiplier | QiRegenMultiplier |
|---|---:|---:|---:|---:|---:|
| High | 80 | 100 | 1.10 | 1.00 | 1.20 |
| Normal | 20 | 79.99 | 1.00 | 1.00 | 1.00 |
| Low | 0.01 | 19.99 | 0.90 | 0.95 | 1.00 |
| Critical | 0 | 0 | 0.80 | 0.95 | 1.00 |

## 4) 结算权重表 `DT_OTA_ScoreRules.csv`

| Name | KillWeight | GoldWeight |
|---|---:|---:|
| DefaultFFA | 100 | 1 |

## 5) 行结构（C++）

行结构定义文件：`Source/CarRenderFactory/Data/OTA_DataTableRows.h`
- `FOTASkillDataRow`
- `FOTAGoldGrowthRow`
- `FOTAQiLevelRow`
- `FOTAScoreRuleRow`
