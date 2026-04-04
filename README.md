# CarRenderFactory

Developed with Unreal Engine 5

## Docs

- 项目进度文档：`PROJECT_PROGRESS.md`
- C++ 接入与使用文档：`CPP_USAGE_GUIDE.md`

- 项目框架图文档：`PROJECT_FRAMEWORK.md`

## Iterative Convergence Workflow

- 单轮执行入口：`scripts/iter_converge.sh`
- 设计文档命名：`docs/iter_round_{N}.md`
- 差异补丁命名：`iter_round_{N}_diff.patch`
- 本地隔离产物目录：`iterative_artifacts/round_{N}/workspace`

示例：

```bash
bash scripts/iter_converge.sh
```
