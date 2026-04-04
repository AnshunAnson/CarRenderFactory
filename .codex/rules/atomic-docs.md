# 原子化文档规则 | Atomic Documentation Rules

## 1. 核心原则

### 1.1 单一职责
- 一篇文档只记录**一个概念**
- 一个概念只出现在**一篇文档**
- 文档之间通过**链接**关联

### 1.2 独立完整
- 不依赖其他文档即可理解
- 包含：是什么 + 为什么 + 怎么做

### 1.3 Token 优化
- 字数：50-200 字（最多 300 字）
- Token：100-400 tokens
- 阅读时间：< 30 秒

---

## 2. 文档结构

### 2.1 文件名规则
```
{category}-{name}.md

示例：
- arch-widget-base.md      (架构-组件)
- api-set-input-mode.md    (API-函数)
- cfg-focus-priority.md    (配置-参数)
- guide-migration-p0.md    (指南-迁移)
- issue-odr-violation.md   (问题-ODR违规)
```

### 2.2 内容模板

#### 概念类
```markdown
# {概念名} | {一句话定义}

**类型**: {概念/组件/配置/问题}
**状态**: {稳定/实验/废弃}
**关联**: [[相关文档1]] [[相关文档2]]

## 核心要点
- 要点1
- 要点2

## 使用场景
场景描述...

## 代码示例
```cpp
// 简短示例
```

## 注意事项
- 注意1
- 注意2
```

#### API 类
```markdown
# {函数名} | {功能简述}

**所属**: [[所属类]]
**签名**: `ReturnType FunctionName(Params)`
**版本**: UE 5.7+

## 参数
| 参数 | 类型 | 说明 |
|------|------|------|
| param | type | desc |

## 返回值
说明...

## 示例
```cpp
// 使用示例
```

## 相关
[[相关函数1]] [[相关函数2]]
```

#### 问题类
```markdown
# {问题简述} | {错误码/现象}

**类型**: {编译错误/运行时错误/性能问题}
**严重**: {P0/P1/P2}
**状态**: {已解决/待修复}

## 现象
...

## 原因
...

## 解决
...

## 预防
...
```

---

## 3. 分类体系

```
docs/
├── _meta/              # 元数据
│   ├── index.md        # 文档索引
│   ├── glossary.md     # 术语表
│   └── changelog.md    # 变更日志
│
├── arch/               # 架构 (architecture)
│   ├── widget-system/
│   ├── event-system/
│   └── data-structures/
│
├── api/                # API 文档
│   ├── uwidget-base/
│   ├── umenu-button/
│   └── umenu-panel/
│
├── cfg/                # 配置 (config)
│   ├── build-cs/
│   └── module-deps/
│
├── guide/              # 指南
│   ├── setup/
│   ├── migration/
│   └── best-practices/
│
├── issue/              # 问题记录
│   ├── resolved/
│   └── known/
│
└── ref/                # 参考
    ├── naming-conv/
    └── git-workflow/
```

---

## 4. 链接规范

### 4.1 内部链接
```markdown
[[文档名]]              # 同目录
[[dir/doc-name]]        # 跨目录
[[#标题]]               # 文档内锚点
```

### 4.2 外部链接
```markdown
[UE Docs](url)          # 引擎文档
[Source](file-path)     # 源码引用
```

---

## 5. 质量检查清单

- [ ] 字数 50-200 字
- [ ] 一个主题
- [ ] 独立可理解
- [ ] 包含示例
- [ ] 链接有效
- [ ] 命名规范
- [ ] 分类正确

---

## 6. 工具命令

### 创建新文档
```bash
# 使用模板创建
new-doc arch widget-base
new-doc api set-input-mode
new-doc issue odr-violation
```

### 检查文档
```bash
# 检查字数
check-doc-length

# 检查死链
check-doc-links

# 生成索引
generate-index
```

---

*规则版本: 1.0*
*最后更新: 2026-04-02*
