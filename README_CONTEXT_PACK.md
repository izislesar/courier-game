# Courier 404 — Agent Context Pack v4

Context v4 upgrades the production-readiness repository from a primarily single-agent execution model to a durable **orchestrator + reusable git-worktree workers** model.

## What v4 changes

- defines explicit lead/orchestrator/worker roles;
- makes Beads mutation orchestrator-only during multi-agent runs;
- defines the shared `git-common-dir` agent bus as coordination state, not task truth;
- documents safe worker reuse and branch resynchronization;
- replaces routine worker cherry-picking with ancestry-preserving merge integration;
- makes the orchestrator the only UE build/cook/package/real-render lane;
- adds worker/orchestrator prompt templates and a prompt emitter;
- adds role-aware context recovery after compaction/model switches;
- adds overnight scheduling/backlog-decomposition rules;
- strengthens context checks for multi-agent drift;
- ignores raw transient performance logs by default while keeping summarized reports as durable evidence.

## Apply

From repository root:

```bash
tar -xzf courier404-context-v4-multi-agent.tar.gz -C .
./scripts/apply_context_v4.sh
```

Then run:

```bash
./scripts/context_check.sh
./scripts/agent-bus.sh init
./scripts/agent-bus.sh status
git status --short
```

## Important invariant

Beads remains the authoritative task graph. The agent bus only transports assignments/results between active sessions.

Worker completion is not issue completion. Only the orchestrator may integrate, run final UE verification, and close the Beads issue.
