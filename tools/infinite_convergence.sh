#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ITER_DIR="$ROOT_DIR/iterations"
DOCS_DIR="$ROOT_DIR/docs"

mkdir -p "$ITER_DIR" "$DOCS_DIR"

latest_round="$(find "$ITER_DIR" -maxdepth 1 -type d -name 'round_*' | sed 's#.*/round_##' | sort -n | tail -1 || true)"
if [[ -z "${latest_round}" ]]; then
  prev_round=0
  next_round=1
else
  prev_round="$latest_round"
  next_round=$((prev_round + 1))
fi

new_round_dir="$ITER_DIR/round_${next_round}"
mkdir -p "$new_round_dir"

if [[ "$prev_round" -gt 0 ]]; then
  cp -a "$ITER_DIR/round_${prev_round}/." "$new_round_dir/"
fi

cat > "$DOCS_DIR/iter_round_${next_round}.md" <<DOC
# Iteration Round ${next_round} 设计文档

## 当前系统核心冗余 / 耦合 / 错误风险点
- 待填写

## 本轮删减 & 收敛策略
- 待填写

## 保留的最小闭环定义
- 待填写

## 明确删除 / 合并 / 收敛清单
- 待填写

## 迭代后目标结构
- 待填写
DOC

cat > "$new_round_dir/manifest.txt" <<MANIFEST
round=${next_round}
source_round=${prev_round}
source_commit=$(git -C "$ROOT_DIR" rev-parse --short HEAD 2>/dev/null || echo unknown)
created_at_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)
purpose=continuous_convergence_iteration
MANIFEST

patch_file="$ROOT_DIR/iter_round_${next_round}_diff.patch"
git -C "$ROOT_DIR" diff -- . ":(exclude)$patch_file" > "$patch_file"

echo "Initialized round ${next_round}."
echo "Design doc: $DOCS_DIR/iter_round_${next_round}.md"
echo "Round dir:   $new_round_dir"
echo "Patch file:  $patch_file"
