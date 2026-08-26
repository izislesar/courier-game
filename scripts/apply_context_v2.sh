#!/usr/bin/env bash
set -euo pipefail

if ! command -v bd >/dev/null 2>&1; then
  echo "ERROR: bd is not installed." >&2
  exit 1
fi
if ! command -v jq >/dev/null 2>&1; then
  echo "ERROR: jq is required." >&2
  exit 1
fi
if ! bd info --json >/dev/null 2>&1; then
  echo "ERROR: Beads is not initialized in this repository. Run the original bootstrap_beads.sh first." >&2
  exit 1
fi

json_id () {
  jq -r 'if type=="array" then .[0].id else .id end'
}

all_issues="$(bd list --json)"

find_title () {
  local title="$1"
  printf '%s\n' "$all_issues" | jq -r --arg t "$title" '
    if type=="array" then
      (.[] | select(.title==$t) | .id)
    elif has("issues") then
      (.issues[] | select(.title==$t) | .id)
    else empty end
  ' | head -n1
}

create_if_missing () {
  local title="$1"
  local type="$2"
  local priority="$3"
  local description="$4"
  local parent="${5:-}"
  local existing
  existing="$(find_title "$title")"
  if [[ -n "$existing" ]]; then
    printf '%s\n' "$existing"
    return 0
  fi
  local out
  if [[ -n "$parent" ]]; then
    out="$(bd create "$title" -t "$type" -p "$priority" --description "$description" --parent "$parent" --json)"
  else
    out="$(bd create "$title" -t "$type" -p "$priority" --description "$description" --json)"
  fi
  printf '%s\n' "$out" | json_id
}

add_dep_safe () {
  local dependent="$1"
  local required="$2"
  bd dep add "$dependent" "$required" >/dev/null 2>&1 || true
}

EPIC_TITLE="Courier 404 — Pre-Production Vertical Slice"
EPIC_ID="$(find_title "$EPIC_TITLE")"
if [[ -z "$EPIC_ID" ]]; then
  echo "ERROR: Could not find existing pre-production epic '$EPIC_TITLE'." >&2
  exit 1
fi

FOUNDATION="$(find_title "Bootstrap UE5 C++ project and reproducible build verification")"
WORLD="$(find_title "Build compact district and apartment vertical-slice blockout with lighting intent")"
CLOCK="$(find_title "Implement simulated clock and day/night lifecycle")"
INTERACTION="$(find_title "Implement first-person player and reusable physical interaction framework")"
ART="$(find_title "Apply cohesive high-fidelity urban lighting/material/presentation pass")"
INTEGRATION="$(find_title "Integrate full pre-prod life loop into one stable playable slice")"
GATE="$(find_title "Run pre-prod release gate, fix critical blockers and write morning status report")"

for required in FOUNDATION WORLD CLOCK INTERACTION ART INTEGRATION GATE; do
  if [[ -z "${!required}" ]]; then
    echo "ERROR: Could not resolve existing issue: $required" >&2
    exit 1
  fi
done

PERF_TITLE="Establish low-end renderer path, scalability budgets and profiling harness"
CITY_TITLE="Implement hierarchical living-city ambient simulation baseline"
PERF_GATE_TITLE="Run low-end performance, scalability and cooked-size gate"

PERF_DESC="Implement the authoritative performance contract in docs/architecture/performance-budget.md. Establish Low/Medium/High scalability, baked/raster baseline with Lumen disabled on Low, optional/fallback-safe Nanite/VSM usage rules, profiling commands/report location, texture/material budgets and package-size measurement. Acceptance: representative packaged-build measurements are possible, Low does not depend on Lumen/Nanite/VSM, and no high-end UE5 feature is a baseline dependency."

CITY_DESC="Implement the pre-production living-city baseline from docs/architecture/city-simulation.md. Use hierarchical fidelity: persistent citizen records, lightweight ambient representation and full interactive NPCs only by significance. Add time-of-day density variation, at least three ambient activities, one non-mission ambient event and nearby civilian reaction to one disturbance. Acceptance: Low scalability reduces density safely and off-screen citizens do not require live Character actors."

PERF_GATE_DESC="Profile the integrated vertical slice against docs/architecture/performance-budget.md in a packaged Development build where environment permits. Record resolution/preset, stat unit/gpu/streaming, representative recurring hitching, memory/streaming state, cooked package size and asset-size offenders. Verify Low with Lumen disabled and no required Nanite/VSM dependency. Fix in-scope dominant regressions; create exact Beads blockers for unmet targets. Write docs/reports/performance/preprod-performance.md."

PERF_ID="$(create_if_missing "$PERF_TITLE" task 0 "$PERF_DESC" "$EPIC_ID")"
CITY_ID="$(create_if_missing "$CITY_TITLE" task 1 "$CITY_DESC" "$EPIC_ID")"
PERF_GATE_ID="$(create_if_missing "$PERF_GATE_TITLE" task 0 "$PERF_GATE_DESC" "$EPIC_ID")"

# Performance baseline starts after foundation and should constrain world/art work.
add_dep_safe "$PERF_ID" "$FOUNDATION"
add_dep_safe "$WORLD" "$PERF_ID"
add_dep_safe "$ART" "$PERF_ID"

# Living city needs the world, clock and interaction baseline.
add_dep_safe "$CITY_ID" "$WORLD"
add_dep_safe "$CITY_ID" "$CLOCK"
add_dep_safe "$CITY_ID" "$INTERACTION"

# Integration must include the living-city proof.
add_dep_safe "$INTEGRATION" "$CITY_ID"

# Performance gate measures the integrated slice, and final gate waits for it.
add_dep_safe "$PERF_GATE_ID" "$INTEGRATION"
add_dep_safe "$PERF_GATE_ID" "$PERF_ID"
add_dep_safe "$PERF_GATE_ID" "$CITY_ID"
add_dep_safe "$GATE" "$PERF_GATE_ID"

mkdir -p docs/reports/performance

echo "Courier 404 v2 context applied."
echo "Epic:       $EPIC_ID"
echo "Perf:       $PERF_ID"
echo "Living city:$CITY_ID"
echo "Perf gate:  $PERF_GATE_ID"
echo
echo "Ready:"
bd ready
echo
echo "Blocked:"
bd blocked || true
