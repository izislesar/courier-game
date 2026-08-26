#!/usr/bin/env bash
set -euo pipefail

role="${1:-}"
root="$(git rev-parse --show-toplevel)"

case "$role" in
  orchestrator)
    cat "$root/docs/agent/prompts/orchestrator.md"
    ;;
  w1|w2|w3)
    sed "s/{{WORKER}}/$role/g" "$root/docs/agent/prompts/worker.md"
    ;;
  *)
    echo "usage: $0 orchestrator|w1|w2|w3" >&2
    exit 2
    ;;
esac
