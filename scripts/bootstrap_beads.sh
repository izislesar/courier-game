#!/usr/bin/env bash
set -euo pipefail

if ! command -v bd >/dev/null 2>&1; then
  echo "ERROR: bd (Beads) is not installed or not on PATH." >&2
  exit 1
fi

if ! command -v jq >/dev/null 2>&1; then
  echo "ERROR: jq is required by this bootstrap script." >&2
  exit 1
fi

if [[ ! -f AGENTS.md || ! -f beads/task-graph.json || ! -f beads/task-descriptions.json ]]; then
  echo "ERROR: run this script from the repository root after unpacking the context pack." >&2
  exit 1
fi

# Initialize Beads only if needed. Newer Beads is safe to initialize quietly;
# fallback to ordinary init if the installed version lacks --quiet.
if ! bd info --json >/dev/null 2>&1; then
  bd init --quiet 2>/dev/null || bd init
fi

# Preserve the rich local instructions after bd init/setup modifications.
# We intentionally do not run `bd setup` here because it may rewrite agent files
# differently across harnesses. `bd prime` is sufficient for runtime guidance.

json_id () {
  jq -r 'if type=="array" then .[0].id else .id end'
}

create_issue () {
  local title="$1"
  local type="$2"
  local priority="$3"
  local description="$4"
  shift 4
  local out
  out="$(bd create "$title" -t "$type" -p "$priority" --description "$description" "$@" --json)"
  printf '%s\n' "$out" | json_id
}

echo "Creating Courier 404 pre-production Beads graph..."

EPIC_ID="$(create_issue \
  "Courier 404 — Pre-Production Vertical Slice" \
  epic 0 \
  "Deliver the complete pre-production vertical slice defined in docs/product/preprod-scope.md. Children are the authoritative overnight execution graph.")"

declare -A ID
declare -A TITLE
declare -A PRIO

while IFS=$'\t' read -r key prio title; do
  TITLE["$key"]="$title"
  PRIO["$key"]="$prio"
done < <(jq -r '.tasks[] | [.key, (.priority|tostring), .title] | @tsv' beads/task-graph.json)

for key in $(jq -r '.tasks[].key' beads/task-graph.json); do
  description="$(jq -r --arg k "$key" '.[$k]' beads/task-descriptions.json)"
  ID["$key"]="$(create_issue "${TITLE[$key]}" task "${PRIO[$key]}" "$description" --parent "$EPIC_ID")"
  echo "  $key -> ${ID[$key]}"
done

# Add blocking dependencies. Beads semantics:
#   bd dep add <dependent> <required>
# so the first waits for the second.
while IFS=$'\t' read -r dependent required; do
  bd dep add "${ID[$dependent]}" "${ID[$required]}" >/dev/null
  echo "  dependency: $dependent needs $required"
done < <(
  jq -r '.tasks[] as $t | $t.deps[]? | [$t.key, .] | @tsv' beads/task-graph.json
)

echo
echo "Beads graph created."
echo "Epic: $EPIC_ID"
echo
echo "Ready work:"
bd ready
echo
echo "Blocked work:"
bd blocked || true
echo
echo "Next: give your lead agent docs/agent/night-run.md plus the night-run prompt from README_CONTEXT_PACK.md"
