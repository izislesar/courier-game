#!/usr/bin/env bash
set -euo pipefail

required=(
  AGENTS.md
  CLAUDE.md
  CONTEXT_VERSION
  docs/product/production-readiness.md
  docs/product/vision.md
  docs/product/gameplay.md
  docs/product/art-direction.md
  docs/architecture/performance-budget.md
  docs/architecture/city-simulation.md
  docs/architecture/ue5-architecture.md
  docs/architecture/testing-and-verification.md
  docs/agent/context-loading.md
  docs/agent/night-run.md
  docs/decisions/active-exceptions.md
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

if grep -q 'overnight goal is the \*\*pre-production vertical slice\*\*' AGENTS.md; then
  echo 'STALE: AGENTS.md still declares pre-production as active goal'
  exit 1
fi

for stale in beads/task-graph.json beads/task-descriptions.json; do
  if [[ -e "$stale" ]]; then
    echo "STALE HOT-CONTEXT ARTIFACT: $stale (run scripts/apply_context_v3.sh)"
    exit 1
  fi
done

if [[ -f .kilo/kilo.jsonc ]]; then
  if ! grep -q '^\.kilo/kilo\.jsonc$' .gitignore; then
    echo 'SECURITY: .kilo/kilo.jsonc exists but is not ignored'
    exit 1
  fi
  if git rev-parse --is-inside-work-tree >/dev/null 2>&1 && git ls-files --error-unmatch .kilo/kilo.jsonc >/dev/null 2>&1; then
    echo 'SECURITY: .kilo/kilo.jsonc is tracked; remove it from git index and rotate exposed keys'
    exit 1
  fi
fi

if ! command -v bd >/dev/null 2>&1; then
  echo 'WARN: bd not found; structural context files are valid but Beads recovery cannot be checked'
else
  bd prime >/dev/null 2>&1 || { echo 'BEADS: bd prime failed'; exit 1; }
  bd ready --json >/dev/null 2>&1 || { echo 'BEADS: bd ready --json failed'; exit 1; }
fi

echo 'Courier 404 context v3 structural/recovery check: OK'
