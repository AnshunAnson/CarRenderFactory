#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ITER_DIR="$ROOT_DIR/iterations"
DOCS_DIR="$ROOT_DIR/docs"

mkdir -p "$ITER_DIR" "$DOCS_DIR"

LATEST_FILE="$ITER_DIR/LATEST_ROUND"
LAST_ROUND=0
if [[ -f "$LATEST_FILE" ]]; then
  LAST_ROUND="$(cat "$LATEST_FILE")"
fi

if ! [[ "$LAST_ROUND" =~ ^[0-9]+$ ]]; then
  echo "Invalid round index in $LATEST_FILE: $LAST_ROUND" >&2
  exit 1
fi

NEXT_ROUND=$((LAST_ROUND + 1))
ROUND_DIR="$ITER_DIR/round_${NEXT_ROUND}"
WORKSPACE_DIR="$ROUND_DIR/workspace"
mkdir -p "$WORKSPACE_DIR"

DESIGN_DOC="$DOCS_DIR/iter_round_${NEXT_ROUND}.md"
if [[ ! -f "$DESIGN_DOC" ]]; then
  cat > "$DESIGN_DOC" <<DOC
# Iteration Round ${NEXT_ROUND} Design

## 当前系统核心冗余 / 耦合 / 错误风险点
- TODO

## 本轮删减 & 收敛策略
- TODO

## 保留的最小闭环定义
- TODO

## 明确删除 / 合并 / 收敛清单
- TODO

## 迭代后目标结构
- TODO
DOC
fi

copy_previous_artifacts() {
  local prev_round="$1"
  local dest="$2"

  if [[ "$prev_round" -eq 0 ]]; then
    cat > "$dest/BASELINE.txt" <<TXT
Baseline source: repository working tree before round 1.
TXT
    return
  fi

  local prev_dir="$ITER_DIR/round_${prev_round}"
  mkdir -p "$dest/previous_round"

  if [[ -d "$prev_dir" ]]; then
    cp -a "$prev_dir" "$dest/previous_round/round_${prev_round}"
  fi

  local prev_design="$DOCS_DIR/iter_round_${prev_round}.md"
  local prev_patch="$ROOT_DIR/iter_round_${prev_round}_diff.patch"

  [[ -f "$prev_design" ]] && cp -a "$prev_design" "$dest/"
  [[ -f "$prev_patch" ]] && cp -a "$prev_patch" "$dest/"
}

write_patch() {
  local patch_file="$1"
  : > "$patch_file"

  git -C "$ROOT_DIR" diff --binary HEAD >> "$patch_file"

  while IFS= read -r untracked; do
    [[ -z "$untracked" ]] && continue
    if [[ "$ROOT_DIR/$untracked" == "$patch_file" ]]; then
      continue
    fi
    git -C "$ROOT_DIR" diff --binary --no-index /dev/null "$ROOT_DIR/$untracked" >> "$patch_file" || true
  done < <(git -C "$ROOT_DIR" ls-files --others --exclude-standard)
}

copy_previous_artifacts "$LAST_ROUND" "$WORKSPACE_DIR"

PATCH_FILE="$ROOT_DIR/iter_round_${NEXT_ROUND}_diff.patch"
write_patch "$PATCH_FILE"

cat > "$ROUND_DIR/MANIFEST.md" <<META
# Round ${NEXT_ROUND} Manifest

- previous_round: ${LAST_ROUND}
- design_doc: docs/iter_round_${NEXT_ROUND}.md
- patch_file: iter_round_${NEXT_ROUND}_diff.patch
- workspace: iterations/round_${NEXT_ROUND}/workspace
- generated_at_utc: $(date -u +"%Y-%m-%dT%H:%M:%SZ")
META

echo "$NEXT_ROUND" > "$LATEST_FILE"

echo "Initialized convergence round ${NEXT_ROUND}."
