# CarRenderFactory C++ 使用文档（UE5.7）

> 目标读者：不知道如何把当前 C++ 玩法代码接到自己项目里的开发者。  
> 适用范围：`Source/CarRenderFactory` 模块（角色、战斗、气势、技能、GameMode、PlayerState）。
> 主线定位：**FPS射击 + 近战博弈手感 + 对战 + 武器 + 背包 + 联机**。摸金是边缘验证模块。

---

## 1. 前置条件

1. Unreal Engine **5.7**（或与项目一致版本）。
2. C++ 工程（非纯蓝图工程）。  
   - 如果你是纯蓝图项目：先在 UE 里 `Tools -> New C++ Class` 创建任意类，让工程转换为 C++ 项目。
3. 已启用 Gameplay Ability System 相关模块（本项目 `Build.cs` 已配置）。

---

## 2. 把代码“接到游戏里”的最短路径

### Step A：把核心类设成默认类

在你的关卡 / GameMode 蓝图里设置：

- `GameMode Class` → `AOTA_GameMode`
- `PlayerController Class` → `AOTA_PlayerController`
- `PlayerState Class` → `AOTA_PlayerState`
- `Default Pawn Class` → 使用基于 `AOTA_Character` 的蓝图角色

> 如果你已经有自己的 GameMode，也可以继承 `AOTA_GameMode` 做二次封装。

---

### Step B：创建角色蓝图并挂载默认组件

以 `AOTA_Character` 为父类创建 BP（例如 `BP_OTA_Character`）。  
该类会自动创建：

- `AbilitySystemComponent`
- `AttributeSet`
- `CombatComponent`
- `QiComponent`

你需要做的是：

1. 给角色配置 Mesh/动画。
2. 确保角色可被 `AOTA_PlayerController` 控制（可移动、可转向）。

---

### Step C：配置输入（重点）

`AOTA_PlayerController` 依赖 Enhanced Input：

1. 创建 `Input Mapping Context`（例如 `IMC_Default`, `IMC_Combat`）。
2. 创建 `Input Action`：
   - `Move`
   - `Look`
   - `Jump`
   - `LightAttack`
   - `Parry`
   - `HeavyAttack`
   - `Skill1`（气盾）
   - `Skill2`（瞬步）
   - `Skill3`（探宝）
3. 在 `AOTA_PlayerController`（或其 BP 子类）里把这些 Action 资产赋值到对应字段。

---

### Step D：配置 PlayerState 的默认能力/效果（可选但推荐）

`AOTA_PlayerState` 提供两组可配置数组：

- `DefaultAbilities`
- `DefaultEffects`

你可以在 `AOTA_PlayerState` 的 BP 子类中填入 GAS Ability/Effect 类，角色进入游戏时会自动初始化并应用。

---

## 3. 当前代码中的核心规则（你会直接感受到）

### 3.1 气势（Qi）规则

- **非摸金状态**：自动回气（受回气倍率影响）。
- **摸金状态**：每秒固定掉气。
- 按 Qi 档位（High/Normal/Low/Critical）自动调整攻击/移速/回气倍率。

---

### 3.2 技能规则

三个通用技能由 `AOTA_Character` 实现：

1. `Skill1`：气盾（挡一次伤害）
2. `Skill2`：瞬步（前冲位移）
3. `Skill3`：探宝（状态持续）

共性约束：

- 共享冷却（默认 8 秒）
- 消耗 Qi（默认 20/15/10）
- 低 Qi 阈值下禁用技能

---

### 3.3 战斗与伤害

- 近战伤害会乘以攻击倍率（受 Qi / 金币成长影响）。
- 克制关系仍然有效（Counter / Countered / Draw）。

---

### 3.4 排名

`AOTA_GameMode` 中提供基础排行：

- `CalculatePlayerScore`
- `GetSortedLeaderboard`

当前评分默认：`击杀 * 权重 + 金币 * 权重`。

---

### 3.5 关于“摸金”在项目中的定位

- 摸金不是主玩法目标，而是用于验证“拾取 → 背包 → 成长/结算输入”的一条边缘链路。
- 如果你当前阶段只做 FPS/近战/PVP 手感，建议先把摸金逻辑开关关闭（或不接场景 Actor），不影响主循环联调。

---

## 4. 常见接入失败点（必看）

1. **只改了 C++ 没配蓝图资产**  
   现象：角色能动但技能和攻击没反应。  
   原因：Input Action 没绑定到 `AOTA_PlayerController` 字段。

2. **PlayerState 不是 AOTA_PlayerState**  
   现象：默认能力/效果不生效，排行榜分数异常。  
   原因：GameMode 里 PlayerStateClass 没设对。

3. **Pawn 不是 AOTA_Character 或其子类**  
   现象：Qi/Combat/Skill 逻辑缺失。  
   原因：默认 Pawn 类错误。

4. **没有 GAS 资产（Ability/Effect）**  
   现象：按键触发后无能力效果。  
   原因：`DefaultAbilities` / `DefaultEffects` 为空，或标签/资产未配置。

---

## 5. 最小验收清单（你可以按这个逐项对）

1. 进入 PIE 后，角色可移动、可视角旋转。
2. `Skill1/2/3` 按键有行为反馈（至少打印日志）。
3. 摸金状态切换时，Qi 从回升变为下降。
4. 造成伤害后，目标生命下降，死亡后进入 Dead 状态。
5. 多玩家场景下，排行榜能返回有序列表。

---

## 6. 建议下一步

1. 先做一个“主循环验证地图”（2-4 个玩家，仅保留射击/近战/武器/背包/联机关键链路）。
2. 把技能触发、命中反馈、Qi 变化做成屏幕调试文本（便于快速定位）。
3. 主循环稳定后，再把摸金点当作边缘模块接入验证。
