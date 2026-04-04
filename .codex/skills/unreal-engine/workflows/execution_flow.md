# Execution Flow (执行流程)

> **权威源**: 本文件定义 UE 任务的标准执行流程
> **版本**: 1.0

---

## 标准执行流程

```
Step 1: 分配 Hardness
    │   → 详见: frameworks/hardness.md
    │
    ▼
Step 2: 运行 Discovery
    │   → 详见: workflows/discovery.md
    │   → 通过 playbooks/discovery.md 执行
    │
    ▼
Step 3: 评估 Scriptability
    │   → 详见: frameworks/scriptability.md
    │   → 选择执行路径 (script/harness 优先)
    │
    ▼
Step 4: 路由到专业化处理
    │   ├── C++ 架构/性能 → playbooks/cpp_architecture.md
    │   ├── 多人/复制/GAS → playbooks/multiplayer_and_net.md
    │   └── MCP Agent 编排 → playbooks/mcp_agent_orchestration.md
    │
    ▼
Step 5: 验证
    │   → 详见: playbooks/validation.md
    │
    ▼
Step 6: 生成 Review Agent 分析和 Context Experience
    │
    ▼
Step 7: 返回输出契约字段
    │   → 详见: contracts/output_contract.md
```

---

## Discovery 采样策略

### H 级别
- **禁止** `First N` 截断关键工件
- 必须完整扫描:
  - `.uproject`
  - modules/plugins
  - 任务相关资产
  - 涉及的子系统

### M 级别
- 允许采样列表
- 必须显式声明采样警告

### L 级别
- 允许快速扫描

---

## Discovery 命令 (PowerShell)

```powershell
# 查找项目文件
Get-ChildItem -Recurse -Filter *.uproject | Select-Object -ExpandProperty FullName

# 查找源代码文件
Get-ChildItem .\Source -Recurse -Include *.h,*.cpp -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找资产文件
Get-ChildItem .\Content -Recurse -Filter *.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找 Input Action
Get-ChildItem .\Content -Recurse -Filter IA_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找 Input Mapping Context
Get-ChildItem .\Content -Recurse -Filter IMC_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName

# 查找 Blueprint
Get-ChildItem .\Content -Recurse -Filter BP_*.uasset -ErrorAction SilentlyContinue | Select-Object -ExpandProperty FullName
```

---

## 流程与 Hardness 对应

| Step | L | M | H |
|------|---|---|---|
| 1. Hardness 分配 | ✅ | ✅ | ✅ |
| 2. Discovery | 快速扫描 | 采样 + 警告 | 完整扫描 |
| 3. Scriptability | ✅ | ✅ | ✅ |
| 4. 专业化路由 | 可选 | ✅ | ✅ (多车道) |
| 5. 验证 | 假设声明 | PIE + 独立检查 | 客户端/服务器 + 复制/GAS |
| 6. Review Agent | ❌ | ✅ | ✅ |
| 7. 输出契约 | 简化版 | 完整版 | 完整版 + 优化建议 |

---

## 工作示例

### L 级别示例 (单命令查找)

```
Step 1: Hardness = L (低风险 Q&A)
Step 2: Discovery = 快速扫描
Step 3: Scriptability = non-scriptable (澄清性回答)
Step 4: 路由 = 无需
Step 5: 验证 = 假设声明
Step 6: Review = 无需
Step 7: 输出 = 简化契约

输出:
Hardness: L
Delegation Plan: none
Verification Evidence: assumption stated, no high-risk path touched
Status: DONE
```

### M 级别示例 (多文件功能修复)

```
Step 1: Hardness = M (多文件变更)
Step 2: Discovery = 采样扫描 + 警告声明
Step 3: Scriptability = scriptable + mcp
Step 4: 路由 = playbooks/cpp_architecture.md
Step 5: 验证 = PIE + 独立检查
Step 6: Review = 独立 verifier
Step 7: 输出 = 完整契约

输出:
Hardness: M
Delegation Plan: Team A(explore) + Team B(executor) -> Team C(verifier)
Verification Evidence: PIE + standalone checks, residual risk listed
Status: DONE
```

### H 级别示例 (复制 + GAS 关键路径)

```
Step 1: Hardness = H (复制/GAS/网络权威)
Step 2: Discovery = 完整扫描 (无截断)
Step 3: Scriptability = scriptable + mcp
Step 4: 路由 = playbooks/multiplayer_and_net.md (并行车道)
Step 5: 验证 = 客户端/服务器 + 复制/GAS 断言
Step 6: Review = 独立 code-reviewer
Step 7: 输出 = 完整契约 + 优化建议

输出:
Hardness: H
Delegation Plan: planner->architect->executor with parallel replication lane + GAS lane -> verifier -> code-reviewer
Verification Evidence: client/server checks, replication/GAS assertions, unresolved list if any
Status: DONE|BLOCKED
```

---

*文档版本: 1.0*
*最后更新: 2026-04-02*
*维护者: SoloCoder (AI Agent Orchestrator)*
