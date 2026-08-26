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
  docs/agent/multi-agent-orchestration.md
  docs/agent/prompts/orchestrator.md
  docs/agent/prompts/worker.md
  docs/decisions/active-exceptions.md
  scripts/agent-bus.sh
  scripts/agent-session-prompt.sh
)

failed=0
for f in "${required[@]}"; do
  if [[ ! -s "$f" ]]; then
    echo "MISSING/EMPTY: $f"
    failed=1
  fi
done
[[ "$failed" -eq 0 ]] || exit 1

if [[ "$(cat CONTEXT_VERSION)" != "courier404-context-v4-multi-agent-orchestration" ]]; then
  echo "STALE: CONTEXT_VERSION is not v4"
  exit 1
fi

for stale in beads/task-graph.json beads/task-descriptions.json; do
  if [[ -e "$stale" ]]; then
    echo "STALE HOT-CONTEXT ARTIFACT: $stale"
    exit 1
  fi
done

if grep -q 'overnight goal is the \*\*pre-production vertical slice\*\*' AGENTS.md; then
  echo 'STALE: AGENTS.md still declares pre-production as the overnight goal'
  exit 1
fi

if ! grep -q 'orchestrator' AGENTS.md || ! grep -q 'agent/w1' docs/agent/multi-agent-orchestration.md; then
  echo 'MULTI-AGENT: role contract missing expected markers'
  exit 1
fi

if ! grep -q '^\.kilo/kilo\.jsonc$' .gitignore; then
  echo 'SECURITY: .kilo/kilo.jsonc must remain ignored'
  exit 1
fi

if [[ -f .kilo/kilo.jsonc ]] && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  if git ls-files --error-unmatch .kilo/kilo.jsonc >/dev/null 2>&1; then
    echo 'SECURITY: .kilo/kilo.jsonc is tracked'
    exit 1
  fi
fi

bash -n scripts/agent-bus.sh
bash -n scripts/agent-session-prompt.sh

if ! command -v bd >/dev/null 2>&1; then
  echo 'WARN: bd not found; Beads recovery not checked'
else
  bd prime >/dev/null 2>&1 || { echo 'BEADS: bd prime failed'; exit 1; }
  bd ready --json >/dev/null 2>&1 || { echo 'BEADS: bd ready --json failed'; exit 1; }
fi

if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  branch="$(git branch --show-current)"
  case "$branch" in
    agent/w1|agent/w2|agent/w3)
      if [[ -n "$(git status --porcelain .beads 2>/dev/null)" ]]; then
        echo "WARN: worker branch $branch has local .beads changes; workers should not mutate Beads"
      fi
      ;;
  esac
fi

echo 'Courier 404 context v4 structural/recovery check: OK'
