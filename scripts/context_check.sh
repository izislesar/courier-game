#!/usr/bin/env bash
set -euo pipefail

required=(
  AGENTS.md
  docs/product/vision.md
  docs/product/preprod-scope.md
  docs/product/gameplay.md
  docs/product/art-direction.md
  docs/architecture/performance-budget.md
  docs/architecture/city-simulation.md
  docs/architecture/ue5-architecture.md
  docs/architecture/testing-and-verification.md
  docs/superpowers/specs/2026-08-25-courier404-preprod-design.md
  docs/superpowers/plans/2026-08-25-courier404-preprod-plan.md
  beads/task-graph.json
  beads/task-descriptions.json
)

failed=0
for f in "${required[@]}"; do
  if [[ ! -s "$f" ]]; then
    echo "MISSING/EMPTY: $f"
    failed=1
  fi
done

if [[ "$failed" -ne 0 ]]; then
  exit 1
fi

jq -e '.tasks | length >= 15' beads/task-graph.json >/dev/null
jq -e 'length >= 15' beads/task-descriptions.json >/dev/null

echo "Context pack structural check: OK"
