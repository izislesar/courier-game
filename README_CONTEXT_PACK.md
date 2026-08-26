# Courier 404 — Agent Context Pack v3 (Production Readiness)

This is an **overlay** for an existing Courier 404 repository whose pre-production vertical slice has already been proven.

It does not recreate Beads tasks and does not reopen completed work.

## What v3 changes

- makes Beads/git recovery authoritative after compaction/model switches;
- replaces the pre-production overnight objective with production-readiness gates;
- introduces layered L0/L1/L2 context loading;
- archives the obsolete bootstrap JSON task graph instead of keeping a second tracker;
- marks pre-production scope as historical;
- separates verification reports from task state;
- documents the generated movable-lighting exception instead of silently contradicting the performance contract;
- prevents repeated visible-window profiling loops from masquerading as autonomous verification;
- adds credential hygiene for `.kilo/kilo.jsonc`;
- makes context validation test the live recovery contract rather than an obsolete bootstrap graph.

## Apply

From the repository root, extract the overlay and run:

```bash
tar -xzf courier404-context-v3-production.tar.gz -C .
./scripts/apply_context_v3.sh
```

Then inspect:

```bash
bd prime
bd ready --json
./scripts/context_check.sh
git status --short
```

## Task-state rule

The pack intentionally creates **zero Beads issues**. It is infrastructure/context, not a hidden roadmap.

After applying it, the lead agent should inspect current Beads state and create a dependency-ordered production epic only if the intended production work is not already represented.
