#!/usr/bin/env bash
set -euo pipefail

# Minimal convergence loop bootstrapper.
# Usage:
#   scripts/converge_iter.sh <round_number>

if [[ $# -ne 1 ]]; then
  echo "Usage: $0 <round_number>" >&2
  exit 1
fi

ROUND="$1"
if ! [[ "$ROUND" =~ ^[0-9]+$ ]] || [[ "$ROUND" -lt 1 ]]; then
  echo "round_number must be an integer >= 1" >&2
  exit 1
fi

PREV=$((ROUND - 1))
ROUND_DIR="iterations/round_${ROUND}"
WORKDIR_COPY="${ROUND_DIR}/workdir_from_round_${PREV}"
DESIGN_DOC="docs/iter_round_${ROUND}.md"
PATCH_FILE="${ROUND_DIR}/iter_round_${ROUND}_diff.patch"

mkdir -p "${WORKDIR_COPY}" docs

# Snapshot minimal previous-round artifacts when available.
if [[ -d "iterations/round_${PREV}" ]]; then
  cp -a "iterations/round_${PREV}/." "${WORKDIR_COPY}/"
else
  # Round 1 fallback: snapshot key project docs as baseline.
  for f in README.md PROJECT_FRAMEWORK.md; do
    [[ -f "$f" ]] && cp "$f" "${WORKDIR_COPY}/"
  done
fi

if [[ ! -f "${DESIGN_DOC}" ]]; then
  cat > "${DESIGN_DOC}" <<DOC
# Iteration Round ${ROUND} Design

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

mkdir -p "${ROUND_DIR}"
git diff > "${PATCH_FILE}" || true

echo "Prepared round ${ROUND}:"
echo "- design: ${DESIGN_DOC}"
echo "- snapshot: ${WORKDIR_COPY}"
echo "- patch: ${PATCH_FILE}"
