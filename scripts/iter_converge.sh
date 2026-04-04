#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
STATE_DIR="$ROOT_DIR/iterations/state"
STATE_FILE="$STATE_DIR/current_round.txt"
ITER_DIR="$ROOT_DIR/iterations"

mkdir -p "$STATE_DIR"

if [[ ! -f "$STATE_FILE" ]]; then
  echo "1" > "$STATE_FILE"
fi

ROUND="$(tr -d '[:space:]' < "$STATE_FILE")"
if [[ -z "$ROUND" || ! "$ROUND" =~ ^[0-9]+$ ]]; then
  echo "Invalid round in $STATE_FILE: '$ROUND'" >&2
  exit 1
fi

ROUND_DIR="$ITER_DIR/round_${ROUND}"
WORKDIR="$ROUND_DIR/workdir"
PATCH_FILE="$ROUND_DIR/iter_round_${ROUND}_diff.patch"
DESIGN_DOC="$ROOT_DIR/docs/iter_round_${ROUND}.md"

if [[ ! -f "$DESIGN_DOC" ]]; then
  echo "Missing design doc: $DESIGN_DOC" >&2
  echo "Create design doc before code changes." >&2
  exit 1
fi

PREV_SOURCE="$ROOT_DIR"
if (( ROUND > 1 )); then
  PREV_ROUND=$((ROUND - 1))
  CANDIDATE="$ITER_DIR/round_${PREV_ROUND}/workdir"
  if [[ -d "$CANDIDATE" ]]; then
    PREV_SOURCE="$CANDIDATE"
  fi
fi

mkdir -p "$ROUND_DIR"
rm -rf "$WORKDIR"
mkdir -p "$WORKDIR"

rsync -a \
  --exclude '.git' \
  --exclude 'iterations' \
  --exclude '.codex' \
  "$PREV_SOURCE/" "$WORKDIR/"

pushd "$ROOT_DIR" >/dev/null

git diff -- . ':(exclude)iterations' > "$PATCH_FILE"

NEXT_ROUND=$((ROUND + 1))
echo "$NEXT_ROUND" > "$STATE_FILE"

popd >/dev/null

echo "Round $ROUND complete"
echo "Workdir: $WORKDIR"
echo "Patch: $PATCH_FILE"
echo "Next round: $NEXT_ROUND"
