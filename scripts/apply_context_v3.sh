#!/usr/bin/env bash
set -euo pipefail

if [[ ! -f Courier404.uproject || ! -f AGENTS.md ]]; then
  echo 'Run this script from the Courier 404 repository root.' >&2
  exit 1
fi

archive='docs/archive/context-v2-bootstrap'
mkdir -p "$archive/scripts" "$archive/beads" "$archive/superpowers/specs" "$archive/superpowers/plans"

move_if_present() {
  local src="$1" dst="$2"
  if [[ -e "$src" ]]; then
    mkdir -p "$(dirname "$dst")"
    mv "$src" "$dst"
    echo "archived: $src -> $dst"
  fi
}

# Remove historical bootstrap/task-graph artifacts from hot context without deleting history.
move_if_present scripts/apply_context_v2.sh "$archive/scripts/apply_context_v2.sh"
move_if_present scripts/bootstrap_beads.sh "$archive/scripts/bootstrap_beads.sh"
move_if_present beads/task-graph.json "$archive/beads/task-graph.json"
move_if_present beads/task-descriptions.json "$archive/beads/task-descriptions.json"
move_if_present docs/superpowers/specs/2026-08-25-courier404-preprod-design.md "$archive/superpowers/specs/2026-08-25-courier404-preprod-design.md"
move_if_present docs/superpowers/plans/2026-08-25-courier404-preprod-plan.md "$archive/superpowers/plans/2026-08-25-courier404-preprod-plan.md"

# Historical scope remains discoverable but is explicitly marked non-current.
if [[ -f docs/product/preprod-scope.md ]] && ! grep -q 'HISTORICAL MILESTONE' docs/product/preprod-scope.md; then
  tmp="$(mktemp)"
  {
    printf '%s\n\n' '> **HISTORICAL MILESTONE — COMPLETED.** This file defines the former pre-production vertical-slice boundary. It is not the current task scope. Current production acceptance lives in `docs/product/production-readiness.md`; current work lives in Beads.'
    cat docs/product/preprod-scope.md
  } > "$tmp"
  mv "$tmp" docs/product/preprod-scope.md
fi

chmod +x scripts/context_check.sh scripts/apply_context_v3.sh

# Credential hygiene: keep local Kilo config local. Never print secret values.
if [[ -f .kilo/kilo.jsonc ]] && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  if git ls-files --error-unmatch .kilo/kilo.jsonc >/dev/null 2>&1; then
    echo 'WARNING: .kilo/kilo.jsonc is currently tracked. Run:'
    echo '  git rm --cached .kilo/kilo.jsonc'
    echo 'Then rotate any credentials that were ever committed.'
  fi
fi

./scripts/context_check.sh

echo
echo 'Context v3 applied.'
echo 'Current task state remains in Beads; this script intentionally does not create or reopen issues.'
echo 'Next agent entry: bd prime && bd ready --json'
