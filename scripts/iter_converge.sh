#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARTIFACT_DIR="${ROOT_DIR}/iterative_artifacts"
mkdir -p "${ARTIFACT_DIR}"

next_round() {
  local max_round
  max_round="$(find "${ARTIFACT_DIR}" -maxdepth 1 -type d -name 'round_*' -printf '%f\n' 2>/dev/null | sed 's/round_//' | awk 'NF' | sort -n | tail -n 1)"
  if [[ -z "${max_round}" ]]; then
    echo 1
  else
    echo $((max_round + 1))
  fi
}

ROUND="${1:-$(next_round)}"
if ! [[ "${ROUND}" =~ ^[0-9]+$ ]]; then
  echo "Round must be numeric." >&2
  exit 1
fi

ROUND_DIR="${ARTIFACT_DIR}/round_${ROUND}"
WORK_DIR="${ROUND_DIR}/workspace"
DESIGN_DOC="${ROOT_DIR}/docs/iter_round_${ROUND}.md"
PATCH_FILE="${ROOT_DIR}/iter_round_${ROUND}_diff.patch"

if [[ -e "${ROUND_DIR}" ]]; then
  echo "${ROUND_DIR} already exists; refusing to overwrite." >&2
  exit 1
fi

PREV_ROUND=$((ROUND - 1))
if (( PREV_ROUND >= 1 )) && [[ -d "${ARTIFACT_DIR}/round_${PREV_ROUND}/workspace" ]]; then
  SOURCE_DIR="${ARTIFACT_DIR}/round_${PREV_ROUND}/workspace"
else
  SOURCE_DIR="${ROOT_DIR}"
fi

mkdir -p "${WORK_DIR}"
rsync -a \
  --exclude '.git' \
  --exclude 'iterative_artifacts' \
  --exclude 'Binaries' \
  --exclude 'DerivedDataCache' \
  --exclude 'Intermediate' \
  --exclude 'Saved' \
  --exclude 'Content' \
  "${SOURCE_DIR}/" "${WORK_DIR}/"

if [[ ! -f "${DESIGN_DOC}" ]]; then
  cat > "${DESIGN_DOC}" <<DOC
# Iteration Round ${ROUND} Design

## Current Redundancy / Coupling / Error Risks
- TODO

## Reduction & Convergence Strategy
- TODO

## Minimum Closed Loop Preserved
- TODO

## Files/Modules to Delete, Merge, or Converge
- TODO

## Target Structure
- TODO
DOC
fi

(
  cd "${ROOT_DIR}"
  git diff -- . ':(exclude)iter_round_*.patch' > "${PATCH_FILE}" || true
)

cat <<MSG
Round ${ROUND} initialized.
- Isolated workspace: ${WORK_DIR}
- Design document: ${DESIGN_DOC}
- Diff patch: ${PATCH_FILE}

Next: apply only design-aligned changes, then rerun to continue with round $((ROUND + 1)).
MSG
