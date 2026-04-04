#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ITER_DIR="$ROOT_DIR/iterations"
DOCS_DIR="$ROOT_DIR/docs"

mkdir -p "$ITER_DIR" "$DOCS_DIR"

LATEST_FILE="$ITER_DIR/LATEST_ROUND"
if [[ -f "$LATEST_FILE" ]]; then
  LAST_ROUND="$(cat "$LATEST_FILE")"
else
  LAST_ROUND=0
fi

if ! [[ "$LAST_ROUND" =~ ^[0-9]+$ ]]; then
  echo "Invalid round index in $LATEST_FILE: $LAST_ROUND" >&2
  exit 1
fi

NEXT_ROUND=$((LAST_ROUND + 1))
ROUND_DIR="$ITER_DIR/round_${NEXT_ROUND}"
mkdir -p "$ROUND_DIR"

DESIGN_DOC="$DOCS_DIR/iter_round_${NEXT_ROUND}.md"
if [[ ! -f "$DESIGN_DOC" ]]; then
  cat > "$DESIGN_DOC" <<DOC
# Iteration Round ${NEXT_ROUND} Design

## Current Redundancy / Coupling / Risk
- TODO

## Reduction & Convergence Strategy
- TODO

## Minimal Closed Loop
- TODO

## Delete / Merge / Converge List
- TODO

## Target Structure
- TODO
DOC
fi

PATCH_FILE="$ROOT_DIR/iter_round_${NEXT_ROUND}_diff.patch"
# Patch is generated against current working tree; untracked files are listed in round metadata.
git -C "$ROOT_DIR" diff > "$PATCH_FILE"

echo "$NEXT_ROUND" > "$LATEST_FILE"

cat > "$ROUND_DIR/README.md" <<META
# Round ${NEXT_ROUND} Artifacts

- Design: docs/iter_round_${NEXT_ROUND}.md
- Patch: iter_round_${NEXT_ROUND}_diff.patch
- Generated at: $(date -u +"%Y-%m-%dT%H:%M:%SZ")
META

echo "Initialized convergence round ${NEXT_ROUND}."
