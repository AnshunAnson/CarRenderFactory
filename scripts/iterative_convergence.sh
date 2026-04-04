#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

round="${1:-1}"
if ! [[ "$round" =~ ^[0-9]+$ ]]; then
  echo "Round must be an integer, got: $round" >&2
  exit 1
fi

round_dir="iterations/round_${round}"
mkdir -p "$round_dir"

snapshot_dir="$round_dir/workdir_snapshot"
mkdir -p "$snapshot_dir"

# 复制当前工程（排除版本库元数据和轮次产物目录本身，避免递归）
rsync -a --delete \
  --exclude '.git' \
  --exclude 'iterations' \
  --exclude 'Saved' \
  --exclude 'Intermediate' \
  ./ "$snapshot_dir/"

# 为本轮生成一个设计文档模板（若不存在）
doc_path="docs/iter_round_${round}.md"
if [[ ! -f "$doc_path" ]]; then
  cat > "$doc_path" <<DOC
# Iteration Round ${round} Design

## 当前系统核心冗余 / 耦合 / 错误风险点
- TODO

## 本轮删减 & 收敛策略
- TODO

## 保留的最小闭环定义
- TODO

## 明确删除 / 合并 / 收敛的模块 / 文件清单
- TODO

## 迭代后目标结构
- TODO
DOC
fi

patch_path="iter_round_${round}_diff.patch"
git diff -- . ":(exclude)$patch_path" > "$patch_path"

cat <<MSG
[iterative-convergence] round=${round}
- snapshot: ${snapshot_dir}
- design:   ${doc_path}
- diff:     ${patch_path}

Next: implement changes strictly from ${doc_path}, then regenerate ${patch_path}.
MSG
