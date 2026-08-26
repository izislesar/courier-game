#!/usr/bin/env bash
set -euo pipefail

if [[ ! -f Courier404.uproject || ! -f AGENTS.md ]]; then
  echo 'Run from Courier 404 repository root.' >&2
  exit 2
fi

chmod +x scripts/agent-bus.sh scripts/agent-session-prompt.sh scripts/context_check.sh scripts/apply_context_v4.sh
mkdir -p docs/agent/prompts docs/reports/performance/logs

# Raw generated perf logs are transient by default. Durable findings belong in reports/Beads.
if ! grep -qxF 'docs/reports/performance/logs/*.log' .gitignore; then
  printf '\n# Transient generated performance logs; promote only intentional evidence\ndocs/reports/performance/logs/*.log\n' >> .gitignore
fi

./scripts/agent-bus.sh init >/dev/null
./scripts/context_check.sh

echo
printf '%s\n' 'Context v4 applied.'
printf '%s\n' 'Orchestrator prompt: ./scripts/agent-session-prompt.sh orchestrator'
printf '%s\n' 'Worker prompts:      ./scripts/agent-session-prompt.sh w1|w2|w3'
printf '%s\n' 'Bus status:          ./scripts/agent-bus.sh status'
