#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
WORKDIR_ROOT="$ROOT_DIR/tools/convergence/workdirs"

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <round_number>"
  exit 1
fi

ROUND="$1"
if ! [[ "$ROUND" =~ ^[0-9]+$ ]]; then
  echo "round_number must be a positive integer"
  exit 1
fi

PREV_ROUND=$((ROUND - 1))
CUR_DIR="$WORKDIR_ROOT/iter_round_${ROUND}"
PREV_DIR="$WORKDIR_ROOT/iter_round_${PREV_ROUND}"

mkdir -p "$WORKDIR_ROOT"
rm -rf "$CUR_DIR"
mkdir -p "$CUR_DIR"

if [[ "$ROUND" -gt 1 && -d "$PREV_DIR" ]]; then
  cp -a "$PREV_DIR/." "$CUR_DIR/"
fi

DOC_PATH="$ROOT_DIR/docs/iter_round_${ROUND}.md"
if [[ ! -f "$DOC_PATH" ]]; then
  cat > "$DOC_PATH" <<DOC
# Iteration Round ${ROUND} Design (N=${ROUND})

## 1) 当前系统核心冗余 / 耦合 / 错误风险点
- 待分析

## 2) 本轮删减 & 收敛策略
- 待定义

## 3) 保留的最小闭环定义
- 待定义

## 4) 明确删除 / 合并 / 收敛清单
- 待定义

## 5) 迭代后目标结构
- 待定义
DOC
fi

PATCH_PATH="$ROOT_DIR/iter_round_${ROUND}_diff.patch"
if git -C "$ROOT_DIR" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  git -C "$ROOT_DIR" diff -- . ":(exclude)iter_round_${ROUND}_diff.patch" > "$PATCH_PATH" || true
else
  : > "$PATCH_PATH"
fi

echo "Round ${ROUND} prepared."
echo "Workdir: $CUR_DIR"
echo "Design:  $DOC_PATH"
echo "Patch:   $PATCH_PATH"
